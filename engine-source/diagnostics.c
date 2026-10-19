#include <stdio.h>
#include "report.h"
#include <windows.h>
#include <winevt.h>
#include <wchar.h>
#include <stdlib.h>

#include "diagnostics.h"
#include "correlation.h"
#include "dump_analyzer.h"

/*
 * Count matching events in the Windows Event Log.
 */
static int count_events(const wchar_t* query)
{
    EVT_HANDLE results;
    EVT_HANDLE event = NULL;
    DWORD returned = 0;
    int count = 0;

    results = EvtQuery(
        NULL,
        L"System",
        query,
        EvtQueryChannelPath
    );

    if (results == NULL)
    {
        return -1; /* Unknown, never report a query failure as zero events. */
    }

    while (EvtNext(
        results,
        1,
        &event,
        1000,
        0,
        &returned))
    {
        if (event != NULL)
        {
            count++;
            EvtClose(event);
            event = NULL;
        }
    }

    DWORD error = GetLastError();
    EvtClose(results);
    return error == ERROR_NO_MORE_ITEMS ? count : -1;
}

/*
 * Format an Event Log event message.
 */
static wchar_t* get_event_message(EVT_HANDLE event)
{
    EVT_HANDLE publisherMetadata = NULL;
    DWORD bufferUsed = 0;
    DWORD bufferSize = 0;
    wchar_t* message = NULL;

    publisherMetadata = EvtOpenPublisherMetadata(
        NULL,
        L"Microsoft-Windows-WER-SystemErrorReporting",
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
    message = (wchar_t*)malloc(bufferSize * sizeof(wchar_t));

    if (message == NULL)
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
        message,
        &bufferUsed))
    {
        free(message);
        EvtClose(publisherMetadata);
        return NULL;
    }

    EvtClose(publisherMetadata);
    return message;
}

/*
 * Parse a hexadecimal value from text.
 */
static unsigned long long parse_hex_value(const wchar_t* text)
{
    unsigned long long value = 0;

    if (text == NULL)
    {
        return 0;
    }

    while (*text == L' ' || *text == L'\t')
    {
        text++;
    }

    if (text[0] == L'0' && (text[1] == L'x' || text[1] == L'X'))
    {
        text += 2;
    }

    while (*text != L'\0')
    {
        int digit;

        if (*text >= L'0' && *text <= L'9')
        {
            digit = *text - L'0';
        }
        else if (*text >= L'a' && *text <= L'f')
        {
            digit = *text - L'a' + 10;
        }
        else if (*text >= L'A' && *text <= L'F')
        {
            digit = *text - L'A' + 10;
        }
        else
        {
            break;
        }

        value = (value << 4) | (unsigned long long)digit;
        text++;
    }

    return value;
}

/*
 * Extract the next hexadecimal value from a message.
 */
static int extract_next_hex(
    const wchar_t** cursor,
    unsigned long long* value
)
{
    const wchar_t* p;

    if (cursor == NULL || *cursor == NULL || value == NULL)
    {
        return 0;
    }

    p = *cursor;

    while (*p != L'\0')
    {
        if (*p == L'0' && (p[1] == L'x' || p[1] == L'X'))
        {
            const wchar_t* start = p;
            p += 2;

            if (!((*p >= L'0' && *p <= L'9') ||
                (*p >= L'a' && *p <= L'f') ||
                (*p >= L'A' && *p <= L'F')))
            {
                /* Do not increment p here; p += 2 already advanced past "0x" */
                continue;
            }

            while ((*p >= L'0' && *p <= L'9') ||
                (*p >= L'a' && *p <= L'f') ||
                (*p >= L'A' && *p <= L'F'))
            {
                p++;
            }

            *value = parse_hex_value(start);
            *cursor = p;
            return 1;
        }

        p++;
    }

    *cursor = p;
    return 0;
}

/*
 * Read and analyze BugCheck Event ID 1001.
 *
 * The parser intentionally reads hexadecimal values
 * from the formatted event message instead of relying
 * on localized field names.
 *
 * NOTE: The meaning of values[1] (Parameter 1) and
 * values[2] (Parameter 2) as "Exception code" and
 * "Fault address" is specific to BugCheck 0x1E
 * (KMODE_EXCEPTION_NOT_HANDLED). For any other
 * BugCheck code that interpretation does not hold,
 * so those specific labels are only printed when
 * bugcheck == 0x1E.
 */
