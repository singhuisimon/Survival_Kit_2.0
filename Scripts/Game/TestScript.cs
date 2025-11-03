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
            if (playerID == 69) {

                playerID = Engine.InternalCalls.Scene_FindEntityByName("Player");

            }

            // Only try to move if we found the player
            //if (playerID == 0) return;

            // Check input and move player
            bool moved = false;
            float moveX = 0f;
            float moveY = 0f;
            float moveZ = 0f;

            if (Engine.Input.IsKeyPressed(Engine.KeyCode.W))
            {
                moveZ -= moveSpeed * deltaTime; // Forward
                Engine.InternalCalls.Log("Moving player forward!");
                Engine.InternalCalls.Log("Player found! ID: " + playerID.ToString());
                playerID = Engine.InternalCalls.Scene_FindEntityByName("Player");


                moved = true;
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.S))
            {
                moveZ += moveSpeed * deltaTime; // Backward
                Engine.InternalCalls.Log("Moving player Backward!");

                moved = true;
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.A))
            {
                moveX -= moveSpeed * deltaTime; // Left
                Engine.InternalCalls.Log("Moving player Left!");

                moved = true;
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.D))
            {
                moveX += moveSpeed * deltaTime; // Right
                Engine.InternalCalls.Log("Moving player Right!");

                moved = true;
            }
 

            // Apply movement if any key was pressed
            if (moved)
            {
                // Call internal Transform functions directly
                Engine.InternalCalls.Transform_Move(playerID, moveX, moveY, moveZ);

                // Log when W is pressed
                if (Engine.Input.IsKeyPressed(Engine.KeyCode.W))
                {
                    Engine.InternalCalls.Log("Moving player forward!");
                }
            }

            // Debug log every 60 frames
            if (frameCount % 60 == 0)
            {
                //Engine.InternalCalls.Log("Frame: " + frameCount.ToString());
            }
        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("TestScript destroyed!");
        }
    }
}
