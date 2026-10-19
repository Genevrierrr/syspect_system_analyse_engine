#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <winevt.h>
#include <wchar.h>

#include "logs.h"


static const char* get_level_name(DWORD level)
{
    switch (level)
    {
    case 1:
        return "Critical";
    case 2:
        return "Error";
    case 3:
        return "Warning";
    case 4:
        return "Information";
    case 5:
        return "Verbose";
    default:
        return "Unknown";
    }
}

static void print_utf8(const char* label, const wchar_t* value)
{
    int requiredSize;
    char* utf8Buffer;

    if (value == NULL)
    {
        printf("%sUnavailable\n", label);
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
        printf("%sUnavailable\n", label);
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
        printf("%sUnavailable\n", label);
    }

    free(utf8Buffer);
}

static void print_event_timestamp(ULONGLONG ullFileTime)
{
    FILETIME ft;
    FILETIME localFt;
    SYSTEMTIME st;

    ft.dwLowDateTime = (DWORD)(ullFileTime & 0xFFFFFFFFULL);
    ft.dwHighDateTime = (DWORD)(ullFileTime >> 32);

    if (FileTimeToLocalFileTime(&ft, &localFt) &&
        FileTimeToSystemTime(&localFt, &st))
    {
        printf(
            "     Time      : %02d/%02d/%04d %02d:%02d:%02d\n",
            st.wDay,
            st.wMonth,
            st.wYear,
            st.wHour,
            st.wMinute,
            st.wSecond
        );
    }
    else
    {
        printf("     Time      : Unknown\n");
    }
}

