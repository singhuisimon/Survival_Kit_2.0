pipeline {
    agent { label 'windows' }

    environment {
        INNO_SETUP = 'C:\\Program Files (x86)\\Inno Setup 6\\ISCC.exe'
        RELEASE_DIR = 'build\\bin\\Release'
        GAME_DIR    = 'Installer\\GAMEDIRECTORY'
        INSTALLER_OUT = 'Installer\\INSTALLER'
    }

    stages {

        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Apply Installer Patches') {
            steps {
                bat '''
                    echo [PATCH] Applying installer file patches...
                    xcopy /Y "InstallerStepModifiedFiles\\AssetManager.h"        "Engine\\Asset\\"
                    xcopy /Y "InstallerStepModifiedFiles\\AssetDatabase.cpp"     "Engine\\Asset\\"
                    xcopy /Y "InstallerStepModifiedFiles\\AssetScanner.cpp"      "Engine\\Asset\\"
                    xcopy /Y "InstallerStepModifiedFiles\\ResourceLoaders.cpp"   "Engine\\Asset\\"
                    xcopy /Y "InstallerStepModifiedFiles\\Editor.cpp"            "Engine\\Editor\\"
                    xcopy /Y "InstallerStepModifiedFiles\\Editor.h"              "Engine\\Editor\\"
                    xcopy /Y "InstallerStepModifiedFiles\\EditorViewportPanel.h" "Engine\\Editor\\"
                    xcopy /Y "InstallerStepModifiedFiles\\Renderer.h"            "Engine\\Graphics\\"
                    xcopy /Y "InstallerStepModifiedFiles\\Renderer.cpp"          "Engine\\Graphics\\"
                    xcopy /Y "InstallerStepModifiedFiles\\Game.cpp"              "Game\\"
                    xcopy /Y "InstallerStepModifiedFiles\\Game.h"                "Game\\"
                    xcopy /Y "InstallerStepModifiedFiles\\Main.cpp"              "AssetCompiler\\Main\\"
                    echo [PATCH] Done.
                '''
            }
        }

        stage('Build') {
            steps {
                bat 'BuildJenkins.bat'
            }
        }

        stage('Stage Game Files') {
            steps {
                bat '''
                    echo [STAGE] Copying Release output to Installer\\GAMEDIRECTORY...
                    if not exist "%GAME_DIR%" mkdir "%GAME_DIR%"
                    robocopy "%RELEASE_DIR%" "%GAME_DIR%" /E /IS /IT /NP /NJH /NJS
                    REM robocopy returns 0-7 for success (8+ = error)
                    if %ERRORLEVEL% GEQ 8 (
                        echo [ERROR] robocopy failed with exit code %ERRORLEVEL%
                        exit /b 1
                    )
                    echo [STAGE] Done.
                    exit /b 0
                '''
            }
        }

        stage('Build Installer') {
            steps {
                bat '''
                    echo [INSTALLER] Checking for Inno Setup...
                    if not exist "%INNO_SETUP%" (
                        echo [ERROR] Inno Setup not found at: %INNO_SETUP%
                        echo [ERROR] Download from: https://jrsoftware.org/isdl.php
                        exit /b 1
                    )
                    echo [INSTALLER] Running ISCC...
                    "%INNO_SETUP%" "Installer\\InstallScript.iss"
                    if %ERRORLEVEL% NEQ 0 (
                        echo [ERROR] ISCC failed with exit code %ERRORLEVEL%
                        exit /b %ERRORLEVEL%
                    )
                    echo [INSTALLER] Installer built successfully.
                '''
            }
        }

        stage('Archive') {
            steps {
                archiveArtifacts artifacts: 'Installer/INSTALLER/*.exe', fingerprint: true
            }
        }
    }

    post {
        success {
            echo 'Installer build complete. Download the artifact from this build page.'
        }
        failure {
            echo 'Build failed. Check the console output above for details.'
        }
    }
}
