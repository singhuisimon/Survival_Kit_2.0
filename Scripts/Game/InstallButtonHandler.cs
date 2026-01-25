// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;

namespace Game
{
    /// <summary>
    /// Handles the Install button click to transition to Level1_Player scene.
    /// </summary>
    public class InstallButtonHandler : ScriptBehaviour
    {
        private const string INSTALL_BUTTON_NAME = "Install Button";
        private const string TARGET_SCENE_PATH = "Resources/Sources/Scenes/Level1_Player.json";

        private uint installButtonId;
        private bool entityFound = false;
        private bool wasMousePressed = false;

        public override void OnStart()
        {
            LogMessage("InstallButtonHandler: Initializing...");

            installButtonId = SceneFindEntityByName(INSTALL_BUTTON_NAME);

            if (installButtonId == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + INSTALL_BUTTON_NAME);
                return;
            }

            entityFound = true;
            wasMousePressed = false;

            LogMessage("InstallButtonHandler: Ready! Install Button ID: " + installButtonId);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entityFound)
                return;

            // Edge detection for mouse click
            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (mouseJustPressed)
            {
                if (Collision2D.IsMouseCollidingWithEntity(installButtonId))
                {
                    LogMessage("InstallButtonHandler: Install button clicked - transitioning to Level1_Player");
                    TransitionToLevel();
                }
            }
        }

        private void TransitionToLevel()
        {
            LogMessage("===========================================");
            LogMessage("TRANSITIONING TO LEVEL 1");
            LogMessage("Scene path: " + TARGET_SCENE_PATH);
            LogMessage("===========================================");

            Publish("LoadScene", TARGET_SCENE_PATH);
        }

        public override void OnDestroy()
        {
            LogMessage("InstallButtonHandler: Destroyed");
        }
    }
}
