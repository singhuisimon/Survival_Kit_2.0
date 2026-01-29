// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Transform;

namespace Game
{
    /// <summary>
    /// Handles the quit confirmation popup.
    /// Attach this script to the Shutdown Button.
    /// When clicked, shows a popup with Yes/No options.
    /// </summary>
    public class QuitConfirmationPopup : ScriptBehaviour
    {
        // Entity names to find
        [SerializeField]
        private string shutdownButtonEntityName = "Shutdown Button";

        [SerializeField]
        private string popupEntityName = "QuitConfirmation Popup";

        [SerializeField]
        private string yesButtonEntityName = "QuitConfirmation YesButton";

        [SerializeField]
        private string noButtonEntityName = "QuitConfirmation NoButton";

        // Button dimensions for click detection (matches scale in scene)
        [SerializeField]
        private float shutdownButtonWidth = 60.0f;

        [SerializeField]
        private float shutdownButtonHeight = 20.0f;

        [SerializeField]
        private float yesNoButtonWidth = 50.0f;

        [SerializeField]
        private float yesNoButtonHeight = 20.0f;

        [SerializeField]
        private float popupWidth = 150.0f;

        [SerializeField]
        private float popupHeight = 100.0f;

        // Animation settings
        [SerializeField]
        private float fadeInDuration = 0.1f;

        // Positions - initialized in OnStart
        private Vector3 hiddenPosition;
        private Vector3 visiblePopupPosition;
        private Vector3 visibleYesPosition;
        private Vector3 visibleNoPosition;

        // Entity IDs
        private uint shutdownButtonEntityID;
        private uint popupEntityID;
        private uint yesButtonEntityID;
        private uint noButtonEntityID;

        // State
        private bool isPopupVisible = false;
        private bool isAnimating = false;
        private float animationTimer = 0.0f;

        // Input state for edge detection
        private bool wasMousePressed = false;
        private bool wasHovering = false;

        public override void OnStart()
        {
            LogMessage("QuitConfirmationPopup: OnStart called!");
            LogMessage("QuitConfirmationPopup: Script EntityID (may be wrong): " + EntityID);

            // Initialize positions
            hiddenPosition = new Vector3(640.0f, -500.0f, -0.2f);
            visiblePopupPosition = new Vector3(640.0f, 360.0f, -0.2f);
            visibleYesPosition = new Vector3(580.0f, 320.0f, -0.3f);
            visibleNoPosition = new Vector3(700.0f, 320.0f, -0.3f);

            // Find shutdown button by name (workaround for EntityID bug)
            shutdownButtonEntityID = SceneFindEntityByName(shutdownButtonEntityName);
            LogMessage("QuitConfirmationPopup: Shutdown Button EntityID (by name): " + shutdownButtonEntityID);

            // Find popup entities
            popupEntityID = SceneFindEntityByName(popupEntityName);
            yesButtonEntityID = SceneFindEntityByName(yesButtonEntityName);
            noButtonEntityID = SceneFindEntityByName(noButtonEntityName);

            LogMessage("QuitConfirmationPopup: Popup EntityID: " + popupEntityID);
            LogMessage("QuitConfirmationPopup: YesButton EntityID: " + yesButtonEntityID);
            LogMessage("QuitConfirmationPopup: NoButton EntityID: " + noButtonEntityID);

            if (shutdownButtonEntityID == 0)
                LogError("QuitConfirmationPopup: Could not find shutdown button entity: " + shutdownButtonEntityName);
            if (popupEntityID == 0)
                LogError("QuitConfirmationPopup: Could not find popup entity: " + popupEntityName);
            if (yesButtonEntityID == 0)
                LogError("QuitConfirmationPopup: Could not find yes button entity: " + yesButtonEntityName);
            if (noButtonEntityID == 0)
                LogError("QuitConfirmationPopup: Could not find no button entity: " + noButtonEntityName);

            // Log the ACTUAL shutdown button position (found by name)
            Vector3 shutdownPos = GetPosition(shutdownButtonEntityID);
            LogMessage("QuitConfirmationPopup: Shutdown button position: (" + shutdownPos.X + ", " + shutdownPos.Y + ", " + shutdownPos.Z + ")");

            // Ensure popup starts hidden
            HidePopup();

            LogMessage("QuitConfirmationPopup: Ready!");
        }

