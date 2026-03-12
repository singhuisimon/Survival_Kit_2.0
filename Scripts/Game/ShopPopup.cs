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
    /// Packs tab: Skin slots with Exchange/Equip/Equipped button states.
    /// ByteChips tab: Exchange buttons, clicking shows Label_PackOwned.
    /// </summary>
    public class ShopPopup : ScriptBehaviour
    {
        // Entity names
        private const string SHOP_BUTTON_NAME = "Shop Button";
        private const string SHOP_PACKS_POPUP_NAME = "Shop Popup";
        private const string SHOP_BYTECHIPS_POPUP_NAME = "Shop ByteChips Popup";
        private const string SHOP_CLOSE_BUTTON_NAME = "Shop Close Button";
        private const string PACKS_BUTTON_NAME = "Shop Packs Button";
        private const string PACKS_BUTTON_SELECTED_NAME = "Shop Packs Button Selected";
        private const string BYTECHIPS_BUTTON_NAME = "Shop ByteChips Button";
        private const string BYTECHIPS_BUTTON_SELECTED_NAME = "Shop ByteChips Button Selected";
        private const string EXCHANGE_BUTTON_1_NAME = "Shop Exchange Button 1";
        private const string EXCHANGE_BUTTON_2_NAME = "Shop Exchange Button 2";
        private const string EXCHANGE_BUTTON_3_NAME = "Shop Exchange Button 3";
        private const string EXCHANGE_BUTTON_4_NAME = "Shop Exchange Button 4";
        private const string EQUIPPED_BUTTON_NAME = "Shop Equipped Button";
        private const string EQUIP_BUTTON_1_NAME = "Shop Equip Button 1";
        private const string EQUIPPED_BUTTON_1_NAME = "Shop Equipped Button 1";
        private const string EQUIP_BUTTON_2_NAME = "Shop Equip Button 2";
        private const string EQUIPPED_BUTTON_2_NAME = "Shop Equipped Button 2";
        private const string EQUIP_BUTTON_3_NAME = "Shop Equip Button 3";
        private const string EQUIPPED_BUTTON_3_NAME = "Shop Equipped Button 3";
        private const string EQUIP_BUTTON_DEFAULT_NAME = "Shop Equip Button Default";
        private const string LABEL_PACK_OWNED_1_NAME = "Label_PackOwned_1";
        private const string LABEL_PACK_OWNED_2_NAME = "Label_PackOwned_2";
        private const string LABEL_PACK_OWNED_3_NAME = "Label_PackOwned_3";
        private const string LABEL_PACK_OWNED_4_NAME = "Label_PackOwned_4";
        private const string SCORE_TEXT_1_NAME = "Shop Score Text 1";
        private const string SCORE_TEXT_2_NAME = "Shop Score Text 2";

        // Skin costs
        private const int SKIN_COST = 10000;          // Skin 1 & 2 (research points)
        private const int SKIN3_BYTECHIP_COST = 50;   // Skin 3 (ByteChips)

        // Positions
        private const float HIDDEN_Y = -500.0f;
        private const float CENTER_X = 640.0f;

        // Entity IDs
        private uint shopButtonId;
        private uint packsPopupId;
        private uint byteChipsPopupId;
        private uint closeButtonId;
        private uint packsButtonId;
        private uint packsButtonSelectedId;
        private uint byteChipsButtonId;
        private uint byteChipsButtonSelectedId;
        private uint exchangeButton1Id;
        private uint exchangeButton2Id;
        private uint exchangeButton3Id;
        private uint exchangeButton4Id;
        private uint equippedButtonId;       // Default skin - Equipped state
        private uint equipButton1Id;         // Skin 1 - Equip state
        private uint equippedButton1Id;      // Skin 1 - Equipped state
        private uint equipButton2Id;         // Skin 2 - Equip state
        private uint equippedButton2Id;      // Skin 2 - Equipped state
        private uint equipButton3Id;         // Skin 3 - Equip state
        private uint equippedButton3Id;      // Skin 3 - Equipped state
        private uint equipButtonDefaultId;   // Default skin - Equip state
        private uint labelPackOwned1Id;
        private uint labelPackOwned2Id;
        private uint labelPackOwned3Id;
        private uint labelPackOwned4Id;
        private uint scoreText1Id;
        private uint scoreText2Id;

        // Visible positions
        private Vector3 popupVisiblePos = new Vector3(640.0f, 360.0f, -0.5f);
        private Vector3 closeButtonVisiblePos = new Vector3(1115.0f, 112.0f, -0.6f);
        private Vector3 packsButtonVisiblePos = new Vector3(195.0f, 146.0f, -0.6f);
        private Vector3 byteChipsButtonVisiblePos = new Vector3(270.0f, 146.0f, -0.6f);
        private Vector3 slot1VisiblePos = new Vector3(573.0f, 550.0f, -0.6f);
        private Vector3 slot2VisiblePos = new Vector3(815.0f, 550.0f, -0.6f);
        private Vector3 exchangeButton3VisiblePos = new Vector3(1057.0f, 550.0f, -0.6f);
        private Vector3 exchangeButton4VisiblePos = new Vector3(330.0f, 550.0f, -0.6f);
        private Vector3 slotDefaultVisiblePos = new Vector3(330.0f, 550.0f, -0.6f);
        private Vector3 labelPackOwned1VisiblePos = new Vector3(276.0f, 536.0f, -0.6f);
        private Vector3 labelPackOwned2VisiblePos = new Vector3(519.0f, 536.0f, -0.6f);
        private Vector3 labelPackOwned3VisiblePos = new Vector3(761.0f, 536.0f, -0.6f);
        private Vector3 labelPackOwned4VisiblePos = new Vector3(1003.0f, 536.0f, -0.6f);
        private Vector3 scoreText1VisiblePos = new Vector3(280.0f, 537.0f, -0.6f);
        private Vector3 scoreText2VisiblePos = new Vector3(470.0f, 537.0f, -0.6f);

        // Event names for popup coordination
        private const string EVENT_POPUP_OPENED = "MainMenuPopupOpened";
        private const string EVENT_POPUP_CLOSED = "MainMenuPopupClosed";
        private const string POPUP_ID = "Shop";

        // State
        private bool isPopupVisible = false;
        private bool entitiesFound = false;
        private bool wasMousePressed = false;
        private bool isPacksTab = true;
        private bool wasSemicolonPressed = false;
        private bool wasApostrophePressed = false;

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
            packsButtonSelectedId = SceneFindEntityByName(PACKS_BUTTON_SELECTED_NAME);
            byteChipsButtonId = SceneFindEntityByName(BYTECHIPS_BUTTON_NAME);
            byteChipsButtonSelectedId = SceneFindEntityByName(BYTECHIPS_BUTTON_SELECTED_NAME);
            LogMessage("ShopPopup: packsButtonId=" + packsButtonId + " packsButtonSelectedId=" + packsButtonSelectedId +
                " byteChipsButtonId=" + byteChipsButtonId + " byteChipsButtonSelectedId=" + byteChipsButtonSelectedId);
            exchangeButton1Id = SceneFindEntityByName(EXCHANGE_BUTTON_1_NAME);
            exchangeButton2Id = SceneFindEntityByName(EXCHANGE_BUTTON_2_NAME);
            exchangeButton3Id = SceneFindEntityByName(EXCHANGE_BUTTON_3_NAME);
            exchangeButton4Id = SceneFindEntityByName(EXCHANGE_BUTTON_4_NAME);
            equippedButtonId = SceneFindEntityByName(EQUIPPED_BUTTON_NAME);
            equipButton1Id = SceneFindEntityByName(EQUIP_BUTTON_1_NAME);
            equippedButton1Id = SceneFindEntityByName(EQUIPPED_BUTTON_1_NAME);
            equipButton2Id = SceneFindEntityByName(EQUIP_BUTTON_2_NAME);
            equippedButton2Id = SceneFindEntityByName(EQUIPPED_BUTTON_2_NAME);
            equipButton3Id = SceneFindEntityByName(EQUIP_BUTTON_3_NAME);
            equippedButton3Id = SceneFindEntityByName(EQUIPPED_BUTTON_3_NAME);
            equipButtonDefaultId = SceneFindEntityByName(EQUIP_BUTTON_DEFAULT_NAME);
            labelPackOwned1Id = SceneFindEntityByName(LABEL_PACK_OWNED_1_NAME);
            labelPackOwned2Id = SceneFindEntityByName(LABEL_PACK_OWNED_2_NAME);
            labelPackOwned3Id = SceneFindEntityByName(LABEL_PACK_OWNED_3_NAME);
            labelPackOwned4Id = SceneFindEntityByName(LABEL_PACK_OWNED_4_NAME);
            scoreText1Id = SceneFindEntityByName(SCORE_TEXT_1_NAME);
            scoreText2Id = SceneFindEntityByName(SCORE_TEXT_2_NAME);

            if (packsPopupId == 0) LogError("ShopPopup: Could not find: " + SHOP_PACKS_POPUP_NAME);
            if (byteChipsPopupId == 0) LogError("ShopPopup: Could not find: " + SHOP_BYTECHIPS_POPUP_NAME);
            if (closeButtonId == 0) LogError("ShopPopup: Could not find: " + SHOP_CLOSE_BUTTON_NAME);

            // Load shop state
            ProgressTracker.LoadProgress();

            entitiesFound = true;
            isPopupVisible = false;
            wasMousePressed = false;
            isPacksTab = true;

            // Subscribe to popup coordination events
            Event.Subscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);

            // Hide all popup elements initially
            HidePopup();

            LogMessage("ShopPopup: Ready! Skin1Purchased=" + ProgressTracker.Skin1Purchased +
                " Skin2Purchased=" + ProgressTracker.Skin2Purchased +
                " EquippedSkin=" + ProgressTracker.EquippedSkin);
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

            // Debug: Semicolon = Add 10,000 research points
            bool semiPressed = Input.IsKeyPressed(KeyCode.Semicolon);
            if (semiPressed && !wasSemicolonPressed)
            {
                ProgressTracker.AddCumulativeScore(10000);
                if (isPopupVisible) UpdateScoreDisplay();
            }
            wasSemicolonPressed = semiPressed;

            // Debug: Apostrophe = Reset all progress
            bool apostrophePressed = Input.IsKeyPressed(KeyCode.Apostrophe);
            if (apostrophePressed && !wasApostrophePressed)
            {
                ProgressTracker.ResetAllProgress();
                if (isPopupVisible)
                {
                    UpdateScoreDisplay();
                    UpdateByteChipsDisplay();
                    if (isPacksTab) UpdatePacksButtonStates();
                    else ShowByteChipsTab();
                }
            }
            wasApostrophePressed = apostrophePressed;
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

                // Check Packs tab button (normal or selected variant)
                if (!isPacksTab && IsPacksTabClicked())
                {
                    LogMessage("ShopPopup: Packs tab clicked");
                    SwitchToPacksTab();
                    return;
                }

                // Check ByteChips tab button (normal or selected variant)
                if (isPacksTab && IsByteChipsTabClicked())
                {
                    LogMessage("ShopPopup: ByteChips tab clicked");
                    SwitchToByteChipsTab();
                    return;
                }

                // Handle button clicks based on active tab
                if (isPacksTab)
                {
                    if (HandlePacksTabClick()) return;
                }
                else
                {
                    if (HandleByteChipsTabClick()) return;
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

        private bool HandlePacksTabClick()
        {
            // === Slot 1: Skin 1 ===
            if (!ProgressTracker.Skin1Purchased)
            {
                if (exchangeButton1Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton1Id))
                {
                    TryPurchaseSkin(1);
                    return true;
                }
            }
            else if (ProgressTracker.EquippedSkin != 1)
            {
                if (equipButton1Id != 0 && Collision2D.IsMouseCollidingWithEntity(equipButton1Id))
                {
                    EquipSkin(1);
                    return true;
                }
            }

            // === Slot 2: Skin 2 ===
            if (!ProgressTracker.Skin2Purchased)
            {
                if (exchangeButton2Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton2Id))
                {
                    TryPurchaseSkin(2);
                    return true;
                }
            }
            else if (ProgressTracker.EquippedSkin != 2)
            {
                if (equipButton2Id != 0 && Collision2D.IsMouseCollidingWithEntity(equipButton2Id))
                {
                    EquipSkin(2);
                    return true;
                }
            }

            // === Slot 3: Skin 3 (costs ByteChips) ===
            if (!ProgressTracker.Skin3Purchased)
            {
                if (exchangeButton3Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton3Id))
                {
                    TryPurchaseSkin3();
                    return true;
                }
            }
            else if (ProgressTracker.EquippedSkin != 3)
            {
                if (equipButton3Id != 0 && Collision2D.IsMouseCollidingWithEntity(equipButton3Id))
                {
                    EquipSkin(3);
                    return true;
                }
            }

            // === Default skin slot ===
            if (ProgressTracker.EquippedSkin != 0)
            {
                if (equipButtonDefaultId != 0 && Collision2D.IsMouseCollidingWithEntity(equipButtonDefaultId))
                {
                    EquipSkin(0);
                    return true;
                }
            }

            return false;
        }

        private bool HandleByteChipsTabClick()
        {
            // ByteChips tab: clicking exchange buttons awards ByteChips + research points (repeatable)
            if (exchangeButton4Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton4Id))
            {
                int researchBonus = ProgressTracker.BytePack1Bought ? 0 : 60;
                if (!ProgressTracker.BytePack1Bought)
                {
                    ProgressTracker.BytePack1Bought = true;
                    ProgressTracker.SaveProgress();
                }
                AwardByteChipsAndResearch(60, researchBonus);
                LogMessage("ShopPopup: ByteChips pack 1 bought - +60 ByteChips" + (researchBonus > 0 ? " +60 Research (first-time bonus)" : " (no bonus)"));
                return true;
            }
            if (exchangeButton1Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton1Id))
            {
                int researchBonus = ProgressTracker.BytePack2Bought ? 0 : 180;
                if (!ProgressTracker.BytePack2Bought)
                {
                    ProgressTracker.BytePack2Bought = true;
                    ProgressTracker.SaveProgress();
                }
                AwardByteChipsAndResearch(180, researchBonus);
                LogMessage("ShopPopup: ByteChips pack 2 bought - +180 ByteChips" + (researchBonus > 0 ? " +180 Research (first-time bonus)" : " (no bonus)"));
                return true;
            }
            if (exchangeButton2Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton2Id))
            {
                int researchBonus = ProgressTracker.BytePack3Bought ? 0 : 300;
                if (!ProgressTracker.BytePack3Bought)
                {
                    ProgressTracker.BytePack3Bought = true;
                    ProgressTracker.SaveProgress();
                }
                AwardByteChipsAndResearch(300, researchBonus);
                LogMessage("ShopPopup: ByteChips pack 3 bought - +300 ByteChips" + (researchBonus > 0 ? " +300 Research (first-time bonus)" : " (no bonus)"));
                return true;
            }
            if (exchangeButton3Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton3Id))
            {
                int researchBonus = ProgressTracker.BytePack4Bought ? 0 : 600;
                if (!ProgressTracker.BytePack4Bought)
                {
                    ProgressTracker.BytePack4Bought = true;
                    ProgressTracker.SaveProgress();
                }
                AwardByteChipsAndResearch(600, researchBonus);
                LogMessage("ShopPopup: ByteChips pack 4 bought - +600 ByteChips" + (researchBonus > 0 ? " +600 Research (first-time bonus)" : " (no bonus)"));
                return true;
            }

            return false;
        }

        private void AwardByteChipsAndResearch(int byteChips, int research)
        {
            ProgressTracker.ByteChips += byteChips;
            ProgressTracker.CumulativeScore += research;
            ProgressTracker.SaveProgress();

            // Update both score displays
            UpdateScoreDisplay();
            UpdateByteChipsDisplay();
        }

        private void TryPurchaseSkin(int skinIndex)
        {
            ProgressTracker.LoadProgress();

            if (ProgressTracker.CumulativeScore < SKIN_COST)
            {
                LogMessage("ShopPopup: Not enough points to purchase skin " + skinIndex +
                    " (have " + ProgressTracker.CumulativeScore + ", need " + SKIN_COST + ")");
                return;
            }

            // Deduct points
            ProgressTracker.CumulativeScore -= SKIN_COST;

            // Mark skin as purchased
            if (skinIndex == 1)
                ProgressTracker.Skin1Purchased = true;
            else if (skinIndex == 2)
                ProgressTracker.Skin2Purchased = true;

            // Save
            ProgressTracker.SaveProgress();

            LogMessage("ShopPopup: Purchased skin " + skinIndex + "! Remaining points: " + ProgressTracker.CumulativeScore);

            // Update score display
            UpdateScoreDisplay();

            // Refresh button states
            UpdatePacksButtonStates();
        }

        private void TryPurchaseSkin3()
        {
            ProgressTracker.LoadProgress();

            if (ProgressTracker.ByteChips < SKIN3_BYTECHIP_COST)
            {
                LogMessage("ShopPopup: Not enough ByteChips to purchase skin 3 (have " +
                    ProgressTracker.ByteChips + ", need " + SKIN3_BYTECHIP_COST + ")");
                return;
            }

            ProgressTracker.ByteChips -= SKIN3_BYTECHIP_COST;
            ProgressTracker.Skin3Purchased = true;
            ProgressTracker.SaveProgress();

            LogMessage("ShopPopup: Purchased skin 3! Remaining ByteChips: " + ProgressTracker.ByteChips);

            UpdateByteChipsDisplay();
            UpdatePacksButtonStates();
        }

        private void EquipSkin(int skinIndex)
        {
            ProgressTracker.LoadProgress();
            ProgressTracker.EquippedSkin = skinIndex;
            ProgressTracker.SaveProgress();

            LogMessage("ShopPopup: Equipped skin " + skinIndex);

            // Refresh button states
            UpdatePacksButtonStates();
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

            // Load latest state
            ProgressTracker.LoadProgress();

            // Show shared elements
            if (closeButtonId != 0) SetPosition(closeButtonId, ref closeButtonVisiblePos);
            if (packsButtonId != 0) SetPosition(packsButtonId, ref packsButtonVisiblePos);
            if (packsButtonSelectedId != 0)
            {
                SetIsVisible(packsButtonSelectedId, true);
                SetPosition(packsButtonSelectedId, ref packsButtonVisiblePos);
            }
            if (byteChipsButtonId != 0) SetPosition(byteChipsButtonId, ref byteChipsButtonVisiblePos);
            if (byteChipsButtonSelectedId != 0)
            {
                SetIsVisible(byteChipsButtonSelectedId, true);
                SetPosition(byteChipsButtonSelectedId, ref byteChipsButtonVisiblePos);
            }

            // Show score texts
            UpdateScoreDisplay();
            UpdateByteChipsDisplay();

            // Default to Packs tab
            UpdateTabButtonVisuals();
            ShowPacksTab();

            LogMessage("ShopPopup: Popup shown (Packs tab)");
        }

        private void UpdateScoreDisplay()
        {
            if (scoreText1Id != 0)
            {
                SetPosition(scoreText1Id, ref scoreText1VisiblePos);
                Text.SetText(scoreText1Id, ProgressTracker.CumulativeScore.ToString("D6"));
                Text.SetFontSize(scoreText1Id, 20.0f);
            }
        }

        private void UpdateByteChipsDisplay()
        {
            if (scoreText2Id != 0)
            {
                SetPosition(scoreText2Id, ref scoreText2VisiblePos);
                Text.SetText(scoreText2Id, ProgressTracker.ByteChips.ToString("D6"));
                Text.SetFontSize(scoreText2Id, 20.0f);
            }
        }

        private void SwitchToPacksTab()
        {
            isPacksTab = true;
            UpdateTabButtonVisuals();
            ShowPacksTab();
            LogMessage("ShopPopup: Switched to Packs tab");
        }

        private void SwitchToByteChipsTab()
        {
            isPacksTab = false;
            UpdateTabButtonVisuals();
            ShowByteChipsTab();
            LogMessage("ShopPopup: Switched to ByteChips tab");
        }

        private bool IsPacksTabClicked()
        {
            if (packsButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(packsButtonId))
                return true;
            if (packsButtonSelectedId != 0 && Collision2D.IsMouseCollidingWithEntity(packsButtonSelectedId))
                return true;
            return false;
        }

        private bool IsByteChipsTabClicked()
        {
            if (byteChipsButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(byteChipsButtonId))
                return true;
            if (byteChipsButtonSelectedId != 0 && Collision2D.IsMouseCollidingWithEntity(byteChipsButtonSelectedId))
                return true;
            return false;
        }

        private void UpdateTabButtonVisuals()
        {
            if (isPacksTab)
            {
                // Packs selected: show selected packs, normal bytechips
                if (packsButtonId != 0) SetColor(packsButtonId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (packsButtonSelectedId != 0) SetColor(packsButtonSelectedId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (byteChipsButtonId != 0) SetColor(byteChipsButtonId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (byteChipsButtonSelectedId != 0) SetColor(byteChipsButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
            }
            else
            {
                // ByteChips selected: show normal packs, selected bytechips
                if (packsButtonId != 0) SetColor(packsButtonId, 1.0f, 1.0f, 1.0f, 1.0f);
                if (packsButtonSelectedId != 0) SetColor(packsButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (byteChipsButtonId != 0) SetColor(byteChipsButtonId, 1.0f, 1.0f, 1.0f, 0.0f);
                if (byteChipsButtonSelectedId != 0) SetColor(byteChipsButtonSelectedId, 1.0f, 1.0f, 1.0f, 1.0f);
            }
        }

        private void ShowPacksTab()
        {
            Vector3 hidePos = new Vector3(CENTER_X, HIDDEN_Y, -0.5f);
            Vector3 hidePos2 = new Vector3(CENTER_X, HIDDEN_Y, -0.6f);

            // Show Packs window, hide ByteChips window
            if (packsPopupId != 0) SetPosition(packsPopupId, ref popupVisiblePos);
            if (byteChipsPopupId != 0) SetPosition(byteChipsPopupId, ref hidePos);

            // Hide ByteChips-only elements
            if (exchangeButton4Id != 0) SetPosition(exchangeButton4Id, ref hidePos2);

            // Update skin button states (including slot 3) and Pack Owned labels
            UpdatePacksButtonStates();
        }

        private void UpdatePacksButtonStates()
        {
            Vector3 hidePos2 = new Vector3(CENTER_X, HIDDEN_Y, -0.6f);

            // === Slot 1 (position 573, 550) ===
            // Hide all slot 1 buttons first
            if (exchangeButton1Id != 0) SetPosition(exchangeButton1Id, ref hidePos2);
            if (equipButton1Id != 0) SetPosition(equipButton1Id, ref hidePos2);
            if (equippedButton1Id != 0) SetPosition(equippedButton1Id, ref hidePos2);

            if (!ProgressTracker.Skin1Purchased)
            {
                // Not purchased - show Exchange
                if (exchangeButton1Id != 0) SetPosition(exchangeButton1Id, ref slot1VisiblePos);
            }
            else if (ProgressTracker.EquippedSkin == 1)
            {
                // Purchased and equipped - show Equipped
                if (equippedButton1Id != 0) SetPosition(equippedButton1Id, ref slot1VisiblePos);
            }
            else
            {
                // Purchased but not equipped - show Equip
                if (equipButton1Id != 0) SetPosition(equipButton1Id, ref slot1VisiblePos);
            }

            // === Slot 2 (position 815, 550) ===
            if (exchangeButton2Id != 0) SetPosition(exchangeButton2Id, ref hidePos2);
            if (equipButton2Id != 0) SetPosition(equipButton2Id, ref hidePos2);
            if (equippedButton2Id != 0) SetPosition(equippedButton2Id, ref hidePos2);

            if (!ProgressTracker.Skin2Purchased)
            {
                // Not purchased - show Exchange
                if (exchangeButton2Id != 0) SetPosition(exchangeButton2Id, ref slot2VisiblePos);
            }
            else if (ProgressTracker.EquippedSkin == 2)
            {
                // Purchased and equipped - show Equipped
                if (equippedButton2Id != 0) SetPosition(equippedButton2Id, ref slot2VisiblePos);
            }
            else
            {
                // Purchased but not equipped - show Equip
                if (equipButton2Id != 0) SetPosition(equipButton2Id, ref slot2VisiblePos);
            }

            // === Slot 3 (position 1057, 550) ===
            if (exchangeButton3Id != 0) SetPosition(exchangeButton3Id, ref hidePos2);
            if (equipButton3Id != 0) SetPosition(equipButton3Id, ref hidePos2);
            if (equippedButton3Id != 0) SetPosition(equippedButton3Id, ref hidePos2);

            if (!ProgressTracker.Skin3Purchased)
            {
                // Not purchased - show Exchange
                if (exchangeButton3Id != 0) SetPosition(exchangeButton3Id, ref exchangeButton3VisiblePos);
            }
            else if (ProgressTracker.EquippedSkin == 3)
            {
                // Purchased and equipped - show Equipped
                if (equippedButton3Id != 0) SetPosition(equippedButton3Id, ref exchangeButton3VisiblePos);
            }
            else
            {
                // Purchased but not equipped - show Equip
                if (equipButton3Id != 0) SetPosition(equipButton3Id, ref exchangeButton3VisiblePos);
            }

            // === Default skin slot (position 330, 550) ===
            if (equippedButtonId != 0) SetPosition(equippedButtonId, ref hidePos2);
            if (equipButtonDefaultId != 0) SetPosition(equipButtonDefaultId, ref hidePos2);

            if (ProgressTracker.EquippedSkin == 0)
            {
                // Default is equipped - show Equipped
                if (equippedButtonId != 0) SetPosition(equippedButtonId, ref slotDefaultVisiblePos);
            }
            else
            {
                // Default is not equipped - show Equip
                if (equipButtonDefaultId != 0) SetPosition(equipButtonDefaultId, ref slotDefaultVisiblePos);
            }

            // === Pack Owned labels on skin slots ===
            // Default slot - always owned
            if (labelPackOwned1Id != 0) SetPosition(labelPackOwned1Id, ref labelPackOwned1VisiblePos);

            // Skin 1 - show if purchased
            if (labelPackOwned2Id != 0)
            {
                if (ProgressTracker.Skin1Purchased)
                    SetPosition(labelPackOwned2Id, ref labelPackOwned2VisiblePos);
                else
                    SetPosition(labelPackOwned2Id, ref hidePos2);
            }

            // Skin 2 - show if purchased
            if (labelPackOwned3Id != 0)
            {
                if (ProgressTracker.Skin2Purchased)
                    SetPosition(labelPackOwned3Id, ref labelPackOwned3VisiblePos);
                else
                    SetPosition(labelPackOwned3Id, ref hidePos2);
            }

            // Skin 3 - show if purchased
            if (labelPackOwned4Id != 0)
            {
                if (ProgressTracker.Skin3Purchased)
                    SetPosition(labelPackOwned4Id, ref labelPackOwned4VisiblePos);
                else
                    SetPosition(labelPackOwned4Id, ref hidePos2);
            }
        }

        private void ShowByteChipsTab()
        {
            Vector3 hidePos = new Vector3(CENTER_X, HIDDEN_Y, -0.5f);
            Vector3 hidePos2 = new Vector3(CENTER_X, HIDDEN_Y, -0.6f);

            // Show ByteChips window, hide Packs window
            if (byteChipsPopupId != 0) SetPosition(byteChipsPopupId, ref popupVisiblePos);
            if (packsPopupId != 0) SetPosition(packsPopupId, ref hidePos);

            // Hide all Packs-tab skin buttons
            if (equipButton1Id != 0) SetPosition(equipButton1Id, ref hidePos2);
            if (equippedButton1Id != 0) SetPosition(equippedButton1Id, ref hidePos2);
            if (equipButton2Id != 0) SetPosition(equipButton2Id, ref hidePos2);
            if (equippedButton2Id != 0) SetPosition(equippedButton2Id, ref hidePos2);
            if (equipButton3Id != 0) SetPosition(equipButton3Id, ref hidePos2);
            if (equippedButton3Id != 0) SetPosition(equippedButton3Id, ref hidePos2);
            if (equippedButtonId != 0) SetPosition(equippedButtonId, ref hidePos2);
            if (equipButtonDefaultId != 0) SetPosition(equipButtonDefaultId, ref hidePos2);

            // Hide all Pack Owned labels (they belong to the Packs tab now)
            if (labelPackOwned1Id != 0) SetPosition(labelPackOwned1Id, ref hidePos2);
            if (labelPackOwned2Id != 0) SetPosition(labelPackOwned2Id, ref hidePos2);
            if (labelPackOwned3Id != 0) SetPosition(labelPackOwned3Id, ref hidePos2);
            if (labelPackOwned4Id != 0) SetPosition(labelPackOwned4Id, ref hidePos2);

            // Always show all exchange buttons (packs are repeatable)
            if (exchangeButton4Id != 0) SetPosition(exchangeButton4Id, ref exchangeButton4VisiblePos);
            if (exchangeButton1Id != 0) SetPosition(exchangeButton1Id, ref slot1VisiblePos);
            if (exchangeButton2Id != 0) SetPosition(exchangeButton2Id, ref slot2VisiblePos);
            if (exchangeButton3Id != 0) SetPosition(exchangeButton3Id, ref exchangeButton3VisiblePos);
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
            if (packsButtonSelectedId != 0) SetPosition(packsButtonSelectedId, ref hidePos2);
            if (byteChipsButtonId != 0) SetPosition(byteChipsButtonId, ref hidePos2);
            if (byteChipsButtonSelectedId != 0) SetPosition(byteChipsButtonSelectedId, ref hidePos2);
            if (exchangeButton1Id != 0) SetPosition(exchangeButton1Id, ref hidePos2);
            if (exchangeButton2Id != 0) SetPosition(exchangeButton2Id, ref hidePos2);
            if (exchangeButton3Id != 0) SetPosition(exchangeButton3Id, ref hidePos2);
            if (exchangeButton4Id != 0) SetPosition(exchangeButton4Id, ref hidePos2);
            if (equippedButtonId != 0) SetPosition(equippedButtonId, ref hidePos2);
            if (equipButton1Id != 0) SetPosition(equipButton1Id, ref hidePos2);
            if (equippedButton1Id != 0) SetPosition(equippedButton1Id, ref hidePos2);
            if (equipButton2Id != 0) SetPosition(equipButton2Id, ref hidePos2);
            if (equippedButton2Id != 0) SetPosition(equippedButton2Id, ref hidePos2);
            if (equipButton3Id != 0) SetPosition(equipButton3Id, ref hidePos2);
            if (equippedButton3Id != 0) SetPosition(equippedButton3Id, ref hidePos2);
            if (equipButtonDefaultId != 0) SetPosition(equipButtonDefaultId, ref hidePos2);
            if (labelPackOwned1Id != 0) SetPosition(labelPackOwned1Id, ref hidePos2);
            if (labelPackOwned2Id != 0) SetPosition(labelPackOwned2Id, ref hidePos2);
            if (labelPackOwned3Id != 0) SetPosition(labelPackOwned3Id, ref hidePos2);
            if (labelPackOwned4Id != 0) SetPosition(labelPackOwned4Id, ref hidePos2);
            if (scoreText1Id != 0) SetPosition(scoreText1Id, ref hidePos2);
            if (scoreText2Id != 0) SetPosition(scoreText2Id, ref hidePos2);

            LogMessage("ShopPopup: Popup hidden");
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_POPUP_OPENED, OnOtherPopupOpened);
            LogMessage("ShopPopup: Destroyed");
        }
    }
}
