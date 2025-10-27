@echo off
REM ====================================
REM Release Build Script for GameEngine
REM ====================================

echo ========================================
echo GameEngine Release Build Script
echo ========================================
echo.

REM Create build directory if it doesn't exist
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

cd build

echo Generating Visual Studio solution...
cmake .. -G "Visual Studio 17 2022" -A x64

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: CMake generation failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ========================================
echo Building Release configuration...
echo ========================================
echo.

cmake --build . --config Release -j

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ========================================
echo Build Complete!
echo ========================================
echo.
echo Executable location: build\bin\Release\GameEngine.exe
echo.
echo To run the game:
echo   cd build\bin\Release
echo   GameEngine.exe
echo.

pause