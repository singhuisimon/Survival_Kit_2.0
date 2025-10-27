@echo off
REM ====================================
REM Build Both Debug & Release for GameEngine
REM ====================================

echo ========================================
echo GameEngine - Build All Configurations
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
    echo ERROR: Debug build failed!
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
    echo ERROR: Release build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo ========================================
echo All Builds Complete!
echo ========================================
echo.
echo Debug executable:   build\bin\Debug\GameEngine.exe
echo Release executable: build\bin\Release\GameEngine.exe
echo.

pause