#include <stdio.h>
#include "report.h"
#include <string.h>
#include <windows.h>

#include "system.h"


typedef LONG(WINAPI* RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);

void scan_system(void)
{
    SYSTEM_INFO systemInfo;
    MEMORYSTATUSEX memoryStatus = {0};
    ULONGLONG uptime;
    HKEY hKey;
    char cpuName[256] = { 0 };
    DWORD cpuNameSize = sizeof(cpuName) - 1;

    GetSystemInfo(&systemInfo);

    memoryStatus.dwLength = sizeof(memoryStatus);
    GlobalMemoryStatusEx(&memoryStatus);

    uptime = GetTickCount64();

    printf("\n");
    printf("SYSTEM\n");
    printf("--------------------\n");

    /* CPU Architecture */
    printf("Architecture       : ");
    switch (systemInfo.wProcessorArchitecture)
    {
    case PROCESSOR_ARCHITECTURE_AMD64:
        printf("x64\n");
        break;
    case PROCESSOR_ARCHITECTURE_ARM64:
        printf("ARM64\n");
        break;
    case PROCESSOR_ARCHITECTURE_INTEL:
        printf("x86\n");
        break;
    default:
        printf("Unknown\n");
        break;
    }

    report.logical_processors = systemInfo.dwNumberOfProcessors;
    report.architecture = systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 ? "x64" : systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64 ? "ARM64" : systemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL ? "x86" : "unknown";

    /* Logical Processors */
    printf("Logical Processors : %lu\n", systemInfo.dwNumberOfProcessors);

    /* CPU Model */
    if (RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0",
        0,
        KEY_READ,
        &hKey) == ERROR_SUCCESS)
    {
        if (RegQueryValueExA(
            hKey,
            "ProcessorNameString",
            NULL,
            NULL,
            (LPBYTE)cpuName,
            &cpuNameSize) == ERROR_SUCCESS)
        {
            cpuName[sizeof(cpuName) - 1] = '\0';
            printf("CPU                : %s\n", cpuName);
        }
        else
        {
            printf("CPU                : Unknown\n");
        }

        RegCloseKey(hKey);
    }
    else
    {
        printf("CPU                : Unknown\n");
    }

    memcpy(report.cpu, cpuName, sizeof(report.cpu));
    report.ram_gb = (double)memoryStatus.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
    /* RAM */
    printf(
        "RAM                : %.2f GB\n",
        (double)memoryStatus.ullTotalPhys / (1024.0 * 1024.0 * 1024.0)
    );

    /* Windows Version via RtlGetVersion (avoids GetVersionExA manifest lie) */
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    RtlGetVersionPtr rtlGetVersion = NULL;
    RTL_OSVERSIONINFOW rovi;
    ZeroMemory(&rovi, sizeof(rovi));
    rovi.dwOSVersionInfoSize = sizeof(rovi);

    if (ntdll != NULL)
    {
        FARPROC procedure = GetProcAddress(ntdll, "RtlGetVersion");
        _Static_assert(sizeof(rtlGetVersion) == sizeof(procedure),
                       "Windows function pointer sizes must match");
        memcpy(&rtlGetVersion, &procedure, sizeof(rtlGetVersion));
    }

    if (rtlGetVersion != NULL && rtlGetVersion(&rovi) == 0)
    {
        printf(
            "Windows Version    : %lu.%lu\n",
            rovi.dwMajorVersion,
            rovi.dwMinorVersion
        );
        report.windows_build = rovi.dwBuildNumber;
        printf("Windows Build      : %lu\n", rovi.dwBuildNumber);
    }
    else
    {
        printf("Windows Version    : Unknown\n");
        printf("Windows Build      : Unknown\n");
    }

    /* Uptime */
    ULONGLONG totalSeconds = uptime / 1000;
    ULONGLONG hours = totalSeconds / 3600;
    ULONGLONG minutes = (totalSeconds % 3600) / 60;
    ULONGLONG seconds = totalSeconds % 60;

    printf(
        "Uptime             : %llu seconds (%lluh %llum %llus)\n",
        totalSeconds,
        hours,
        minutes,
        seconds
    );
}
