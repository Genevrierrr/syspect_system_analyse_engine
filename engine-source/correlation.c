#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <setupapi.h>
#include <winevt.h>
#include <wchar.h>

#include "correlation.h"
#include "dump_analyzer.h"


#define MAX_CORRELATED_DEVICES 64

#define DEVICE_ID_LENGTH 512
#define DEVICE_NAME_LENGTH 768
#define DRIVER_NAME_LENGTH 768
#define MANUFACTURER_LENGTH 768
#define DRIVER_PROVIDER_LENGTH 768
#define DRIVER_VERSION_LENGTH 128
#define DRIVER_KEY_LENGTH 512

typedef struct
{
    char deviceId[DEVICE_ID_LENGTH];
    char deviceName[DEVICE_NAME_LENGTH];
    char driverName[DRIVER_NAME_LENGTH];
    char manufacturer[MANUFACTURER_LENGTH];
    char driverProvider[DRIVER_PROVIDER_LENGTH];
    char driverVersion[DRIVER_VERSION_LENGTH];
    char driverKey[DRIVER_KEY_LENGTH];
    int occurrences;
} CorrelatedDevice;

/*
 * Convert UTF-16 text to UTF-8.
 */
static void wide_to_utf8(
    const wchar_t* source,
    char* destination,
    int destinationSize
)
{
    if (source == NULL || destination == NULL || destinationSize <= 0)
    {
        return;
    }

    destination[0] = '\0';

    if (WideCharToMultiByte(
            CP_UTF8,
            0,
            source,
            -1,
            destination,
            destinationSize,
            NULL,
            NULL) <= 0)
    {
        destination[0] = '\0';
    }
}

/*
 * Extract the Device Instance ID directly from the Kernel-PnP 219
 * structured XML EventData (language-independent).
 */
static int extract_device_id_from_event_values(
    EVT_HANDLE event,
    char* deviceId,
    int deviceIdSize
)
{
    EVT_HANDLE context;
    EVT_VARIANT* values = NULL;
    DWORD valueCount = 0;
    DWORD bufferUsed = 0;
    int success = 0;

    LPCWSTR properties[] =
    {
        L"Event/EventData/Data[@Name='DriverName']"
    };

    if (event == NULL || deviceId == NULL || deviceIdSize <= 0)
    {
        return 0;
    }

    deviceId[0] = '\0';

    context = EvtCreateRenderContext(1, properties, EvtRenderContextValues);
    if (context == NULL)
    {
        return 0;
    }

    if (!EvtRender(
        context,
        event,
        EvtRenderEventValues,
        0,
        NULL,
        &bufferUsed,
        &valueCount))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER || bufferUsed == 0)
        {
            EvtClose(context);
            return 0;
        }
    }

    values = (EVT_VARIANT*)malloc(bufferUsed);
    if (values == NULL)
    {
        EvtClose(context);
        return 0;
    }

    if (EvtRender(
        context,
        event,
        EvtRenderEventValues,
        bufferUsed,
        values,
        &bufferUsed,
        &valueCount))
    {
        if (valueCount > 0 &&
            values[0].Type == EvtVarTypeString &&
            values[0].StringVal != NULL &&
            values[0].StringVal[0] != L'\0')
        {
            wide_to_utf8(values[0].StringVal, deviceId, deviceIdSize);
            success = (deviceId[0] != '\0');
        }
    }

    free(values);
    EvtClose(context);
    return success;
}

/*
 * Fallback: Extract Device: field from a formatted Kernel-PnP event message.
 */
