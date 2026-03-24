using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Transform;

namespace Game
{
    public class CreditsPopup : ScriptBehaviour
    {
        // Entity names
        private const string CREDITSBUTTONNAME = "Credits Button";
        private const string POPUPMAINNAME = "Credits Popup Main";
        private const string POPUPRESOURCESNAME = "Credits Popup Resources";
        private const string CLOSEBUTTONNAME = "Credits Close Button";
        private const string CLOSEBUTTON2NAME = "Credits Close Button 2";

        // Positions
        private const float HIDDENY = -500.0f;
        private const float VISIBLEY = 360.0f;
        private const float CENTERX = 640.0f;

        // Absolute button positions when visible
        private const float CLOSEBUTTONX = 766.0f;
        private const float CLOSEBUTTONY = 50.0f;
        private const float CLOSEBUTTON2X = 1243.0f;
        private const float CLOSEBUTTON2Y = 269.0f;

        // Entity IDs
        private uint creditsButtonId;
        private uint popupMainId;
        private uint popupResourcesId;
        private uint closeButtonId;
        private uint closeButton2Id;

        // Event names for popup coordination
        private const string EVENTPOPUPOPENED = "MainMenuPopupOpened";
        private const string EVENTPOPUPCLOSED = "MainMenuPopupClosed";
        private const string POPUPID = "Credits";

        // State
        private bool isMainVisible = false;
        private bool isResourcesVisible = false;
        private bool entitiesFound = false;
        private bool wasMousePressed = false;

        public override void OnStart()
        {
            LogMessage("CreditsPopup Initializing...");

            creditsButtonId = SceneFindEntityByName(CREDITSBUTTONNAME);
            popupMainId = SceneFindEntityByName(POPUPMAINNAME);
            popupResourcesId = SceneFindEntityByName(POPUPRESOURCESNAME);
            closeButtonId = SceneFindEntityByName(CLOSEBUTTONNAME);
            closeButton2Id = SceneFindEntityByName(CLOSEBUTTON2NAME);


            if (creditsButtonId == 0) { LogError("CreditsPopup Could not find entity: " + CREDITSBUTTONNAME); return; }
            if (popupMainId == 0) { LogError("CreditsPopup Could not find entity: " + POPUPMAINNAME); return; }
            if (popupResourcesId == 0) { LogError("CreditsPopup Could not find entity: " + POPUPRESOURCESNAME); return; }
            if (closeButtonId == 0) { LogError("CreditsPopup Could not find entity: " + CLOSEBUTTONNAME); return; }
            if (closeButton2Id == 0) { LogError("CreditsPopup Could not find entity: " + CLOSEBUTTON2NAME); return; }

            entitiesFound = true;
            isMainVisible = false;
            isResourcesVisible = false;
            wasMousePressed = false;

            Event.Subscribe(EVENTPOPUPOPENED, OnOtherPopupOpened);
            LogMessage("CreditsPopup All entities found, ready!");
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
            // Close button 1 closes ONLY Credits Popup Main
            if (isMainVisible && Collision2D.IsMouseCollidingWithEntity(closeButtonId))
            {
                LogMessage("CreditsPopup Close button 1 clicked - hiding Main popup");
                HideMain();
                return;
            }

            // Close button 2 closes ONLY Credits Popup Resources
            if (isResourcesVisible && Collision2D.IsMouseCollidingWithEntity(closeButton2Id))
            {
                LogMessage("CreditsPopup Close button 2 clicked - hiding Resources popup");
                HideResources();
                return;
            }

            // Credits button opens both popups
            if (!isMainVisible && !isResourcesVisible && Collision2D.IsMouseCollidingWithEntity(creditsButtonId))
            {
                LogMessage("CreditsPopup Credits button clicked - showing popup");
                ShowPopup();
            }
        }

        private void OnOtherPopupOpened(string eventName, string payload)
        {
            if (payload != POPUPID && (isMainVisible || isResourcesVisible))
            {
                LogMessage("CreditsPopup Another popup opened: " + payload + " - closing credits");
                HideAll();
            }
        }

        private void ShowPopup()
        {
            isMainVisible = true;
            isResourcesVisible = true;
            Event.Publish(EVENTPOPUPOPENED, POPUPID);

            // Resources in front of Main
            Vector3 mainPos = new Vector3(CENTERX, VISIBLEY, -0.5f);
            SetPosition(popupMainId, ref mainPos);

            Vector3 resPos = new Vector3(CENTERX, VISIBLEY, -0.51f);
            SetPosition(popupResourcesId, ref resPos);

            Vector3 closePos = new Vector3(CLOSEBUTTONX, CLOSEBUTTONY, -0.6f);
            SetPosition(closeButtonId, ref closePos);

            Vector3 close2Pos = new Vector3(CLOSEBUTTON2X, CLOSEBUTTON2Y, -0.6f);
            SetPosition(closeButton2Id, ref close2Pos);

            LogMessage("CreditsPopup Popup shown");
        }

        private void HideMain()
        {
            isMainVisible = false;

            Vector3 mainHiddenPos = new Vector3(CENTERX, HIDDENY, -0.5f);
            SetPosition(popupMainId, ref mainHiddenPos);

            Vector3 closeHiddenPos = new Vector3(CLOSEBUTTONX, HIDDENY, -0.6f);
            SetPosition(closeButtonId, ref closeHiddenPos);

            if (!isResourcesVisible)
                Event.Publish(EVENTPOPUPCLOSED, POPUPID);

            LogMessage("CreditsPopup Main popup hidden");
        }

        private void HideResources()
        {
            isResourcesVisible = false;

            Vector3 resHiddenPos = new Vector3(CENTERX, HIDDENY, -0.51f);
            SetPosition(popupResourcesId, ref resHiddenPos);

            Vector3 close2HiddenPos = new Vector3(CLOSEBUTTON2X, HIDDENY, -0.6f);
            SetPosition(closeButton2Id, ref close2HiddenPos);

            if (!isMainVisible)
                Event.Publish(EVENTPOPUPCLOSED, POPUPID);

            LogMessage("CreditsPopup Resources popup hidden");
        }

        private void HideAll()
        {
            isMainVisible = false;
            isResourcesVisible = false;

            Vector3 mainHiddenPos = new Vector3(CENTERX, HIDDENY, -0.5f);
            SetPosition(popupMainId, ref mainHiddenPos);

            Vector3 resHiddenPos = new Vector3(CENTERX, HIDDENY, -0.51f);
            SetPosition(popupResourcesId, ref resHiddenPos);

            Vector3 closeHiddenPos = new Vector3(CLOSEBUTTONX, HIDDENY, -0.6f);
            SetPosition(closeButtonId, ref closeHiddenPos);

            Vector3 close2HiddenPos = new Vector3(CLOSEBUTTON2X, HIDDENY, -0.6f);
            SetPosition(closeButton2Id, ref close2HiddenPos);

            Event.Publish(EVENTPOPUPCLOSED, POPUPID);
            LogMessage("CreditsPopup All popups hidden");
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENTPOPUPOPENED, OnOtherPopupOpened);
            LogMessage("CreditsPopup Destroyed");
        }
    }
}
