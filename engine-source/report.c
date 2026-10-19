#include <stdio.h>
#include <windows.h>
#include "report.h"
#include "dump_analyzer.h"

DiagnosticReport report = {NULL, "", 0, 0, 0, -1, -1, -1,
                          {-1,-1,-1,-1,-1,-1,-1}, "unavailable"};

static void json_char(unsigned char c) {
    if (c == '"' || c == '\\') { putchar('\\'); putchar(c); }
    else if (c < 32) printf("\\u%04x", c);
    else putchar(c);
}
static void string(const char *s) {
    if (!s || !*s) { printf("null"); return; }
    putchar('"');
    while (*s) json_char((unsigned char)*s++);
    putchar('"');
}
static void number(long n) {
    if (n < 0) printf("null"); else printf("%ld", n);
}
void write_json_report(FILE *log) {
    static const char *keys[] = {"whea", "kernel_power", "bugcheck", "kernel_pnp", "display", "disk", "ntfs"};
    CrashAnalysis crash;
    printf("{\"schema_version\":\"1.0\",\"engine\":{\"name\":\"PC Diagnostic Engine\",\"mode\":\"native\"},\"system\":{\"architecture\":");
    string(report.architecture);
    printf(",\"cpu\":"); string(report.cpu);
    printf(",\"logical_processors\":%lu,\"ram_gb\":", report.logical_processors);
    if (report.ram_gb > 0) printf("%.2f", report.ram_gb); else printf("null");
    printf(",\"windows_build\":");
    if (report.windows_build) printf("%lu", report.windows_build); else printf("null");
    printf(",\"gpu\":null},\"hardware\":{\"devices\":"); number(report.devices);
    printf(",\"problematic_devices\":"); number(report.problematic_devices);
    printf("},\"drivers\":{\"detected\":"); number(report.drivers);
    printf(",\"highlights\":null},\"events\":{\"scope\":\"retained_matching_history\"");
    for (int i=0; i<7; ++i) { printf(",\"%s\":", keys[i]); number(report.events[i]); }
    printf("},\"crash\":");
    if (get_last_crash_analysis(&crash)) {
        printf("{\"bugcheck\":\"0x%08X\",\"exception_code\":\"0x%016llX\",\"fault_address\":\"0x%016llX\",\"module\":", crash.bugcheckCode, crash.exceptionCode, crash.faultAddress);
        string(crash.moduleName); printf(",\"process\":"); string(crash.processName);
        printf(",\"symbol\":"); string(crash.symbolName);
        printf(",\"dump_path\":"); string(crash.dumpPath);
        printf(",\"time\":");
        FILETIME ft = {(DWORD)crash.crashTime, (DWORD)(crash.crashTime >> 32)};
        SYSTEMTIME st;
        if (crash.crashTime && FileTimeToSystemTime(&ft, &st))
            printf("\"%04u-%02u-%02uT%02u:%02u:%02u.%03uZ\"", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
        else printf("null");
        printf("}");
    } else printf("null");
    printf(",\"status\":\"partial\",\"findings\":null,\"timeline\":null,\"coverage\":{\"dump_status\":"); string(report.dump_status);
    printf(",\"findings\":\"text_report_only\",\"timeline\":\"text_report_only\",\"driver_details\":\"text_report_only\",\"gpu\":\"not_collected\"},\"raw_report\":\"");
    rewind(log);
    int c;
    while ((c=fgetc(log)) != EOF) json_char((unsigned char)c);
    printf("\"}\n");
}
