@echo off
echo ========================================
echo Building GLVM Game Engine...
echo ========================================

REM Change to project directory
cd /d "%~dp0"

REM Clean previous build
echo Cleaning previous build...
C:\msys64\msys2_shell.cmd -ucrt64 -defterm -here -no-start -c "/ucrt64/bin/mingw32-make -f MakefileMSYS2 clean"

REM Build the project
echo Building project...
C:\msys64\msys2_shell.cmd -ucrt64 -defterm -here -no-start -c "/ucrt64/bin/mingw32-make -f MakefileMSYS2"

if %errorlevel% equ 0 (
    echo.
    echo ========================================
    echo Build completed successfully!
    echo ========================================
    echo.
    echo Executable created: build\winGame.exe
    echo To run the game, use: run.bat
    echo.
) else (
    echo.
    echo ========================================
    echo Build failed!
    echo ========================================
    echo Please check the error messages above.
    echo.
)

pause
