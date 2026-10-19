#include <stdio.h>
#include "report.h"
#include <stdlib.h>
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <wchar.h>

#include "hardware.h"


static void print_utf8(const char* label, const wchar_t* value)
{
    int requiredSize;
    char* utf8Buffer;

    if (label == NULL)
    {
        label = "";
    }

    if (value == NULL || value[0] == L'\0')
    {
        printf("%sUnknown\n", label);
        return;
    }

    requiredSize = WideCharToMultiByte(
        CP_UTF8,
        0,
        value,
        -1,
        NULL,
        0,
        NULL,
        NULL
    );

    if (requiredSize <= 0)
    {
        printf("%sUnknown\n", label);
        return;
    }

    utf8Buffer = (char*)malloc((size_t)requiredSize);
    if (utf8Buffer == NULL)
    {
        printf("%sMemory allocation failed\n", label);
        return;
    }

    if (WideCharToMultiByte(
            CP_UTF8,
            0,
            value,
            -1,
            utf8Buffer,
            requiredSize,
            NULL,
            NULL) > 0)
    {
        printf("%s%s\n", label, utf8Buffer);
    }
    else
    {
        printf("%sUnknown\n", label);
    }

    free(utf8Buffer);
}

void scan_hardware(void)
{
    HDEVINFO deviceInfo;
    SP_DEVINFO_DATA deviceData;
    DWORD index = 0;
    DWORD detectedCount = 0;
    DWORD problemCount = 0;

    printf("\n");
    printf("HARDWARE\n");
    printf("--------------------\n");

    deviceInfo = SetupDiGetClassDevsW(
        NULL,
        NULL,
        NULL,
        DIGCF_PRESENT | DIGCF_ALLCLASSES
    );

    if (deviceInfo == INVALID_HANDLE_VALUE)
    {
        printf("Could not access Windows device database.\n");
        return;
    }

    ZeroMemory(&deviceData, sizeof(deviceData));
    deviceData.cbSize = sizeof(SP_DEVINFO_DATA);

    while (SetupDiEnumDeviceInfo(deviceInfo, index, &deviceData))
    {
        wchar_t deviceName[256] = { 0 };
        wchar_t manufacturer[256] = { 0 };
        wchar_t hardwareId[512] = { 0 };
        ULONG status = 0;
        ULONG problem = 0;
        DWORD requiredSize = 0;
        DWORD propertyType = 0;

        BOOL deviceNameResult = SetupDiGetDeviceRegistryPropertyW(
            deviceInfo,
            &deviceData,
            SPDRP_DEVICEDESC,
            &propertyType,
            (PBYTE)deviceName,
            sizeof(deviceName),
            &requiredSize
        );

        if (deviceNameResult)
        {
            printf("[%lu] ", (unsigned long)index);
            print_utf8("", deviceName);
            detectedCount++;

            if (SetupDiGetDeviceRegistryPropertyW(
                    deviceInfo,
                    &deviceData,
                    SPDRP_MFG,
                    &propertyType,
                    (PBYTE)manufacturer,
                    sizeof(manufacturer),
                    &requiredSize))
            {
                print_utf8("     Manufacturer : ", manufacturer);
            }
            else
            {
                printf("     Manufacturer : Unknown\n");
            }

            if (SetupDiGetDeviceRegistryPropertyW(
                    deviceInfo,
                    &deviceData,
                    SPDRP_HARDWAREID,
                    &propertyType,
                    (PBYTE)hardwareId,
                    sizeof(hardwareId),
                    &requiredSize))
            {
                /* SPDRP_HARDWAREID is REG_MULTI_SZ. The first ID is the primary one. */
                print_utf8("     Hardware ID  : ", hardwareId);
            }
            else
            {
                printf("     Hardware ID  : Unknown\n");
            }

            CONFIGRET result = CM_Get_DevNode_Status(
                &status,
                &problem,
                deviceData.DevInst,
                0
            );

            if (result == CR_SUCCESS)
            {
                if (problem == 0)
                {
                    printf("     Status       : OK\n");
                }
                else
                {
                    printf("     Status       : PROBLEM\n");
                    printf("     Problem Code : %lu\n", (unsigned long)problem);
                    problemCount++;
                }
            }
            else
            {
                printf("     Status       : UNKNOWN\n");
            }
        }

        index++;
    }

    printf("\nTotal detected devices    : %lu\n", (unsigned long)detectedCount);
    printf("Problematic devices found : %lu\n", (unsigned long)problemCount);

    report.devices = (long)detectedCount;
    report.problematic_devices = (long)problemCount;
    SetupDiDestroyDeviceInfoList(deviceInfo);
}
