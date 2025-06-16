@echo off
REM Quick run script without pauses

REM Change to project directory
cd /d "%~dp0"

REM Check if executable exists
if not exist "build\winGame.exe" (
    echo Error: winGame.exe not found! Run build.bat first.
    exit /b 1
)

REM Change to build directory and run the game
cd build
winGame.exe
