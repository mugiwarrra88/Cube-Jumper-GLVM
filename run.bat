@echo off
echo ========================================
echo Running GLVM Game Engine...
echo ========================================

REM Change to project directory
cd /d "%~dp0"

REM Check if executable exists
if not exist "build\winGame.exe" (
    echo Error: winGame.exe not found in build directory!
    echo Please run build.bat first to compile the game.
    echo.
    pause
    exit /b 1
)

REM Change to build directory and run the game
echo Starting game from build directory...
echo.
cd build
winGame.exe

echo.
echo ========================================
echo Game finished.
echo ========================================
pause