static void print_event_message(
    EVT_HANDLE event,
    const wchar_t* providerName
)
{
    EVT_HANDLE publisherMetadata = NULL;
    DWORD bufferUsed = 0;
    DWORD bufferSize = 0;
    wchar_t* messageBuffer = NULL;

    publisherMetadata = EvtOpenPublisherMetadata(
        NULL,
        providerName,
        NULL,
        0,
        0
    );

    if (publisherMetadata == NULL)
    {
        printf("     Message   : Unavailable\n");
        return;
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
        DWORD error = GetLastError();

        if (error != ERROR_INSUFFICIENT_BUFFER)
        {
            printf("     Message   : Unavailable\n");
            EvtClose(publisherMetadata);
            return;
        }
    }

    if (bufferUsed == 0)
    {
        printf("     Message   : Unavailable\n");
        EvtClose(publisherMetadata);
        return;
    }

    bufferSize = bufferUsed;

    messageBuffer = (wchar_t*)malloc(
        (size_t)bufferSize * sizeof(wchar_t)
    );

    if (messageBuffer == NULL)
    {
        printf("     Message   : Memory allocation failed\n");
        EvtClose(publisherMetadata);
        return;
    }

    if (EvtFormatMessage(
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
        print_utf8(
            "     Message   : ",
            messageBuffer
        );
    }
    else
    {
        printf("     Message   : Unavailable\n");
    }

    free(messageBuffer);
    EvtClose(publisherMetadata);
}

static void print_event_information(EVT_HANDLE event)
{
    EVT_HANDLE context;
    EVT_VARIANT* values = NULL;

    DWORD valueCount = 0;
    DWORD bufferSize = 0;
    DWORD bufferUsed = 0;

    LPCWSTR properties[] =
    {
        L"Event/System/Provider/@Name",
        L"Event/System/EventID",
        L"Event/System/Level",
        L"Event/System/TimeCreated/@SystemTime"
    };

    context = EvtCreateRenderContext(
        4,
        properties,
        EvtRenderContextValues
    );

    if (context == NULL)
    {
        printf("     Could not create render context.\n");
        return;
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
        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
        {
            printf("     Could not read event values.\n");
            EvtClose(context);
            return;
        }
    }

    bufferSize = bufferUsed;
    values = (EVT_VARIANT*)malloc(bufferSize);

    if (values == NULL)
    {
        printf("     Memory allocation failed.\n");
        EvtClose(context);
        return;
    }

    if (!EvtRender(
        context,
        event,
        EvtRenderEventValues,
        bufferSize,
        values,
        &bufferUsed,
        &valueCount))
    {
        printf("     Could not render event.\n");

        free(values);
        EvtClose(context);
        return;
    }

    printf("     Event detected\n");

    if (valueCount > 0 &&
        values[0].Type == EvtVarTypeString &&
        values[0].StringVal != NULL)
    {
        print_utf8(
            "     Provider  : ",
            values[0].StringVal
        );
    }

    DWORD eventId = 0;

    if (valueCount > 1)
    {
        if (values[1].Type == EvtVarTypeUInt16)
        {
            eventId = values[1].UInt16Val;
        }
        else if (values[1].Type == EvtVarTypeUInt32)
        {
            eventId = values[1].UInt32Val;
        }

        printf(
            "     Event ID  : %lu\n",
            eventId
        );
    }

    DWORD level = 0;

    if (valueCount > 2)
    {
        if (values[2].Type == EvtVarTypeByte)
        {
            level = values[2].ByteVal;
        }
        else if (values[2].Type == EvtVarTypeUInt16)
        {
            level = values[2].UInt16Val;
        }
        else if (values[2].Type == EvtVarTypeUInt32)
        {
            level = values[2].UInt32Val;
        }

        printf(
            "     Level     : %s (%lu)\n",
            get_level_name(level),
            level
        );
    }

    if (valueCount > 3)
    {
        if (values[3].Type == EvtVarTypeFileTime)
        {
            print_event_timestamp(
                values[3].FileTimeVal
            );
        }
        else if (
            values[3].Type == EvtVarTypeString &&
            values[3].StringVal != NULL)
        {
            print_utf8(
                "     Time      : ",
                values[3].StringVal
            );
        }
    }

    if (valueCount > 0 &&
        values[0].Type == EvtVarTypeString &&
        values[0].StringVal != NULL)
    {
        print_event_message(
            event,
            values[0].StringVal
        );
    }
    else
    {
        printf("     Message   : Unavailable\n");
    }

    free(values);
    EvtClose(context);
}

static void scan_log_channel(
    const char* channel,
    const char* query
)
{
    EVT_HANDLE results;
    EVT_HANDLE event;

    DWORD returned = 0;

    wchar_t wideChannel[256];
    wchar_t wideQuery[1024];

    MultiByteToWideChar(
        CP_UTF8,
        0,
        channel,
        -1,
        wideChannel,
        256
    );

    MultiByteToWideChar(
        CP_UTF8,
        0,
        query,
        -1,
        wideQuery,
        1024
    );

    results = EvtQuery(
        NULL,
        wideChannel,
        wideQuery,
        EvtQueryChannelPath |
        EvtQueryReverseDirection
    );

    if (results == NULL)
    {
        printf(
            "     Channel unavailable: %s\n",
            channel
        );

        return;
    }

    int count = 0;

    while (count < 10)
    {
        event = NULL;

        if (!EvtNext(
            results,
            1,
            &event,
            1000,
            0,
            &returned))
        {
            break;
        }

        if (event != NULL)
        {
            print_event_information(event);

            printf("\n");

            EvtClose(event);

            count++;
        }
    }

    printf(
        "     Events scanned: %d\n",
        count
    );

    EvtClose(results);
}

void scan_logs(void)
{
    printf("\n");
    printf("WINDOWS EVENT LOGS\n");
    printf("--------------------\n");

    printf("\n[WHEA-Logger]\n");

    scan_log_channel(
        "System",
        "*[System[Provider[@Name='WHEA-Logger']]]"
    );

    printf("\n[Kernel-Power]\n");

    scan_log_channel(
        "System",
        "*[System[Provider[@Name='Microsoft-Windows-Kernel-Power']]]"
    );

    printf("\n[BugCheck / WER]\n");

    scan_log_channel(
        "System",
        "*[System[Provider[@Name='Microsoft-Windows-WER-SystemErrorReporting'] and EventID=1001]]"
    );

    printf("\n[Display]\n");

    scan_log_channel(
        "System",
        "*[System[Provider[@Name='Display']]]"
    );

    printf("\n[Kernel-PnP]\n");

    scan_log_channel(
        "System",
        "*[System[Provider[@Name='Microsoft-Windows-Kernel-PnP']]]"
    );

    printf("\n[Disk]\n");

    scan_log_channel(
        "System",
        "*[System[Provider[@Name='Disk']]]"
    );

    printf("\n[Ntfs]\n");

    scan_log_channel(
        "System",
        "*[System[Provider[@Name='Ntfs']]]"
    );
}
