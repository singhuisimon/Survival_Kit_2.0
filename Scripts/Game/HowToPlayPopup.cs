using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// Controls the HowToPlay popup in the main menu.
    /// - Clicking HowToPlay Button opens the popup, defaulting to the General tab.
    /// - Clicking each tab button swaps the popup content and updates selected state.
    /// - Clicking the close button hides everything.
    /// </summary>
    public class HowToPlayPopup : ScriptBehaviour
    {
        // Entity names
        private const string HOWTOPLAY_BUTTON_NAME = "HowToPlay Button";
        private const string CLOSE_BUTTON_NAME = "HowToPlayCrossButton";

        private const string GENERAL_TAB_BUTTON_NAME = "HowToPlayGeneralTabButton";
        private const string MOVEMENT_TAB_BUTTON_NAME = "HowToPlayMovementTabButton";
        private const string COMBAT_TAB_BUTTON_NAME = "HowToPlayCombatTabButton";
        private const string ENEMIES_TAB_BUTTON_NAME = "HowToPlayEnemiesTabButton";

        private const string GENERAL_TAB_BUTTON_SELECTED_NAME = "HowToPlayGeneralTabButtonSelected";
        private const string MOVEMENT_TAB_BUTTON_SELECTED_NAME = "HowToPlayMovementTabButtonSelected";
        private const string COMBAT_TAB_BUTTON_SELECTED_NAME = "HowToPlayCombatTabButtonSelected";
        private const string ENEMIES_TAB_BUTTON_SELECTED_NAME = "HowToPlayEnemiesTabButtonSelected";

        private const string GENERAL_POPUP_NAME = "HowToPlayGeneralPopUp";
        private const string MOVEMENT_POPUP_NAME = "HowToPlayMovementPopUp";
        private const string COMBAT_POPUP_NAME = "HowToPlayCombatPopUp";
        private const string ENEMIES_POPUP_NAME = "HowToPlayEnemiesPopUp";

        // Event coordination
        private const string EVENT_POPUP_OPENED = "MainMenuPopupOpened";
        private const string EVENT_POPUP_CLOSED = "MainMenuPopupClosed";
        private const string POPUP_ID = "HowToPlay";

        // Entity IDs
        private uint howToPlayButtonId;
        private uint closeButtonId;

        private uint generalTabButtonId;
        private uint movementTabButtonId;
        private uint combatTabButtonId;
        private uint enemiesTabButtonId;

        private uint generalTabButtonSelectedId;
        private uint movementTabButtonSelectedId;
        private uint combatTabButtonSelectedId;
        private uint enemiesTabButtonSelectedId;

        private uint generalPopupId;
        private uint movementPopupId;
        private uint combatPopupId;
        private uint enemiesPopupId;

        // State
        private bool entitiesFound = false;
        private bool wasMousePressed = false;
        private bool isPopupVisible = false;

        private enum Tab { General, Movement, Combat, Enemies }
        private Tab activeTab = Tab.General;

        public override void OnStart()
        {
            LogMessage("HowToPlayPopup: Initializing...");

            howToPlayButtonId = SceneFindEntityByName(HOWTOPLAY_BUTTON_NAME);
            closeButtonId = SceneFindEntityByName(CLOSE_BUTTON_NAME);

            generalTabButtonId = SceneFindEntityByName(GENERAL_TAB_BUTTON_NAME);
            movementTabButtonId = SceneFindEntityByName(MOVEMENT_TAB_BUTTON_NAME);
            combatTabButtonId = SceneFindEntityByName(COMBAT_TAB_BUTTON_NAME);
            enemiesTabButtonId = SceneFindEntityByName(ENEMIES_TAB_BUTTON_NAME);

            generalTabButtonSelectedId = SceneFindEntityByName(GENERAL_TAB_BUTTON_SELECTED_NAME);
            movementTabButtonSelectedId = SceneFindEntityByName(MOVEMENT_TAB_BUTTON_SELECTED_NAME);
            combatTabButtonSelectedId = SceneFindEntityByName(COMBAT_TAB_BUTTON_SELECTED_NAME);
            enemiesTabButtonSelectedId = SceneFindEntityByName(ENEMIES_TAB_BUTTON_SELECTED_NAME);

            generalPopupId = SceneFindEntityByName(GENERAL_POPUP_NAME);
            movementPopupId = SceneFindEntityByName(MOVEMENT_POPUP_NAME);
            combatPopupId = SceneFindEntityByName(COMBAT_POPUP_NAME);
            enemiesPopupId = SceneFindEntityByName(ENEMIES_POPUP_NAME);

            if (howToPlayButtonId == 0) LogMessage("HowToPlayPopup: HowToPlay Button not found");
            if (closeButtonId == 0) LogMessage("HowToPlayPopup: HowToPlayCrossButton not found");

            entitiesFound = true;
            wasMousePressed = false;
            isPopupVisible = false;

            HidePopup();

            Event.Subscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);
            LogMessage("HowToPlayPopup: Ready!");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound) return;

            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (mouseJustPressed) HandleMouseClick();
        }

        private void HandleMouseClick()
        {
            if (!isPopupVisible)
            {
                // Open popup when HowToPlay button is clicked
                if (howToPlayButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(howToPlayButtonId))
                {
                    LogMessage("HowToPlayPopup: HowToPlay button clicked - showing popup");
                    ShowPopup();
                    return;
                }
            }
            else
            {
                // Close button
                if (closeButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(closeButtonId))
                {
                    LogMessage("HowToPlayPopup: Close button clicked - hiding popup");
                    HidePopup();
                    return;
                }

                // Tab buttons
                if (generalTabButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(generalTabButtonId) ||
                    generalTabButtonSelectedId != 0 && Collision2D.IsMouseCollidingWithEntity(generalTabButtonSelectedId))
                {
                    LogMessage("HowToPlayPopup: General tab clicked");
                    SwitchTab(Tab.General);
                    return;
                }

                if (movementTabButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(movementTabButtonId) ||
                    movementTabButtonSelectedId != 0 && Collision2D.IsMouseCollidingWithEntity(movementTabButtonSelectedId))
                {
                    LogMessage("HowToPlayPopup: Movement tab clicked");
                    SwitchTab(Tab.Movement);
                    return;
                }

                if (combatTabButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(combatTabButtonId) ||
                    combatTabButtonSelectedId != 0 && Collision2D.IsMouseCollidingWithEntity(combatTabButtonSelectedId))
                {
                    LogMessage("HowToPlayPopup: Combat tab clicked");
                    SwitchTab(Tab.Combat);
                    return;
                }

                if (enemiesTabButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(enemiesTabButtonId) ||
                    enemiesTabButtonSelectedId != 0 && Collision2D.IsMouseCollidingWithEntity(enemiesTabButtonSelectedId))
                {
                    LogMessage("HowToPlayPopup: Enemies tab clicked");
                    SwitchTab(Tab.Enemies);
                    return;
                }
            }
        }

        private void ShowPopup()
        {
            isPopupVisible = true;
            Event.Publish(EVENT_POPUP_OPENED, POPUP_ID);

            // Show close button and all tab buttons
            if (closeButtonId != 0) SetIsVisible(closeButtonId, true);
            if (generalTabButtonId != 0) SetIsVisible(generalTabButtonId, true);
            if (movementTabButtonId != 0) SetIsVisible(movementTabButtonId, true);
            if (combatTabButtonId != 0) SetIsVisible(combatTabButtonId, true);
            if (enemiesTabButtonId != 0) SetIsVisible(enemiesTabButtonId, true);
            if (generalTabButtonSelectedId != 0) SetIsVisible(generalTabButtonSelectedId, true);
            if (movementTabButtonSelectedId != 0) SetIsVisible(movementTabButtonSelectedId, true);
            if (combatTabButtonSelectedId != 0) SetIsVisible(combatTabButtonSelectedId, true);
            if (enemiesTabButtonSelectedId != 0) SetIsVisible(enemiesTabButtonSelectedId, true);

            // Default to General tab
            SwitchTab(Tab.General);
            LogMessage("HowToPlayPopup: Popup shown");
        }

        private void HidePopup()
        {
            bool wasVisible = isPopupVisible;
            isPopupVisible = false;

            if (closeButtonId != 0) SetIsVisible(closeButtonId, false);

            if (generalTabButtonId != 0) SetIsVisible(generalTabButtonId, false);
            if (movementTabButtonId != 0) SetIsVisible(movementTabButtonId, false);
            if (combatTabButtonId != 0) SetIsVisible(combatTabButtonId, false);
            if (enemiesTabButtonId != 0) SetIsVisible(enemiesTabButtonId, false);

            if (generalTabButtonSelectedId != 0) SetIsVisible(generalTabButtonSelectedId, false);
            if (movementTabButtonSelectedId != 0) SetIsVisible(movementTabButtonSelectedId, false);
            if (combatTabButtonSelectedId != 0) SetIsVisible(combatTabButtonSelectedId, false);
            if (enemiesTabButtonSelectedId != 0) SetIsVisible(enemiesTabButtonSelectedId, false);

            if (generalPopupId != 0) SetIsVisible(generalPopupId, false);
            if (movementPopupId != 0) SetIsVisible(movementPopupId, false);
            if (combatPopupId != 0) SetIsVisible(combatPopupId, false);
            if (enemiesPopupId != 0) SetIsVisible(enemiesPopupId, false);

            if (wasVisible) Event.Publish(EVENT_POPUP_CLOSED, POPUP_ID);
            LogMessage("HowToPlayPopup: Popup hidden");
        }

        private void SwitchTab(Tab tab)
        {
            activeTab = tab;

            // Show only the active popup window
            if (generalPopupId != 0) SetIsVisible(generalPopupId, tab == Tab.General);
            if (movementPopupId != 0) SetIsVisible(movementPopupId, tab == Tab.Movement);
            if (combatPopupId != 0) SetIsVisible(combatPopupId, tab == Tab.Combat);
            if (enemiesPopupId != 0) SetIsVisible(enemiesPopupId, tab == Tab.Enemies);

            // Swap normal/selected tab buttons using alpha
            if (generalTabButtonId != 0) SetColor(generalTabButtonId, 1.0f, 1.0f, 1.0f, tab == Tab.General ? 0.0f : 1.0f);
            if (generalTabButtonSelectedId != 0) SetColor(generalTabButtonSelectedId, 1.0f, 1.0f, 1.0f, tab == Tab.General ? 1.0f : 0.0f);
            if (movementTabButtonId != 0) SetColor(movementTabButtonId, 1.0f, 1.0f, 1.0f, tab == Tab.Movement ? 0.0f : 1.0f);
            if (movementTabButtonSelectedId != 0) SetColor(movementTabButtonSelectedId, 1.0f, 1.0f, 1.0f, tab == Tab.Movement ? 1.0f : 0.0f);
            if (combatTabButtonId != 0) SetColor(combatTabButtonId, 1.0f, 1.0f, 1.0f, tab == Tab.Combat ? 0.0f : 1.0f);
            if (combatTabButtonSelectedId != 0) SetColor(combatTabButtonSelectedId, 1.0f, 1.0f, 1.0f, tab == Tab.Combat ? 1.0f : 0.0f);
            if (enemiesTabButtonId != 0) SetColor(enemiesTabButtonId, 1.0f, 1.0f, 1.0f, tab == Tab.Enemies ? 0.0f : 1.0f);
            if (enemiesTabButtonSelectedId != 0) SetColor(enemiesTabButtonSelectedId, 1.0f, 1.0f, 1.0f, tab == Tab.Enemies ? 1.0f : 0.0f);

            LogMessage("HowToPlayPopup: Switched to tab " + tab.ToString());
        }

        private void OnOtherPopupOpened(string eventName, string payload)
        {
            if (payload != POPUP_ID && isPopupVisible)
            {
                LogMessage("HowToPlayPopup: Another popup opened - closing HowToPlay");
                HidePopup();
            }
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);
            LogMessage("HowToPlayPopup: Destroyed");
        }
    }
}
