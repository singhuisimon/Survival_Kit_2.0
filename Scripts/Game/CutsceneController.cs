// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;

namespace Game
{
    public class CutsceneController : ScriptBehaviour
    {
        [SerializeField]
        private string mainMenuScenePath = "Sources/Scenes/MainMenu.json";

        [SerializeField]
        private float minimumDisplayTime = 1.0f;

        // Entity IDs
        private uint epilepsyImageID;
        private uint pg1ID;
        private uint pg2ID;
        private uint pg3ID;

        // State
        private float elapsedTime = 0.0f;
        private bool canTransition = false;
        private bool cutsceneStarted = false;
        private float cutsceneTime = 0.0f;
        private bool hasLoadedMenu = false;

        public override void OnStart()
        {
            LogMessage("CutsceneController: Started");

            elapsedTime = 0.0f;
            canTransition = false;
            cutsceneStarted = false;
            cutsceneTime = 0.0f;
            hasLoadedMenu = false;

            epilepsyImageID = SceneFindEntityByName("Epilepsy Warning Image");
            pg1ID = SceneFindEntityByName("Cutscene Page 1");
            pg2ID = SceneFindEntityByName("Cutscene Page 2");
            pg3ID = SceneFindEntityByName("Cutscene Page 3");

            LogMessage("CutsceneController: Found entities - Epilepsy: " + epilepsyImageID
                + ", pg1: " + pg1ID + ", pg2: " + pg2ID + ", pg3: " + pg3ID);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (hasLoadedMenu)
                return;

            if (!cutsceneStarted)
            {
                elapsedTime += deltaTime;

                if (!canTransition && elapsedTime >= minimumDisplayTime)
                {
                    canTransition = true;
                    LogMessage("CutsceneController: Ready for input");
                }

                if (canTransition && CheckAnyInput())
                {
                    LogMessage("CutsceneController: Input detected - starting cutscene");
                    cutsceneStarted = true;
                    cutsceneTime = 0.0f;
                }
            }
            else
            {
                cutsceneTime += deltaTime;
                UpdateCutscene();
            }
        }

        private void UpdateCutscene()
        {
            // Epilepsy fade-out: 0s opacity 1 -> 0.45s opacity 0
            if (cutsceneTime <= 0.45f)
            {
                float alpha = 1.0f - (cutsceneTime / 0.45f);
                SpriteRenderer.SetColor(epilepsyImageID, 1.0f, 1.0f, 1.0f, alpha);
            }
            else
            {
                SpriteRenderer.SetColor(epilepsyImageID, 1.0f, 1.0f, 1.0f, 0.0f);
            }

            // pg1: visible until 3.3s, fades to 0 by 4.0s
            if (cutsceneTime <= 3.3f)
            {
                SpriteRenderer.SetColor(pg1ID, 1.0f, 1.0f, 1.0f, 1.0f);
            }
            else if (cutsceneTime <= 4.0f)
            {
                float t = (cutsceneTime - 3.3f) / (4.0f - 3.3f);
                float alpha = 1.0f - t;
                SpriteRenderer.SetColor(pg1ID, 1.0f, 1.0f, 1.0f, alpha);
            }
            else
            {
                SpriteRenderer.SetColor(pg1ID, 1.0f, 1.0f, 1.0f, 0.0f);
            }

            // pg2: visible until 5.3s, snaps to 0 at 5.31s
            if (cutsceneTime <= 5.3f)
            {
                SpriteRenderer.SetColor(pg2ID, 1.0f, 1.0f, 1.0f, 1.0f);
            }
            else
            {
                SpriteRenderer.SetColor(pg2ID, 1.0f, 1.0f, 1.0f, 0.0f);
            }

            // pg3: keep visible and start loading main menu as soon as pg3 appears
            SpriteRenderer.SetColor(pg3ID, 1.0f, 1.0f, 1.0f, 1.0f);
            if (cutsceneTime > 5.81f && !hasLoadedMenu)
            {
                hasLoadedMenu = true;
                // === COMPREHENSIVE DEBUG LOGGING ===
                LogMessage("=== CutsceneController: SCENE TRANSITION DEBUG ===");
                LogMessage("CutsceneController: Current time = " + cutsceneTime.ToString("F2") + "s");
                LogMessage("CutsceneController: Attempting to load scene...");
                LogMessage("CutsceneController: Scene path = '" + mainMenuScenePath + "'");
                LogMessage("CutsceneController: Path length = " + mainMenuScenePath.Length);
                LogMessage("CutsceneController: Loading main menu during pg3");

                if (string.IsNullOrEmpty(mainMenuScenePath))
                {
                    LogError("CutsceneController: ERROR - Scene path is null or empty!");
                    return;
                }
                LogMessage("CutsceneController: Calling Scene.SceneLoadFromFile()...");
                //Publish("LoadScene", mainMenuScenePath);
                bool success = Scene.SceneLoadFromFile(mainMenuScenePath);
                if (success)
                {
                    LogMessage("Main menu loaded successfully!");
                }
                else
                {
                    LogMessage("=== CutsceneController: SCENE NOT LOAD SUCCESS ===");
                }


            }
        }

        private bool CheckAnyInput()
        {
            if (CheckKeyboardInput())
                return true;
            if (CheckMouseInput())
                return true;
            return false;
        }

        private bool CheckKeyboardInput()
        {
            for (int key = (int)KeyCode.A; key <= (int)KeyCode.Z; key++)
            {
                if (Input.IsKeyPressed((KeyCode)key))
                    return true;
            }

            for (int key = (int)KeyCode.D0; key <= (int)KeyCode.D9; key++)
            {
                if (Input.IsKeyPressed((KeyCode)key))
                    return true;
            }

            if (Input.IsKeyPressed(KeyCode.Space))
                return true;
            if (Input.IsKeyPressed(KeyCode.Enter))
                return true;
            if (Input.IsKeyPressed(KeyCode.Escape))
                return true;
            if (Input.IsKeyPressed(KeyCode.Up) ||
                Input.IsKeyPressed(KeyCode.Down) ||
                Input.IsKeyPressed(KeyCode.Left) ||
                Input.IsKeyPressed(KeyCode.Right))
                return true;

            return false;
        }

        private bool CheckMouseInput()
        {
            if (Input.IsMouseButtonPressed(MouseButton.Left))
                return true;
            if (Input.IsMouseButtonPressed(MouseButton.Right))
                return true;
            if (Input.IsMouseButtonPressed(MouseButton.Middle))
                return true;
            return false;
        }

        public override void OnDestroy()
        {
            LogMessage("CutsceneController: Destroyed");
        }
    }
}
