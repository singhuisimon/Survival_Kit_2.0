using Engine;
using System;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Camera;
using static Engine.Transform;


namespace Game
{
    public class StartGameButton : ScriptBehaviour
    {
        // serialization fields
        [SerializeField]
        private string mainMenuCameraName = "MainMenuCamera";

        [SerializeField]
        private string gameCameraName = "MainCamera";

        [SerializeField]
        private bool showMainMenuInitially = true;

        [SerializeField]
        private float buttonWidth = 2.4f;    // Set this to match button scale on the editor

        [SerializeField]
        private float buttonHeight = 0.8f;   // Set this to match button scale on the editor

        [SerializeField]
        private string buttonEntityName = "MainMenuStartButton";   // entity name of button



        // private fields
        private uint mainMenuCameraID;
        private uint gameCameraID;
        private bool camerasInitialized = false;
        private bool isShowingMainMenu;

        // for keyedge detection
        private bool enterWasPressed = false;

        private uint buttonEntityID;    // for entity
        //private bool wasMouseButtonPressed = false;

        private bool hasGameStarted = false;    //this is to ensure that when player click on MouseButton left, it wouldnt trigger the camera switch again

        //IDK ANYMORE
        // private bool wasRightMouseButtonPressed = false;

        public override void OnStart()
        {
            LogMessage("StartGameButton: Initializing...");

            // Find the actual button entity by name
            buttonEntityID = SceneFindEntityByName(buttonEntityName);

            // checking if it exists
            if (buttonEntityID == 0)
            {
                LogError("StartGameButton: Button entity not found: " + buttonEntityName);
                return;
            }

            isShowingMainMenu = showMainMenuInitially;

            // Find and cache camera entities
            InitializeCameras();

            // Set initial camera state
            if (camerasInitialized)
            {
                UpdateCameraStates();
                LogMessage("StartGameButton: Ready!");
            }
            else
            {
                LogError("StartGameButton: Failed to find cameras! Check entity names.");
            }

            // starting in main menu
            if (showMainMenuInitially)
            {
                ResetToMainMenu();
            }
            else
            {
                // If you want to start directly in game, you can do:
                isShowingMainMenu = false;
                UpdateCameraStates();
            }

            Subscribe("GameRestart", OnGameRestart);

        }

        public override void OnUpdate(float deltaTime)
        {
            if (!camerasInitialized)
            {
                return;
            }

            // just pressed detection for enter
            bool enterIsPressed = Input.IsKeyPressed(KeyCode.Enter);
            bool enterJustPressed = enterIsPressed && !enterWasPressed;
            enterWasPressed = enterIsPressed;

            // only allow starting the game if the main menu camera is actually active
            bool mainMenuActive = CameraGetEnabled(mainMenuCameraID);

            //Vector2 mousePos = Input.GetMousePosition();

            // Retry initialization if it failed
            // if (!camerasInitialized)
            // {
            //     InitializeCameras();
            //     if (camerasInitialized)
            //     {
            //         UpdateCameraStates();
            //     }
            //     return;
            // }

            // === DEBUG: LogMessage mouse position constantly ===
            //Vector2 mousePos = Input.GetMousePosition();

            //change key
            //if (Input.IsKeyPressed(KeyCode.Enter))
            if (enterJustPressed && mainMenuActive)
            {
                //Vector3 buttonPos = Transform.Position;

                LogMessage("===================");
                LogMessage("CLICK DETECTED!");
                LogMessage("===================");

                // check if the mouse is over the button (detecting the button entity?)
                if (IsMouseOverButton())
                {
                    LogMessage("Click was over the button!");
                    OnButtonClicked();
                }
                else
                {
                    LogMessage("Click was NOT over the button");
                    OnButtonClicked();  // change camera anyway or else it will no longer change camera
                }

                // float minX = buttonPos.X - (buttonWidth / 2f);
                // float maxX = buttonPos.X + (buttonWidth / 2f);
                // float minY = buttonPos.Y - (buttonHeight / 2f);
                // float maxY = buttonPos.Y + (buttonHeight / 2f);

                // Try to switch anyway for testing
                //OnButtonClicked();
            }


            //    bool isMouseLeftButtonPressed = Input.IsMouseButtonPressed(MouseButton.Left);

            //     //if (Input.IsMouseButtonPressed(MouseButton.Left))
            //     if (isMouseLeftButtonPressed && !wasMouseButtonPressed)
            //     {
            //         //Vector3 buttonPos = Transform.Position;

            //         LogMessage("===================");
            //         LogMessage("CLICK DETECTED!");
            //         LogMessage("===================");

            //         // check if the mouse is over the button (detecting the button entity?)
            //         if (IsMouseOverButton()) 
            //         {
            //             LogMessage("Click was over the button!");
            //             OnButtonClicked();
            //         }
            //         else 
            //         {
            //             LogMessage("Click was NOT over the button");
            //             OnButtonClicked();  // change camera anyway or else it will no longer change camera
            //         }

            //         // float minX = buttonPos.X - (buttonWidth / 2f);
            //         // float maxX = buttonPos.X + (buttonWidth / 2f);
            //         // float minY = buttonPos.Y - (buttonHeight / 2f);
            //         // float maxY = buttonPos.Y + (buttonHeight / 2f);

            //         // Try to switch anyway for testing
            //         //OnButtonClicked();
            //     }

            // update previous state for next frame
            //wasMouseButtonPressed = isMouseLeftButtonPressed;

            // Space to immediately test if Camera can toggle (debug, comment during actual gameplay)
            // if (Input.IsKeyPressed(KeyCode.Space))
            // {
            //     LogMessage("Space key pressed - toggling camera");
            //     OnButtonClicked();
            // }

            // detect right click
            // bool isRightPressed = Input.IsMouseButtonPressed(MouseButton.Right);

            // if (isRightPressed && !wasRightMouseButtonPressed)
            // {
            //     // right-click JUST happened
            //     ResetToMainMenu();
            // }

            // wasRightMouseButtonPressed = isRightPressed;

        }

