@echo off
setlocal
cd /d "%~dp0"
pinceszellozteto.exe --batch --config config/baseline.ini --out simulation-output
if errorlevel 1 (
  echo Simulation failed. Read the error above.
  pause
  exit /b 1
)
start "" "%~dp0simulation-output\analysis.html"
