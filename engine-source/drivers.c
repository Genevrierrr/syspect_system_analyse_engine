#include <stdio.h>
#include "report.h"
#include <stdlib.h>
#include <windows.h>
#include <setupapi.h>
#include <wchar.h>

#include "drivers.h"


static int wide_to_utf8(
    const wchar_t* source,
    char* destination,
    int destinationSize
)
{
    if (source == NULL || destination == NULL || destinationSize <= 0)
    {
        return 0;
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
        return 0;
    }

    return 1;
}

static int read_driver_reg_string(
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
    return wide_to_utf8(wideBuffer, buffer, (int)bufferSize);
}

static void print_driver_info(
    HDEVINFO deviceInfo,
    PSP_DEVINFO_DATA deviceData
)
{
    wchar_t wideServiceName[128] = { 0 };
    wchar_t wideDriverKey[256] = { 0 };
    char serviceName[384] = { 0 };
    char driverKey[768] = { 0 };
    char providerName[768] = { 0 };
    char driverVersion[256] = { 0 };
    char driverDate[256] = { 0 };
    DWORD propertyType = 0;
    DWORD requiredSize = 0;
    HKEY hDrvKey;

    if (SetupDiGetDeviceRegistryPropertyW(
            deviceInfo,
            deviceData,
            SPDRP_SERVICE,
            &propertyType,
            (PBYTE)wideServiceName,
            sizeof(wideServiceName),
            &requiredSize))
    {
        wide_to_utf8(wideServiceName, serviceName, sizeof(serviceName));
    }

    if (SetupDiGetDeviceRegistryPropertyW(
            deviceInfo,
            deviceData,
            SPDRP_DRIVER,
            &propertyType,
            (PBYTE)wideDriverKey,
            sizeof(wideDriverKey),
            &requiredSize))
    {
        wide_to_utf8(wideDriverKey, driverKey, sizeof(driverKey));
    }

    hDrvKey = SetupDiOpenDevRegKey(
        deviceInfo,
        deviceData,
        DICS_FLAG_GLOBAL,
        0,
        DIREG_DRV,
        KEY_READ
    );

    if (hDrvKey != INVALID_HANDLE_VALUE)
    {
        read_driver_reg_string(hDrvKey, L"ProviderName", providerName, sizeof(providerName));
        read_driver_reg_string(hDrvKey, L"DriverVersion", driverVersion, sizeof(driverVersion));
        read_driver_reg_string(hDrvKey, L"DriverDate", driverDate, sizeof(driverDate));

        if (serviceName[0] == '\0')
        {
            read_driver_reg_string(hDrvKey, L"Service", serviceName, sizeof(serviceName));
        }

        RegCloseKey(hDrvKey);

        printf("     Driver Type  : Installed Driver\n");
        printf("     Provider     : %s\n", providerName[0] ? providerName : "Unknown");
        printf("     Driver Date  : %s\n", driverDate[0] ? driverDate : "Unknown");
        printf("     Driver Ver.  : %s\n", driverVersion[0] ? driverVersion : "Unknown");
        printf("     Service      : %s\n", serviceName[0] ? serviceName : "None");
        printf("     Driver Key   : %s\n", driverKey[0] ? driverKey : "Unknown");
        return;
    }

    printf("     Driver Info  : No installed driver key\n");
}

void scan_drivers(void)
{
    HDEVINFO deviceInfo;
    SP_DEVINFO_DATA deviceData;
    DWORD index = 0;
    DWORD detectedCount = 0;

    printf("\n");
    printf("DRIVERS\n");
    printf("--------------------\n");

    deviceInfo = SetupDiGetClassDevsW(
        NULL,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_ALLCLASSES
    );

    if (deviceInfo == INVALID_HANDLE_VALUE)
    {
        printf("Could not access Windows driver database.\n");
        return;
    }

    ZeroMemory(&deviceData, sizeof(deviceData));
    deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

    while (SetupDiEnumDeviceInfo(deviceInfo, index, &deviceData))
    {
        wchar_t wideDeviceName[256] = { 0 };
        char deviceName[768] = { 0 };
        DWORD requiredSize = 0;
        DWORD propertyType = 0;

        BOOL nameResult = SetupDiGetDeviceRegistryPropertyW(
            deviceInfo,
            &deviceData,
            SPDRP_DEVICEDESC,
            &propertyType,
            (PBYTE)wideDeviceName,
            sizeof(wideDeviceName),
            &requiredSize
        );

        if (nameResult && wide_to_utf8(wideDeviceName, deviceName, sizeof(deviceName)))
        {
            printf("[%lu] %s\n", (unsigned long)index, deviceName);
            print_driver_info(deviceInfo, &deviceData);
            printf("\n");
            detectedCount++;
        }

        index++;
    }

    printf("Total detected driver devices: %lu\n", (unsigned long)detectedCount);
    report.drivers = (long)detectedCount;
    SetupDiDestroyDeviceInfoList(deviceInfo);
}
