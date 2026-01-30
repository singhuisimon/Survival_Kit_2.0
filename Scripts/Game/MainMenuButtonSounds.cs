// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using System;
using static Engine.Scene;
using static Engine.Audio;
using static Engine.Logger;

namespace Game
{
    /// <summary>
    /// Handles click sound effects for all buttons in the main menu.
    /// Plays UI sounds when clicking buttons.
    /// </summary>
    public class MainMenuButtonSounds : ScriptBehaviour
    {
        // Entity names for audio sources
        private const string CLICK_AUDIO_ENTITY = "UI Click Sound";

        // Button entity names to track
        private static readonly string[] BUTTON_NAMES = {
            "Shutdown Button",
            "Settings Button",
            "HowToPlay Button",
            "Credits Button",
            "Install Button",
            "Highscore Button",
            "Quit Yes Button",
            "Quit No Button",
            "Credits Close Button",
            "Credits Close Button 2",
            "Start Game Yes Button"
        };

        // Audio entity IDs
        private uint clickAudioId;

        // Button entity IDs
        private uint[] buttonIds;

        // State
        private bool entitiesFound = false;
        private bool wasMousePressed = false;

        public override void OnStart()
        {
            LogMessage("MainMenuButtonSounds: Initializing...");

            // Find audio entity
            clickAudioId = SceneFindEntityByName(CLICK_AUDIO_ENTITY);

            if (clickAudioId == 0)
            {
                LogError("MainMenuButtonSounds: Could not find entity: " + CLICK_AUDIO_ENTITY);
                return;
            }

            // Find all button entities
            buttonIds = new uint[BUTTON_NAMES.Length];

            for (int i = 0; i < BUTTON_NAMES.Length; i++)
            {
                buttonIds[i] = SceneFindEntityByName(BUTTON_NAMES[i]);

                if (buttonIds[i] == 0)
                {
                    LogMessage("MainMenuButtonSounds: Button not found (may be optional): " + BUTTON_NAMES[i]);
                }
            }

            entitiesFound = true;
            wasMousePressed = false;

            LogMessage("MainMenuButtonSounds: Ready!");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound)
                return;

            // Check mouse button state
            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            // Check each button for click
            if (mouseJustPressed)
            {
                for (int i = 0; i < buttonIds.Length; i++)
                {
                    if (buttonIds[i] == 0)
                        continue;

                    bool isHovering = Collision2D.IsMouseCollidingWithEntity(buttonIds[i]);

                    // Play click sound when clicking a button
                    if (isHovering)
                    {
                        PlayClickSound();
                        break; // Only play once per click
                    }
                }
            }
        }

        private void PlayClickSound()
        {
            if (clickAudioId != 0)
            {
                AudioStop(clickAudioId);
                AudioPlay(clickAudioId);
            }
        }

        public override void OnDestroy()
        {
            LogMessage("MainMenuButtonSounds: Destroyed");
        }
    }
}
