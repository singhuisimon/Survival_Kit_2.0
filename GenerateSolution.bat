@echo off
REM Check if CMake is installed
echo [1/4] Checking for CMake...
cmake --version > nul 2>&1
if %errorlevel% neq 0 (
	echo ERROR: CMake not found in PATH, please install CMake for Windows!
	exit /b 1
)

echo [2/4] Creating build directory
if not exist build mkdir build

echo [3/4] Running CMake configuration
cmake -S . -B build -G "Visual Studio 17 2022"

echo [4/4] Solution generated!

pause