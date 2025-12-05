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
        private bool wWasPressed = false;
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

        }

        public override void OnUpdate(float deltaTime)
        {
            // handle input based on current state
            HandleInput();

            // check with the win/lose conditions during gameplay
            if (currentState == ScreenState.Playing)
            {
                CheckWinCondition();
                CheckLoseCondition();
            }
        }

        private void HandleInput()
        {
            // detect the enter key
            bool enterIsPressed = Input.IsKeyPressed(KeyCode.Enter);
            bool enterJustPressed = enterIsPressed && !enterWasPressed;
            enterWasPressed = enterIsPressed;

            // detect the W key (shortcut to test to view win screen)
            bool wIsPressed = Input.IsKeyPressed(KeyCode.W);
            bool wJustPressed = wIsPressed && !wWasPressed;
            wWasPressed = wIsPressed;

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
            if (wJustPressed)
            {
                Log("Testing Win screen...");
                ShowWinScreen();
            }
            
            if (lJustPressed)
            {
                Log("Testing Lose screen...");
                ShowLoseScreen();
            }
        }

        // EVENT HANDLERS

        // private void OnPlayerDied(string eventName, string payload)
        // {
        //     Log("Player died event received!");
        //     ShowLoseScreen();
        // }

        // private void OnEnemyKilled(string eventName, string payload)
        // {
        //     if (currentState != ScreenState.Playing)
        //         return;
            
        //     enemiesKilled++;
        //     Log("Enemy killed! Total: " + enemiesKilled + "/" + enemiesRequiredForWin);
        // }

        // win / lose condition checking

        private void CheckWinCondition()
        {
 
            // PLACEHOLDER: ADD WIN CONDITION HERE
            // once enemy death events is implemented (idk) event OnEnemyKilled will track the count
            
            // if (enemiesKilled >= enemiesRequiredForWin)
            // {
            //     ShowWinScreen();
            // }
            
            // can count remaining enemies in scene for alternative
            /*
            int remainingEnemies = CountRemainingEnemies();
            if (remainingEnemies == 0 && enemiesKilled > 0)
            {
                ShowWinScreen();
            }
            */
        }
        
        private void CheckLoseCondition()
        {
            // PLACEHOLDER: ADD LOSE CONDITION HERE
            // once player health is implemented and event OnPlayerDied will trigger the lose screen
            
            // manual checks if needed
            /*
            if (playerCurrentHealth <= 0)
            {
                ShowLoseScreen();
            }
            */
        }

        // Screen Transistions

        private void ShowWinScreen() 
        {
            if (currentState == ScreenState.Won)
            return; // player is already on win screen

            Log("PLAYER WINS!!");

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
            currentState = ScreenState.Playing;

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
            // add code here
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

        // for testing purposes
        private void LogCameraStatus(string name, uint id)
        {
            if (id != 0)
                Log("Found " + name + " (ID: " + id + ")");
            else
                LogWarning(name + " not found!");
        }

    }
}   // end of namespace Game