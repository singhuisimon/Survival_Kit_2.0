using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.Input;
using static Engine.Transform;
using static Engine.Audio;

namespace Game{
    public class PlayerMoveAudio : ScriptBehaviour {

        [SerializeField] private string playerName = "Audio_PlayerMove";
        [SerializeField] private float maxSpeed = 100.0f; // Should match SpaceshipController's spaceshipSpeed
        [SerializeField] private float speedThreshold = 0.11f; // 11% of max speed
        
        // Track previous frame's key states to detect "just pressed"
        private bool wasWPressed = false;
        private bool wasAPressed = false;
        private bool wasSPressed = false;
        private bool wasDPressed = false;
        
        private uint playerEntityID = 0;
        private uint audioEntityID = 0; // This script's entity (has AudioComponent)
        
        // Track position for speed calculation
        private Vector3 lastPlayerPos = Vector3.Zero;
        private bool hasLastPos = false;
        
        private float audioCooldown = 0.0f;
        private const float MIN_AUDIO_INTERVAL = 0.1f; // Minimum time between audio plays

        public override void OnStart(){
            // Find the player entity
            playerEntityID = SceneFindEntityByName(playerName);
            
            if (playerEntityID == 0){
                LogError("[PlayerMoveAudio] Player entity not found: " + playerName);
                return;
            }
            
            // Get this entity's ID (the audio entity)
            audioEntityID = EntityID;
            
            // Initialize last position
            lastPlayerPos = GetPosition(playerEntityID);
            hasLastPos = true;
            
            LogMessage("[PlayerMoveAudio] Initialized - Player ID: " + playerEntityID + ", Audio ID: " + audioEntityID);
        }

        public override void OnUpdate(float deltaTime){
            if (GameState.IsPaused)
                return;

            if (playerEntityID == 0 || audioEntityID == 0)
                return;
                
            // Get current player position
            Vector3 currentPos = GetPosition(playerEntityID);
            
            // Calculate current speed from position delta
            float currentSpeed = 0.0f;
            if (hasLastPos && deltaTime > 0.0f){
                Vector3 displacement = currentPos - lastPlayerPos;
                currentSpeed = displacement.Magnitude / deltaTime;
            }
            
            // Update cooldown timer
            if (audioCooldown > 0.0f){
                audioCooldown -= deltaTime;
            }
            
            // Get current key states
            bool isWPressed = IsKeyPressed(KeyCode.W);
            bool isAPressed = IsKeyPressed(KeyCode.A);
            bool isSPressed = IsKeyPressed(KeyCode.S);
            bool isDPressed = IsKeyPressed(KeyCode.D);
            
            // Detect if any key was JUST pressed (transition from not pressed to pressed)
            bool wJustPressed = isWPressed && !wasWPressed;
            bool aJustPressed = isAPressed && !wasAPressed;
            bool sJustPressed = isSPressed && !wasSPressed;
            bool dJustPressed = isDPressed && !wasDPressed;
            
            // Check if any WASD key was just pressed
            bool anyMovementKeyJustPressed = wJustPressed || aJustPressed || sJustPressed || dJustPressed;
            
            if (anyMovementKeyJustPressed){
                float speedPercentage = currentSpeed / maxSpeed;
                
                // Check if player is stationary or at ≤11% of max speed
                if (speedPercentage <= speedThreshold){
                    // Play audio if not on cooldown
                    if (audioCooldown <= 0.0f){
                        AudioPlay(audioEntityID);
                        audioCooldown = MIN_AUDIO_INTERVAL;
                        
                        LogMessage("[PlayerMoveAudio] Playing audio - Speed: " + currentSpeed + 
                                   " (" + (speedPercentage * 100.0f) + "% of max)");
                    }
                }
            }
            
            // Update previous frame states
            wasWPressed = isWPressed;
            wasAPressed = isAPressed;
            wasSPressed = isSPressed;
            wasDPressed = isDPressed;
            
            // Update last position for next frame
            lastPlayerPos = currentPos;
        }

        public override void OnFixedUpdate(float deltaTime){
            // Not needed for this implementation
        }

        public override void OnDestroy(){
            // Cleanup if needed
        }

    }

}