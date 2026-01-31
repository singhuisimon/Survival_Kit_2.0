using Engine;
using Game;
using System.Collections.Generic;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;

namespace Game 
{
    /// <summary>
    /// Moving wall of death that travels at constant speed and kills the player on contact.
    /// Attach to 'Obstacle_Wall_WallofDeath' prefab with tag "obstacle_wall_wallofdeath".
    /// 
    /// INSPECTOR SETUP (Pick ONE axis, then pick direction):
    /// - Check ONE axis: HorizontalX, VerticalY, or DepthZ
    /// - Check PositiveDirection for +axis, uncheck for -axis
    /// 
    /// Examples:
    /// - Move Right:   HorizontalX=true,  PositiveDirection=true
    /// - Move Left:    HorizontalX=true,  PositiveDirection=false
    /// - Move Up:      VerticalY=true,    PositiveDirection=true
    /// - Move Down:    VerticalY=true,    PositiveDirection=false
    /// - Move Forward: DepthZ=true,       PositiveDirection=true
    /// - Move Back:    DepthZ=true,       PositiveDirection=false
    /// </summary>
    public class WallOfDeath : ScriptBehaviour
    {
        // AXIS SELECTION (Check ONE)
        [SerializeField] private bool HorizontalX = true;  // Left/Right movement
        [SerializeField] private bool VerticalY = false;   // Up/Down movement
        [SerializeField] private bool DepthZ = false;      // Forward/Back movement
        
        // DIRECTION (Checked = Positive, Unchecked = Negative)
        [SerializeField] private bool PositiveDirection = true;
        
        // Movement & Damage
        [SerializeField] private float Speed = 50.0f;
        [SerializeField] private float Damage = 9999.0f;

        // Internal state
        private string playerName = "Player";
        private uint playerID = 0;
        private Vector3 movementVector;
        private bool isInitialized = false;
        private string directionName = "";

        [SerializeField] private float elapsedTime = 0.0f;
        [SerializeField] private float wallLifeTime = 500.0f;

        public override void OnStart()
        {
            // Find player
            playerID = SceneFindEntityByName(playerName);
            if (playerID == 0)
            {
                LogMessage("[WallOfDeath] ERROR: Player entity not found in scene!");
                return;
            }

            // Validate and calculate movement vector
            movementVector = CalculateMovementVector();
            
            LogMessage("[WallOfDeath] Initialized - EntityID: " + EntityID);
            LogMessage("[WallOfDeath] Direction: " + directionName + " | Speed: " + Speed + " | Damage: " + Damage);
            LogMessage("[WallOfDeath] Movement Vector: X=" + movementVector.X + 
                      " Y=" + movementVector.Y + " Z=" + movementVector.Z);

            elapsedTime = 0.0f;
            isInitialized = true;
        }

        public override void OnUpdate(float deltaTime){

            elapsedTime += deltaTime;

            if(elapsedTime >= wallLifeTime){
                SceneDestroyEntity((uint)EntityID);
                return;
            }

        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!isInitialized || playerID == 0)
                return;

            // Move the wall
            MoveWall(deltaTime);

            // Check for collision with player
            CheckCollisionWithPlayer();
        }

        public override void OnDestroy()
        {
            LogMessage("[WallOfDeath] Wall destroyed");
        }

        /// <summary>
        /// Calculate movement vector based on boolean settings
        /// </summary>
        private Vector3 CalculateMovementVector()
        {
            // Count how many axes are selected
            int axisCount = 0;
            if (HorizontalX) axisCount++;
            if (VerticalY) axisCount++;
            if (DepthZ) axisCount++;

            // Validate - must have exactly one axis selected
            if (axisCount == 0)
            {
                LogMessage("[WallOfDeath] ERROR: No axis selected! Defaulting to Horizontal X");
                HorizontalX = true;
            }
            else if (axisCount > 1)
            {
                LogMessage("[WallOfDeath] WARNING: Multiple axes selected! Using first found.");
            }

            // Determine direction multiplier
            float directionMultiplier = PositiveDirection ? 1.0f : -1.0f;

            // Build movement vector based on selected axis
            if (HorizontalX)
            {
                directionName = PositiveDirection ? "Right (+X)" : "Left (-X)";
                return new Vector3(directionMultiplier, 0.0f, 0.0f);
            }
            else if (VerticalY)
            {
                directionName = PositiveDirection ? "Up (+Y)" : "Down (-Y)";
                return new Vector3(0.0f, directionMultiplier, 0.0f);
            }
            else if (DepthZ)
            {
                directionName = PositiveDirection ? "Forward (+Z)" : "Backward (-Z)";
                return new Vector3(0.0f, 0.0f, directionMultiplier);
            }

            // Fallback (should never reach here)
            directionName = "Right (+X) [Default]";
            return new Vector3(1.0f, 0.0f, 0.0f);
        }

        /// <summary>
        /// Move the wall in the specified direction at constant speed
        /// </summary>
        private void MoveWall(float deltaTime)
        {
            // Get current position
            Vector3 currentPos = GetPosition((uint)EntityID);
            
            // Calculate new position
            Vector3 newPos = new Vector3(
                currentPos.X + (movementVector.X * Speed * deltaTime),
                currentPos.Y + (movementVector.Y * Speed * deltaTime),
                currentPos.Z + (movementVector.Z * Speed * deltaTime)
            );
            
            // Set new position
            SetPosition((uint)EntityID, ref newPos);
        }

        /// <summary>
        /// Check if player collided with this wall
        /// </summary>
        private void CheckCollisionWithPlayer()
        {
            // Query what the player collided with
            List<uint> playerCollisions = CollisionManager.GetPlayerCollisions(playerID);
            
            if (playerCollisions != null && playerCollisions.Count > 0)
            {
                // Check if THIS wall is in the player's collision list
                foreach (uint collidedEntityId in playerCollisions)
                {
                    if (collidedEntityId == (uint)EntityID)
                    {
                        // Player touched this moving wall - deal lethal damage
                        DamageSystem.DealDamage(playerID, Damage, (uint)EntityID);
                        LogMessage("[WallOfDeath] PLAYER HIT MOVING WALL! Dealing " + Damage + " damage");
                        
                        // Only deal damage once per frame
                        return;
                    }
                }
            }
        }
    }
}