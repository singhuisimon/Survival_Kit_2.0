using Engine;
using System;

namespace Game
{
    public class StartGameButton : ScriptBehaviour
    {
        // ===== SERIALIZED FIELDS =====
        [SerializeField]
        private string mainMenuCameraName = "MainMenuCamera";
        
        [SerializeField]
        private string gameCameraName = "GameCamera";

        [SerializeField]
        private bool showMainMenuInitially = true;

        [SerializeField]
        private float buttonWidth = 2.4f;    // Set this to match your button scale
        
        [SerializeField]
        private float buttonHeight = 0.1f;   // Set this to match your button scale

        // ===== PRIVATE FIELDS =====
        private ulong mainMenuCameraID;
        private ulong gameCameraID;
        private bool camerasInitialized = false;
        private bool isShowingMainMenu;

        // ===== LIFECYCLE METHODS =====
        
        public override void OnStart()
        {
            Log("StartGameButton: Initializing...");
            
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
        }

        public override void OnUpdate(float deltaTime) 
        {
            // Retry initialization if it failed
            if (!camerasInitialized)
            {
                InitializeCameras();
                if (camerasInitialized)
                {
                    UpdateCameraStates();
                }
                return;
            }

            // === DEBUG: Log mouse position constantly ===
            Vector2 mousePos = Input.GetMousePosition();
            Log("Mouse: ({mousePos.X:F2}, {mousePos.Y:F2})");
            
            // === DEBUG: Log on click ===
            if (Input.IsMouseButtonPressed(0))
            {
                Vector3 buttonPos = Transform.Position;
                
                Log("===================");
                Log("CLICK DETECTED!");
                Log("Mouse: ({mousePos.X}, {mousePos.Y})");
                Log("Button: ({buttonPos.X}, {buttonPos.Y}, {buttonPos.Z})");
                Log("Button Width: {buttonWidth}, Height: {buttonHeight}");
                
                float minX = buttonPos.X - (buttonWidth / 2f);
                float maxX = buttonPos.X + (buttonWidth / 2f);
                float minY = buttonPos.Y - (buttonHeight / 2f);
                float maxY = buttonPos.Y + (buttonHeight / 2f);
                
                Log("Button Bounds X: {minX} to {maxX}");
                Log("Button Bounds Y: {minY} to {maxY}");
                Log("Is Over: {IsMouseOverButton()}");
                Log("===================");
                
                // Try to switch anyway for testing
                OnButtonClicked();
            }
            
            // Keyboard shortcut for testing camera switching
            if (Input.IsKeyPressed(KeyCode.Space))
            {
                Log("Space key pressed - toggling camera");
                OnButtonClicked();
            }
        }

        public override void OnDestroy()
        {
            Log("StartGameButton: Destroyed");
        }

        // ===== HELPER METHODS =====
        
        // Check if mouse cursor is within button bounds, uses simple AABB collision
        private bool IsMouseOverButton()
        {
            // Get mouse position (in screen/window coordinates)
            Vector2 mousePos = Input.GetMousePosition();
            
            // Get button center position (from this entity's transform)
            Vector3 buttonPos = Transform.Position;
            
            // Calculate button bounds
            float minX = buttonPos.X - (buttonWidth / 2f);
            float maxX = buttonPos.X + (buttonWidth / 2f);
            float minY = buttonPos.Y - (buttonHeight / 2f);
            float maxY = buttonPos.Y + (buttonHeight / 2f);
            
            // Check if mouse is within bounds
            bool withinX = mousePos.X >= minX && mousePos.X <= maxX;
            bool withinY = mousePos.Y >= minY && mousePos.Y <= maxY;
            
            return withinX && withinY;
        }

        // Handle button click - to toggle between cameras
        private void OnButtonClicked()
        {
            Log("StartGameButton: Clicked! Switching cameras...");
            
            // Toggle state
            isShowingMainMenu = !isShowingMainMenu;
            
            // Update which camera is active
            UpdateCameraStates();
            
            // Optional: Add audio feedback
            // Audio.Play("ButtonClick");
        }

        // Find camera entities by name
        private void InitializeCameras()
        {
            // Find entities by name
            uint mainMenuID = InternalCalls.Scene_FindEntityByName(mainMenuCameraName);
            uint gameID = InternalCalls.Scene_FindEntityByName(gameCameraName);
            
            // Convert to ulong (EntityID is ulong in the engine)
            mainMenuCameraID = (ulong)mainMenuID;
            gameCameraID = (ulong)gameID;
            
            // Check if both cameras were found
            if (mainMenuID == 0)
            {
                LogError("StartGameButton: Camera not found: '{mainMenuCameraName}'");
            }
            
            if (gameID == 0)
            {
                LogError("StartGameButton: Camera not found: '{gameCameraName}'");
            }
            
            // Mark as initialized only if both cameras exist
            camerasInitialized = (mainMenuID != 0 && gameID != 0);
            
            if (camerasInitialized)
            {
                Log("StartGameButton: Cameras found - MainMenu ID: {mainMenuCameraID}, Game ID: {gameCameraID}");
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