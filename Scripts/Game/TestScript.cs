using System;
using Engine;

namespace Game
{
    public class TestScript
    {
        public int EntityID;
        private int frameCount = 0;
        private uint playerID = 69;
        private float moveSpeed = 5.0f;
        private float fireCD = 0.0f;
        private float fireTimer = 0.1f;

        public void OnStart()
        {
            Engine.InternalCalls.Log("TestScript started!");
        }

        public void OnUpdate(float deltaTime)
        {
            frameCount++;
            if (playerID == 69)
            {

                playerID = Engine.InternalCalls.Scene_FindEntityByName("Player");

            }
            // Only try to move if we found the player
            // if (playerID == 0) return;

            bool moved = false;
            float moveX = 0f;
            float moveY = 0f;
            float moveZ = 0f;

            // --- Arrow keys ---
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.Up))        // 265
            {
                moveZ -= moveSpeed * deltaTime; // Forward
                moved = true;   
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.Down))      // 264
            {
                moveZ += moveSpeed * deltaTime; // Backward
                moved = true;
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.Left))      // 263
            {
                moveX -= moveSpeed * deltaTime; // Left
                moved = true;
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.Right))     // 262
            {
                moveX += moveSpeed * deltaTime; // Right
                moved = true;
            }

            fireCD -= deltaTime;

            if (Engine.Input.IsKeyPressed(Engine.KeyCode.Enter) && fireCD <= 0)
            {
                Engine.InternalCalls.Log("Firing Bullet!");
                // Set cd
                fireCD = fireTimer;

                // Create bullet ent
                uint bullet = Engine.InternalCalls.Scene_CreateEntity("Bullet");

                // Add script to bullet
                Engine.InternalCalls.Entity_AddScript(bullet, "Game.Projectile");

                // Get player position
                Engine.Vector3 v3 = default; // must be initialized before ref
                Engine.InternalCalls.Transform_GetPosition((uint)EntityID, ref v3);

                // Place slightly in front (+Z adjust as you prefer)
                Engine.Vector3 spawn = new Engine.Vector3(v3.X, v3.Y, v3.Z + 0.5f);
                Engine.InternalCalls.Transform_SetPosition(bullet, ref spawn);

            }

            // Apply movement if any key was pressed
            if (moved)
            {
                Engine.InternalCalls.Transform_Move(playerID, moveX, moveY, moveZ);
            }

        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("TestScript destroyed!");
        }
    }
}
