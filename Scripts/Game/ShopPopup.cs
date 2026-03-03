using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.SpriteRenderer;

namespace Game
{
    /// <summary>
    /// Handles the shop popup in the main menu.
    /// Two tabs: Packs (default) and ByteChips.
    /// Packs tab: 3 Exchange buttons + 1 Equipped button
    /// ByteChips tab: 4 Exchange buttons (Equipped becomes Exchange)
    /// </summary>
    public class ShopPopup : ScriptBehaviour
    {
        // Entity names
        private const string SHOP_BUTTON_NAME = "Shop Button";
        private const string SHOP_PACKS_POPUP_NAME = "Shop Popup";
        private const string SHOP_BYTECHIPS_POPUP_NAME = "Shop ByteChips Popup";
        private const string SHOP_CLOSE_BUTTON_NAME = "Shop Close Button";
        private const string PACKS_BUTTON_NAME = "Shop Packs Button";
        private const string BYTECHIPS_BUTTON_NAME = "Shop ByteChips Button";
        private const string EXCHANGE_BUTTON_1_NAME = "Shop Exchange Button 1";
        private const string EXCHANGE_BUTTON_2_NAME = "Shop Exchange Button 2";
        private const string EXCHANGE_BUTTON_3_NAME = "Shop Exchange Button 3";
        private const string EXCHANGE_BUTTON_4_NAME = "Shop Exchange Button 4";
        private const string EQUIPPED_BUTTON_NAME = "Shop Equipped Button";

        // Positions
        private const float HIDDEN_Y = -500.0f;
        private const float CENTER_X = 640.0f;

        // Entity IDs
        private uint shopButtonId;
        private uint packsPopupId;
        private uint byteChipsPopupId;
        private uint closeButtonId;
        private uint packsButtonId;
        private uint byteChipsButtonId;
        private uint exchangeButton1Id;
        private uint exchangeButton2Id;
        private uint exchangeButton3Id;
        private uint exchangeButton4Id;
        private uint equippedButtonId;

        // Visible positions
        private Vector3 popupVisiblePos = new Vector3(640.0f, 360.0f, -0.5f);
        private Vector3 closeButtonVisiblePos = new Vector3(1115.0f, 112.0f, -0.6f);
        private Vector3 packsButtonVisiblePos = new Vector3(192.0f, 147.0f, -0.6f);
        private Vector3 byteChipsButtonVisiblePos = new Vector3(254.0f, 147.0f, -0.6f);
        private Vector3 exchangeButton1VisiblePos = new Vector3(573.0f, 550.0f, -0.6f);
        private Vector3 exchangeButton2VisiblePos = new Vector3(815.0f, 550.0f, -0.6f);
        private Vector3 exchangeButton3VisiblePos = new Vector3(1057.0f, 550.0f, -0.6f);
        private Vector3 exchangeButton4VisiblePos = new Vector3(330.0f, 550.0f, -0.6f);
        private Vector3 equippedButtonVisiblePos = new Vector3(330.0f, 550.0f, -0.6f);

        // Event names for popup coordination
        private const string EVENT_POPUP_OPENED = "MainMenuPopupOpened";
        private const string EVENT_POPUP_CLOSED = "MainMenuPopupClosed";
        private const string POPUP_ID = "Shop";

        // State
        private bool isPopupVisible = false;
        private bool entitiesFound = false;
        private bool wasMousePressed = false;
        private bool isPacksTab = true; // true = Packs tab, false = ByteChips tab

