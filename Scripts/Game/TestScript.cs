using System;
using Engine;

namespace Game
{
    public class TestScript
    {
        // ===== Entity Reference =====
        public int EntityID = 3 ;

        // ===== Serialized Fields (Editable in Inspector) =====
        [SerializeField]
        private float moveSpeed = 1.0f;

        [SerializeField]
        private float maxSpeed = 1.0f;

        [SerializeField]
        private float dashForce = 2.0f;

        [SerializeField]
        private bool moveAllowed = true;

        // ===== Private Fields =====
        private bool isDashing = false;
        private float dashCooldown = 0.0f;
        private const float DASH_COOLDOWN_TIME = 1.0f;

        // ===== Lifecycle Methods =====
        public void OnStart()
        {
            Engine.InternalCalls.Log("=== PlayerMovement Started ===");
            Engine.InternalCalls.Log("EntityID: " + EntityID);

            // Ensure rigidbody exists and configure it
            Engine.InternalCalls.Entity_AddRigidBody((uint)EntityID);
            Engine.InternalCalls.Rigidbody_SetIsKinematic((uint)EntityID, false);
            Engine.InternalCalls.Rigidbody_SetUseGravity((uint)EntityID, true);
            Engine.InternalCalls.Rigidbody_SetMass((uint)EntityID, 1.0f);

            Engine.InternalCalls.Log("Rigidbody configured successfully");
        }

        public void OnUpdate(float deltaTime)
        {
            if (!moveAllowed)
                return;

            // Update dash cooldown
            if (dashCooldown > 0)
                dashCooldown -= deltaTime;

            // ===== Get Current Velocity =====
            Engine.Vector3 currentVel;
            Engine.InternalCalls.Rigidbody_GetVelocity((uint)EntityID, out currentVel);

            // ===== WASD Input =====
            float inputX = 0.0f;
            float inputZ = 0.0f;

            if (Engine.Input.IsKeyPressed(Engine.KeyCode.W))
                inputZ += 1.0f;
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.S))
                inputZ -= 1.0f;
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.A))
                inputX -= 1.0f;
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.D))
                inputX += 1.0f;

            bool hasInput = (inputX != 0.0f) || (inputZ != 0.0f);

            // ===== Apply Movement =====
            if (hasInput && !isDashing)
            {
                // Normalize diagonal movement
                float inputLengthSq = inputX * inputX + inputZ * inputZ;
                if (inputLengthSq > 1.0f)
                {
                    float invLength = 1.0f / SimpleSqrt(inputLengthSq);
                    inputX *= invLength;
                    inputZ *= invLength;
                }

                // Calculate desired velocity
                Engine.Vector3 desiredVel = new Engine.Vector3(
                    inputX * maxSpeed,
                    currentVel.Y,  // Preserve Y velocity (gravity)
                    inputZ * maxSpeed
                );

                // Calculate velocity change needed
                Engine.Vector3 velChange = new Engine.Vector3(
                    desiredVel.X - currentVel.X,
                    0,
                    desiredVel.Z - currentVel.Z
                );

                // Apply force (acceleration)
                Engine.Vector3 force = new Engine.Vector3(
                    velChange.X * moveSpeed,
                    0,
                    velChange.Z * moveSpeed
                );

                Engine.InternalCalls.Rigidbody_AddForce((uint)EntityID, ref force);
            }

            // ===== Dash Mechanic =====
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.Space) && !isDashing && dashCooldown <= 0)
            {
                Dash(inputX, inputZ);
            }
        }

        private void Dash(float inputX, float inputZ)
        {
            // Use current input direction, or dash forward if no input
            if (inputX == 0.0f && inputZ == 0.0f)
                inputZ = 1.0f;

            // Normalize
            float inputLengthSq = inputX * inputX + inputZ * inputZ;
            if (inputLengthSq > 1.0f)
            {
                float invLength = 1.0f / SimpleSqrt(inputLengthSq);
                inputX *= invLength;
                inputZ *= invLength;
            }

            // Apply dash impulse
            Engine.Vector3 dashImpulse = new Engine.Vector3(
                inputX * dashForce,
                0,
                inputZ * dashForce
            );

            Engine.InternalCalls.Rigidbody_AddForce((uint)EntityID, ref dashImpulse);

            isDashing = true;
            dashCooldown = DASH_COOLDOWN_TIME;

            Engine.InternalCalls.Log("DASH!");
        }

        // ===== Simple Square Root (No Dependencies) =====
        private float SimpleSqrt(float value)
        {
            if (value <= 0.0f) return 0.0f;
            if (value == 1.0f) return 1.0f;

            // Newton-Raphson method (simple, no bit tricks)
            float x = value;
            float y = 1.0f;
            float epsilon = 0.0001f;

            // Just 3 iterations - enough for game physics
            for (int i = 0; i < 3; i++)
            {
                y = (x + value / x) * 0.5f;
                if ((x - y) < epsilon && (x - y) > -epsilon)
                    break;
                x = y;
            }

            return y;
        }

        public void Stop()
        {
            moveAllowed = false;
            Engine.InternalCalls.Rigidbody_Stop((uint)EntityID);
            Engine.InternalCalls.Log("Player movement stopped");
        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("=== PlayerMovement Destroyed ===");
        }
    }
}
