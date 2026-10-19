$ErrorActionPreference = 'Stop'
$SourceRoot = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$InstallRoot = Join-Path $env:LOCALAPPDATA 'Programs\Syspect'
$StartMenu = Join-Path $env:APPDATA 'Microsoft\Windows\Start Menu\Programs\Syspect'
$Desktop = [Environment]::GetFolderPath('Desktop')

if (-not (Test-Path -LiteralPath (Join-Path $SourceRoot 'diagnosticengine.exe'))) {
    throw 'diagnosticengine.exe is missing from the package.'
}

# Refuse to update an active host; user data is stored separately and never removed.
$RunningHost = Get-CimInstance Win32_Process -Filter "Name='pythonw.exe' OR Name='python.exe'" -ErrorAction SilentlyContinue | Where-Object {
    $_.CommandLine -and $_.CommandLine.Contains((Join-Path $InstallRoot 'backend\server.py'))
}
if ($RunningHost) { throw 'Quit Syspect from its header before updating, then run setup again.' }
New-Item -ItemType Directory -Force -Path $InstallRoot | Out-Null
if ($SourceRoot.TrimEnd('\') -ne $InstallRoot.TrimEnd('\')) {
    Get-ChildItem -LiteralPath $SourceRoot -Force | Where-Object { $_.Name -notin @('downloads','.env','tests','engine-source') } | ForEach-Object {
        Copy-Item -LiteralPath $_.FullName -Destination $InstallRoot -Recurse -Force
    }
}

$launcherBuild = Join-Path $InstallRoot 'desktop\Build-NativeLauncher.ps1'
$nativeLauncher = Join-Path $InstallRoot 'Syspect.exe'
if (-not (Test-Path -LiteralPath $nativeLauncher)) {
    try {
        & powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File $launcherBuild | Out-Host
    } catch {
        Write-Warning 'Native Syspect.exe launcher could not be built. The silent script launcher will be used instead.'
    }
}

New-Item -ItemType Directory -Force -Path $StartMenu | Out-Null
$wsh = New-Object -ComObject WScript.Shell

function New-SyspectShortcut {
    param([string]$Path)
    $shortcut = $wsh.CreateShortcut($Path)
    if (Test-Path -LiteralPath $nativeLauncher) {
        $shortcut.TargetPath = $nativeLauncher
        $shortcut.Arguments = ''
    } else {
        $shortcut.TargetPath = Join-Path $env:WINDIR 'System32\wscript.exe'
        $shortcut.Arguments = '"' + (Join-Path $InstallRoot 'Launch-Syspect.vbs') + '"'
    }
    $shortcut.WorkingDirectory = $InstallRoot
    $shortcut.Description = 'Syspect PC Diagnostic Engine'
    if (Test-Path -LiteralPath $nativeLauncher) { $shortcut.IconLocation = "$nativeLauncher,0" }
    else { $shortcut.IconLocation = (Join-Path $InstallRoot 'diagnosticengine.exe') + ',0' }
    $shortcut.Save()
}

New-SyspectShortcut (Join-Path $StartMenu 'Syspect.lnk')
New-SyspectShortcut (Join-Path $Desktop 'Syspect.lnk')

$uninstallShortcut = $wsh.CreateShortcut((Join-Path $StartMenu 'Uninstall Syspect.lnk'))
$uninstallShortcut.TargetPath = 'powershell.exe'
$uninstallShortcut.Arguments = '-NoLogo -NoProfile -ExecutionPolicy Bypass -File "' + (Join-Path $InstallRoot 'installer\Uninstall-Syspect.ps1') + '"'
$uninstallShortcut.WorkingDirectory = $InstallRoot
$uninstallShortcut.Description = 'Uninstall Syspect'
$uninstallShortcut.Save()

$regPath = 'HKCU:\Software\Microsoft\Windows\CurrentVersion\Uninstall\Syspect'
New-Item -Path $regPath -Force | Out-Null
Set-ItemProperty -Path $regPath -Name DisplayName -Value 'Syspect'
Set-ItemProperty -Path $regPath -Name DisplayVersion -Value '1.1.0'
Set-ItemProperty -Path $regPath -Name Publisher -Value 'Erlik Studio'
Set-ItemProperty -Path $regPath -Name InstallLocation -Value $InstallRoot
Set-ItemProperty -Path $regPath -Name UninstallString -Value ('powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "' + (Join-Path $InstallRoot 'installer\Uninstall-Syspect.ps1') + '"')
Set-ItemProperty -Path $regPath -Name NoModify -Value 1 -Type DWord
Set-ItemProperty -Path $regPath -Name NoRepair -Value 1 -Type DWord

Write-Host ''
Write-Host 'Syspect 1.1.0 installed successfully.' -ForegroundColor Green
Write-Host "Install location: $InstallRoot"
Start-Process (Join-Path $StartMenu 'Syspect.lnk')
