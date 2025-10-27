@echo off
REM ====================================
REM Debug Build Script for GameEngine
REM ====================================

echo ========================================
echo GameEngine Debug Build Script
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
echo Building Debug configuration...
echo ========================================
echo.

cmake --build . --config Debug -j

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ========================================
echo Debug Build Complete!
echo ========================================
echo.
echo Executable location: build\bin\Debug\GameEngine.exe
echo.
echo To run the game:
echo   cd build\bin\Debug
echo   GameEngine.exe
echo.

pause