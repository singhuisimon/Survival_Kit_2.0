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
    /// Handles the shop popup in the main menu. Two tabs: Packs (default) and ByteChips.
    /// Packs tab: Skin slots with Exchange/Equip/Equipped button states.
    ///            Grey exchange button shows if player cannot afford the skin.
    /// ByteChips tab: Exchange buttons for purchasing ByteChip packs.
    /// </summary>
    public class ShopPopup : ScriptBehaviour
    {
        // Entity names
        private const string SHOPBUTTONNAME = "Shop Button";
        private const string SHOPPACKSPOPUPNAME = "Shop Popup";
        private const string SHOPBYTECHIPSPOPUPNAME = "Shop ByteChips Popup";
        private const string SHOPCLOSEBUTTONNAME = "Shop Close Button";
        private const string PACKSBUTTONNAME = "Shop Packs Button";
        private const string PACKSBUTTONSELECTEDNAME = "Shop Packs Button Selected";
        private const string BYTECHIPSBUTTONNAME = "Shop ByteChips Button";
        private const string BYTECHIPSBUTTONSELECTEDNAME = "Shop ByteChips Button Selected";

        private const string EXCHANGEBUTTON1NAME = "Shop Exchange Button 1";
        private const string EXCHANGEBUTTON2NAME = "Shop Exchange Button 2";
        private const string EXCHANGEBUTTON3NAME = "Shop Exchange Button 3";
        private const string EXCHANGEBUTTON4NAME = "Shop Exchange Button 4";

        // Grey (disabled) exchange buttons
        private const string GREYEXCHANGEBUTTON1NAME = "EXCHANGE_GREY_BUTTON_1";
        private const string GREYEXCHANGEBUTTON2NAME = "EXCHANGE_GREY_BUTTON_2";
        private const string GREYEXCHANGEBUTTON3NAME = "EXCHANGE_GREY_BUTTON_3";

        private const string EQUIPPEDBUTTONNAME = "Shop Equipped Button";
        private const string EQUIPBUTTON1NAME = "Shop Equip Button 1";
        private const string EQUIPPEDBUTTON1NAME = "Shop Equipped Button 1";
        private const string EQUIPBUTTON2NAME = "Shop Equip Button 2";
        private const string EQUIPPEDBUTTON2NAME = "Shop Equipped Button 2";
        private const string EQUIPBUTTON3NAME = "Shop Equip Button 3";
        private const string EQUIPPEDBUTTON3NAME = "Shop Equipped Button 3";
        private const string EQUIPBUTTONDEFAULTNAME = "Shop Equip Button Default";
        private const string SCORETEXT1NAME = "Shop Score Text 1";
        private const string SCORETEXT2NAME = "Shop Score Text 2";

        // Skin costs
        private const int SKINCOST = 10000; // Skin 1 & 2 research points
        private const int SKIN3BYTECHIPCOST = 50;    // Skin 3 ByteChips

        // Positions
        private const float HIDDENY = -500.0f;
        private const float CENTERX = 640.0f;

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

        // Grey exchange button IDs
        private uint greyExchangeButton1Id;
        private uint greyExchangeButton2Id;
        private uint greyExchangeButton3Id;

        private uint equippedButtonId;    // Default skin - Equipped state
        private uint equipButton1Id;      // Skin 1 - Equip state
        private uint equippedButton1Id;   // Skin 1 - Equipped state
        private uint equipButton2Id;      // Skin 2 - Equip state
        private uint equippedButton2Id;   // Skin 2 - Equipped state
        private uint equipButton3Id;      // Skin 3 - Equip state
        private uint equippedButton3Id;   // Skin 3 - Equipped state
        private uint equipButtonDefaultId;
        private uint scoreText1Id;
        private uint scoreText2Id;

        // Visible positions
        private Vector3 popupVisiblePos = new Vector3(640.0f, 360.0f, -0.5f);
        private Vector3 closeButtonVisiblePos = new Vector3(1115.0f, 112.0f, -0.6f);
        private Vector3 packsButtonVisiblePos = new Vector3(195.0f, 146.0f, -0.6f);
        private Vector3 byteChipsButtonVisiblePos = new Vector3(270.0f, 146.0f, -0.6f);
        private Vector3 slot1VisiblePos = new Vector3(573.0f, 550.0f, -0.7f);
        private Vector3 slot2VisiblePos = new Vector3(815.0f, 550.0f, -0.7f);
        private Vector3 exchangeButton3VisiblePos = new Vector3(1057.0f, 550.0f, -0.7f);
        private Vector3 exchangeButton4VisiblePos = new Vector3(330.0f, 550.0f, -0.7f);
        private Vector3 slotDefaultVisiblePos = new Vector3(330.0f, 550.0f, -0.7f);
        private Vector3 scoreText1VisiblePos = new Vector3(280.0f, 537.0f, -0.6f);
        private Vector3 scoreText2VisiblePos = new Vector3(470.0f, 537.0f, -0.6f);

        // Event coordination
        private const string EVENTPOPUPOPENED = "MainMenuPopupOpened";
        private const string EVENTPOPUPCLOSED = "MainMenuPopupClosed";
        private const string POPUPID = "Shop";

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

            shopButtonId = SceneFindEntityByName(SHOPBUTTONNAME);
            if (shopButtonId == 0) { LogError("ShopPopup: Could not find entity: " + SHOPBUTTONNAME); return; }

            packsPopupId = SceneFindEntityByName(SHOPPACKSPOPUPNAME);
            byteChipsPopupId = SceneFindEntityByName(SHOPBYTECHIPSPOPUPNAME);
            closeButtonId = SceneFindEntityByName(SHOPCLOSEBUTTONNAME);
            packsButtonId = SceneFindEntityByName(PACKSBUTTONNAME);
            packsButtonSelectedId = SceneFindEntityByName(PACKSBUTTONSELECTEDNAME);
            byteChipsButtonId = SceneFindEntityByName(BYTECHIPSBUTTONNAME);
            byteChipsButtonSelectedId = SceneFindEntityByName(BYTECHIPSBUTTONSELECTEDNAME);

            exchangeButton1Id = SceneFindEntityByName(EXCHANGEBUTTON1NAME);
            exchangeButton2Id = SceneFindEntityByName(EXCHANGEBUTTON2NAME);
            exchangeButton3Id = SceneFindEntityByName(EXCHANGEBUTTON3NAME);
            exchangeButton4Id = SceneFindEntityByName(EXCHANGEBUTTON4NAME);

            greyExchangeButton1Id = SceneFindEntityByName(GREYEXCHANGEBUTTON1NAME);
            greyExchangeButton2Id = SceneFindEntityByName(GREYEXCHANGEBUTTON2NAME);
            greyExchangeButton3Id = SceneFindEntityByName(GREYEXCHANGEBUTTON3NAME);

            equippedButtonId = SceneFindEntityByName(EQUIPPEDBUTTONNAME);
            equipButton1Id = SceneFindEntityByName(EQUIPBUTTON1NAME);
            equippedButton1Id = SceneFindEntityByName(EQUIPPEDBUTTON1NAME);
            equipButton2Id = SceneFindEntityByName(EQUIPBUTTON2NAME);
            equippedButton2Id = SceneFindEntityByName(EQUIPPEDBUTTON2NAME);
            equipButton3Id = SceneFindEntityByName(EQUIPBUTTON3NAME);
            equippedButton3Id = SceneFindEntityByName(EQUIPPEDBUTTON3NAME);
            equipButtonDefaultId = SceneFindEntityByName(EQUIPBUTTONDEFAULTNAME);
            scoreText1Id = SceneFindEntityByName(SCORETEXT1NAME);
            scoreText2Id = SceneFindEntityByName(SCORETEXT2NAME);

            if (packsPopupId == 0) LogError("ShopPopup: Could not find " + SHOPPACKSPOPUPNAME);
            if (byteChipsPopupId == 0) LogError("ShopPopup: Could not find " + SHOPBYTECHIPSPOPUPNAME);
            if (closeButtonId == 0) LogError("ShopPopup: Could not find " + SHOPCLOSEBUTTONNAME);
            if (greyExchangeButton1Id == 0) LogMessage("ShopPopup: EXCHANGE_GREY_BUTTON_1 not found");
            if (greyExchangeButton2Id == 0) LogMessage("ShopPopup: EXCHANGE_GREY_BUTTON_2 not found");
            if (greyExchangeButton3Id == 0) LogMessage("ShopPopup: EXCHANGE_GREY_BUTTON_3 not found");

            ProgressTracker.LoadProgress();

            entitiesFound = true;
            isPopupVisible = false;
            wasMousePressed = false;
            isPacksTab = true;

            Event.Subscribe(EVENTPOPUPOPENED, OnOtherPopupOpened);
            HidePopup();
            LogMessage("ShopPopup: Ready! Skin1Purchased=" + ProgressTracker.Skin1Purchased +
                       " Skin2Purchased=" + ProgressTracker.Skin2Purchased +
                       " Skin3Purchased=" + ProgressTracker.Skin3Purchased +
                       " EquippedSkin=" + ProgressTracker.EquippedSkin);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound) return;

            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (mouseJustPressed) HandleMouseClick();

            // Debug: Semicolon adds 10,000 research points
            bool semiPressed = Input.IsKeyPressed(KeyCode.Semicolon);
            if (semiPressed && !wasSemicolonPressed)
            {
                ProgressTracker.AddCumulativeScore(10000);
                if (isPopupVisible) UpdateScoreDisplay();
            }
            wasSemicolonPressed = semiPressed;

            // Debug: Apostrophe resets all progress
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
                // Close button
                if (closeButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(closeButtonId))
                {
                    LogMessage("ShopPopup: Close button clicked");
                    HidePopup();
                    return;
                }

                // Tab switching
                if (!isPacksTab && IsPacksTabClicked())
                {
                    LogMessage("ShopPopup: Packs tab clicked");
                    SwitchToPacksTab();
                    return;
                }
                if (isPacksTab && IsByteChipsTabClicked())
                {
                    LogMessage("ShopPopup: ByteChips tab clicked");
                    SwitchToByteChipsTab();
                    return;
                }

                // Handle tab-specific clicks
                if (isPacksTab) { if (HandlePacksTabClick()) return; }
                else { if (HandleByteChipsTabClick()) return; }

                // Clicked outside popup - close
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
                if (Collision2D.IsMouseCollidingWithEntity(shopButtonId))
                {
                    LogMessage("ShopPopup: Shop button clicked - showing popup");
                    ShowPopup();
                }
            }
        }

        private bool HandlePacksTabClick()
        {
            // Slot 1 - Skin 1 (costs research points)
            if (!ProgressTracker.Skin1Purchased)
            {
                if (exchangeButton1Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton1Id))
                {
                    TryPurchaseSkin(1);
                    return true;
                }
                if (greyExchangeButton1Id != 0 && Collision2D.IsMouseCollidingWithEntity(greyExchangeButton1Id))
                {
                    LogMessage("ShopPopup: Not enough research points for Skin 1");
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

            // Slot 2 - Skin 2 (costs research points)
            if (!ProgressTracker.Skin2Purchased)
            {
                if (exchangeButton2Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton2Id))
                {
                    TryPurchaseSkin(2);
                    return true;
                }
                if (greyExchangeButton2Id != 0 && Collision2D.IsMouseCollidingWithEntity(greyExchangeButton2Id))
                {
                    LogMessage("ShopPopup: Not enough research points for Skin 2");
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

            // Slot 3 - Skin 3 (costs ByteChips)
            if (!ProgressTracker.Skin3Purchased)
            {
                if (exchangeButton3Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton3Id))
                {
                    TryPurchaseSkin3();
                    return true;
                }
                if (greyExchangeButton3Id != 0 && Collision2D.IsMouseCollidingWithEntity(greyExchangeButton3Id))
                {
                    LogMessage("ShopPopup: Not enough ByteChips for Skin 3");
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

            // Default skin slot
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
            if (exchangeButton4Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton4Id))
            {
                int researchBonus = ProgressTracker.BytePack1Bought ? 0 : 60;
                if (!ProgressTracker.BytePack1Bought) { ProgressTracker.BytePack1Bought = true; ProgressTracker.SaveProgress(); }
                AwardByteChipsAndResearch(60, researchBonus);
                LogMessage("ShopPopup: ByteChips pack 1 bought - 60 ByteChips, " + (researchBonus > 0 ? researchBonus + " Research first-time bonus" : "no bonus"));
                return true;
            }
            if (exchangeButton1Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton1Id))
            {
                int researchBonus = ProgressTracker.BytePack2Bought ? 0 : 180;
                if (!ProgressTracker.BytePack2Bought) { ProgressTracker.BytePack2Bought = true; ProgressTracker.SaveProgress(); }
                AwardByteChipsAndResearch(180, researchBonus);
                LogMessage("ShopPopup: ByteChips pack 2 bought - 180 ByteChips, " + (researchBonus > 0 ? researchBonus + " Research first-time bonus" : "no bonus"));
                return true;
            }
            if (exchangeButton2Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton2Id))
            {
                int researchBonus = ProgressTracker.BytePack3Bought ? 0 : 300;
                if (!ProgressTracker.BytePack3Bought) { ProgressTracker.BytePack3Bought = true; ProgressTracker.SaveProgress(); }
                AwardByteChipsAndResearch(300, researchBonus);
                LogMessage("ShopPopup: ByteChips pack 3 bought - 300 ByteChips, " + (researchBonus > 0 ? researchBonus + " Research first-time bonus" : "no bonus"));
                return true;
            }
            if (exchangeButton3Id != 0 && Collision2D.IsMouseCollidingWithEntity(exchangeButton3Id))
            {
                int researchBonus = ProgressTracker.BytePack4Bought ? 0 : 600;
                if (!ProgressTracker.BytePack4Bought) { ProgressTracker.BytePack4Bought = true; ProgressTracker.SaveProgress(); }
                AwardByteChipsAndResearch(600, researchBonus);
                LogMessage("ShopPopup: ByteChips pack 4 bought - 600 ByteChips, " + (researchBonus > 0 ? researchBonus + " Research first-time bonus" : "no bonus"));
                return true;
            }
            return false;
        }

        private void AwardByteChipsAndResearch(int byteChips, int research)
        {
            ProgressTracker.ByteChips += byteChips;
            ProgressTracker.CumulativeScore += research;
            ProgressTracker.SaveProgress();
            UpdateScoreDisplay();
            UpdateByteChipsDisplay();
        }

        private void TryPurchaseSkin(int skinIndex)
        {
            if (ProgressTracker.CumulativeScore < SKINCOST)
            {
                LogMessage("ShopPopup: Not enough points to purchase skin " + skinIndex +
                           " - have " + ProgressTracker.CumulativeScore + ", need " + SKINCOST);
                return;
            }

            ProgressTracker.CumulativeScore -= SKINCOST;

            if (skinIndex == 1) ProgressTracker.Skin1Purchased = true;
            else if (skinIndex == 2) ProgressTracker.Skin2Purchased = true;

            ProgressTracker.SaveProgress();
            LogMessage("ShopPopup: Purchased skin " + skinIndex +
                       "! Remaining points: " + ProgressTracker.CumulativeScore +
                       " | Skin" + skinIndex + "Purchased saved to progress.json");

            UpdateScoreDisplay();
            UpdatePacksButtonStates();
        }

        private void TryPurchaseSkin3()
        {
            if (ProgressTracker.ByteChips < SKIN3BYTECHIPCOST)
            {
                LogMessage("ShopPopup: Not enough ByteChips to purchase skin 3 - have " +
                           ProgressTracker.ByteChips + ", need " + SKIN3BYTECHIPCOST);
                return;
            }

            ProgressTracker.ByteChips -= SKIN3BYTECHIPCOST;
            ProgressTracker.Skin3Purchased = true;

            ProgressTracker.SaveProgress();
            LogMessage("ShopPopup: Purchased skin 3! Remaining ByteChips: " + ProgressTracker.ByteChips +
                       " | Skin3Purchased saved to progress.json");

            UpdateByteChipsDisplay();
            UpdatePacksButtonStates();
        }

        private void EquipSkin(int skinIndex)
        {
            try
            {
                LogMessage("ShopPopup: EquipSkin " + skinIndex + " called");
                ProgressTracker.EquippedSkin = skinIndex;
                ProgressTracker.SaveProgress();
                LogMessage("ShopPopup: Equipped skin " + skinIndex + ", updating button states...");
                UpdatePacksButtonStates();
                LogMessage("ShopPopup: EquipSkin " + skinIndex + " completed successfully");
            }
            catch (Exception e)
            {
                LogError("ShopPopup: EquipSkin CRASHED: " + e.Message + "\n" + e.StackTrace);
            }
        }

        private void OnOtherPopupOpened(string eventName, string payload)
        {
            if (payload != POPUPID && isPopupVisible)
            {
                LogMessage("ShopPopup: Another popup opened: " + payload + " - closing shop");
                HidePopup();
            }
        }

        private void ShowPopup()
        {
            if (isPopupVisible) return;
            isPopupVisible = true;
            isPacksTab = true;
            Event.Publish(EVENTPOPUPOPENED, POPUPID);

            ProgressTracker.LoadProgress();

            if (closeButtonId != 0) SetPosition(closeButtonId, ref closeButtonVisiblePos);

            // Activate all tab buttons before positioning/coloring
            if (packsButtonId != 0) SetIsVisible(packsButtonId, true);
            if (packsButtonSelectedId != 0) SetIsVisible(packsButtonSelectedId, true);
            if (byteChipsButtonId != 0) SetIsVisible(byteChipsButtonId, true);
            if (byteChipsButtonSelectedId != 0) SetIsVisible(byteChipsButtonSelectedId, true);

            UpdateScoreDisplay();
            UpdateByteChipsDisplay();
            UpdateTabButtonVisuals();
            ShowPacksTab();
            LogMessage("ShopPopup: Popup shown - Packs tab");
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

        private void SwitchToPacksTab() { isPacksTab = true; UpdateTabButtonVisuals(); ShowPacksTab(); LogMessage("ShopPopup: Switched to Packs tab"); }
        private void SwitchToByteChipsTab() { isPacksTab = false; UpdateTabButtonVisuals(); ShowByteChipsTab(); LogMessage("ShopPopup: Switched to ByteChips tab"); }

        private bool IsPacksTabClicked()
        {
            if (packsButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(packsButtonId)) return true;
            if (packsButtonSelectedId != 0 && Collision2D.IsMouseCollidingWithEntity(packsButtonSelectedId)) return true;
            return false;
        }

        private bool IsByteChipsTabClicked()
        {
            if (byteChipsButtonId != 0 && Collision2D.IsMouseCollidingWithEntity(byteChipsButtonId)) return true;
            if (byteChipsButtonSelectedId != 0 && Collision2D.IsMouseCollidingWithEntity(byteChipsButtonSelectedId)) return true;
            return false;
        }

        private void UpdateTabButtonVisuals()
        {
            // Move all 4 tab buttons to their visible positions first
            if (packsButtonId != 0) SetPosition(packsButtonId, ref packsButtonVisiblePos);
            if (packsButtonSelectedId != 0) SetPosition(packsButtonSelectedId, ref packsButtonVisiblePos);
            if (byteChipsButtonId != 0) SetPosition(byteChipsButtonId, ref byteChipsButtonVisiblePos);
            if (byteChipsButtonSelectedId != 0) SetPosition(byteChipsButtonSelectedId, ref byteChipsButtonVisiblePos);

            // Swap which one is visible via alpha
            if (isPacksTab)
            {
                if (packsButtonId != 0) SetColor(packsButtonId, 1.0f, 1.0f, 1.0f, 0.0f); // hide normal
                if (packsButtonSelectedId != 0) SetColor(packsButtonSelectedId, 1.0f, 1.0f, 1.0f, 1.0f); // show selected
                if (byteChipsButtonId != 0) SetColor(byteChipsButtonId, 1.0f, 1.0f, 1.0f, 1.0f); // show normal
                if (byteChipsButtonSelectedId != 0) SetColor(byteChipsButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f); // hide selected
            }
            else
            {
                if (packsButtonId != 0) SetColor(packsButtonId, 1.0f, 1.0f, 1.0f, 1.0f); // show normal
                if (packsButtonSelectedId != 0) SetColor(packsButtonSelectedId, 1.0f, 1.0f, 1.0f, 0.0f); // hide selected
                if (byteChipsButtonId != 0) SetColor(byteChipsButtonId, 1.0f, 1.0f, 1.0f, 0.0f); // hide normal
                if (byteChipsButtonSelectedId != 0) SetColor(byteChipsButtonSelectedId, 1.0f, 1.0f, 1.0f, 1.0f); // show selected
            }
        }

        private void ShowPacksTab()
        {
            Vector3 hidePos = new Vector3(CENTERX, HIDDENY, -0.5f);
            Vector3 hidePos2 = new Vector3(CENTERX, HIDDENY, -0.6f);

            if (packsPopupId != 0) SetPosition(packsPopupId, ref popupVisiblePos);
            if (byteChipsPopupId != 0) SetPosition(byteChipsPopupId, ref hidePos);
            if (exchangeButton4Id != 0) SetPosition(exchangeButton4Id, ref hidePos2);

            UpdatePacksButtonStates();
        }

        private void UpdatePacksButtonStates()
        {
            try
            {
                Vector3 hidePos2 = new Vector3(CENTERX, HIDDENY, -0.6f);

                LogMessage("ShopPopup: UpdatePacksButtonStates - Skin1=" + ProgressTracker.Skin1Purchased +
                           " Skin2=" + ProgressTracker.Skin2Purchased +
                           " Skin3=" + ProgressTracker.Skin3Purchased +
                           " Equipped=" + ProgressTracker.EquippedSkin);

                // --- Slot 1 (573, 550) ---
                if (exchangeButton1Id != 0) SetPosition(exchangeButton1Id, ref hidePos2);
                if (greyExchangeButton1Id != 0) SetPosition(greyExchangeButton1Id, ref hidePos2);
                if (equipButton1Id != 0) SetPosition(equipButton1Id, ref hidePos2);
                if (equippedButton1Id != 0) SetPosition(equippedButton1Id, ref hidePos2);

                if (!ProgressTracker.Skin1Purchased)
                {
                    if (ProgressTracker.CumulativeScore >= SKINCOST)
                    {
                        if (exchangeButton1Id != 0) SetPosition(exchangeButton1Id, ref slot1VisiblePos);
                    }
                    else
                    {
                        if (greyExchangeButton1Id != 0) SetPosition(greyExchangeButton1Id, ref slot1VisiblePos);
                    }
                }
                else if (ProgressTracker.EquippedSkin == 1)
                {
                    if (equippedButton1Id != 0) SetPosition(equippedButton1Id, ref slot1VisiblePos);
                }
                else
                {
                    if (equipButton1Id != 0) SetPosition(equipButton1Id, ref slot1VisiblePos);
                }

                // --- Slot 2 (815, 550) ---
                if (exchangeButton2Id != 0) SetPosition(exchangeButton2Id, ref hidePos2);
                if (greyExchangeButton2Id != 0) SetPosition(greyExchangeButton2Id, ref hidePos2);
                if (equipButton2Id != 0) SetPosition(equipButton2Id, ref hidePos2);
                if (equippedButton2Id != 0) SetPosition(equippedButton2Id, ref hidePos2);

                if (!ProgressTracker.Skin2Purchased)
                {
                    if (ProgressTracker.CumulativeScore >= SKINCOST)
                    {
                        if (exchangeButton2Id != 0) SetPosition(exchangeButton2Id, ref slot2VisiblePos);
                    }
                    else
                    {
                        if (greyExchangeButton2Id != 0) SetPosition(greyExchangeButton2Id, ref slot2VisiblePos);
                    }
                }
                else if (ProgressTracker.EquippedSkin == 2)
                {
                    if (equippedButton2Id != 0) SetPosition(equippedButton2Id, ref slot2VisiblePos);
                }
                else
                {
                    if (equipButton2Id != 0) SetPosition(equipButton2Id, ref slot2VisiblePos);
                }

                // --- Slot 3 (1057, 550) ---
                if (exchangeButton3Id != 0) SetPosition(exchangeButton3Id, ref hidePos2);
                if (greyExchangeButton3Id != 0) SetPosition(greyExchangeButton3Id, ref hidePos2);
                if (equipButton3Id != 0) SetPosition(equipButton3Id, ref hidePos2);
                if (equippedButton3Id != 0) SetPosition(equippedButton3Id, ref hidePos2);

                if (!ProgressTracker.Skin3Purchased)
                {
                    if (ProgressTracker.ByteChips >= SKIN3BYTECHIPCOST)
                    {
                        if (exchangeButton3Id != 0) SetPosition(exchangeButton3Id, ref exchangeButton3VisiblePos);
                    }
                    else
                    {
                        if (greyExchangeButton3Id != 0) SetPosition(greyExchangeButton3Id, ref exchangeButton3VisiblePos);
                    }
                }
                else if (ProgressTracker.EquippedSkin == 3)
                {
                    if (equippedButton3Id != 0) SetPosition(equippedButton3Id, ref exchangeButton3VisiblePos);
                }
                else
                {
                    if (equipButton3Id != 0) SetPosition(equipButton3Id, ref exchangeButton3VisiblePos);
                }

                // --- Default skin slot (330, 550) ---
                if (equippedButtonId != 0) SetPosition(equippedButtonId, ref hidePos2);
                if (equipButtonDefaultId != 0) SetPosition(equipButtonDefaultId, ref hidePos2);

                if (ProgressTracker.EquippedSkin == 0)
                {
                    if (equippedButtonId != 0) SetPosition(equippedButtonId, ref slotDefaultVisiblePos);
                }
                else
                {
                    if (equipButtonDefaultId != 0) SetPosition(equipButtonDefaultId, ref slotDefaultVisiblePos);
                }

                LogMessage("ShopPopup: UpdatePacksButtonStates completed successfully");
            }
            catch (Exception e)
            {
                LogError("ShopPopup: UpdatePacksButtonStates CRASHED: " + e.Message + "\n" + e.StackTrace);
            }
        }

        private void ShowByteChipsTab()
        {
            Vector3 hidePos = new Vector3(CENTERX, HIDDENY, -0.5f);
            Vector3 hidePos2 = new Vector3(CENTERX, HIDDENY, -0.6f);

            if (byteChipsPopupId != 0) SetPosition(byteChipsPopupId, ref popupVisiblePos);
            if (packsPopupId != 0) SetPosition(packsPopupId, ref hidePos);

            // Hide all Packs-tab skin buttons including grey ones
            if (equipButton1Id != 0) SetPosition(equipButton1Id, ref hidePos2);
            if (equippedButton1Id != 0) SetPosition(equippedButton1Id, ref hidePos2);
            if (equipButton2Id != 0) SetPosition(equipButton2Id, ref hidePos2);
            if (equippedButton2Id != 0) SetPosition(equippedButton2Id, ref hidePos2);
            if (equipButton3Id != 0) SetPosition(equipButton3Id, ref hidePos2);
            if (equippedButton3Id != 0) SetPosition(equippedButton3Id, ref hidePos2);
            if (equippedButtonId != 0) SetPosition(equippedButtonId, ref hidePos2);
            if (equipButtonDefaultId != 0) SetPosition(equipButtonDefaultId, ref hidePos2);
            if (greyExchangeButton1Id != 0) SetPosition(greyExchangeButton1Id, ref hidePos2);
            if (greyExchangeButton2Id != 0) SetPosition(greyExchangeButton2Id, ref hidePos2);
            if (greyExchangeButton3Id != 0) SetPosition(greyExchangeButton3Id, ref hidePos2);

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

            if (wasVisible) Event.Publish(EVENTPOPUPCLOSED, POPUPID);

            Vector3 hidePos = new Vector3(CENTERX, HIDDENY, -0.5f);
            Vector3 hidePos2 = new Vector3(CENTERX, HIDDENY, -0.6f);

            if (packsPopupId != 0) SetPosition(packsPopupId, ref hidePos);
            if (byteChipsPopupId != 0) SetPosition(byteChipsPopupId, ref hidePos);
            if (closeButtonId != 0) SetPosition(closeButtonId, ref hidePos2);

            if (packsButtonId != 0) { SetPosition(packsButtonId, ref hidePos2); SetIsVisible(packsButtonId, false); }
            if (packsButtonSelectedId != 0) { SetPosition(packsButtonSelectedId, ref hidePos2); SetIsVisible(packsButtonSelectedId, false); }
            if (byteChipsButtonId != 0) { SetPosition(byteChipsButtonId, ref hidePos2); SetIsVisible(byteChipsButtonId, false); }
            if (byteChipsButtonSelectedId != 0) { SetPosition(byteChipsButtonSelectedId, ref hidePos2); SetIsVisible(byteChipsButtonSelectedId, false); }

            if (exchangeButton1Id != 0) SetPosition(exchangeButton1Id, ref hidePos2);
            if (exchangeButton2Id != 0) SetPosition(exchangeButton2Id, ref hidePos2);
            if (exchangeButton3Id != 0) SetPosition(exchangeButton3Id, ref hidePos2);
            if (exchangeButton4Id != 0) SetPosition(exchangeButton4Id, ref hidePos2);

            if (greyExchangeButton1Id != 0) SetPosition(greyExchangeButton1Id, ref hidePos2);
            if (greyExchangeButton2Id != 0) SetPosition(greyExchangeButton2Id, ref hidePos2);
            if (greyExchangeButton3Id != 0) SetPosition(greyExchangeButton3Id, ref hidePos2);

            if (equippedButtonId != 0) SetPosition(equippedButtonId, ref hidePos2);
            if (equipButton1Id != 0) SetPosition(equipButton1Id, ref hidePos2);
            if (equippedButton1Id != 0) SetPosition(equippedButton1Id, ref hidePos2);
            if (equipButton2Id != 0) SetPosition(equipButton2Id, ref hidePos2);
            if (equippedButton2Id != 0) SetPosition(equippedButton2Id, ref hidePos2);
            if (equipButton3Id != 0) SetPosition(equipButton3Id, ref hidePos2);
            if (equippedButton3Id != 0) SetPosition(equippedButton3Id, ref hidePos2);
            if (equipButtonDefaultId != 0) SetPosition(equipButtonDefaultId, ref hidePos2);
            if (scoreText1Id != 0) SetPosition(scoreText1Id, ref hidePos2);
            if (scoreText2Id != 0) SetPosition(scoreText2Id, ref hidePos2);

            LogMessage("ShopPopup: Popup hidden");
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENTPOPUPOPENED, OnOtherPopupOpened);
            LogMessage("ShopPopup: Destroyed");
        }
    }
}
