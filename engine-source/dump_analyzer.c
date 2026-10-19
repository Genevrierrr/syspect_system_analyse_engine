#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "dump_analyzer.h"

#define MAX_PATH_LENGTH 1024
#define MAX_COMMAND_LENGTH 4096
#define DEBUGGER_TIMEOUT_MS (120 * 1000)

/*
 * Stores the most recent structured crash analysis.
 */
static CrashAnalysis lastCrashAnalysis;


static int get_file_write_time_value(const char* path, unsigned long long* value)
{
    WIN32_FILE_ATTRIBUTE_DATA data;
    ULARGE_INTEGER timeValue;

    if (path == NULL || value == NULL)
    {
        return 0;
    }

    if (!GetFileAttributesExA(path, GetFileExInfoStandard, &data))
    {
        return 0;
    }

    timeValue.LowPart = data.ftLastWriteTime.dwLowDateTime;
    timeValue.HighPart = data.ftLastWriteTime.dwHighDateTime;
    *value = timeValue.QuadPart;
    return 1;
}

static void store_latest_crash_analysis(
    const char* dumpPath,
    CrashAnalysis* analysis
)
{
    unsigned long long writeTime = 0;

    if (dumpPath == NULL || analysis == NULL || !analysis->valid)
    {
        return;
    }

    get_file_write_time_value(dumpPath, &writeTime);
    analysis->dumpWriteTime = writeTime;

    strncpy(analysis->dumpPath, dumpPath, sizeof(analysis->dumpPath) - 1);
    analysis->dumpPath[sizeof(analysis->dumpPath) - 1] = '\0';

    if (!lastCrashAnalysis.valid ||
        (analysis->crashTime ? analysis->crashTime : analysis->dumpWriteTime) >=
        (lastCrashAnalysis.crashTime ? lastCrashAnalysis.crashTime : lastCrashAnalysis.dumpWriteTime))
    {
        lastCrashAnalysis = *analysis;
    }
}

/*
 * Check whether a file exists.
 */
static int file_exists(const char* path)
{
    DWORD attributes;

    if (path == NULL || path[0] == '\0')
    {
        return 0;
    }

    attributes = GetFileAttributesA(path);
    if (attributes == INVALID_FILE_ATTRIBUTES)
    {
        return 0;
    }

    if (attributes & FILE_ATTRIBUTE_DIRECTORY)
    {
        return 0;
    }

    return 1;
}

/*
 * Validate a Windows file path before placing it
 * inside a quoted debugger command line.
 */
static int is_safe_path(const char* path)
{
    size_t i;
    size_t length;

    if (path == NULL)
    {
        return 0;
    }

    length = strlen(path);
    if (length == 0 || length >= MAX_PATH_LENGTH)
    {
        return 0;
    }

    for (i = 0; i < length; i++)
    {
        unsigned char c = (unsigned char)path[i];

        if (c == '"' ||
            c == '\r' ||
            c == '\n' ||
            c == '|' ||
            c == '&' ||
            c == '^' ||
            c == '%' ||
            c == '<' ||
            c == '>' ||
            c < 0x20)
        {
            return 0;
        }
    }

    return 1;
}

/*
 * Find a Windows console debugger executable (cdb.exe or kd.exe).
 */
