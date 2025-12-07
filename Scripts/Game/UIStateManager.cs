using Engine;
using System;

namespace Game
{
    // As of right now, using keys W and L to test for Win and Lose screen
    // while waiting for further gameplay integration

    public class UIStateManager : ScriptBehaviour
    {
        // serialize the camera entities
         [SerializeField]
        private string mainMenuCameraName = "MainMenuCamera";

        [SerializeField]
        private string gameCameraName = "MainCamera";

        [SerializeField]
        private string winCameraName = "WinCamera";

        [SerializeField]
        private string loseCameraName = "LoseCamera";

        // current state
        private enum ScreenState
        {
            MainMenu,       // showing main menu screen
            Playing,        // game is active
            Won,            // showing Win screen
            Lost            // showing Lose screen
        }

        private ScreenState currentState = ScreenState.MainMenu;    //initial scene (what player see when they start game)

        // camera IDs
        private uint mainMenuCameraID = 0;
        private uint gameCameraID = 0;
        private uint winCameraID = 0;
        private uint loseCameraID = 0;

        // input states (checking if each key is pressed)
        private bool enterWasPressed = false;
        private bool pWasPressed = false;
        private bool lWasPressed = false;

        public override void OnStart()
        {
            // find all the cameras in the scene
            mainMenuCameraID = InternalCalls.Scene_FindEntityByName(mainMenuCameraName);
            gameCameraID = InternalCalls.Scene_FindEntityByName(gameCameraName);
            winCameraID = InternalCalls.Scene_FindEntityByName(winCameraName);
            loseCameraID = InternalCalls.Scene_FindEntityByName(loseCameraName);

            // initialize state
             currentState = ScreenState.MainMenu;

             // put here for events (once implemented)
             // added the PlayerHasDied event
             EventSystem.Subscribe("PlayerHasDied", OnPlayerDied);
             // added the player has won event
             EventSystem.Subscribe("PlayerWin", OnPlayerWin);


        }

        public override void OnUpdate(float deltaTime)
        {
            // handle input based on current state
            HandleInput();

        }

        private void HandleInput()
        {
            // detect the enter key
            bool enterIsPressed = Input.IsKeyPressed(KeyCode.Enter);
            bool enterJustPressed = enterIsPressed && !enterWasPressed;
            enterWasPressed = enterIsPressed;

            // detect the W key (shortcut to test to view win screen)
            bool pIsPressed = Input.IsKeyPressed(KeyCode.P);
            bool pJustPressed = pIsPressed && !pWasPressed;
            pWasPressed = pIsPressed;

            //detect the L key (shortcut to test to view lose screen)
            bool lIsPressed = Input.IsKeyPressed(KeyCode.L);
            bool lJustPressed = lIsPressed && !lWasPressed;
            lWasPressed = lIsPressed;

            // handle enter (this should only work IF player is on win/lose screen)
            if (enterJustPressed)
            {
                if (currentState == ScreenState.Won || currentState == ScreenState.Lost)
                {
                    ReturnToMainMenu();
                }
            }

            // testing shortcut (remember to remove during submission
            // if (pJustPressed)
            // {
            //     Log("Testing Win screen...");
            //     ShowWinScreen();
            // }

            // if (lJustPressed)
            // {
            //     Log("Testing Lose screen...");
            //     ShowLoseScreen();
            // }
        }

        // EVENT HANDLERS

        private void OnPlayerDied(string eventName, string payload)
        {
            Log("Player died event received!");
            EventSystem.Publish("ChangeToLost", loseCameraName);
            ShowLoseScreen();
        }

        private void OnPlayerWin(string eventName, string payload) 
        {
            Log("Game win event received! Botnets killed: " + payload);
            ShowWinScreen();
        }
    

        // Screen Transistions

        private void ShowWinScreen()
        {
            if (currentState == ScreenState.Won)
            return; // player is already on win screen

            Log("PLAYER WINS!!");

            // update the win state
            currentState = ScreenState.Won;

            // disable all other cameras
            DisableAllCameras();

            // enable the win camera
            if (winCameraID != 0)
            {
                InternalCalls.Camera_SetEnabled(winCameraID, true);
                Log("Enabled WinCamera");
            }
            else
            {
                LogError("WinCamera not found!");
            }
        }

        private void ShowLoseScreen()
        {
            if (currentState == ScreenState.Lost)
            return; // player already on lose screen

            Log("PLAYER LOST!!");

            // update the lose state
            currentState = ScreenState.Lost;

            // disable all other cameras
            DisableAllCameras();

            // enable the lose camera
            if (loseCameraID != 0)
            {
                InternalCalls.Camera_SetEnabled(loseCameraID, true);
                Log("Enabled LoseCamera");
            }
            else
            {
                LogError("LoseCamera not found!");
            }
        }

        private void ReturnToMainMenu()
        {
            Log("Returning to main menu");

            currentState = ScreenState.MainMenu;

            // disable all other cameras
            DisableAllCameras();

            // enable the main menu camera
            if (mainMenuCameraID != 0)
            {
                InternalCalls.Camera_SetEnabled(mainMenuCameraID, true);
                Log("Enabled MainMenuCamera");
            }
            else
            {
                LogError("MainMenuCamera not found!");
            }

            // reset the game state here
            // add code whenever needed here

            // letting it know it should restart the game
            EventSystem.Publish("GameRestart", ""); 
        }

        // HELPER FUNCTIONS
        private void DisableAllCameras()
        {
            if (mainMenuCameraID != 0)
                InternalCalls.Camera_SetEnabled(mainMenuCameraID, false);

            if (gameCameraID != 0)
                InternalCalls.Camera_SetEnabled(gameCameraID, false);

            if (winCameraID != 0)
                InternalCalls.Camera_SetEnabled(winCameraID, false);

            if (loseCameraID != 0)
                InternalCalls.Camera_SetEnabled(loseCameraID, false);
        }

        // clean up event subscriptions
        public override void OnDestroy()
        {
            EventSystem.Unsubscribe("PlayerHasDied", OnPlayerDied);
            EventSystem.Unsubscribe("PlayerWin", OnPlayerWin); 
            Log("UIStateManager destroyed");
        }

    }
}   // end of namespace Game