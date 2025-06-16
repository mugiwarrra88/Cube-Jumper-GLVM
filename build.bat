@echo off
setlocal

echo ========================================
echo GLVM Engine - Quick Build
echo ========================================

cd /d "%~dp0"

echo Checking MSYS2...
if not exist "C:\msys64\msys2_shell.cmd" (
    echo ERROR: MSYS2 not found!
    pause
    exit /b 1
)

echo Cleaning build directory...
if exist build rmdir /s /q build
mkdir build
mkdir build\Systems
mkdir build\WinApi
mkdir build\GraphicAPI

echo Starting build process...
echo This may take a few minutes...
echo.

C:\msys64\msys2_shell.cmd -ucrt64 -defterm -here -no-start -c "/ucrt64/bin/mingw32-make -f MakefileMSYS2"

if exist "build\winGame.exe" (
    echo.
    echo ========================================
    echo SUCCESS: Build completed!
    echo ========================================
    echo Executable: build\winGame.exe
    echo.
    echo Press any key to run the game...
    pause > nul
    
    echo Starting game...
    cd build
    start "" winGame.exe
    cd ..
) else (
    echo.
    echo ========================================
    echo ERROR: Build failed!
    echo ========================================
    echo Check the error messages above for details
)

pause