static int find_debugger(char* debuggerPath, size_t debuggerPathSize)
{
    const char* fixedCandidates[] =
    {
        "C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\cdb.exe",
        "C:\\Program Files (x86)\\Windows Kits\\10\\Debuggers\\x64\\kd.exe",
        "C:\\Program Files\\Windows Kits\\10\\Debuggers\\x64\\cdb.exe",
        "C:\\Program Files\\Windows Kits\\10\\Debuggers\\x64\\kd.exe"
    };
    size_t fixedCount = sizeof(fixedCandidates) / sizeof(fixedCandidates[0]);
    size_t i;
    const char* windowsApps = "C:\\Program Files\\WindowsApps";
    WIN32_FIND_DATAA findData;
    HANDLE findHandle;
    char searchPattern[MAX_PATH_LENGTH];
    char candidate[MAX_PATH_LENGTH];

    if (debuggerPath == NULL || debuggerPathSize == 0)
    {
        return 0;
    }

    debuggerPath[0] = '\0';

    /*
     * 1. Check standard Windows SDK locations first.
     */
    for (i = 0; i < fixedCount; i++)
    {
        if (file_exists(fixedCandidates[i]))
        {
            strncpy(debuggerPath, fixedCandidates[i], debuggerPathSize - 1);
            debuggerPath[debuggerPathSize - 1] = '\0';
            return 1;
        }
    }

    /*
     * 2. Search Microsoft WinDbg Store packages (prefer console cdb.exe / kd.exe).
     */
    snprintf(
        searchPattern,
        sizeof(searchPattern),
        "%s\\Microsoft.WinDbg_*",
        windowsApps
    );

    findHandle = FindFirstFileA(searchPattern, &findData);
    if (findHandle != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
            {
                continue;
            }

            snprintf(
                candidate,
                sizeof(candidate),
                "%s\\%s\\amd64\\cdb.exe",
                windowsApps,
                findData.cFileName
            );

            if (file_exists(candidate))
            {
                snprintf(debuggerPath, debuggerPathSize, "%s", candidate);
                debuggerPath[debuggerPathSize - 1] = '\0';
                FindClose(findHandle);
                return 1;
            }

            snprintf(
                candidate,
                sizeof(candidate),
                "%s\\%s\\amd64\\kd.exe",
                windowsApps,
                findData.cFileName
            );

            if (file_exists(candidate))
            {
                snprintf(debuggerPath, debuggerPathSize, "%s", candidate);
                debuggerPath[debuggerPathSize - 1] = '\0';
                FindClose(findHandle);
                return 1;
            }
        } while (FindNextFileA(findHandle, &findData));

        FindClose(findHandle);
    }

    /*
     * 3. Fall back to system PATH.
     */
    if (SearchPathA(NULL, "cdb.exe", NULL, (DWORD)debuggerPathSize, debuggerPath, NULL) != 0)
    {
        return 1;
    }

    if (SearchPathA(NULL, "kd.exe", NULL, (DWORD)debuggerPathSize, debuggerPath, NULL) != 0)
    {
        return 1;
    }

    return 0;
}

/*
 * Run the debugger against a kernel dump.
 */
