@echo off
setlocal
cd /d "%~dp0"
if exist "%~dp0Syspect.exe" (
  start "" "%~dp0Syspect.exe"
) else (
  start "" wscript.exe "%~dp0Launch-Syspect.vbs"
)
endlocal
