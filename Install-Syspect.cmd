@echo off
setlocal
cd /d "%~dp0"
powershell.exe -NoLogo -NoProfile -ExecutionPolicy Bypass -File "%~dp0installer\Install-Syspect.ps1"
if errorlevel 1 (
  echo.
  echo Syspect installation failed.
  pause
)
endlocal
