#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <io.h>
#include "report.h"

#include "system.h"
#include "hardware.h"
#include "drivers.h"
#include "logs.h"
#include "diagnostics.h"
#include "bsod.h"

int main(int argc, char **argv)
{
    int json = argc == 2 && strcmp(argv[1], "--json") == 0;
    if (argc > 1 && !json) { fprintf(stderr, "Usage: diagnostic.exe [--json]\n"); return 2; }
    FILE *capture = NULL;
    int saved = -1;
    if (json) {
        capture = tmpfile();
        if (!capture) { fprintf(stderr, "Cannot create report buffer\n"); return 3; }
        saved = _dup(_fileno(stdout));
        if (saved < 0 || _dup2(_fileno(capture), _fileno(stdout)) != 0) { fclose(capture); return 3; }
    }
    /*
     * Use UTF-8 for all console output.
     * Individual modules must not change the console code page.
     */
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    printf("PC Diagnostic Engine\n");
    printf("====================\n");

    scan_system();
    scan_hardware();
    scan_drivers();
    scan_logs();
    scan_bsod_dumps();
    analyze_diagnostics();

    if (json) {
        int failed = fflush(stdout) != 0;
        if (_dup2(saved, _fileno(stdout)) != 0) { _close(saved); fclose(capture); return 3; }
        _close(saved);
        if (failed) { fclose(capture); return 3; }
        write_json_report(capture);
        int read_error = ferror(capture);
        fclose(capture);
        if (read_error || fflush(stdout) != 0) return 3;
    }
    return 0;
}