static int get_bugcheck_evidence(void)
{
    EVT_HANDLE results;
    EVT_HANDLE event = NULL;
    DWORD returned = 0;
    const wchar_t* cursor;
    unsigned long long values[5];
    int valueCount = 0;
    wchar_t* message = NULL;
    int success = 0;

    results = EvtQuery(
        NULL,
        L"System",
        L"*[System[Provider[@Name='Microsoft-Windows-WER-SystemErrorReporting'] and EventID=1001]]",
        EvtQueryChannelPath | EvtQueryReverseDirection
    );

    if (results == NULL)
    {
        return -1; /* Unknown, never report a query failure as zero events. */
    }

    if (!EvtNext(
        results,
        1,
        &event,
        1000,
        0,
        &returned))
    {
        EvtClose(results);
        return 0;
    }

    message = get_event_message(event);
    if (message == NULL)
    {
        EvtClose(event);
        EvtClose(results);
        return 0;
    }

    cursor = message;

    /*
     * Expected values:
     * 0x0000001e
     * 0xffffffffc0000094
     * 0xfffff8007eeafff9
     * 0x0000000000000001
     * 0x00000000d3c970a0
     */
    while (valueCount < 5 &&
        extract_next_hex(&cursor, &values[valueCount]))
    {
        valueCount++;
    }

    printf("\n");
    printf("[BUGCHECK DETAILS]\n");
    printf("--------------------\n");

    if (valueCount >= 5)
    {
        unsigned long long bugcheck = values[0];

        /*
         * Windows commonly reports Parameter 1 for 0x1E
         * as a sign-extended NTSTATUS (e.g.
         * 0xffffffffc0000094), so mask down to the low
         * 32 bits before comparing against a bare
         * NTSTATUS constant.
         */
        unsigned long long exceptionCode = values[1] & 0xFFFFFFFFULL;

        printf("BugCheck Code   : 0x%08llX\n", bugcheck);

        if (bugcheck == 0x1E)
        {
            printf("BugCheck Name   : KMODE_EXCEPTION_NOT_HANDLED\n");
        }
        else
        {
            printf("BugCheck Name   : Unknown / Not mapped\n");
        }

        printf("Parameter 1     : 0x%016llX\n", values[1]);
        printf("Parameter 2     : 0x%016llX\n", values[2]);
        printf("Parameter 3     : 0x%016llX\n", values[3]);
        printf("Parameter 4     : 0x%016llX\n", values[4]);

        /*
         * Parameter 1 / Parameter 2 are only known to mean
         * "exception code" / "fault address" for BugCheck
         * 0x1E. Do not print those labels for other codes,
         * since the interpretation would be misleading.
         */
        if (bugcheck == 0x1E)
        {
            if (exceptionCode == 0xC0000094ULL)
            {
                printf("Exception       : INTEGER_DIVIDE_BY_ZERO\n");
            }
            else
            {
                printf("Exception       : 0x%016llX\n", values[1]);
            }

            printf("Fault Address   : 0x%016llX\n", values[2]);
        }

        printf("\n");
        printf(
            "Assessment      : Windows recorded a kernel-mode\n"
            "                  exception that caused a system crash.\n"
        );

        if (bugcheck == 0x1E)
        {
            printf(
                "Analysis        : The crash is classified as\n"
                "                  KMODE_EXCEPTION_NOT_HANDLED.\n"
            );

            if (exceptionCode == 0xC0000094ULL)
            {
                printf(
                    "Exception Info  : The exception code indicates\n"
                    "                  an integer divide-by-zero operation.\n"
                );
            }
        }

        printf(
            "Next Step       : Correlate the fault address with\n"
            "                  loaded kernel modules and drivers.\n"
        );

        success = 1;
    }
    else
    {
        printf("Could not parse BugCheck parameters.\n");
    }

    free(message);
    EvtClose(event);
    EvtClose(results);

    return success;
}

/*
 * Main diagnostics engine.
 */
