param([string]$PublicOrigin = 'https://prognosis-scarily-qualify.ngrok-free.dev')
$ErrorActionPreference = 'Stop'
$Root = [IO.Path]::GetFullPath((Split-Path -Parent $PSScriptRoot))
$Python = Join-Path $Root 'runtime\python.exe'
$Ngrok = Join-Path $PSScriptRoot 'ngrok.exe'
$DataDir = Join-Path $env:LOCALAPPDATA 'Syspect\ngrok-test'
$ReadyFile = Join-Path $DataDir 'host.json'
$PolicyFile = Join-Path $DataDir 'traffic-policy.json'
$ServerProcess = $null
$TunnelProcess = $null
$PreviousOrigin = $env:SYSPECT_PUBLIC_ORIGIN
$PublicOrigin = $PublicOrigin.TrimEnd('/')
$Uri = [Uri]$PublicOrigin
if ($Uri.Scheme -ne 'https' -or $Uri.AbsolutePath -ne '/' -or $Uri.Query -or $Uri.UserInfo) { throw 'Enter a valid HTTPS origin.' }
New-Item -ItemType Directory -Path $DataDir -Force | Out-Null

function New-PrivateToken {
    $bytes = New-Object byte[] 18
    $rng = [Security.Cryptography.RandomNumberGenerator]::Create()
    try { $rng.GetBytes($bytes) } finally { $rng.Dispose() }
    return [Convert]::ToBase64String($bytes).Replace('+','-').Replace('/','_').TrimEnd('=')
}

try {
    if (-not (Test-Path -LiteralPath $Ngrok)) { throw 'tools\ngrok.exe is missing. Extract the whole ZIP first.' }
    Write-Host 'SYSPECT - NGROK TEST' -ForegroundColor Cyan
    Write-Host "Public URL: $PublicOrigin"
    Write-Host 'Scans run on THIS computer, even if you open the URL on a phone.'
    Write-Host 'The test uses a separate database; your normal Syspect accounts are not exposed.'
    Write-Host ''
    Write-Host 'Copy your authtoken from https://dashboard.ngrok.com/get-started/your-authtoken'
    Write-Host 'Paste it below. Input is hidden and is not saved to disk.'
    $Secret = Read-Host 'Ngrok authtoken' -AsSecureString
    $Pointer = [Runtime.InteropServices.Marshal]::SecureStringToBSTR($Secret)
    try { $Token = [Runtime.InteropServices.Marshal]::PtrToStringBSTR($Pointer) }
    finally { [Runtime.InteropServices.Marshal]::ZeroFreeBSTR($Pointer) }
    if ([string]::IsNullOrWhiteSpace($Token)) { throw 'An authtoken is required.' }

    $env:SYSPECT_PUBLIC_ORIGIN = $PublicOrigin
    $Arguments = '"' + (Join-Path $Root 'backend\server.py') + '" --port 37941 --data-dir "' + $DataDir + '" --ready-file "' + $ReadyFile + '"'
    $ServerProcess = Start-Process -FilePath $Python -ArgumentList $Arguments -WorkingDirectory $Root -WindowStyle Hidden -PassThru
    $Port = $null
    for ($i=0; $i -lt 75; $i++) {
        Start-Sleep -Milliseconds 200
        if (Test-Path -LiteralPath $ReadyFile) {
            try {
                $Ready = Get-Content -LiteralPath $ReadyFile -Raw | ConvertFrom-Json
                if ($Ready.pid -eq $ServerProcess.Id) {
                    $Health = Invoke-RestMethod -Uri "http://127.0.0.1:$($Ready.port)/api/v1/health" -TimeoutSec 2
                    if ($Health.database -eq 'ok') { $Port = [int]$Ready.port; break }
                }
            } catch { }
        }
        if ($ServerProcess.HasExited) { break }
    }
    if (-not $Port) { throw "Syspect test server could not start. Close any existing test launcher. Log: $DataDir\server.log" }

    $AccessPassword = New-PrivateToken
    $Policy = @{ on_http_request = @(@{actions=@(@{type='basic-auth';config=@{credentials=@("syspect:$AccessPassword");enforce=$true}})}) }
    [IO.File]::WriteAllText($PolicyFile,($Policy | ConvertTo-Json -Depth 8),[Text.UTF8Encoding]::new($false))
    $Start = New-Object Diagnostics.ProcessStartInfo
    $Start.FileName = $Ngrok
    $Start.Arguments = 'http http://127.0.0.1:' + $Port + ' --url "' + $PublicOrigin + '" --traffic-policy-file "' + $PolicyFile + '" --inspect=false --log stdout --log-format json'
    $Start.UseShellExecute = $false
    $Start.CreateNoWindow = $true
    $Start.RedirectStandardOutput = $true
    $Start.RedirectStandardError = $true
    $Start.EnvironmentVariables['NGROK_AUTHTOKEN'] = $Token
    $TunnelProcess = [Diagnostics.Process]::Start($Start)
    $Stdout = $TunnelProcess.StandardOutput.ReadToEndAsync()
    $Stderr = $TunnelProcess.StandardError.ReadToEndAsync()
    $Token = $null
    $Start.EnvironmentVariables.Remove('NGROK_AUTHTOKEN')
    Start-Sleep -Seconds 2
    if ($TunnelProcess.HasExited) { throw ($Stderr.GetAwaiter().GetResult()) }

    Write-Host ''
    Write-Host 'TUNNEL STARTING - use this URL once ngrok connects:' -ForegroundColor Green
    Write-Host $PublicOrigin -ForegroundColor Cyan
    Write-Host 'Browser access username: syspect'
    Write-Host "Browser access password: $AccessPassword" -ForegroundColor Yellow
    Write-Host ''
    Write-Host 'After the browser password prompt, create a Syspect test account.'
    Write-Host 'Keep this window open. Press Ctrl+C here to stop the tunnel and test server.'
    while (-not $TunnelProcess.WaitForExit(1000)) {
        if ($ServerProcess.HasExited) { throw 'Syspect test server stopped.' }
    }
    if ($TunnelProcess.ExitCode -ne 0) { throw ($Stderr.GetAwaiter().GetResult()) }
} catch {
    Write-Host $_.Exception.Message -ForegroundColor Red
} finally {
    foreach ($OwnedProcess in @($TunnelProcess,$ServerProcess)) {
        if ($OwnedProcess -and -not $OwnedProcess.HasExited) { $OwnedProcess.Kill(); $OwnedProcess.WaitForExit() }
    }
    $env:SYSPECT_PUBLIC_ORIGIN = $PreviousOrigin
    $Token = $null
    if (Test-Path -LiteralPath $PolicyFile) { Remove-Item -LiteralPath $PolicyFile -Force }
    Write-Host 'Test stopped. Saved test reports remain in the separate test database.'
}
