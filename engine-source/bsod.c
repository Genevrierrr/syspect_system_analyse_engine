#include <stdio.h>
#include "report.h"
#include <stdlib.h>
#include <windows.h>
#include <dbghelp.h>

#include "bsod.h"
#include "dump_analyzer.h"


#define DUMP_SEARCH_PATH "C:\\Windows\\Minidump\\*.dmp"
#define MAX_DUMP_PATH 512

#define MDMP_SIGNATURE 0x504D444D
#define PAGE_SIGNATURE 0x45474150

static void print_wide_utf8_n(const char* label, const wchar_t* value, int charCount)
{
    int requiredSize;
    char* utf8Buffer;

    if (label == NULL)
    {
        label = "";
    }

    if (value == NULL || charCount <= 0)
    {
        printf("%sUnknown\n", label);
        return;
    }

    requiredSize = WideCharToMultiByte(
        CP_UTF8,
        0,
        value,
        charCount,
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

    utf8Buffer = (char*)malloc((size_t)requiredSize + 1);
    if (utf8Buffer == NULL)
    {
        printf("%sMemory allocation failed\n", label);
        return;
    }

    if (WideCharToMultiByte(
            CP_UTF8,
            0,
            value,
            charCount,
            utf8Buffer,
            requiredSize,
            NULL,
            NULL) > 0)
    {
        utf8Buffer[requiredSize] = '\0';
        printf("%s%s\n", label, utf8Buffer);
    }
    else
    {
        printf("%sUnknown\n", label);
    }

    free(utf8Buffer);
}

static void print_file_time(FILETIME fileTime)
{
    FILETIME localFileTime;
    SYSTEMTIME systemTime;

    if (!FileTimeToLocalFileTime(&fileTime, &localFileTime))
    {
        printf("Unknown");
        return;
    }

    if (!FileTimeToSystemTime(&localFileTime, &systemTime))
    {
        printf("Unknown");
        return;
    }

    printf(
        "%02d/%02d/%04d %02d:%02d:%02d",
        systemTime.wDay,
        systemTime.wMonth,
        systemTime.wYear,
        systemTime.wHour,
        systemTime.wMinute,
        systemTime.wSecond
    );
}

static DWORD read_u32(const unsigned char* data)
{
    return
        ((DWORD)data[0]) |
        ((DWORD)data[1] << 8) |
        ((DWORD)data[2] << 16) |
        ((DWORD)data[3] << 24);
}

static int read_dump_header(
    const char* filePath,
    unsigned char* headerBuffer,
    DWORD headerSize
)
{
    HANDLE fileHandle;
    DWORD bytesRead = 0;

    fileHandle = CreateFileA(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        printf("Could not open dump file.\n");
        printf("Windows Error    : %lu\n", GetLastError());
        return 0;
    }

    if (!ReadFile(fileHandle, headerBuffer, headerSize, &bytesRead, NULL))
    {
        printf("Could not read dump header.\n");
        printf("Windows Error    : %lu\n", GetLastError());
        CloseHandle(fileHandle);
        return 0;
    }

    CloseHandle(fileHandle);

    if (bytesRead < 4)
    {
        printf("Dump file is too small to identify.\n");
        return 0;
    }

    return 1;
}

static int analyze_mdmp(const char* filePath)
{
    HANDLE fileHandle;
    HANDLE mappingHandle;
    LPVOID baseAddress;
    LARGE_INTEGER fileSize;
    PMINIDUMP_HEADER header;
    ULONG streamSize = 0;
    int hasUserExceptionStream = 0;

    printf("\n");
    printf("MINIDUMP ANALYSIS\n");
    printf("--------------------\n");

    fileHandle = CreateFileA(
        filePath,
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (fileHandle == INVALID_HANDLE_VALUE)
    {
        printf("Could not open minidump.\n");
        printf("Windows Error    : %lu\n", GetLastError());
        return 0;
    }

    if (!GetFileSizeEx(fileHandle, &fileSize) ||
        fileSize.QuadPart < (LONGLONG)sizeof(MINIDUMP_HEADER))
    {
        printf("Minidump file is too small.\n");
        CloseHandle(fileHandle);
        return 0;
    }

    mappingHandle = CreateFileMappingA(
        fileHandle,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL
    );

    if (mappingHandle == NULL)
    {
        printf("Could not create dump mapping.\n");
        CloseHandle(fileHandle);
        return 0;
    }

    baseAddress = MapViewOfFile(
        mappingHandle,
        FILE_MAP_READ,
        0,
        0,
        0
    );

    if (baseAddress == NULL)
    {
        printf("Could not map dump into memory.\n");
        CloseHandle(mappingHandle);
        CloseHandle(fileHandle);
        return 0;
    }

    header = (PMINIDUMP_HEADER)baseAddress;

    if (header->Signature != MDMP_SIGNATURE)
    {
        printf("Minidump signature validation failed.\n");
        UnmapViewOfFile(baseAddress);
        CloseHandle(mappingHandle);
        CloseHandle(fileHandle);
        return 0;
    }

    printf("Signature        : VALID (MDMP)\n");
    printf("Version          : 0x%08X\n", header->Version);
    printf("Stream Count     : %u\n", header->NumberOfStreams);

    /*
     * Exception information.
     */
    PMINIDUMP_EXCEPTION_STREAM exceptionStream = NULL;
    if (MiniDumpReadDumpStream(
        baseAddress,
        ExceptionStream,
        NULL,
        (PVOID*)&exceptionStream,
        &streamSize) && exceptionStream != NULL)
    {
        hasUserExceptionStream = 1;
        printf("\n");
        printf("EXCEPTION\n");
        printf("--------------------\n");
        printf("Thread ID        : %u\n", exceptionStream->ThreadId);
        printf("Exception Code   : 0x%08X\n", exceptionStream->ExceptionRecord.ExceptionCode);
        printf(
            "Exception Address: 0x%llX\n",
            (unsigned long long)exceptionStream->ExceptionRecord.ExceptionAddress
        );
    }

    /*
     * System information.
     */
    PMINIDUMP_SYSTEM_INFO systemInfo = NULL;
    streamSize = 0;
    if (MiniDumpReadDumpStream(
        baseAddress,
        SystemInfoStream,
        NULL,
        (PVOID*)&systemInfo,
        &streamSize) && systemInfo != NULL)
    {
        printf("\n");
        printf("DUMP SYSTEM INFORMATION\n");
        printf("--------------------\n");
        printf("Processors       : %u\n", systemInfo->NumberOfProcessors);
        printf("Windows Version  : %u.%u\n", systemInfo->MajorVersion, systemInfo->MinorVersion);
        printf("Windows Build    : %u\n", systemInfo->BuildNumber);
    }

    /*
     * Module list.
     */
    PMINIDUMP_MODULE_LIST moduleList = NULL;
    streamSize = 0;
    if (MiniDumpReadDumpStream(
        baseAddress,
        ModuleListStream,
        NULL,
        (PVOID*)&moduleList,
        &streamSize) && moduleList != NULL)
    {
        printf("\n");
        printf("LOADED MODULES\n");
        printf("--------------------\n");
        printf("Module Count     : %u\n", moduleList->NumberOfModules);

        ULONG limit = moduleList->NumberOfModules;
        if (limit > 20)
        {
            limit = 20;
        }

        for (ULONG i = 0; i < limit; i++)
        {
            PMINIDUMP_MODULE module = &moduleList->Modules[i];

            printf("\n");
            printf("[Module %lu]\n", i + 1);
            printf("Base Address     : 0x%llX\n", (unsigned long long)module->BaseOfImage);
            printf("Image Size       : 0x%X\n", module->SizeOfImage);

            if (module->ModuleNameRva != 0 &&
                (LONGLONG)module->ModuleNameRva + (LONGLONG)sizeof(MINIDUMP_STRING) <= fileSize.QuadPart)
            {
                PMINIDUMP_STRING name = (PMINIDUMP_STRING)((BYTE*)baseAddress + module->ModuleNameRva);

                if (name->Length > 0 &&
                    (LONGLONG)module->ModuleNameRva + (LONGLONG)sizeof(ULONG32) + (LONGLONG)name->Length <= fileSize.QuadPart)
                {
                    print_wide_utf8_n(
                        "Module Name      : ",
                        name->Buffer,
                        (int)(name->Length / sizeof(wchar_t))
                    );
                }
            }
        }
    }

    UnmapViewOfFile(baseAddress);
    CloseHandle(mappingHandle);
    CloseHandle(fileHandle);

    printf("\n");
    printf("Minidump structure analysis completed successfully.\n");

    /*
     * Kernel minidumps may not contain a user-mode ExceptionStream.
     * In that case, require the debugger-based analyzer to produce
     * structured crash data before reporting full analysis success.
     */
    if (!hasUserExceptionStream)
    {
        return analyze_kernel_dump_file(filePath);
    }

    return 1;
}

static int analyze_kernel_dump(const char* filePath)
{
    unsigned char header[64];
    ZeroMemory(header, sizeof(header));

    printf("\n");
    printf("WINDOWS KERNEL CRASH DUMP\n");
    printf("--------------------\n");
    printf("Dump Format      : Windows Crash Dump\n");
    printf("Signature        : PAGE\n");

    if (read_dump_header(filePath, header, sizeof(header)))
    {
        printf("Format Status    : RECOGNIZED\n");
    }

    printf("\n");
    printf(
        "Important        : This file is not a classic\n"
        "                   user-mode/minidump file.\n"
    );
    printf(
        "Analysis Method  : Kernel crash dump parser\n"
        "                   required for full analysis.\n"
    );
    printf("\n");
    printf(
        "BugCheck details can be correlated with\n"
        "Windows Event Log Event ID 1001.\n"
    );

    /*
     * Pass the PAGE-format kernel dump to the
     * debugger-based kernel dump analyzer.
     */
    return analyze_kernel_dump_file(filePath);
}

static int analyze_dump(const char* filePath)
{
    unsigned char header[16];
    ZeroMemory(header, sizeof(header));

    printf("\n");
    printf("DUMP ANALYSIS\n");
    printf("--------------------\n");
    printf("File             : %s\n", filePath);

    if (!read_dump_header(filePath, header, sizeof(header)))
    {
        return 0;
    }

    DWORD signature = read_u32(header);

    if (signature == MDMP_SIGNATURE)
    {
        return analyze_mdmp(filePath);
    }

    if (signature == PAGE_SIGNATURE)
    {
        return analyze_kernel_dump(filePath);
    }

    printf("Unknown dump signature.\n");
    printf(
        "Signature        : %02X %02X %02X %02X\n",
        header[0],
        header[1],
        header[2],
        header[3]
    );

    return 0;
}

void scan_bsod_dumps(void)
{
    WIN32_FIND_DATAA findData;
    HANDLE findHandle;
    int dumpCount = 0;
    int analyzedCount = 0;
    char dumpPath[MAX_DUMP_PATH];

    reset_crash_analysis();

    printf("\n");
    printf("BSOD / MINIDUMP ANALYZER\n");
    printf("--------------------\n");
    printf("Scanning Windows Minidump directory...\n");

    findHandle = FindFirstFileA(DUMP_SEARCH_PATH, &findData);

    if (findHandle == INVALID_HANDLE_VALUE)
    {
        DWORD error = GetLastError();

        if (error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND)
        {
            printf("\n");
            report.dump_status = "no_dumps";
            printf("No dump files were found.\n");
            printf("Status          : NO DUMPS DETECTED\n");
            return;
        }

        printf("\n");
        printf("Could not access the dump directory.\n");
        printf("Windows Error   : %lu\n", error);
        return;
    }

    do
    {
        ULONGLONG fileSize =
            ((ULONGLONG)findData.nFileSizeHigh << 32) |
            findData.nFileSizeLow;

        printf("\n");
        printf("[Dump %d]\n", dumpCount + 1);
        printf("File            : %s\n", findData.cFileName);
        printf("Size            : %llu KB\n", fileSize / 1024);
        printf("Created         : ");
        print_file_time(findData.ftCreationTime);
        printf("\n");
        printf("Modified        : ");
        print_file_time(findData.ftLastWriteTime);
        printf("\n");

        snprintf(
            dumpPath,
            sizeof(dumpPath),
            "C:\\Windows\\Minidump\\%s",
            findData.cFileName
        );

        if (analyze_dump(dumpPath))
        {
            analyzedCount++;
        }

        dumpCount++;
    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);

    printf("\n");
    printf("--------------------\n");
    printf("Dump files found      : %d\n", dumpCount);
    printf("Successfully analyzed : %d\n", analyzedCount);

    report.dump_status = analyzedCount == dumpCount ? "analyzed" : analyzedCount > 0 ? "partial" : "analysis_failed";
    if (analyzedCount > 0)
    {
        printf("Status                : DUMPS ANALYZED\n");
    }
    else
    {
        printf("Status                : ANALYSIS FAILED\n");
    }
}