        public override void OnUpdate(float deltaTime)
        {
            // Handle animation
            if (isAnimating)
            {
                animationTimer += deltaTime;
                float t = animationTimer / fadeInDuration;
                if (t > 1.0f) t = 1.0f;

                // Scale animation (start small, grow to full size)
                float scale = t;
                Vector3 popupScale = new Vector3(popupWidth * scale, popupHeight * scale, 1.0f);
                Vector3 buttonScale = new Vector3(yesNoButtonWidth * scale, yesNoButtonHeight * scale, 1.0f);

                SetScale(popupEntityID, ref popupScale);
                SetScale(yesButtonEntityID, ref buttonScale);
                SetScale(noButtonEntityID, ref buttonScale);

                if (t >= 1.0f)
                {
                    isAnimating = false;
                    LogMessage("QuitConfirmationPopup: Animation complete");
                }
            }

            // Detect mouse click (edge detection)
            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            // Get mouse position and convert to world coordinates
            // Engine uses: mouse.y = viewport_height - mouse.y (flip Y)
            Vector2 rawMouse = Input.GetMousePosition();
            float screenHeight = 720.0f;

            // Flip Y to match engine's world coordinate system
            Vector2 mousePos = new Vector2(rawMouse.X, screenHeight - rawMouse.Y);

            Vector3 shutdownPos = GetPosition(shutdownButtonEntityID);

            // Check hover state (log only when state changes to avoid spam)
            bool isHovering = IsMouseOverButton(mousePos, shutdownPos, shutdownButtonWidth, shutdownButtonHeight);
            if (isHovering && !wasHovering)
            {
                LogMessage("QuitConfirmationPopup: HOVER START on Shutdown Button!");
                LogMessage("  Raw: (" + rawMouse.X + ", " + rawMouse.Y + ") | Converted: (" + mousePos.X + ", " + mousePos.Y + ") | Button: (" + shutdownPos.X + ", " + shutdownPos.Y + ")");
            }
            else if (!isHovering && wasHovering)
            {
                LogMessage("QuitConfirmationPopup: HOVER END");
            }
            wasHovering = isHovering;

            if (!mouseJustPressed)
                return;

            // Debug: Log on click - help map coordinates
            LogMessage("=== CLICK DEBUG ===");
            LogMessage("Raw mouse: (" + rawMouse.X + ", " + rawMouse.Y + ")");
            LogMessage("Converted mouse (Y flipped): (" + mousePos.X + ", " + mousePos.Y + ")");
            LogMessage("Button world pos: (" + shutdownPos.X + ", " + shutdownPos.Y + ")");
            LogMessage("Button bounds: X[" + (shutdownPos.X - shutdownButtonWidth/2) + "-" + (shutdownPos.X + shutdownButtonWidth/2) + "] Y[" + (shutdownPos.Y - shutdownButtonHeight/2) + "-" + (shutdownPos.Y + shutdownButtonHeight/2) + "]");
            LogMessage("Is over button: " + isHovering);
            LogMessage("===================");

            if (isPopupVisible)
            {
                // Check Yes button click
                if (IsMouseOverButton(mousePos, visibleYesPosition, yesNoButtonWidth, yesNoButtonHeight))
                {
                    LogMessage("QuitConfirmationPopup: Yes clicked - quitting game");
                    OnYesClicked();
                    return;
                }

                // Check No button click
                if (IsMouseOverButton(mousePos, visibleNoPosition, yesNoButtonWidth, yesNoButtonHeight))
                {
                    LogMessage("QuitConfirmationPopup: No clicked - hiding popup");
                    OnNoClicked();
                    return;
                }

                // Click outside popup closes it
                if (!IsMouseOverButton(mousePos, visiblePopupPosition, popupWidth, popupHeight))
                {
                    LogMessage("QuitConfirmationPopup: Clicked outside popup - hiding");
                    HidePopup();
                }
            }
            else
            {
                // Check Shutdown button click (this entity) - shutdownPos already fetched above for debug
                if (IsMouseOverButton(mousePos, shutdownPos, shutdownButtonWidth, shutdownButtonHeight))
                {
                    LogMessage("QuitConfirmationPopup: Shutdown button clicked - showing popup");
                    ShowPopup();
                }
            }
        }

        private bool IsMouseOverButton(Vector2 mousePos, Vector3 buttonPos, float width, float height)
        {
            float halfW = width * 0.5f;
            float halfH = height * 0.5f;

            float minX = buttonPos.X - halfW;
            float maxX = buttonPos.X + halfW;
            float minY = buttonPos.Y - halfH;
            float maxY = buttonPos.Y + halfH;

            return mousePos.X >= minX && mousePos.X <= maxX &&
                   mousePos.Y >= minY && mousePos.Y <= maxY;
        }

        private void ShowPopup()
        {
            if (isPopupVisible)
                return;

            isPopupVisible = true;
            isAnimating = true;
            animationTimer = 0.0f;

            // Move entities to visible positions
            Vector3 popupPos = visiblePopupPosition;
            Vector3 yesPos = visibleYesPosition;
            Vector3 noPos = visibleNoPosition;

            SetPosition(popupEntityID, ref popupPos);
            SetPosition(yesButtonEntityID, ref yesPos);
            SetPosition(noButtonEntityID, ref noPos);

            // Start with small scale for animation
            Vector3 smallScale = new Vector3(0.1f, 0.1f, 1.0f);
            SetScale(popupEntityID, ref smallScale);
            SetScale(yesButtonEntityID, ref smallScale);
            SetScale(noButtonEntityID, ref smallScale);

            LogMessage("QuitConfirmationPopup: Popup shown");
        }

        private void HidePopup()
        {
            isPopupVisible = false;
            isAnimating = false;

            // Move entities off-screen
            Vector3 hidePos = hiddenPosition;
            Vector3 yesHidePos = new Vector3(580.0f, -500.0f, -0.3f);
            Vector3 noHidePos = new Vector3(700.0f, -500.0f, -0.3f);

            SetPosition(popupEntityID, ref hidePos);
            SetPosition(yesButtonEntityID, ref yesHidePos);
            SetPosition(noButtonEntityID, ref noHidePos);

            // Reset scale
            Vector3 popupScale = new Vector3(popupWidth, popupHeight, 1.0f);
            Vector3 buttonScale = new Vector3(yesNoButtonWidth, yesNoButtonHeight, 1.0f);

            SetScale(popupEntityID, ref popupScale);
            SetScale(yesButtonEntityID, ref buttonScale);
            SetScale(noButtonEntityID, ref buttonScale);

            LogMessage("QuitConfirmationPopup: Popup hidden");
        }

        private void OnYesClicked()
        {
            // Publish event to quit the game
            Publish("QuitGame", "");
            LogMessage("QuitConfirmationPopup: QuitGame event published");
        }

        private void OnNoClicked()
        {
            HidePopup();
        }

        public override void OnDestroy()
        {
            LogMessage("QuitConfirmationPopup: Destroyed");
        }
    }
}
