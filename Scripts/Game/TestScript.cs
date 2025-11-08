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
                fireCD = fireTimer;

                // NEW WAY: Instantiate from prefab
                string prefabPath = "Resources/Sources/Prefabs/Bullet.prefab";
                uint bullet = Engine.InternalCalls.Prefab_Instantiate(prefabPath);

                if (bullet != 0)
                {
                    // Get player position
                    Engine.Vector3 v3 = default;
                    Engine.InternalCalls.Transform_GetPosition((uint)EntityID, ref v3);

                    // Place bullet in front of player
                    Engine.Vector3 spawn = new Engine.Vector3(v3.X, v3.Y, v3.Z + 0.5f);
                    Engine.InternalCalls.Transform_SetPosition(bullet, ref spawn);
                }
                else
                {
                    Engine.InternalCalls.LogError("Failed to instantiate bullet prefab!");
                }
            }
        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("TestScript destroyed!");
        }
    }
}
