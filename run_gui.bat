@echo off
cd /d "%~dp0"
if exist simulator_gui.exe (
  start "" simulator_gui.exe
) else (
  echo GUI exe not found. Build it first with build_gui.bat
)
