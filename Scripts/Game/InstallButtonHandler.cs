// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Transform;

namespace Game
{
    public class InstallButtonHandler : ScriptBehaviour
    {
        private const string INSTALL_BUTTON_NAME = "Install Button";
        private const int NUM_ERROR_POPUPS = 8;

        private const float HIDDEN_Y = -500.0f;

        // Visible positions for each popup (you can update these later)
        private static readonly float[] POPUP_X = { 300.0f, 400.0f, 500.0f, 600.0f, 700.0f, 800.0f, 900.0f, 1000.0f };
        private static readonly float[] POPUP_Y = { 360.0f, 360.0f, 360.0f, 360.0f, 360.0f, 360.0f, 360.0f, 360.0f };

        private uint installButtonId;
        private uint[] errorPopupIds = new uint[NUM_ERROR_POPUPS];

        private bool entityFound = false;
        private bool wasMousePressed = false;
        private bool popupsShown = false;

        public override void OnStart()
        {
            LogMessage("InstallButtonHandler: Initializing...");

            installButtonId = SceneFindEntityByName(INSTALL_BUTTON_NAME);

            if (installButtonId == 0)
            {
                LogError("InstallButtonHandler: Could not find entity: " + INSTALL_BUTTON_NAME);
                return;
            }

            for (int i = 0; i < NUM_ERROR_POPUPS; i++)
            {
                string name = "Error Popup " + (i + 1);
                errorPopupIds[i] = SceneFindEntityByName(name);

                if (errorPopupIds[i] == 0)
                {
                    LogError("InstallButtonHandler: Could not find entity: " + name);
                    return;
                }
            }

            entityFound = true;
            wasMousePressed = false;

            LogMessage("InstallButtonHandler: Ready! Install Button ID: " + installButtonId);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entityFound)
                return;

            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (mouseJustPressed)
            {
                if (Collision2D.IsMouseCollidingWithEntity(installButtonId))
                {
                    LogMessage("InstallButtonHandler: Install button clicked - showing error popups");
                    ShowErrorPopups();
                }
            }
        }

        private void ShowErrorPopups()
        {
            popupsShown = true;

            for (int i = 0; i < NUM_ERROR_POPUPS; i++)
            {
                Vector3 pos = new Vector3(POPUP_X[i], POPUP_Y[i], -0.7f - i * 0.01f);
                SetPosition(errorPopupIds[i], ref pos);
                LogMessage("InstallButtonHandler: Error Popup " + (i + 1) + " shown at (" + POPUP_X[i] + ", " + POPUP_Y[i] + ")");
            }
        }

        public override void OnDestroy()
        {
            LogMessage("InstallButtonHandler: Destroyed");
        }
    }
}