        public override void OnDestroy()
        {
            LogMessage("StartGameButton: Destroyed");
            Unsubscribe("GameRestart", OnGameRestart);
        }

        public void OnGameRestart(string eventName, string payload)
        {
            // reset internal state and cameras in order to sync with UIStateManager
            ResetToMainMenu();
        }

        // wonky and not working :/
        private bool IsMouseOverButton()
        {
            // Get mouse position (in screen/window coordinates)
            Vector2 mousePos = Input.GetMousePosition();
            //Vector3 mouseWorld = ScreenToPseudoWorld(mousePos,0);


            // getting button world position directly from engine
            Vector3 buttonPos;
            buttonPos = GetPosition(buttonEntityID);




            LogMessage("Checking bounds...");
            LogMessage("Mouse pos: " + mousePos.X + ", " + mousePos.Y);
            //LogMessage("MouseWorld pos: " + mouseWorld.X + ", " + mouseWorld.Y);
            LogMessage("Button pos: " + buttonPos.X + ", " + buttonPos.Y);


            // Calculate button bounds

            float halfW = buttonWidth * 0.5f;
            float halfH = buttonHeight * 0.5f;

            float minX = buttonPos.X - halfW;
            float maxX = buttonPos.X + halfW;
            float minY = buttonPos.Y - halfH;
            float maxY = buttonPos.Y + halfH;

            // Check if mouse is within bounds
            LogMessage("Bounds X: " + minX + " to " + maxX);
            LogMessage("Bounds Y: " + minY + " to " + maxY);

            // Check if mouse is within bounds
            bool withinX = mousePos.X >= minX && mousePos.X <= maxX;
            bool withinY = mousePos.Y >= minY && mousePos.Y <= maxY;

            LogMessage("Within X: " + withinX + ", Within Y: " + withinY);

            bool hit = withinX && withinY;
            LogMessage("IsMouseOverButton result: " + hit);

            return hit;


        }

        // Handle button click - to toggle between cameras
        private void OnButtonClicked()
        {
            if (hasGameStarted)
            {
                LogMessage("StartGameButton: Game already started, ignoring click.");
                return;
            }

            LogMessage("StartGameButton: Clicked! Switching cameras...");

            // Toggle state
            isShowingMainMenu = false;

            hasGameStarted = true; //lets player know game has started

            // Update which camera is active
            UpdateCameraStates();

            // Optional: Add audio feedback
            // Audio.Play("ButtonClick");
        }

        private void ResetToMainMenu()
        {
            LogMessage("StartGameButton: Going back to main menu");

            // Set logical state
            isShowingMainMenu = true;

            // allow the game to be started again
            hasGameStarted = false;

            // Turn on main menu camera, turn off game camera
            UpdateCameraStates();

            // If you have other game variables that mark "game started",
            // reset them here too (score, timers, player health etc).
        }

        // Find camera entities by name
        private void InitializeCameras()
        {
            // Find entities by name (MainMenuCamera and GameCamera)
            uint mainMenuID = SceneFindEntityByName(mainMenuCameraName);
            uint gameID = SceneFindEntityByName(gameCameraName);

            // Convert to uint (EntityID is uint in the engine)
            mainMenuCameraID = mainMenuID;
            gameCameraID = gameID;

            // Check if both cameras were found
            if (mainMenuID == 0)
            {
                LogError("StartGameButton: Camera not found: " + mainMenuCameraName);
            }

            if (gameID == 0)
            {
                LogError("StartGameButton: Camera not found: " + gameCameraName);
            }

            // put it as initialized only if both MainMenuCamera and GameCamera exists
            camerasInitialized = (mainMenuID != 0 && gameID != 0);

            if (camerasInitialized)
            {
                LogMessage("StartGameButton: Cameras found - MainMenu ID: " + mainMenuCameraID + ", Game ID: " + gameCameraID);
            }
        }

        // Enable and disable based on current menu state
        private void UpdateCameraStates()
        {
            if (!camerasInitialized)
            {
                LogWarning("StartGameButton: Cannot update cameras - not initialized");
                return;
            }

            if (isShowingMainMenu)
            {
                LogMessage("StartGameButton: Switching to Main Menu Camera");
                CameraSetEnabled(mainMenuCameraID, true);
                CameraSetEnabled(gameCameraID, false);
            }
            else
            {
                LogMessage("StartGameButton: Switching to Game Camera");
                CameraSetEnabled(mainMenuCameraID, false);
                CameraSetEnabled(gameCameraID, true);
                Publish("StartingGame", true.ToString());
            }
        }


    }
}