using System;
using Engine;

namespace Game
{
    public class TestScript
    {
        public int EntityID;
        private int frameCount = 0;
        private uint playerID = 99;
        private float fireCD = 0.0f;
        private float fireTimer = 0.1f;

        [SerializeField]
        private int health = 100;

        [SerializeField("Run Speed")]
        private float speed = 0.1f;

        [SerializeField]
        private bool isActive = true;

        [SerializeField]
        private string characterName = "Player";

        public void OnStart()
        {
            Engine.InternalCalls.Log("TestScript started!");
            // Cache and set up the player entity + rigidbody once
            if (playerID == 99)
            {
                playerID = Engine.InternalCalls.Scene_FindEntityByName("Player");
                Engine.InternalCalls.Entity_AddRigidBody(playerID);
                Engine.InternalCalls.Log("Player Rigidbody added/ensured.");
                Engine.InternalCalls.Log("Player successfully set up");
            }
        }

        public void OnUpdate(float deltaTime)
        {
            frameCount++;

            // -------- Rigidbody WASD movement for Player (no System.Math) --------
            if (frameCount % 60 == 0)
            {
                Engine.InternalCalls.Log("Health: " + health + " | Speed: " + speed + " | isActive: " + isActive + " | characterName: " + characterName);
            }
            Engine.Vector3 inputDir = default;

            // Forward / backward (Z)
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.W))
                inputDir.Z += 1.0f;
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.S))
                inputDir.Z -= 1.0f;

            // Right / left (X)
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.D))
                inputDir.X += 1.0f;
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.A))
                inputDir.X -= 1.0f;

            bool hasInput = (inputDir.X != 0.0f) || (inputDir.Z != 0.0f);

            Engine.Vector3 vel;
            Engine.InternalCalls.Rigidbody_GetVelocity(playerID, out vel);

            if (hasInput)
            {
                // Normalize XZ manually for diagonals, without System.Math
                float normX = inputDir.X;
                float normZ = inputDir.Z;

                // If moving diagonally (both X and Z non-zero), scale by 1/sqrt(2)
                if (normX != 0.0f && normZ != 0.0f)
                {
                    const float INV_SQRT2 = 0.70710678f; // ~1/sqrt(2)
                    normX *= INV_SQRT2;
                    normZ *= INV_SQRT2;
                }

                // Set horizontal velocity; keep Y so gravity/jumps still work
                vel.X += normX * speed;
                vel.Z += normZ * speed;

                // Apply velocity to the rigidbody
                Engine.InternalCalls.Rigidbody_SetVelocity(playerID, ref vel);

                // Log the current velocity
                Engine.InternalCalls.Log(
                    "Player velocity: X=" + vel.X +
                    ", Y=" + vel.Y +
                    ", Z=" + vel.Z
                );
            }

            // -------- Existing fire / bullet logic --------
            fireCD -= deltaTime;
            //Engine.InternalCalls.Log(string.Concat("FireCD: ", fireCD.ToString()));

            if (Engine.Input.IsKeyPressed(Engine.KeyCode.Enter) && fireCD <= 0)
            {
                fireCD = fireTimer;

                // Create bullet ent
                uint bullet = Engine.InternalCalls.Scene_CreateEntity("Bullet");

                // Add script to bullet
                Engine.InternalCalls.Entity_AddScript(bullet, "Game.Projectile");
                Engine.InternalCalls.Entity_AddRigidBody(bullet);

                Engine.Vector3 v3 = default;
                Engine.Vector3 spawn = new Engine.Vector3(v3.X, v3.Y, v3.Z + 0.5f);
                Engine.InternalCalls.Transform_SetPosition(bullet, ref spawn);

                Engine.Vector3 bulletVel = new Engine.Vector3(0, 0, 1400f);
                Engine.InternalCalls.Rigidbody_SetVelocity(bullet, ref bulletVel);

                Engine.InternalCalls.Log("Firing Bullet!");
            }
        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("TestScript destroyed!");
        }
    }
}