void analyze_diagnostics(void)
{
    int findings = 0;
    int wheaCount;
    int kernelPowerCount;
    int bugCheckCount;
    int kernelPnpCount;
    int displayCount;
    int diskCount;
    int ntfsCount;
    CrashAnalysis crashAnalysis;

    ZeroMemory(&crashAnalysis, sizeof(crashAnalysis));

    printf("\n");
    printf("DIAGNOSTICS\n");
    printf("--------------------\n");
    printf("Analyzing collected diagnostic evidence...\n");

    /*
     * Retrieve structured crash information produced
     * by the BSOD / dump analyzer.
     */
    if (get_last_crash_analysis(&crashAnalysis))
    {
        printf("\n");
        printf("[HIGH] Kernel crash module identified\n");
        printf("--------------------\n");
        if (crashAnalysis.dumpPath[0] != '\0')
        {
            printf("Dump File       : %s\n", crashAnalysis.dumpPath);
        }

        printf("BugCheck        : 0x%08X\n", crashAnalysis.bugcheckCode);
        printf("Parameter 1     : 0x%016llX\n", crashAnalysis.parameter1);
        printf("Parameter 2     : 0x%016llX\n", crashAnalysis.parameter2);
        printf("Parameter 3     : 0x%016llX\n", crashAnalysis.parameter3);
        printf("Parameter 4     : 0x%016llX\n", crashAnalysis.parameter4);

        if (crashAnalysis.bugcheckCode == 0x1E)
        {
            printf("Exception       : 0x%016llX\n", crashAnalysis.exceptionCode);
            printf("Fault Address   : 0x%016llX\n", crashAnalysis.faultAddress);
        }

        if (crashAnalysis.processName[0] != '\0')
        {
            printf("Process         : %s\n", crashAnalysis.processName);
        }
        else
        {
            printf("Process         : Unknown\n");
        }

        if (crashAnalysis.moduleName[0] != '\0')
        {
            printf("Faulting Module : %s\n", crashAnalysis.moduleName);
        }
        else
        {
            printf("Faulting Module : Unknown\n");
        }

        if (crashAnalysis.imageName[0] != '\0')
        {
            printf("Image           : %s\n", crashAnalysis.imageName);
        }
        else
        {
            printf("Image           : Unknown\n");
        }

        if (crashAnalysis.symbolName[0] != '\0')
        {
            printf("Symbol          : %s\n", crashAnalysis.symbolName);
        }
        else
        {
            printf("Symbol          : Unknown\n");
        }

        printf("\n");
        printf(
            "Assessment      : The crash dump identifies the\n"
            "                  reported module as the faulting module.\n"
        );
        printf(
            "Important       : Faulting module identification does not\n"
            "                  by itself prove the ultimate root cause.\n"
        );
        printf("Confidence      : HIGH\n");

        findings++;
    }

    /*
     * WHEA
     */
    printf("\nEVENT HISTORY: retained records, not restricted to the analyzed crash.\n");
    printf("Use CRASH TIME WINDOW below for time-specific evidence.\n");

    wheaCount = count_events(
        L"*[System[Provider[@Name='WHEA-Logger']]]"
    );

    printf("\n");
    printf("[WHEA HARDWARE ERRORS]\n");
    printf("Events found : %d\n", wheaCount);

    if (wheaCount > 0)
    {
        printf("Status       : DETECTED\n");
        findings++;
    }
    else if (wheaCount < 0)
    {
        printf("Status       : UNAVAILABLE (event query failed)\n");
    }
    else
    {
        printf("Status       : NOT DETECTED\n");
    }

    /*
     * Kernel-Power 41
     */
    kernelPowerCount = count_events(
        L"*[System[Provider[@Name='Microsoft-Windows-Kernel-Power'] and EventID=41]]"
    );

    if (kernelPowerCount > 0)
    {
        printf("\n");
        printf("[HIGH] Unexpected shutdown or restart detected\n");
        printf("Evidence       : %d Kernel-Power 41 event(s)\n", kernelPowerCount);
        printf("Assessment     : Windows recorded one or more unexpected shutdown/restart events.\n");
        printf("Possible Cause : The event itself does not identify the root cause. Power loss, forced restart, system crash, hardware instability, or other conditions may produce this event.\n");
        printf("Confidence     : MEDIUM\n");
        printf("Recommendation : Correlate the event timestamp with BugCheck, WHEA, driver, thermal, and power-related evidence.\n");

        findings++;
    }

    /*
     * BugCheck 1001
     */
    bugCheckCount = count_events(
        L"*[System[Provider[@Name='Microsoft-Windows-WER-SystemErrorReporting'] and EventID=1001]]"
    );

    if (bugCheckCount > 0)
    {
        printf("\n");
        printf("[HIGH] System crash / BugCheck evidence detected\n");
        printf("Evidence       : %d BugCheck event(s)\n", bugCheckCount);
        printf("Assessment     : Windows recorded BugCheck-related events.\n");
        printf("Possible Cause : A system crash occurred. The actual cause requires BugCheck parameters, dump analysis, and driver correlation.\n");
        printf("Confidence     : HIGH\n");
        printf("Recommendation : Analyze the BugCheck details and correlate the crash timestamp with drivers, devices, and hardware events.\n");

        get_bugcheck_evidence();
        findings++;
    }

    /*
     * Display 4101
     */
    displayCount = count_events(
        L"*[System[Provider[@Name='Display'] and EventID=4101]]"
    );

    if (displayCount > 0)
    {
        printf("\n");
        printf("[MEDIUM] Display driver recovery events detected\n");
        printf("Evidence       : %d Display 4101 event(s)\n", displayCount);
        printf("Assessment     : Windows recorded display driver recovery events.\n");
        printf("Possible Cause : A graphics driver or GPU-related operation may have stopped responding temporarily.\n");
        printf("Confidence     : MEDIUM\n");
        printf("Recommendation : Correlate display events with GPU driver version, GPU load, temperature, clocks, and crash timestamps.\n");

        findings++;
    }

    /*
     * Kernel-PnP 219
     */
    kernelPnpCount = count_events(
        L"*[System[Provider[@Name='Microsoft-Windows-Kernel-PnP'] and EventID=219]]"
    );

    if (kernelPnpCount > 0)
    {
        printf("\n");
        printf("[MEDIUM] Repeated device driver initialization events detected\n");
        printf("Evidence       : %d Kernel-PnP 219 event(s)\n", kernelPnpCount);
        printf("Assessment     : Windows recorded Kernel-PnP 219 driver initialization events.\n");
        printf("Possible Cause : Possible driver initialization, power-management, device startup, or dependency-related issue.\n");
        printf("Confidence     : MEDIUM\n");
        printf("Recommendation : Use the Correlation Engine to identify the affected device and installed driver before changing or reinstalling drivers.\n");

        findings++;
    }

    /*
     * Disk errors
     */
    diskCount = count_events(
        L"*[System[Provider[@Name='Disk'] and (EventID=7 or EventID=51 or EventID=153)]]"
    );

    if (diskCount > 0)
    {
        printf("\n");
        printf("[HIGH] Disk error events detected\n");
        printf("Evidence       : %d Disk error event(s)\n", diskCount);
        printf("Assessment     : Windows recorded storage-related errors.\n");
        printf("Recommendation : Correlate the affected disk/device with storage controller and driver information.\n");

        findings++;
    }

    /*
     * NTFS errors
     */
    ntfsCount = count_events(
        L"*[System[Provider[@Name='Ntfs'] and (EventID=55 or EventID=57)]]"
    );

    if (ntfsCount > 0)
    {
        printf("\n");
        printf("[HIGH] NTFS file-system errors detected\n");
        printf("Evidence       : %d NTFS error(s)\n", ntfsCount);
        printf("Assessment     : Windows recorded file-system consistency or I/O-related errors.\n");
        printf("Recommendation : Correlate the event timestamp with disk, storage controller, and system crash evidence.\n");

        findings++;
    }

    report.events[0] = wheaCount;
    report.events[1] = kernelPowerCount;
    report.events[2] = bugCheckCount;
    report.events[3] = kernelPnpCount;
    report.events[4] = displayCount;
    report.events[5] = diskCount;
    report.events[6] = ntfsCount;

    /*
     * Diagnostic summary
     */
    printf("\n");
    printf("DIAGNOSTIC SUMMARY\n");
    printf("--------------------\n");
    printf("Diagnostic findings detected: %d\n", findings);
    printf("\n");
    printf("Important: Event presence is evidence, not automatic proof of root cause.\n");
    printf("Further correlation is required before assigning a definitive cause.\n");

    /*
     * Run correlation when Kernel-PnP evidence exists.
     */
    if (kernelPnpCount > 0)
    {
        analyze_correlations();
    }

    printf("\n");
    printf("Diagnostic analysis completed.\n");
}
