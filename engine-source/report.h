#ifndef SYSPECT_REPORT_H
#define SYSPECT_REPORT_H
#include <stdio.h>
typedef struct {
    const char *architecture;
    char cpu[256];
    unsigned long logical_processors, windows_build;
    double ram_gb;
    long devices, problematic_devices, drivers;
    int events[7];
    const char *dump_status;
} DiagnosticReport;
extern DiagnosticReport report;
void write_json_report(FILE *log);
#endif