        public override void OnStart()
        {
            LogMessage("ShopPopup: Initializing...");

            shopButtonId = SceneFindEntityByName(SHOP_BUTTON_NAME);
            if (shopButtonId == 0)
            {
                LogError("ShopPopup: Could not find entity: " + SHOP_BUTTON_NAME);
                return;
            }

            packsPopupId = SceneFindEntityByName(SHOP_PACKS_POPUP_NAME);
            byteChipsPopupId = SceneFindEntityByName(SHOP_BYTECHIPS_POPUP_NAME);
            closeButtonId = SceneFindEntityByName(SHOP_CLOSE_BUTTON_NAME);
            packsButtonId = SceneFindEntityByName(PACKS_BUTTON_NAME);
            byteChipsButtonId = SceneFindEntityByName(BYTECHIPS_BUTTON_NAME);
            exchangeButton1Id = SceneFindEntityByName(EXCHANGE_BUTTON_1_NAME);
            exchangeButton2Id = SceneFindEntityByName(EXCHANGE_BUTTON_2_NAME);
            exchangeButton3Id = SceneFindEntityByName(EXCHANGE_BUTTON_3_NAME);
            exchangeButton4Id = SceneFindEntityByName(EXCHANGE_BUTTON_4_NAME);
            equippedButtonId = SceneFindEntityByName(EQUIPPED_BUTTON_NAME);

            if (packsPopupId == 0) LogError("ShopPopup: Could not find: " + SHOP_PACKS_POPUP_NAME);
            if (byteChipsPopupId == 0) LogError("ShopPopup: Could not find: " + SHOP_BYTECHIPS_POPUP_NAME);
            if (closeButtonId == 0) LogError("ShopPopup: Could not find: " + SHOP_CLOSE_BUTTON_NAME);

            entitiesFound = true;
            isPopupVisible = false;
            wasMousePressed = false;
            isPacksTab = true;

            // Subscribe to popup coordination events
            Event.Subscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);

            // Hide all popup elements initially
            HidePopup();

