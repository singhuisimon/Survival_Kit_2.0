using System;

namespace Game
{
    public class TestScript
    {
        public int EntityID;
        private int frameCount = 0;
        private uint playerID = 69;
        private float moveSpeed = 5.0f;

        public void OnStart()
        {
            Engine.InternalCalls.Log("TestScript started!");

            // Find player using your Scene_FindEntityByName
           // playerID = Engine.InternalCalls.Scene_FindEntityByName("Player");

/*            if (playerID != 0)
            {
                Engine.InternalCalls.Log("Player found! ID: " + playerID.ToString());
            }
            else
            {
                Engine.InternalCalls.LogWarning("Player not found in scene!");
            }*/
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
