@echo off
setlocal

cd /d %~dp0

echo ===========================================
echo [INSTALLER] Step 1/5: Applying Patches
echo ===========================================

xcopy /Y "InstallerStepModifiedFiles\AssetManager.h"        "Engine\Asset\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy AssetManager.h & exit /b 1)

xcopy /Y "InstallerStepModifiedFiles\AssetDatabase.cpp"     "Engine\Asset\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy AssetDatabase.cpp & exit /b 1)

xcopy /Y "InstallerStepModifiedFiles\AssetScanner.cpp"      "Engine\Asset\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy AssetScanner.cpp & exit /b 1)

xcopy /Y "InstallerStepModifiedFiles\ResourceLoaders.cpp"   "Engine\Asset\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy ResourceLoaders.cpp & exit /b 1)

xcopy /Y "InstallerStepModifiedFiles\Editor.cpp"            "Engine\Editor\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy Editor.cpp & exit /b 1)

xcopy /Y "InstallerStepModifiedFiles\Editor.h"              "Engine\Editor\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy Editor.h & exit /b 1)

xcopy /Y "InstallerStepModifiedFiles\EditorViewportPanel.h" "Engine\Editor\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy EditorViewportPanel.h & exit /b 1)

xcopy /Y "InstallerStepModifiedFiles\Renderer.h"            "Engine\Graphics\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy Renderer.h & exit /b 1)

xcopy /Y "InstallerStepModifiedFiles\Renderer.cpp"          "Engine\Graphics\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy Renderer.cpp & exit /b 1)

xcopy /Y "InstallerStepModifiedFiles\Game.cpp"              "Game\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy Game.cpp & exit /b 1)

xcopy /Y "InstallerStepModifiedFiles\Game.h"                "Game\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy Game.h & exit /b 1)

xcopy /Y "InstallerStepModifiedFiles\Main.cpp"              "AssetCompiler\Main\"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Failed to copy Main.cpp & exit /b 1)

echo [OK] All patches applied.

echo ===========================================
echo [INSTALLER] Step 2/5: Building
echo ===========================================

call BuildJenkins.bat
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Build failed & exit /b %ERRORLEVEL%)

echo [OK] Build complete.

echo ===========================================
echo [INSTALLER] Step 3/5: Compiling Assets
echo ===========================================

"build\bin\Release\AssetCompiler.exe" --force --verbose
if %ERRORLEVEL% NEQ 0 (echo [ERROR] Asset compilation failed & exit /b %ERRORLEVEL%)

robocopy "Resources\Compiled" "build\bin\Release\Resources\Compiled" /E /IS /IT /NP /NJH /NJS
if %ERRORLEVEL% GEQ 8 (echo [ERROR] Failed to copy compiled assets & exit /b 1)

echo [OK] Assets compiled.

echo ===========================================
echo [INSTALLER] Step 4/5: Staging Game Files
echo ===========================================

if not exist "Installer\GAMEDIRECTORY" mkdir "Installer\GAMEDIRECTORY"

robocopy "build\bin\Release" "Installer\GAMEDIRECTORY" /E /IS /IT /NP /NJH /NJS
if %ERRORLEVEL% GEQ 8 (echo [ERROR] robocopy failed & exit /b 1)

echo [OK] Files staged.

echo ===========================================
echo [INSTALLER] Step 5/5: Building Installer
echo ===========================================

"C:\Program Files (x86)\Inno Setup 6\ISCC.exe" "Installer\InstallScript.iss"
if %ERRORLEVEL% NEQ 0 (echo [ERROR] ISCC failed & exit /b %ERRORLEVEL%)

echo ===========================================
echo [INSTALLER] Done!
echo Output: Installer\INSTALLER\GuardianOfTheMotherboard_Setup.exe
echo ===========================================

endlocal
exit /b 0
