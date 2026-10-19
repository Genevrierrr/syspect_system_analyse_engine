$ErrorActionPreference = 'Stop'
$Root = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$Compiler = Join-Path $env:WINDIR 'Microsoft.NET\Framework64\v4.0.30319\csc.exe'
if (-not (Test-Path -LiteralPath $Compiler)) { throw '.NET Framework C# compiler is missing.' }
& $Compiler /nologo /target:winexe /optimize+ /platform:anycpu /reference:System.Windows.Forms.dll (('/out:') + (Join-Path $Root 'SyspectSetup.exe')) (Join-Path $PSScriptRoot 'SetupLauncher.cs')
if ($LASTEXITCODE -ne 0) { throw 'Setup launcher build failed.' }
Write-Output 'Built SyspectSetup.exe. Distribute the entire Syspect folder as a ZIP.'
