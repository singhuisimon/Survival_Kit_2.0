@echo off
setlocal

echo ==========================================
echo Building C# Scripts (Debug + Release)
echo ==========================================
echo.

REM Go up one directory to project root (from Scripts/ to project root)
cd ..

REM Set up local dotnet (now relative to project root)
set DOTNET_ROOT=%CD%\External\dotnet
set PATH=%DOTNET_ROOT%;%PATH%
set DOTNET_MULTILEVEL_LOOKUP=0

echo Using dotnet from: %DOTNET_ROOT%
echo.

REM Verify dotnet.exe exists
if not exist "%DOTNET_ROOT%\dotnet.exe" (
    echo ERROR: dotnet.exe not found at %DOTNET_ROOT%
    echo Please make sure .NET SDK is extracted to External\dotnet
    pause
    exit /b 1
)

REM ==========================================
REM Build Debug Configuration
REM ==========================================
echo.
echo ------------------------------------------
echo Building Debug configuration...
echo ------------------------------------------
"%DOTNET_ROOT%\dotnet.exe" build Scripts\GameScripts.csproj -c Debug

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Debug build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo Debug build completed successfully!

REM ==========================================
REM Build Release Configuration
REM ==========================================
echo.
echo ------------------------------------------
echo Building Release configuration...
echo ------------------------------------------
"%DOTNET_ROOT%\dotnet.exe" build Scripts\GameScripts.csproj -c Release

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo ERROR: Release build failed!
    pause
    exit /b %ERRORLEVEL%
)

echo Release build completed successfully!

echo.
echo ==========================================
echo All builds completed successfully!
echo ==========================================
echo.
echo Output locations:
echo   Debug:   Scripts\bin\Debug\net8.0\
echo   Release: Scripts\bin\Release\net8.0\
echo.

endlocal
pause
