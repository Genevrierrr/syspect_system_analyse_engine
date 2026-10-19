$ErrorActionPreference = 'Stop'
$InstallRoot = [IO.Path]::GetFullPath((Join-Path $env:LOCALAPPDATA 'Programs\Syspect'))
$CurrentRoot = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
if ($CurrentRoot -ne $InstallRoot) { throw 'Run uninstall from the installed Syspect directory.' }
$RunningHost = Get-CimInstance Win32_Process -Filter "Name='pythonw.exe' OR Name='python.exe'" -ErrorAction SilentlyContinue | Where-Object {
    $_.CommandLine -and $_.CommandLine.Contains((Join-Path $InstallRoot 'backend\server.py'))
}
if ($RunningHost) { throw 'Quit Syspect from its header before uninstalling.' }
$StartMenu = [IO.Path]::GetFullPath((Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Syspect'))
$DesktopShortcut = Join-Path ([Environment]::GetFolderPath('Desktop')) 'Syspect.lnk'
Remove-Item -LiteralPath $DesktopShortcut -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath $StartMenu -Recurse -Force -ErrorAction SilentlyContinue
Remove-Item -LiteralPath 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\Syspect' -Recurse -Force -ErrorAction SilentlyContinue
# Both absolute targets above are explicit Syspect locations. Never touch the data directory.
Set-Location -LiteralPath $env:TEMP
Remove-Item -LiteralPath $InstallRoot -Recurse -Force
Write-Output 'Syspect uninstalled. Your accounts and reports remain in LocalAppData\Syspect\data.'
