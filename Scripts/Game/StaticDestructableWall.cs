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
    public class StaticDestructableWall: ScriptBehaviour
    {
        [SerializeField] private bool CollidedWithPlayer = false;

        // Movement & Damage
        [SerializeField] private float Health = 25.0f;
        [SerializeField] private float Damage = 9999.0f;

        // Internal state
        private string playerName = "Player";
        private uint playerID = 0;
        private bool isInitialized = false;

        public override void OnStart()
        {
            // Find player
            playerID = SceneFindEntityByName(playerName);
            if (playerID == 0)
            {
                LogMessage("[StaticWallOfDeath] ERROR: Player entity not found in scene!");
                return;
            }
            isInitialized = true;
        }

        public override void OnUpdate(float deltaTime)
        {
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!isInitialized || playerID == 0)
                return;

            // Check for collision with player
            CheckCollisionWithPlayer();
        }

        public override void OnDestroy()
        {
            LogMessage("[WallOfDeath] Wall destroyed");
        }

        /// <summary>
        /// Check if player collided with this wall
        /// </summary>
        private void CheckCollisionWithPlayer()
        {
            if (CollidedWithPlayer)
            {
                return;
            }

            // Query what the player collided with
            List<uint> playerCollisions = CollisionManager.GetPlayerCollisions(playerID);

            if (playerCollisions != null && playerCollisions.Count > 0)
            {
                // Check if THIS wall is in the player's collision list
                foreach (uint collidedEntityId in playerCollisions)
                {
                    if (collidedEntityId == (uint)EntityID)
                    {
                        CollidedWithPlayer = true;
                        // Player touched this moving wall - deal lethal damage
                        DamageSystem.DealDamage(playerID, Damage, (uint)EntityID);
                        LogMessage("[StaticWallOfDeath] PLAYER HIT WALL! Dealing " + Damage + " damage");

                        // Only deal damage once per frame
                        return;
                    }
                }
            }
        }
    }
}