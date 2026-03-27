@echo off
setlocal

REM ===========================================
REM Always run from the project root
cd /d %~dp0

REM ===========================================
REM Build GameScripts.dll FIRST
REM ===========================================
echo ===========================================
echo Building C# Scripts (Debug + Release)
echo ===========================================
echo.

set DOTNET_ROOT=%CD%\External\dotnet
set PATH=%DOTNET_ROOT%;%PATH%
set DOTNET_MULTILEVEL_LOOKUP=0

if not exist "%DOTNET_ROOT%\dotnet.exe" (
    echo ERROR: dotnet.exe not found at %DOTNET_ROOT%
    pause
    exit /b 1
)

REM Clean stale DLLs
if exist "Scripts\bin\Debug\net8.0\GameScripts.dll" del /Q "Scripts\bin\Debug\net8.0\GameScripts.dll"
if exist "Scripts\bin\Release\net8.0\GameScripts.dll" del /Q "Scripts\bin\Release\net8.0\GameScripts.dll"

REM Build Debug and Release configs
echo.
echo --- Building Debug (GameScripts.dll) ---
"%DOTNET_ROOT%\dotnet.exe" build Scripts\GameScripts.csproj -c Debug
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Debug Gamescripts build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo --- Building Release (GameScripts.dll) ---
"%DOTNET_ROOT%\dotnet.exe" build Scripts\GameScripts.csproj -c Release
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Release Gamescripts build failed!
    pause
    exit /b %ERRORLEVEL%
)

REM ===========================================
REM Build C++ Game Engine (CMake/MSBuild)
REM ===========================================
echo.
echo ===========================================
echo GameEngine - Build All Configurations
echo ===========================================
echo.

REM Make build directory if necessary
if not exist "build" (
    echo Creating build directory...
    mkdir build
)

REM Enter build directory
cd build

REM Generate Visual Studio solution (from build/)
echo Generating Visual Studio solution...
cmake .. -G "Visual Studio 17 2022" -A x64
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake generation failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo --- Building Debug configuration (GameEngine) ---
cmake --build . --config Debug -j
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Debug build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo.
echo --- Building Release configuration (GameEngine) ---
cmake --build . --config Release -j
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Release build failed!
    pause
    exit /b %ERRORLEVEL%
)

REM ===============================================
REM Ensure the *latest* GameScripts.dll is present
REM ===============================================
cd ..

copy /Y "Scripts\bin\Debug\net8.0\GameScripts.dll" "build\bin\Debug\GameScripts.dll"
copy /Y "Scripts\bin\Release\net8.0\GameScripts.dll" "build\bin\Release\GameScripts.dll"

REM ===============================================
REM Compile Assets (Textures, Meshes, Fonts)
REM ===============================================
echo.
echo ===========================================
echo Compiling Assets
echo ===========================================
echo.

"build\bin\Release\AssetCompiler.exe" --force --verbose
echo [NOTE] AssetCompiler exit code: %ERRORLEVEL% (non-zero is expected - Audio/Material/Shader compilers not implemented)

REM Copy compiled assets to both Debug and Release output
robocopy "Resources\Compiled" "build\bin\Debug\Resources\Compiled" /E /IS /IT /NP /NJH /NJS
robocopy "Resources\Compiled" "build\bin\Release\Resources\Compiled" /E /IS /IT /NP /NJH /NJS

echo.
echo ===========================================
echo All builds completed successfully!
echo ===========================================
echo Debug executable:   build\bin\Debug\GameEngine.exe
echo Release executable: build\bin\Release\GameEngine.exe
echo Output DLLs:
echo   build\bin\Debug\GameScripts.dll
echo   build\bin\Release\GameScripts.dll
echo.

endlocal
pause
