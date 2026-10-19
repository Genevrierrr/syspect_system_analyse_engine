$ErrorActionPreference = 'Stop'
$Root = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$Source = Join-Path $PSScriptRoot 'SyspectLauncher.cs'
$Output = Join-Path $Root 'Syspect.exe'
$candidates = @(
    (Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'),
    (Join-Path $env:WINDIR 'Microsoft.NET\Framework\v4.0.30319\csc.exe')
)
$csc = $candidates | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
if (-not $csc) { throw 'The built-in .NET Framework C# compiler was not found.' }
& $csc /nologo /target:winexe /optimize+ /platform:anycpu /reference:System.Windows.Forms.dll /out:$Output $Source
if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $Output)) { throw 'Syspect.exe could not be built.' }
Write-Host "Built $Output"
