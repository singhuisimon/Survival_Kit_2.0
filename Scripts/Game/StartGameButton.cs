using Engine;
using System;

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

        private uint buttonEntityID;    // for entity
        private bool wasMouseButtonPressed = false;

        private bool hasGameStarted = false;    //this is to ensure that when player click on MouseButton left, it wouldnt trigger the camera switch again

        //IDK ANYMORE
        private bool wasRightMouseButtonPressed = false;

        public override void OnStart()
        {
            Log("StartGameButton: Initializing...");

            // Find the actual button entity by name
            buttonEntityID = InternalCalls.Scene_FindEntityByName(buttonEntityName);

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
                Log("StartGameButton: Ready!");
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

        }

        public override void OnUpdate(float deltaTime) 
        {
            if (!camerasInitialized) 
            {
                return;
            }

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

            // === DEBUG: Log mouse position constantly ===
            //Vector2 mousePos = Input.GetMousePosition();

           bool isMouseLeftButtonPressed = Input.IsMouseButtonPressed(MouseButton.Left);
            
            //if (Input.IsMouseButtonPressed(MouseButton.Left))
            if (isMouseLeftButtonPressed && !wasMouseButtonPressed)
            {
                //Vector3 buttonPos = Transform.Position;
                
                Log("===================");
                Log("CLICK DETECTED!");
                Log("===================");

                // check if the mouse is over the button (detecting the button entity?)
                if (IsMouseOverButton()) 
                {
                    Log("Click was over the button!");
                    OnButtonClicked();
                }
                else 
                {
                    Log("Click was NOT over the button");
                    OnButtonClicked();  // change camera anyway or else it will no longer change camera
                }
                
                // float minX = buttonPos.X - (buttonWidth / 2f);
                // float maxX = buttonPos.X + (buttonWidth / 2f);
                // float minY = buttonPos.Y - (buttonHeight / 2f);
                // float maxY = buttonPos.Y + (buttonHeight / 2f);
                
                // Try to switch anyway for testing
                //OnButtonClicked();
            }

            // update previous state for next frame
            wasMouseButtonPressed = isMouseLeftButtonPressed;
            
            // Space to immediately test if Camera can toggle (debug, comment during actual gameplay)
            if (Input.IsKeyPressed(KeyCode.Space))
            {
                Log("Space key pressed - toggling camera");
                OnButtonClicked();
            }

            if (Input.IsMouseButtonPressed(MouseButton.Right))
            {
                ResetToMainMenu();
            }
        }

        public override void OnDestroy()
        {
            Log("StartGameButton: Destroyed");
        }
        
        // wonky and not working :/
        private bool IsMouseOverButton()
        {
            // Get mouse position (in screen/window coordinates)
            Vector2 mousePos = Input.GetMousePosition();
            //Vector3 mouseWorld = ScreenToPseudoWorld(mousePos,0);

            
            // getting button world position directly from engine
            Vector3 buttonPos;         
            InternalCalls.Transform_GetPosition(buttonEntityID, out buttonPos);
            



            Log("Checking bounds...");
            Log("Mouse pos: " + mousePos.X + ", " + mousePos.Y);
            //Log("MouseWorld pos: " + mouseWorld.X + ", " + mouseWorld.Y);
            Log("Button pos: " + buttonPos.X + ", " + buttonPos.Y);

            
            // Calculate button bounds

            float halfW = buttonWidth * 0.5f;
            float halfH = buttonHeight * 0.5f;

            float minX = buttonPos.X - halfW;
            float maxX = buttonPos.X + halfW;
            float minY = buttonPos.Y - halfH;
            float maxY = buttonPos.Y + halfH;
            
            // Check if mouse is within bounds
            Log("Bounds X: " + minX + " to " + maxX);
            Log("Bounds Y: " + minY + " to " + maxY);

            // Check if mouse is within bounds
            bool withinX = mousePos.X >= minX && mousePos.X <= maxX;
            bool withinY = mousePos.Y >= minY && mousePos.Y <= maxY;

            Log("Within X: " + withinX + ", Within Y: " + withinY);

            bool hit = withinX && withinY;
            Log("IsMouseOverButton result: " + hit);
            
            return hit;


        }

        // Handle button click - to toggle between cameras
        private void OnButtonClicked()
        {
            if (hasGameStarted)
            {
                Log("StartGameButton: Game already started, ignoring click.");
                return;
            }

            Log("StartGameButton: Clicked! Switching cameras...");
            
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
            Log("StartGameButton: Going back to main menu");

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
            uint mainMenuID = InternalCalls.Scene_FindEntityByName(mainMenuCameraName);
            uint gameID = InternalCalls.Scene_FindEntityByName(gameCameraName);
            
            // Convert to ulong (EntityID is ulong in the engine)
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
                Log("StartGameButton: Cameras found - MainMenu ID: " + mainMenuCameraID + ", Game ID: " + gameCameraID);
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
                Log("StartGameButton: Switching to Main Menu Camera");
                InternalCalls.Camera_SetEnabled(mainMenuCameraID, true);
                InternalCalls.Camera_SetEnabled(gameCameraID, false);
            }
            else
            {
                Log("StartGameButton: Switching to Game Camera");
                InternalCalls.Camera_SetEnabled(mainMenuCameraID, false);
                InternalCalls.Camera_SetEnabled(gameCameraID, true);
            }
        }


    }
}