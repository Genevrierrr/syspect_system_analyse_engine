#ifndef DUMP_ANALYZER_H
#define DUMP_ANALYZER_H

#define CRASH_DUMP_PATH_LENGTH 1024

typedef struct
{
    int valid;
    unsigned int bugcheckCode;
    unsigned long long parameter1;
    unsigned long long parameter2;
    unsigned long long parameter3;
    unsigned long long parameter4;
    unsigned long long exceptionCode;
    unsigned long long faultAddress;
    unsigned long long dumpWriteTime;
    unsigned long long crashTime; /* UTC FILETIME from dump, zero if unavailable. */
    char dumpPath[CRASH_DUMP_PATH_LENGTH];
    char processName[256];
    char moduleName[256];
    char imageName[256];
    char symbolName[256];
} CrashAnalysis;

int analyze_kernel_dump_file(const char* dumpPath);
int get_last_crash_analysis(CrashAnalysis* analysis);
void reset_crash_analysis(void);

#endif