            LogMessage("ShopPopup: Ready!");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound)
                return;

            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (mouseJustPressed)
            {
                HandleMouseClick();
            }
        }

        private void HandleMouseClick()
        {
            if (isPopupVisible)
            {
                // Check close button
                if (closeButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(closeButtonId))
                {
                    LogMessage("ShopPopup: Close button clicked");
                    HidePopup();
                    return;
                }

                // Check Packs tab button
                if (!isPacksTab && packsButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(packsButtonId))
                {
                    LogMessage("ShopPopup: Packs tab clicked");
                    SwitchToPacksTab();
                    return;
                }

                // Check ByteChips tab button
                if (isPacksTab && byteChipsButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(byteChipsButtonId))
                {
                    LogMessage("ShopPopup: ByteChips tab clicked");
                    SwitchToByteChipsTab();
                    return;
                }

                // Check if clicked outside popup to close
                bool overPacksPopup = packsPopupId != 0 && Collision2D.IsMouseCollidingWithEntity(packsPopupId);
                bool overByteChipsPopup = byteChipsPopupId != 0 && Collision2D.IsMouseCollidingWithEntity(byteChipsPopupId);
                if (!overPacksPopup && !overByteChipsPopup)
                {
                    LogMessage("ShopPopup: Clicked outside - closing");
                    HidePopup();
                }
            }
            else
            {
                // Check shop button
                if (Collision2D.IsMouseCollidingWithEntity(shopButtonId))
                {
                    LogMessage("ShopPopup: Shop button clicked - showing popup");
                    ShowPopup();
                }
            }
        }

        private void OnOtherPopupOpened(string eventName, string payload)
        {
            if (payload != POPUP_ID && isPopupVisible)
            {
                LogMessage("ShopPopup: Another popup opened (" + payload + ") - closing shop");
                HidePopup();
            }
        }

        private void ShowPopup()
        {
            if (isPopupVisible)
                return;

            isPopupVisible = true;
            isPacksTab = true;
            Event.Publish(EVENT_POPUP_OPENED, POPUP_ID);

            // Show shared elements
            if (closeButtonId != 0) SetPosition(closeButtonId, ref closeButtonVisiblePos);
            if (packsButtonId != 0) SetPosition(packsButtonId, ref packsButtonVisiblePos);
            if (byteChipsButtonId != 0) SetPosition(byteChipsButtonId, ref byteChipsButtonVisiblePos);
            if (exchangeButton1Id != 0) SetPosition(exchangeButton1Id, ref exchangeButton1VisiblePos);
            if (exchangeButton2Id != 0) SetPosition(exchangeButton2Id, ref exchangeButton2VisiblePos);
            if (exchangeButton3Id != 0) SetPosition(exchangeButton3Id, ref exchangeButton3VisiblePos);

            // Default to Packs tab
            ShowPacksTab();

            LogMessage("ShopPopup: Popup shown (Packs tab)");
        }

        private void SwitchToPacksTab()
        {
            isPacksTab = true;
            ShowPacksTab();
            LogMessage("ShopPopup: Switched to Packs tab");
        }

        private void SwitchToByteChipsTab()
        {
            isPacksTab = false;
            ShowByteChipsTab();
            LogMessage("ShopPopup: Switched to ByteChips tab");
        }

        private void ShowPacksTab()
        {
            Vector3 hidePos = new Vector3(CENTER_X, HIDDEN_Y, -0.5f);
            Vector3 hidePos2 = new Vector3(CENTER_X, HIDDEN_Y, -0.6f);

            // Show Packs window, hide ByteChips window
            if (packsPopupId != 0) SetPosition(packsPopupId, ref popupVisiblePos);
            if (byteChipsPopupId != 0) SetPosition(byteChipsPopupId, ref hidePos);

            // Show Equipped button, hide Exchange Button 4
            if (equippedButtonId != 0) SetPosition(equippedButtonId, ref equippedButtonVisiblePos);
            if (exchangeButton4Id != 0) SetPosition(exchangeButton4Id, ref hidePos2);
        }

        private void ShowByteChipsTab()
        {
            Vector3 hidePos = new Vector3(CENTER_X, HIDDEN_Y, -0.5f);
            Vector3 hidePos2 = new Vector3(CENTER_X, HIDDEN_Y, -0.6f);

            // Show ByteChips window, hide Packs window
            if (byteChipsPopupId != 0) SetPosition(byteChipsPopupId, ref popupVisiblePos);
            if (packsPopupId != 0) SetPosition(packsPopupId, ref hidePos);

            // Show Exchange Button 4, hide Equipped button
            if (exchangeButton4Id != 0) SetPosition(exchangeButton4Id, ref exchangeButton4VisiblePos);
            if (equippedButtonId != 0) SetPosition(equippedButtonId, ref hidePos2);
        }

        private void HidePopup()
        {
            bool wasVisible = isPopupVisible;
            isPopupVisible = false;
            if (wasVisible)
                Event.Publish(EVENT_POPUP_CLOSED, POPUP_ID);

            Vector3 hidePos = new Vector3(CENTER_X, HIDDEN_Y, -0.5f);
            Vector3 hidePos2 = new Vector3(CENTER_X, HIDDEN_Y, -0.6f);

            if (packsPopupId != 0) SetPosition(packsPopupId, ref hidePos);
            if (byteChipsPopupId != 0) SetPosition(byteChipsPopupId, ref hidePos);
            if (closeButtonId != 0) SetPosition(closeButtonId, ref hidePos2);
            if (packsButtonId != 0) SetPosition(packsButtonId, ref hidePos2);
            if (byteChipsButtonId != 0) SetPosition(byteChipsButtonId, ref hidePos2);
            if (exchangeButton1Id != 0) SetPosition(exchangeButton1Id, ref hidePos2);
            if (exchangeButton2Id != 0) SetPosition(exchangeButton2Id, ref hidePos2);
            if (exchangeButton3Id != 0) SetPosition(exchangeButton3Id, ref hidePos2);
            if (exchangeButton4Id != 0) SetPosition(exchangeButton4Id, ref hidePos2);
            if (equippedButtonId != 0) SetPosition(equippedButtonId, ref hidePos2);

            LogMessage("ShopPopup: Popup hidden");
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);
            LogMessage("ShopPopup: Destroyed");
        }
    }
}