static int extract_device_id(
    const wchar_t* message,
    char* deviceId,
    int deviceIdSize
)
{
    const wchar_t* deviceStart;
    const wchar_t* deviceEnd;
    wchar_t temporary[DEVICE_ID_LENGTH];
    size_t length;

    if (message == NULL || deviceId == NULL || deviceIdSize <= 0)
    {
        return 0;
    }

    deviceId[0] = '\0';

    deviceStart = wcsstr(message, L"Device:");
    if (deviceStart == NULL)
    {
        return 0;
    }

    deviceStart += wcslen(L"Device:");

    while (*deviceStart == L' ' || *deviceStart == L'\t')
    {
        deviceStart++;
    }

    deviceEnd = wcschr(deviceStart, L'\n');
    if (deviceEnd == NULL)
    {
        deviceEnd = wcschr(deviceStart, L'\r');
    }
    if (deviceEnd == NULL)
    {
        deviceEnd = deviceStart + wcslen(deviceStart);
    }

    while (deviceEnd > deviceStart &&
        (deviceEnd[-1] == L'\r' || deviceEnd[-1] == L' ' || deviceEnd[-1] == L'\t'))
    {
        deviceEnd--;
    }

    length = (size_t)(deviceEnd - deviceStart);
    if (length == 0)
    {
        return 0;
    }

    if (length >= DEVICE_ID_LENGTH)
    {
        length = DEVICE_ID_LENGTH - 1;
    }

    wcsncpy(temporary, deviceStart, length);
    temporary[length] = L'\0';

    wide_to_utf8(temporary, deviceId, deviceIdSize);
    return deviceId[0] != '\0';
}

/*
 * Render Kernel-PnP event message.
 */
static wchar_t* render_event_message(EVT_HANDLE event)
{
    EVT_HANDLE publisherMetadata = NULL;
    DWORD bufferUsed = 0;
    DWORD bufferSize = 0;
    wchar_t* messageBuffer = NULL;

    publisherMetadata = EvtOpenPublisherMetadata(
        NULL,
        L"Microsoft-Windows-Kernel-PnP",
        NULL,
        0,
        0
    );

    if (publisherMetadata == NULL)
    {
        return NULL;
    }

    if (!EvtFormatMessage(
        publisherMetadata,
        event,
        0,
        0,
        NULL,
        EvtFormatMessageEvent,
        0,
        NULL,
        &bufferUsed))
    {
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            EvtClose(publisherMetadata);
            return NULL;
        }
    }

    if (bufferUsed == 0)
    {
        EvtClose(publisherMetadata);
        return NULL;
    }

    bufferSize = bufferUsed;
    messageBuffer = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));

    if (messageBuffer == NULL)
    {
        EvtClose(publisherMetadata);
        return NULL;
    }

    if (!EvtFormatMessage(
        publisherMetadata,
        event,
        0,
        0,
        NULL,
        EvtFormatMessageEvent,
        bufferSize,
        messageBuffer,
        &bufferUsed))
    {
        free(messageBuffer);
        EvtClose(publisherMetadata);
        return NULL;
    }

    EvtClose(publisherMetadata);
    return messageBuffer;
}

/*
 * Read a string value from a registry key.
 */
static int read_registry_string(
    HKEY key,
    const wchar_t* valueName,
    char* buffer,
    DWORD bufferSize
)
{
    wchar_t wideBuffer[512];
    DWORD type = 0;
    DWORD size = sizeof(wideBuffer);

    if (key == NULL || valueName == NULL || buffer == NULL || bufferSize == 0)
    {
        return 0;
    }

    buffer[0] = '\0';
    ZeroMemory(wideBuffer, sizeof(wideBuffer));

    if (RegQueryValueExW(
        key,
        valueName,
        NULL,
        &type,
        (LPBYTE)wideBuffer,
        &size) != ERROR_SUCCESS)
    {
        return 0;
    }

    if (type != REG_SZ && type != REG_EXPAND_SZ)
    {
        return 0;
    }

    wideBuffer[(sizeof(wideBuffer) / sizeof(wideBuffer[0])) - 1] = L'\0';
    return WideCharToMultiByte(
        CP_UTF8,
        0,
        wideBuffer,
        -1,
        buffer,
        (int)bufferSize,
        NULL,
        NULL
    ) > 0;
}

/*
 * Compare an event device ID with a PnP instance ID.
 */
