$ErrorActionPreference = 'Stop'
$Compiler = if ($env:CC) { $env:CC } else { 'gcc' }
$sources = @('main.c','system.c','hardware.c','drivers.c','logs.c','diagnostics.c','dump_analyzer.c','correlation.c','report.c') | ForEach-Object { Join-Path $PSScriptRoot $_ }
$output = Join-Path $PSScriptRoot 'diagnosticengine.exe'
& $Compiler -std=c17 -Wall -Wextra -Werror -O2 @sources -o $output -lsetupapi -lcfgmgr32 -lwevtapi -ldbghelp -ladvapi32
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
Copy-Item -LiteralPath $output -Destination (Join-Path $PSScriptRoot 'diagnostic.exe') -Force
Write-Output "Built $output (diagnostic.exe compatibility alias refreshed)"
