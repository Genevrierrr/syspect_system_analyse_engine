param([int]$Port = 37841, [switch]$NoWindow)
$ErrorActionPreference = 'Stop'
$AppRoot = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$Python = Join-Path $AppRoot 'runtime\pythonw.exe'
$Server = Join-Path $AppRoot 'backend\server.py'
try {
    if (-not (Test-Path -LiteralPath $Python)) { throw 'The bundled runtime is missing. Extract the entire Syspect package.' }
    $Config = @{}
    $EnvFile = Join-Path $AppRoot '.env'
    if (Test-Path -LiteralPath $EnvFile) {
        foreach ($Line in [IO.File]::ReadAllLines($EnvFile)) {
            if ($Line.Trim() -match '^(SYSPECT_[A-Z_]+)\s*=\s*(.*)$') {
                $Config[$Matches[1]] = [Environment]::ExpandEnvironmentVariables($Matches[2].Trim().Trim('"'))
            }
        }
    }
    $DataDir = Join-Path $env:LOCALAPPDATA 'Syspect\data'
    if ($Config.ContainsKey('SYSPECT_DATA_DIR')) { $DataDir = $Config['SYSPECT_DATA_DIR'] }
    if ($env:SYSPECT_DATA_DIR) { $DataDir = $env:SYSPECT_DATA_DIR }
    if ($Config.ContainsKey('SYSPECT_PORT')) { $Port = [int]$Config['SYSPECT_PORT'] }
    if ($env:SYSPECT_PORT) { $Port = [int]$env:SYSPECT_PORT }
    $DataDir = [IO.Path]::GetFullPath($DataDir)
    New-Item -ItemType Directory -Path $DataDir -Force | Out-Null
    $ReadyFile = Join-Path $DataDir 'host.json'
    $Url = $null
    if (Test-Path -LiteralPath $ReadyFile) {
        try {
            $Ready = Get-Content -LiteralPath $ReadyFile -Raw | ConvertFrom-Json
            $Info = Invoke-RestMethod -Uri "http://127.0.0.1:$($Ready.port)/api/v1/app" -TimeoutSec 2
            $ExpectedBytes = [Text.Encoding]::UTF8.GetBytes($AppRoot + (Join-Path $DataDir 'syspect.sqlite3'))
            $Sha = [Security.Cryptography.SHA256]::Create()
            $Expected = ([BitConverter]::ToString($Sha.ComputeHash($ExpectedBytes))).Replace('-','').ToLowerInvariant()
            $Sha.Dispose()
            if ($Info.version -eq '1.1.0' -and $Info.instance -eq $Expected -and -not $Info.stopping) { $Url = "http://127.0.0.1:$($Ready.port)/index.html" }
        } catch { }
    }
    if (-not $Url) {
        $Arguments = '"' + $Server + '" --port ' + $Port + ' --data-dir "' + $DataDir + '" --ready-file "' + $ReadyFile + '"'
        $HostProcess = Start-Process -FilePath $Python -ArgumentList $Arguments -WorkingDirectory $AppRoot -WindowStyle Hidden -PassThru
        for ($i = 0; $i -lt 80; $i++) {
            Start-Sleep -Milliseconds 200
            if (Test-Path -LiteralPath $ReadyFile) {
                try {
                    $Ready = Get-Content -LiteralPath $ReadyFile -Raw | ConvertFrom-Json
                    if ($Ready.pid -eq $HostProcess.Id) {
                        $Info = Invoke-RestMethod -Uri "http://127.0.0.1:$($Ready.port)/api/v1/app" -TimeoutSec 2
                        if ($Info.version -eq '1.1.0' -and $Info.database -eq 'sqlite') { $Url = "http://127.0.0.1:$($Ready.port)/index.html"; break }
                    }
                } catch { }
            }
            if ($HostProcess.HasExited) { break }
        }
        if (-not $Url) { throw "Syspect could not start. See $DataDir\server.log. Close other Syspect versions before updating." }
    }
    if ($NoWindow) { Write-Output $Url; exit 0 }
    $Edge = @(
        (Join-Path ${env:ProgramFiles(x86)} 'Microsoft\Edge\Application\msedge.exe'),
        (Join-Path $env:ProgramFiles 'Microsoft\Edge\Application\msedge.exe')
    ) | Where-Object { Test-Path -LiteralPath $_ } | Select-Object -First 1
    if ($Edge) {
        $Profile = Join-Path $env:LOCALAPPDATA 'Syspect\EdgeProfile'
        Start-Process -FilePath $Edge -ArgumentList @("--app=$Url",'--start-maximized','--no-first-run',('--user-data-dir="' + $Profile + '"')) | Out-Null
    } else { Start-Process $Url | Out-Null }
} catch {
    if ($NoWindow) { throw }
    Add-Type -AssemblyName System.Windows.Forms
    [System.Windows.Forms.MessageBox]::Show($_.Exception.Message,'Syspect') | Out-Null
    exit 1
}