static int device_ids_match(
    const char* eventDeviceId,
    const char* instanceId
)
{
    if (eventDeviceId == NULL || instanceId == NULL)
    {
        return 0;
    }

    if (_stricmp(eventDeviceId, instanceId) == 0)
    {
        return 1;
    }

    if (eventDeviceId[0] == '{')
    {
        const char* eventEnd = strchr(eventDeviceId, '}');
        if (eventEnd != NULL)
        {
            size_t guidLength = (size_t)(eventEnd - eventDeviceId) + 1;
            if (instanceId[0] == '{' &&
                _strnicmp(eventDeviceId, instanceId, guidLength) == 0)
            {
                return 1;
            }
        }
    }

    return 0;
}

/*
 * Extract the base PCI hardware ID (e.g., PCI\VEN_8086&DEV_461D).
 */
static int extract_pci_hardware_id(
    const char* deviceId,
    char* hardwareId,
    int hardwareIdSize
)
{
    const char* ven;
    const char* dev;
    size_t length;

    if (deviceId == NULL || hardwareId == NULL || hardwareIdSize <= 0)
    {
        return 0;
    }

    hardwareId[0] = '\0';

    if (_strnicmp(deviceId, "PCI\\", 4) != 0)
    {
        return 0;
    }

    ven = strstr(deviceId, "VEN_");
    dev = strstr(deviceId, "&DEV_");

    if (ven == NULL || dev == NULL)
    {
        return 0;
    }

    dev += 5;
    length = (size_t)(dev - deviceId) + 4;

    if (length >= (size_t)hardwareIdSize)
    {
        length = (size_t)hardwareIdSize - 1;
    }

    size_t copyLength = 0;
    while (copyLength < length && deviceId[copyLength] != '\0')
    {
        copyLength++;
    }
    memcpy(hardwareId, deviceId, copyLength);
    memset(hardwareId + copyLength, 0, length - copyLength);
    hardwareId[length] = '\0';

    return 1;
}

/*
 * Check whether a hardware ID property contains the requested PCI hardware ID.
 */
static int hardware_id_matches(
    HDEVINFO deviceInfo,
    PSP_DEVINFO_DATA deviceData,
    const char* targetHardwareId
)
{
    BYTE buffer[4096];
    DWORD propertyType = 0;
    DWORD requiredSize = 0;
    DWORD offset = 0;

    if (targetHardwareId == NULL || targetHardwareId[0] == '\0')
    {
        return 0;
    }

    ZeroMemory(buffer, sizeof(buffer));

    if (!SetupDiGetDeviceRegistryPropertyA(
        deviceInfo,
        deviceData,
        SPDRP_HARDWAREID,
        &propertyType,
        buffer,
        sizeof(buffer),
        &requiredSize))
    {
        return 0;
    }

    while (offset < requiredSize)
    {
        char* hardwareId = (char*)(buffer + offset);

        if (hardwareId[0] == '\0')
        {
            break;
        }

        if (_strnicmp(hardwareId, targetHardwareId, strlen(targetHardwareId)) == 0)
        {
            return 1;
        }

        offset += (DWORD)strlen(hardwareId) + 1;
    }

    return 0;
}

/*
 * Fill driver information from an installed driver key.
 */
static void read_installed_driver_information(
    HDEVINFO deviceInfo,
    PSP_DEVINFO_DATA deviceData,
    char* driverName,
    int driverNameSize,
    char* driverProvider,
    int driverProviderSize,
    char* driverVersion,
    int driverVersionSize,
    char* driverKey,
    int driverKeySize
)
{
    DWORD propertyType = 0;
    DWORD requiredSize = 0;

    SetupDiGetDeviceRegistryPropertyA(
        deviceInfo,
        deviceData,
        SPDRP_DRIVER,
        &propertyType,
        (PBYTE)driverKey,
        driverKeySize,
        &requiredSize
    );

    /*
     * Also attempt SPDRP_SERVICE directly for reliable driver service name resolution.
     */
    SetupDiGetDeviceRegistryPropertyA(
        deviceInfo,
        deviceData,
        SPDRP_SERVICE,
        &propertyType,
        (PBYTE)driverName,
        driverNameSize,
        &requiredSize
    );

    HKEY driverRegistryKey = SetupDiOpenDevRegKey(
        deviceInfo,
        deviceData,
        DICS_FLAG_GLOBAL,
        0,
        DIREG_DRV,
        KEY_READ
    );

    if (driverRegistryKey == INVALID_HANDLE_VALUE)
    {
        return;
    }

    if (driverName[0] == '\0')
    {
        read_registry_string(
            driverRegistryKey,
            L"Service",
            driverName,
            (DWORD)driverNameSize
        );
    }

    read_registry_string(
        driverRegistryKey,
        L"DriverVersion",
        driverVersion,
        (DWORD)driverVersionSize
    );

    read_registry_string(
        driverRegistryKey,
        L"ProviderName",
        driverProvider,
        (DWORD)driverProviderSize
    );

    RegCloseKey(driverRegistryKey);
}

