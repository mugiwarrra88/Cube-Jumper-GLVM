@echo off
setlocal

echo ========================================
echo GLVM Engine - FAST BUILD
echo ========================================

cd /d "%~dp0"

REM Автоматически определяем количество ядер для максимальной скорости
set PARALLEL_JOBS=%NUMBER_OF_PROCESSORS%
if "%PARALLEL_JOBS%"=="" set PARALLEL_JOBS=4

REM Проверяем нужна ли полная очистка (только если передан аргумент clean)
if /I "%~1"=="clean" (
    echo Full clean build requested...
    if exist build rmdir /s /q build
) else (
    echo Using incremental build for maximum speed...
)

echo Checking MSYS2...
if not exist "C:\msys64\msys2_shell.cmd" (
    echo ERROR: MSYS2 not found!
    pause
    exit /b 1
)

REM Создаем директории только если их нет
if not exist build mkdir build
if not exist build\Systems mkdir build\Systems
if not exist build\WinApi mkdir build\WinApi
if not exist build\GraphicAPI mkdir build\GraphicAPI

echo Starting FAST build with %PARALLEL_JOBS% parallel jobs...
echo This will be much faster than before!
echo.

REM Максимально быстрая сборка с оптимизациями
C:\msys64\msys2_shell.cmd -ucrt64 -defterm -here -no-start -c "/ucrt64/bin/mingw32-make -f MakefileMSYS2 -j%PARALLEL_JOBS%"

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
