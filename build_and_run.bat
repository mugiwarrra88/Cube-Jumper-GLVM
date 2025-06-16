@echo off
echo ========================================
echo GLVM Game Engine - Build and Run
echo ========================================

REM Change to project directory
cd /d "%~dp0"

echo [1/2] Building the game...
echo.

REM Clean previous build
echo Cleaning previous build...
C:\msys64\msys2_shell.cmd -ucrt64 -defterm -here -no-start -c "/ucrt64/bin/mingw32-make -f MakefileMSYS2 clean"

REM Build the project
echo Building project...
C:\msys64\msys2_shell.cmd -ucrt64 -defterm -here -no-start -c "/ucrt64/bin/mingw32-make -f MakefileMSYS2"

if %errorlevel% neq 0 (
    echo.
    echo ========================================
    echo Build failed!
    echo ========================================
    echo Please check the error messages above.
    echo.
    pause
    exit /b 1
)

echo.
echo ========================================
echo Build completed successfully!
echo ========================================
echo.

echo [2/2] Running the game...
echo.

REM Check if executable exists
if not exist "build\winGame.exe" (
    echo Error: winGame.exe not found in build directory!
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