static int run_debugger(
    const char* debuggerPath,
    const char* dumpPath,
    const char* outputPath,
    int* timedOut,
    DWORD* exitCodeOut
)
{
    STARTUPINFOA startupInfo;
    PROCESS_INFORMATION processInfo;
    SECURITY_ATTRIBUTES securityAttributes;
    HANDLE outputFile;
    HANDLE nullInput;
    char commandLine[MAX_COMMAND_LENGTH];
    DWORD waitResult;

    *timedOut = 0;
    *exitCodeOut = 0;

    if (!is_safe_path(dumpPath))
    {
        return 0;
    }

    ZeroMemory(&startupInfo, sizeof(startupInfo));
    ZeroMemory(&processInfo, sizeof(processInfo));
    ZeroMemory(&securityAttributes, sizeof(securityAttributes));

    startupInfo.cb = sizeof(startupInfo);
    securityAttributes.nLength = sizeof(securityAttributes);
    securityAttributes.bInheritHandle = TRUE;

    outputFile = CreateFileA(
        outputPath,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &securityAttributes,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (outputFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }

    nullInput = CreateFileA(
        "NUL",
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        &securityAttributes,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (nullInput == INVALID_HANDLE_VALUE)
    {
        CloseHandle(outputFile);
        return 0;
    }

    startupInfo.dwFlags |= STARTF_USESTDHANDLES;
    startupInfo.hStdOutput = outputFile;
    startupInfo.hStdError = outputFile;
    startupInfo.hStdInput = nullInput;

    snprintf(
        commandLine,
        sizeof(commandLine),
        "\"%s\" -z \"%s\" -c \".echo CODEX_DUMP_TIME_BEGIN;.time -h 0;.echo CODEX_DUMP_TIME_END;.symfix;.reload;!analyze -v;lm;q\"",
        debuggerPath,
        dumpPath
    );

    if (!CreateProcessA(
        NULL,
        commandLine,
        NULL,
        NULL,
        TRUE,
        CREATE_NO_WINDOW,
        NULL,
        NULL,
        &startupInfo,
        &processInfo))
    {
        CloseHandle(nullInput);
        CloseHandle(outputFile);
        return 0;
    }

    waitResult = WaitForSingleObject(processInfo.hProcess, DEBUGGER_TIMEOUT_MS);

    if (waitResult == WAIT_TIMEOUT)
    {
        *timedOut = 1;
        TerminateProcess(processInfo.hProcess, 1);
        WaitForSingleObject(processInfo.hProcess, 5000);
    }
    else if (waitResult != WAIT_OBJECT_0)
    {
        TerminateProcess(processInfo.hProcess, 1);
        WaitForSingleObject(processInfo.hProcess, 5000);

        CloseHandle(processInfo.hThread);
        CloseHandle(processInfo.hProcess);
        CloseHandle(nullInput);
        CloseHandle(outputFile);
        return 0;
    }

    if (!GetExitCodeProcess(processInfo.hProcess, exitCodeOut))
    {
        *exitCodeOut = (DWORD)-1;
    }

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);
    CloseHandle(nullInput);
    CloseHandle(outputFile);

    return *timedOut ? 0 : 1;
}

/*
 * Remove leading whitespace from a string.
 */
static char* skip_whitespace(char* text)
{
    if (text == NULL)
    {
        return NULL;
    }

    while (*text == ' ' || *text == '\t')
    {
        text++;
    }

    return text;
}

/*
 * Check if a line starts with a specific field prefix.
 * Prevents collisions like "FAILURE_IMAGE_NAME:" matching "IMAGE_NAME:".
 */
static int starts_with(const char* text, const char* prefix)
{
    if (text == NULL || prefix == NULL)
    {
        return 0;
    }

    return strncmp(text, prefix, strlen(prefix)) == 0;
}

/*
 * Parse a hexadecimal value from WinDbg output.
 * Handles optional "0x" prefixes and 64-bit backtick separators (e.g. fffff800`7eeafff9).
 */
static int parse_windbg_hex(const char* text, unsigned long long* outValue)
{
    unsigned long long value = 0;
    int digits = 0;

    if (text == NULL || outValue == NULL)
    {
        return 0;
    }

    while (*text == ' ' || *text == '\t' || *text == ':')
    {
        text++;
    }

    if (text[0] == '0' && (text[1] == 'x' || text[1] == 'X'))
    {
        text += 2;
    }

    while (*text != '\0')
    {
        char c = *text;

        if (c == '`')
        {
            text++;
            continue;
        }

        if (c >= '0' && c <= '9')
        {
            value = (value << 4) | (unsigned long long)(c - '0');
            digits++;
        }
        else if (c >= 'a' && c <= 'f')
        {
            value = (value << 4) | (unsigned long long)(c - 'a' + 10);
            digits++;
        }
        else if (c >= 'A' && c <= 'F')
        {
            value = (value << 4) | (unsigned long long)(c - 'A' + 10);
            digits++;
        }
        else
        {
            break;
        }

        text++;
    }

    if (digits == 0)
    {
        return 0;
    }

    *outValue = value;
    return 1;
}

/*
 * Copy a string value after a debugger field name.
 */
static void copy_field_value(
    char* destination,
    size_t destinationSize,
    const char* line,
    const char* fieldName
)
{
    const char* valueStart;
    const char* valueEnd;
    size_t length;

    if (destination == NULL ||
        destinationSize == 0 ||
        line == NULL ||
        fieldName == NULL)
    {
        return;
    }

    if (!starts_with(line, fieldName))
    {
        return;
    }

    valueStart = line + strlen(fieldName);
    while (*valueStart == ' ' || *valueStart == '\t')
    {
        valueStart++;
    }

    valueEnd = valueStart + strlen(valueStart);
    while (valueEnd > valueStart &&
        (valueEnd[-1] == '\r' ||
            valueEnd[-1] == '\n' ||
            valueEnd[-1] == ' ' ||
            valueEnd[-1] == '\t'))
    {
        valueEnd--;
    }

    length = (size_t)(valueEnd - valueStart);
    if (length == 0)
    {
        return;
    }

    if (length >= destinationSize)
    {
        length = destinationSize - 1;
    }

    memcpy(destination, valueStart, length);
    destination[length] = '\0';
}

/*
 * Parse "Probably caused by : vgk.sys ( vgk+14fff9 )" as fallback
 * if IMAGE_NAME or SYMBOL_NAME was not separately emitted.
 */
static void parse_probably_caused_by(const char* trimmedLine, CrashAnalysis* analysis)
{
    const char* colon;
    const char* openParen;
    const char* closeParen;
    char imageBuf[256];
    char symbolBuf[256];
    size_t len;

    colon = strchr(trimmedLine, ':');
    if (colon == NULL)
    {
        return;
    }

    colon++;
    while (*colon == ' ' || *colon == '\t')
    {
        colon++;
    }

    openParen = strchr(colon, '(');
    if (openParen != NULL)
    {
        len = (size_t)(openParen - colon);
        while (len > 0 && (colon[len - 1] == ' ' || colon[len - 1] == '\t'))
        {
            len--;
        }

        if (len > 0 && analysis->imageName[0] == '\0')
        {
            if (len >= sizeof(imageBuf))
            {
                len = sizeof(imageBuf) - 1;
            }
            memcpy(imageBuf, colon, len);
            imageBuf[len] = '\0';
            snprintf(analysis->imageName, sizeof(analysis->imageName), "%s", imageBuf);
        }

        openParen++;
        while (*openParen == ' ' || *openParen == '\t')
        {
            openParen++;
        }

        closeParen = strchr(openParen, ')');
        if (closeParen != NULL)
        {
            len = (size_t)(closeParen - openParen);
            while (len > 0 && (openParen[len - 1] == ' ' || openParen[len - 1] == '\t'))
            {
                len--;
            }

            if (len > 0 && analysis->symbolName[0] == '\0')
            {
                if (len >= sizeof(symbolBuf))
                {
                    len = sizeof(symbolBuf) - 1;
                }
                memcpy(symbolBuf, openParen, len);
                symbolBuf[len] = '\0';
                snprintf(analysis->symbolName, sizeof(analysis->symbolName), "%s", symbolBuf);
            }
        }
    }
}

/*
 * Print important information extracted from debugger output
 * and populate structured crash analysis data.
 */
/* Only called inside the explicit ".time -h 0" output block. */
static int parse_dump_utc_time(const char* line, unsigned long long* ticks)
{
    char weekday[4], month[4];
    int day, hour, minute, year, consumed = 0;
    double second;
    const char* months[] = {"Jan","Feb","Mar","Apr","May","Jun",
                            "Jul","Aug","Sep","Oct","Nov","Dec"};
    if (sscanf(line, "Debug session time: %3s %3s %d %d:%d:%lf %d%n",
        weekday, month, &day, &hour, &minute, &second, &year, &consumed) != 7 ||
        consumed == 0 || year < 1601 || year > 9999 || day < 1 || day > 31 ||
        hour < 0 || hour > 23 || minute < 0 || minute > 59 ||
        !(second >= 0 && second < 60)) return 0;
    SYSTEMTIME st = {0}, checked;
    FILETIME ft;
    ULARGE_INTEGER value;
    for (int i=0; i<12; ++i)
        if (strcmp(month, months[i]) == 0) st.wMonth = (WORD)(i+1);
    if (!st.wMonth) return 0;
    st.wYear=(WORD)year; st.wDay=(WORD)day;
    st.wHour=(WORD)hour; st.wMinute=(WORD)minute;
    st.wSecond=(WORD)second;
    st.wMilliseconds=(WORD)((second - st.wSecond)*1000.0 + 0.0001);
    if (!SystemTimeToFileTime(&st, &ft) || !FileTimeToSystemTime(&ft, &checked) ||
        checked.wYear != st.wYear || checked.wMonth != st.wMonth ||
        checked.wDay != st.wDay) return 0;
    value.LowPart=ft.dwLowDateTime; value.HighPart=ft.dwHighDateTime;
    *ticks=value.QuadPart;
    return 1;
}

static void print_analysis(const char* outputPath, CrashAnalysis* currentAnalysis)
{
    FILE* file;
    char line[2048];
    int foundAnalysis = 0;
    int inDumpTime = 0;

    file = fopen(outputPath, "r");
    if (file == NULL)
    {
        printf("Could not read debugger output.\n");
        return;
    }

    ZeroMemory(currentAnalysis, sizeof(*currentAnalysis));

    printf("\n");
    printf("KERNEL DUMP DEBUGGER ANALYSIS\n");
    printf("-----------------------------\n");

    while (fgets(line, sizeof(line), file) != NULL)
    {
        char* trimmedLine = skip_whitespace(line);
        unsigned long long hexVal = 0;
        if (strcmp(trimmedLine, "CODEX_DUMP_TIME_BEGIN\n") == 0 ||
            strcmp(trimmedLine, "CODEX_DUMP_TIME_BEGIN\r\n") == 0) {
            inDumpTime = 1;
            continue;
        }
        if (strncmp(trimmedLine, "CODEX_DUMP_TIME_END", 19) == 0) {
            inDumpTime = 0;
            continue;
        }
        if (inDumpTime && parse_dump_utc_time(trimmedLine, &currentAnalysis->crashTime))
            printf("Dump crash time (UTC): %s", trimmedLine + strlen("Debug session time: "));

        /*
         * BUGCHECK_CODE: 1e
         */
        if (starts_with(trimmedLine, "BUGCHECK_CODE:"))
        {
            if (parse_windbg_hex(trimmedLine + strlen("BUGCHECK_CODE:"), &hexVal))
            {
                currentAnalysis->bugcheckCode = (unsigned int)hexVal;
            }
        }
        /*
         * Fallback: "BugCheck 1E, {ffffffffc0000094, fffff8007eeafff9, ...}"
         */
        else if (starts_with(trimmedLine, "BugCheck ") && currentAnalysis->bugcheckCode == 0)
        {
            if (parse_windbg_hex(trimmedLine + strlen("BugCheck "), &hexVal))
            {
                currentAnalysis->bugcheckCode = (unsigned int)hexVal;
            }
        }

        /*
         * BUGCHECK_P1 or Arg1
         */
        if (starts_with(trimmedLine, "BUGCHECK_P1:"))
        {
            if (parse_windbg_hex(trimmedLine + strlen("BUGCHECK_P1:"), &hexVal))
            {
                currentAnalysis->parameter1 = hexVal;
            }
        }
        else if (starts_with(trimmedLine, "Arg1:") && currentAnalysis->parameter1 == 0)
        {
            if (parse_windbg_hex(trimmedLine + strlen("Arg1:"), &hexVal))
            {
                currentAnalysis->parameter1 = hexVal;
            }
        }

        /*
         * BUGCHECK_P2 or Arg2
         */
        if (starts_with(trimmedLine, "BUGCHECK_P2:"))
        {
            if (parse_windbg_hex(trimmedLine + strlen("BUGCHECK_P2:"), &hexVal))
            {
                currentAnalysis->parameter2 = hexVal;
            }
        }
        else if (starts_with(trimmedLine, "Arg2:") && currentAnalysis->parameter2 == 0)
        {
            if (parse_windbg_hex(trimmedLine + strlen("Arg2:"), &hexVal))
            {
                currentAnalysis->parameter2 = hexVal;
            }
        }

        if (starts_with(trimmedLine, "BUGCHECK_P3:"))
        {
            if (parse_windbg_hex(trimmedLine + strlen("BUGCHECK_P3:"), &hexVal))
            {
                currentAnalysis->parameter3 = hexVal;
            }
        }
        else if (starts_with(trimmedLine, "Arg3:") && currentAnalysis->parameter3 == 0)
        {
            if (parse_windbg_hex(trimmedLine + strlen("Arg3:"), &hexVal))
            {
                currentAnalysis->parameter3 = hexVal;
            }
        }

        if (starts_with(trimmedLine, "BUGCHECK_P4:"))
        {
            if (parse_windbg_hex(trimmedLine + strlen("BUGCHECK_P4:"), &hexVal))
            {
                currentAnalysis->parameter4 = hexVal;
            }
        }
        else if (starts_with(trimmedLine, "Arg4:") && currentAnalysis->parameter4 == 0)
        {
            if (parse_windbg_hex(trimmedLine + strlen("Arg4:"), &hexVal))
            {
                currentAnalysis->parameter4 = hexVal;
            }
        }

        /*
         * Exact field matches using starts_with so BUCKET_ID_* or FAILURE_*
         * lines do not overwrite primary fields.
         */
        if (starts_with(trimmedLine, "PROCESS_NAME:"))
        {
            copy_field_value(
                currentAnalysis->processName,
                sizeof(currentAnalysis->processName),
                trimmedLine,
                "PROCESS_NAME:"
            );
        }
        else if (starts_with(trimmedLine, "MODULE_NAME:"))
        {
            copy_field_value(
                currentAnalysis->moduleName,
                sizeof(currentAnalysis->moduleName),
                trimmedLine,
                "MODULE_NAME:"
            );
        }
        else if (starts_with(trimmedLine, "IMAGE_NAME:"))
        {
            copy_field_value(
                currentAnalysis->imageName,
                sizeof(currentAnalysis->imageName),
                trimmedLine,
                "IMAGE_NAME:"
            );
        }
        else if (starts_with(trimmedLine, "SYMBOL_NAME:"))
        {
            copy_field_value(
                currentAnalysis->symbolName,
                sizeof(currentAnalysis->symbolName),
                trimmedLine,
                "SYMBOL_NAME:"
            );
        }
        else if (starts_with(trimmedLine, "Probably caused by"))
        {
            parse_probably_caused_by(trimmedLine, currentAnalysis);
        }

        /*
         * Print important WinDbg analysis lines.
         */
        if (strstr(trimmedLine, "BugCheck") != NULL ||
            strstr(trimmedLine, "BUGCHECK") != NULL ||
            starts_with(trimmedLine, "MODULE_NAME:") ||
            starts_with(trimmedLine, "IMAGE_NAME:") ||
            starts_with(trimmedLine, "Probably caused by") ||
            starts_with(trimmedLine, "FAULTING_IP:") ||
            starts_with(trimmedLine, "PROCESS_NAME:") ||
            starts_with(trimmedLine, "STACK_TEXT:") ||
            starts_with(trimmedLine, "EXCEPTION_CODE:") ||
            starts_with(trimmedLine, "Failure.Bucket") ||
            starts_with(trimmedLine, "FAILURE_BUCKET_ID:") ||
            starts_with(trimmedLine, "SYMBOL_NAME:"))
        {
            printf("%s", line);
            foundAnalysis = 1;
        }
    }

    fclose(file);

    /*
     * Derive moduleName from imageName (e.g., "vgk.sys" -> "vgk") if missing.
     */
    if (currentAnalysis->moduleName[0] == '\0' &&
        currentAnalysis->imageName[0] != '\0')
    {
        char* dot;
        strncpy(
            currentAnalysis->moduleName,
            currentAnalysis->imageName,
            sizeof(currentAnalysis->moduleName) - 1
        );
        currentAnalysis->moduleName[sizeof(currentAnalysis->moduleName) - 1] = '\0';
        dot = strrchr(currentAnalysis->moduleName, '.');
        if (dot != NULL)
        {
            *dot = '\0';
        }
    }

    if (currentAnalysis->bugcheckCode == 0x1E)
    {
        currentAnalysis->exceptionCode = currentAnalysis->parameter1 & 0xFFFFFFFFULL;
        currentAnalysis->faultAddress = currentAnalysis->parameter2;
    }

    /*
     * Mark analysis valid when BugCheck code was extracted.
     */
    if (currentAnalysis->bugcheckCode != 0)
    {
        currentAnalysis->valid = 1;
    }

    if (!foundAnalysis)
    {
        printf("Debugger did not return recognizable analysis fields.\n");
    }
}

/*
 * Analyze a Windows kernel crash dump.
 */
int analyze_kernel_dump_file(const char* dumpPath)
{
    char debuggerPath[MAX_PATH_LENGTH];
    char outputPath[MAX_PATH_LENGTH];
    char tempDirectory[MAX_PATH_LENGTH];
    int timedOut = 0;
    DWORD exitCode = 0;
    CrashAnalysis currentAnalysis;

    ZeroMemory(&currentAnalysis, sizeof(currentAnalysis));

    printf("\n");
    printf("KERNEL DUMP ANALYZER\n");
    printf("--------------------\n");

    if (dumpPath == NULL)
    {
        printf("Dump path is invalid.\n");
        return 0;
    }

    printf("Dump File        : %s\n", dumpPath);

    if (!is_safe_path(dumpPath))
    {
        printf(
            "Dump path contains characters that are not\n"
            "permitted. Refusing to proceed for safety.\n"
        );
        return 0;
    }

    if (!file_exists(dumpPath))
    {
        printf("Dump file does not exist or is not accessible.\n");
        return 0;
    }

    if (!find_debugger(debuggerPath, sizeof(debuggerPath)))
    {
        printf("Debugger         : NOT FOUND\n");
        printf("Status           : DEBUGGER REQUIRED\n\n");
        printf(
            "Install Debugging Tools for Windows (cdb.exe / kd.exe)\n"
            "or WinDbg to enable kernel dump analysis.\n"
        );
        return 0;
    }

    printf("Debugger         : %s\n", debuggerPath);

    if (GetTempPathA(sizeof(tempDirectory), tempDirectory) == 0)
    {
        printf("Could not determine temporary directory.\n");
        return 0;
    }

    if (GetTempFileNameA(tempDirectory, "dmp", 0, outputPath) == 0)
    {
        printf("Could not create a unique temporary output file.\n");
        return 0;
    }

    if (!run_debugger(debuggerPath, dumpPath, outputPath, &timedOut, &exitCode))
    {
        if (timedOut)
        {
            printf(
                "Debugger did not finish within %d seconds and was\n"
                "terminated.\n"
                "This may happen if symbol loading is slow or blocked.\n",
                DEBUGGER_TIMEOUT_MS / 1000
            );
        }
        else
        {
            printf(
                "Could not start debugger, or the dump path was\n"
                "rejected for safety.\n"
            );
        }

        DeleteFileA(outputPath);
        return 0;
    }

    printf("Debugger Status  : COMPLETED (exit code %lu)\n", (unsigned long)exitCode);

    if (exitCode != 0)
    {
        printf(
            "Note             : Debugger returned a non-zero exit\n"
            "                   code; analysis may still be useful.\n"
        );
    }

    print_analysis(outputPath, &currentAnalysis);

    if (currentAnalysis.valid)
    {
        store_latest_crash_analysis(dumpPath, &currentAnalysis);
    }

    printf("\n");
    printf("Raw Analysis     : temporary output deleted after use.\n");

    DeleteFileA(outputPath);
    return currentAnalysis.valid ? 1 : 0;
}

void reset_crash_analysis(void)
{
    ZeroMemory(&lastCrashAnalysis, sizeof(lastCrashAnalysis));
}

/*
 * Return the newest structured crash analysis found during this scan.
 */
int get_last_crash_analysis(CrashAnalysis* analysis)
{
    if (analysis == NULL)
    {
        return 0;
    }

    if (!lastCrashAnalysis.valid)
    {
        return 0;
    }

    *analysis = lastCrashAnalysis;
    return 1;
}
