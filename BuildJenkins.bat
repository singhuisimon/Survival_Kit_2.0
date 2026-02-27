@echo off
setlocal

REM ===========================================
REM BuildJenkins.bat - CI Build Script
REM No pauses, no interactive prompts
REM ===========================================

cd /d %~dp0

REM ===========================================
REM Build GameScripts.dll FIRST
REM ===========================================
echo ===========================================
echo [CI] Building C# Scripts (Debug + Release)
echo ===========================================
echo.

set DOTNET_ROOT=%CD%\External\dotnet
set PATH=%DOTNET_ROOT%;%PATH%
set DOTNET_MULTILEVEL_LOOKUP=0
set DOTNET_CLI_TELEMETRY_OPTOUT=1

if not exist "%DOTNET_ROOT%\dotnet.exe" (
    echo [CI ERROR] dotnet.exe not found at %DOTNET_ROOT%
    exit /b 1
)

REM Clean stale DLLs
if exist "Scripts\bin\Debug\net8.0\GameScripts.dll" del /Q "Scripts\bin\Debug\net8.0\GameScripts.dll"
if exist "Scripts\bin\Release\net8.0\GameScripts.dll" del /Q "Scripts\bin\Release\net8.0\GameScripts.dll"

echo.
echo --- [CI] Building Debug (GameScripts.dll) ---
"%DOTNET_ROOT%\dotnet.exe" build Scripts\GameScripts.csproj -c Debug --ignore-failed-sources -nologo -v:m
if %ERRORLEVEL% NEQ 0 (
    echo [CI ERROR] Debug GameScripts build failed!
    exit /b %ERRORLEVEL%
)

echo.
echo --- [CI] Building Release (GameScripts.dll) ---
"%DOTNET_ROOT%\dotnet.exe" build Scripts\GameScripts.csproj -c Release --ignore-failed-sources -nologo -v:m
if %ERRORLEVEL% NEQ 0 (
    echo [CI ERROR] Release GameScripts build failed!
    exit /b %ERRORLEVEL%
)

REM ===========================================
REM Build C++ Game Engine (CMake/MSBuild)
REM ===========================================
echo.
echo ===========================================
echo [CI] GameEngine - Build All Configurations
echo ===========================================
echo.

if not exist "build" (
    echo [CI] Creating build directory...
    mkdir build
)

cd build

echo [CI] Generating Visual Studio solution...
cmake .. -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% NEQ 0 (
    echo [CI ERROR] CMake generation failed!
    exit /b %ERRORLEVEL%
)

echo.
echo --- [CI] Building Debug configuration ---
cmake --build . --config Debug -j
if %ERRORLEVEL% NEQ 0 (
    echo [CI ERROR] Debug build failed!
    exit /b %ERRORLEVEL%
)

echo.
echo --- [CI] Building Release configuration ---
cmake --build . --config Release -j
if %ERRORLEVEL% NEQ 0 (
    echo [CI ERROR] Release build failed!
    exit /b %ERRORLEVEL%
)

REM ===========================================
REM Copy GameScripts.dll to output directories
REM ===========================================
cd ..

copy /Y "Scripts\bin\Debug\net8.0\GameScripts.dll" "build\bin\Debug\GameScripts.dll"
copy /Y "Scripts\bin\Release\net8.0\GameScripts.dll" "build\bin\Release\GameScripts.dll"

REM ===========================================
REM Verify outputs exist
REM ===========================================
echo.
echo [CI] Verifying build outputs...

set BUILD_OK=true

if not exist "build\bin\Debug\GameEngine.exe" (
    echo [CI ERROR] Missing: build\bin\Debug\GameEngine.exe
    set BUILD_OK=false
)
if not exist "build\bin\Release\GameEngine.exe" (
    echo [CI ERROR] Missing: build\bin\Release\GameEngine.exe
    set BUILD_OK=false
)
if not exist "build\bin\Debug\GameScripts.dll" (
    echo [CI ERROR] Missing: build\bin\Debug\GameScripts.dll
    set BUILD_OK=false
)
if not exist "build\bin\Release\GameScripts.dll" (
    echo [CI ERROR] Missing: build\bin\Release\GameScripts.dll
    set BUILD_OK=false
)

if "%BUILD_OK%"=="false" (
    echo [CI ERROR] One or more build outputs are missing!
    exit /b 1
)

echo.
echo ===========================================
echo [CI] All builds completed successfully!
echo ===========================================
echo   Debug:   build\bin\Debug\GameEngine.exe
echo   Release: build\bin\Release\GameEngine.exe
echo   DLLs:    build\bin\Debug\GameScripts.dll
echo            build\bin\Release\GameScripts.dll
echo.

endlocal
exit /b 0