/*
 * Resolve device information.
 */
static int resolve_device_information(
    const char* eventDeviceId,
    char* deviceName,
    int deviceNameSize,
    char* driverName,
    int driverNameSize,
    char* manufacturer,
    int manufacturerSize,
    char* driverProvider,
    int driverProviderSize,
    char* driverVersion,
    int driverVersionSize,
    char* driverKey,
    int driverKeySize
)
{
    HDEVINFO deviceInfo;
    SP_DEVINFO_DATA deviceData;
    DWORD index = 0;
    char pciHardwareId[DEVICE_ID_LENGTH];

    deviceName[0] = '\0';
    driverName[0] = '\0';
    manufacturer[0] = '\0';
    driverProvider[0] = '\0';
    driverVersion[0] = '\0';
    driverKey[0] = '\0';
    pciHardwareId[0] = '\0';

    extract_pci_hardware_id(eventDeviceId, pciHardwareId, sizeof(pciHardwareId));

    deviceInfo = SetupDiGetClassDevsA(
        NULL,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_ALLCLASSES
    );

    if (deviceInfo == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

    while (SetupDiEnumDeviceInfo(deviceInfo, index, &deviceData))
    {
        char instanceId[DEVICE_ID_LENGTH];
        DWORD instanceSize = sizeof(instanceId);
        int matched = 0;

        instanceId[0] = '\0';

        if (SetupDiGetDeviceInstanceIdA(
            deviceInfo,
            &deviceData,
            instanceId,
            instanceSize,
            NULL))
        {
            if (device_ids_match(eventDeviceId, instanceId))
            {
                matched = 1;
            }

            if (!matched && pciHardwareId[0] != '\0')
            {
                if (hardware_id_matches(deviceInfo, &deviceData, pciHardwareId))
                {
                    matched = 1;
                }
            }

            if (matched)
            {
                DWORD propertyType = 0;
                DWORD requiredSize = 0;
                wchar_t wideDeviceName[256] = { 0 };
                wchar_t wideManufacturer[256] = { 0 };

                if (!SetupDiGetDeviceRegistryPropertyW(
                    deviceInfo,
                    &deviceData,
                    SPDRP_FRIENDLYNAME,
                    &propertyType,
                    (PBYTE)wideDeviceName,
                    sizeof(wideDeviceName),
                    &requiredSize))
                {
                    SetupDiGetDeviceRegistryPropertyW(
                        deviceInfo,
                        &deviceData,
                        SPDRP_DEVICEDESC,
                        &propertyType,
                        (PBYTE)wideDeviceName,
                        sizeof(wideDeviceName),
                        &requiredSize
                    );
                }

                if (wideDeviceName[0] != L'\0')
                {
                    wide_to_utf8(wideDeviceName, deviceName, deviceNameSize);
                }

                propertyType = 0;
                requiredSize = 0;
                if (SetupDiGetDeviceRegistryPropertyW(
                    deviceInfo,
                    &deviceData,
                    SPDRP_MFG,
                    &propertyType,
                    (PBYTE)wideManufacturer,
                    sizeof(wideManufacturer),
                    &requiredSize))
                {
                    wide_to_utf8(wideManufacturer, manufacturer, manufacturerSize);
                }

                read_installed_driver_information(
                    deviceInfo,
                    &deviceData,
                    driverName,
                    driverNameSize,
                    driverProvider,
                    driverProviderSize,
                    driverVersion,
                    driverVersionSize,
                    driverKey,
                    driverKeySize
                );

                SetupDiDestroyDeviceInfoList(deviceInfo);
                return 1;
            }
        }

        index++;
    }

    SetupDiDestroyDeviceInfoList(deviceInfo);
    return 0;
}

/*
 * Add or update correlated device.
 */
static void add_device_occurrence(
    CorrelatedDevice* devices,
    int* deviceCount,
    const char* deviceId
)
{
    int i;

    if (devices == NULL ||
        deviceCount == NULL ||
        deviceId == NULL ||
        deviceId[0] == '\0')
    {
        return;
    }

    for (i = 0; i < *deviceCount; i++)
    {
        if (_stricmp(devices[i].deviceId, deviceId) == 0)
        {
            devices[i].occurrences++;
            return;
        }
    }

    if (*deviceCount >= MAX_CORRELATED_DEVICES)
    {
        return;
    }

    strncpy(
        devices[*deviceCount].deviceId,
        deviceId,
        DEVICE_ID_LENGTH - 1
    );
    devices[*deviceCount].deviceId[DEVICE_ID_LENGTH - 1] = '\0';
    devices[*deviceCount].deviceName[0] = '\0';
    devices[*deviceCount].driverName[0] = '\0';
    devices[*deviceCount].manufacturer[0] = '\0';
    devices[*deviceCount].driverProvider[0] = '\0';
    devices[*deviceCount].driverVersion[0] = '\0';
    devices[*deviceCount].driverKey[0] = '\0';
    devices[*deviceCount].occurrences = 1;

    (*deviceCount)++;
}

/*
 * Collect recent Kernel-PnP 219 events.
 */
static int collect_kernel_pnp_events(
    CorrelatedDevice* devices,
    int* deviceCount,
    int* totalEvents,
    const wchar_t* query
)
{
    EVT_HANDLE results;
    EVT_HANDLE event;
    DWORD returned = 0;
    int processed = 0;

    *deviceCount = 0;
    *totalEvents = 0;

    results = EvtQuery(
        NULL,
        L"System",
        query,
        EvtQueryChannelPath | EvtQueryReverseDirection
    );

    if (results == NULL)
    {
        return 0;
    }

    while (processed < 10000)
    {
        char deviceId[DEVICE_ID_LENGTH];
        deviceId[0] = '\0';
        event = NULL;

        if (!EvtNext(results, 1, &event, 1000, 0, &returned) || event == NULL)
        {
            DWORD error = GetLastError();
            EvtClose(results);
            return error == ERROR_NO_MORE_ITEMS;
        }

        /*
         * First try structured XML EventData (language-independent),
         * then fall back to formatted message parsing.
         */
        if (extract_device_id_from_event_values(event, deviceId, sizeof(deviceId)))
        {
            add_device_occurrence(devices, deviceCount, deviceId);
        }
        else
        {
            wchar_t* message = render_event_message(event);
            if (message != NULL)
            {
                if (extract_device_id(message, deviceId, sizeof(deviceId)))
                {
                    add_device_occurrence(devices, deviceCount, deviceId);
                }
                free(message);
            }
        }

        EvtClose(event);
        processed++;
        (*totalEvents)++;
    }

    EvtClose(results);
    return 0; /* Explicitly report the scan limit as incomplete. */
}

/*
 * Resolve every correlated device.
 */
static void resolve_all_devices(
    CorrelatedDevice* devices,
    int deviceCount
)
{
    int i;

    for (i = 0; i < deviceCount; i++)
    {
        if (!resolve_device_information(
            devices[i].deviceId,
            devices[i].deviceName,
            sizeof(devices[i].deviceName),
            devices[i].driverName,
            sizeof(devices[i].driverName),
            devices[i].manufacturer,
            sizeof(devices[i].manufacturer),
            devices[i].driverProvider,
            sizeof(devices[i].driverProvider),
            devices[i].driverVersion,
            sizeof(devices[i].driverVersion),
            devices[i].driverKey,
            sizeof(devices[i].driverKey)))
        {
            strncpy(
                devices[i].deviceName,
                "Device resolution unavailable",
                DEVICE_NAME_LENGTH - 1
            );
            devices[i].deviceName[DEVICE_NAME_LENGTH - 1] = '\0';
        }

        if (devices[i].driverName[0] == '\0')
        {
            strncpy(devices[i].driverName, "Unknown", DRIVER_NAME_LENGTH - 1);
            devices[i].driverName[DRIVER_NAME_LENGTH - 1] = '\0';
        }

        if (devices[i].manufacturer[0] == '\0')
        {
            strncpy(devices[i].manufacturer, "Unknown", MANUFACTURER_LENGTH - 1);
            devices[i].manufacturer[MANUFACTURER_LENGTH - 1] = '\0';
        }

        if (devices[i].driverProvider[0] == '\0')
        {
            strncpy(devices[i].driverProvider, "Unknown", DRIVER_PROVIDER_LENGTH - 1);
            devices[i].driverProvider[DRIVER_PROVIDER_LENGTH - 1] = '\0';
        }

        if (devices[i].driverVersion[0] == '\0')
        {
            strncpy(devices[i].driverVersion, "Unknown", DRIVER_VERSION_LENGTH - 1);
            devices[i].driverVersion[DRIVER_VERSION_LENGTH - 1] = '\0';
        }

        if (devices[i].driverKey[0] == '\0')
        {
            strncpy(devices[i].driverKey, "Unknown", DRIVER_KEY_LENGTH - 1);
            devices[i].driverKey[DRIVER_KEY_LENGTH - 1] = '\0';
        }
    }
}

/*
 * Print correlation results.
 */
static void print_correlation_results(
    CorrelatedDevice* devices,
    int deviceCount,
    int totalEvents
)
{
    int i;

    printf("\n");
    printf("KERNEL-PNP CORRELATION\n");
    printf("--------------------\n");
    printf("Events analyzed : %d\n", totalEvents);
    printf("Unique devices  : %d\n", deviceCount);
    if (deviceCount == MAX_CORRELATED_DEVICES)
        printf("Device list reached capacity; additional identities may be omitted.\n");

    if (deviceCount == 0)
    {
        printf("Status          : NO DEVICE CORRELATION FOUND\n");
        return;
    }

    printf("\n");
    printf("Affected devices:\n");

    for (i = 0; i < deviceCount; i++)
    {
        printf("\n");
        printf("[Device %d]\n", i + 1);
        printf("Device Name     : %s\n", devices[i].deviceName);
        printf("Device ID       : %s\n", devices[i].deviceId);
        printf("Driver          : %s\n", devices[i].driverName);
        printf("Manufacturer    : %s\n", devices[i].manufacturer);
        printf("Driver Provider : %s\n", devices[i].driverProvider);
        printf("Driver Version  : %s\n", devices[i].driverVersion);
        printf("Driver Key      : %s\n", devices[i].driverKey);
        printf("Occurrences     : %d\n", devices[i].occurrences);

        if (devices[i].occurrences >= 20)
        {
            printf("Frequency       : REPEATED\n");
        }
        else if (devices[i].occurrences >= 5)
        {
            printf("Frequency       : RECURRENT\n");
        }
        else
        {
            printf("Frequency       : LOW\n");
        }
    }
}

/* UTC is used for querying and display; FILETIME is in 100 ns units. */
#define CRASH_WINDOW_TICKS (300ULL * 10000000ULL)

static int utc_timestamp(unsigned long long ticks, wchar_t* text, size_t size)
{
    ULARGE_INTEGER value;
    FILETIME ft;
    SYSTEMTIME st;
    value.QuadPart = ticks;
    ft.dwLowDateTime = value.LowPart;
    ft.dwHighDateTime = value.HighPart;
    if (!FileTimeToSystemTime(&ft, &st)) return 0;
    return swprintf(text, size, L"%04u-%02u-%02uT%02u:%02u:%02u.%03uZ",
        (unsigned)st.wYear, (unsigned)st.wMonth, (unsigned)st.wDay,
        (unsigned)st.wHour, (unsigned)st.wMinute, (unsigned)st.wSecond,
        (unsigned)st.wMilliseconds) > 0;
}

static int crash_time_filter(unsigned long long ticks, wchar_t* filter, size_t size)
{
    wchar_t start[40], end[40];
    if (ticks < CRASH_WINDOW_TICKS ||
        ticks > ~0ULL - CRASH_WINDOW_TICKS) return 0;
    if (!utc_timestamp(ticks - CRASH_WINDOW_TICKS, start, 40) ||
        !utc_timestamp(ticks + CRASH_WINDOW_TICKS, end, 40)) return 0;
    int written = swprintf(filter, size,
        L"TimeCreated[@SystemTime >= '%ls' and @SystemTime <= '%ls']", start, end);
    return written > 0 && (size_t)written < size;
}

/* Print structured timestamps and event IDs without inferring causation. */
static int event_phase(const wchar_t* provider, unsigned id,
                       unsigned long long eventTime, unsigned long long crashTime)
{
    if ((wcscmp(provider, L"Microsoft-Windows-Kernel-Power")==0 && id==41) ||
        (wcscmp(provider, L"Microsoft-Windows-Kernel-General")==0 && id==12) ||
        (wcscmp(provider, L"EventLog")==0 && (id==6005 || id==6008)) ||
        (wcscmp(provider, L"Microsoft-Windows-WER-SystemErrorReporting")==0 &&
         (id==1001 || id==1019))) return 2;
    return eventTime <= crashTime ? 0 : 1;
}

static void print_window_events(const wchar_t* filter, unsigned long long crashTime, int phase)
{
    const char* headings[] = {
        "AT OR BEFORE DUMP CRASH TIME (not automatic cause evidence)",
        "AFTER DUMP CRASH TIME (may include restart initialization)",
        "RESTART / CRASH-REPORT RECORDS (recording time, not crash time)"
    };
    printf("\n%s\n", headings[phase]);
    wchar_t query[2048];
    int written = swprintf(query, 2048,
        L"*[System[(Provider[@Name='Microsoft-Windows-WHEA-Logger'] or "
        L"Provider[@Name='Microsoft-Windows-Kernel-Power'] or "
        L"Provider[@Name='Microsoft-Windows-Kernel-PnP'] or "
        L"Provider[@Name='Microsoft-Windows-WER-SystemErrorReporting'] or "
        L"Provider[@Name='Display'] or Provider[@Name='disk'] or "
        L"Provider[@Name='Ntfs'] or Provider[@Name='Microsoft-Windows-Ntfs'] or "
        L"Provider[@Name='Microsoft-Windows-Kernel-General'] or Provider[@Name='EventLog']) and %ls]]",
        filter);
    if (written <= 0 || written >= 2048) {
        printf("Window query construction failed.\n");
        return;
    }
    EVT_HANDLE results = EvtQuery(NULL, L"System", query,
        EvtQueryChannelPath | EvtQueryReverseDirection);
    EVT_HANDLE context = EvtCreateRenderContext(0, NULL, EvtRenderContextSystem);
    if (!results || !context) {
        printf("Window events unavailable (query/context error).\n");
        if (results) EvtClose(results);
        if (context) EvtClose(context);
        return;
    }
    unsigned count = 0, unreadable = 0, shown = 0;
    int complete = 0;
    while (count < 10000) {
        EVT_HANDLE event = NULL;
        DWORD returned = 0, used = 0, properties = 0;
        if (!EvtNext(results, 1, &event, 1000, 0, &returned)) {
            complete = GetLastError() == ERROR_NO_MORE_ITEMS;
            break;
        }
        ++count;
        EvtRender(context, event, EvtRenderEventValues, 0, NULL, &used, &properties);
        EVT_VARIANT* values = used ? malloc(used) : NULL;
        if (values && EvtRender(context, event, EvtRenderEventValues,
                used, values, &used, &properties) &&
            properties > EvtSystemTimeCreated &&
            values[EvtSystemProviderName].Type == EvtVarTypeString &&
            values[EvtSystemProviderName].StringVal &&
            values[EvtSystemEventID].Type == EvtVarTypeUInt16 &&
            values[EvtSystemTimeCreated].Type == EvtVarTypeFileTime) {
            if (event_phase(values[EvtSystemProviderName].StringVal,
                    values[EvtSystemEventID].UInt16Val,
                    values[EvtSystemTimeCreated].FileTimeVal, crashTime) != phase) {
                free(values);
                EvtClose(event);
                continue;
            }
            ++shown;
            wchar_t stamp[40];
            char provider[256], timeText[80];
            if (utc_timestamp(values[EvtSystemTimeCreated].FileTimeVal, stamp, 40)) {
                wide_to_utf8(stamp, timeText, sizeof(timeText));
                wide_to_utf8(values[EvtSystemProviderName].StringVal, provider, sizeof(provider));
                printf("  %s | %s | Event %u\n", timeText, provider,
                    (unsigned)values[EvtSystemEventID].UInt16Val);
            } else ++unreadable;
        } else ++unreadable;
        free(values);
        EvtClose(event);
    }
    printf("Group events: %u; window scanned: %u; unreadable: %u; scan: %s\n",
        shown, count, unreadable, complete ? "COMPLETE" : "PARTIAL (limit or read error)");
    printf("Time proximity is context, not proof of causation.\n");
    EvtClose(context);
    EvtClose(results);
}

/*
 * Main correlation engine.
 */
void analyze_correlations(void)
{
    CorrelatedDevice devices[MAX_CORRELATED_DEVICES];
    int deviceCount = 0;
    int totalEvents = 0;

    printf("\n");
    printf("CORRELATION ENGINE\n");
    printf("--------------------\n");
    printf(
        "Analyzing relationships between Windows events,\n"
        "devices and installed drivers...\n"
    );

    printf("Scope: GENERAL HISTORY, up to 10000 retained PnP events; not crash evidence.\n");
    if (!collect_kernel_pnp_events(devices, &deviceCount, &totalEvents,
        L"*[System[Provider[@Name='Microsoft-Windows-Kernel-PnP'] and EventID=219]]"))
    {
        printf("History scan incomplete (query/read error or limit).\n");
    }

    resolve_all_devices(devices, deviceCount);
    print_correlation_results(devices, deviceCount, totalEvents);

    CrashAnalysis crash;
    wchar_t filter[256], query[512], stamp[40];
    char timeText[80];
    printf("\nCRASH TIME WINDOW\n--------------------\n");
    if (!get_last_crash_analysis(&crash) || !crash.crashTime ||
        !crash_time_filter(crash.crashTime, filter, 256) ||
        !utc_timestamp(crash.crashTime, stamp, 40)) {
        printf("UNAVAILABLE: dump-internal crash time could not be read; file timestamps are not substituted.\n");
        return;
    }
    wide_to_utf8(stamp, timeText, sizeof(timeText));
    printf("Dump: %s\n", crash.dumpPath);
    printf("Reference (UTC): %s\n", timeText);
    printf("Source: dump-internal debug session time (.time -h 0), UTC.\n");
    printf("Window: +/- 5 minutes, inclusive. Event times are recording times.\n");
    for (int phase=0; phase<3; ++phase)
        print_window_events(filter, crash.crashTime, phase);
    for (int phase=0; phase<2; ++phase) {
    wchar_t phaseFilter[384];
    swprintf(phaseFilter, 384, L"%ls and TimeCreated[@SystemTime %ls '%ls']",
        filter, phase == 0 ? L"<=" : L">", stamp);
    int written = swprintf(query, 512,
        L"*[System[Provider[@Name='Microsoft-Windows-Kernel-PnP'] and EventID=219 and %ls]]",
        phaseFilter);
    if (written <= 0 || written >= 512 ||
        !collect_kernel_pnp_events(devices, &deviceCount, &totalEvents, query)) {
        printf("Crash-window PnP scan incomplete; no definitive absence conclusion.\n");
        return;
    }
    printf("\nPnP scope: %s; installed drivers describe the current system.\n",
        phase == 0 ? "AT OR BEFORE CRASH" : "AFTER CRASH / POSSIBLE RESTART INITIALIZATION");
    resolve_all_devices(devices, deviceCount);
    print_correlation_results(devices, deviceCount, totalEvents);
    }
}
