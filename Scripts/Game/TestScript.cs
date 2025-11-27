using System;
using System.Threading;
using Engine;

namespace Game
{
    public class TestScript
    {
        // ===== Entity Reference =====
        public int EntityID = 3;

        // ===== Serialized Fields (Editable in Inspector) =====
        [SerializeField]
        private float moveSpeed = 1.0f;

        [SerializeField]
        private float maxSpeed = 1.0f;

        [SerializeField]
        private float dashForce = 2.0f;

        [SerializeField]
        private bool moveAllowed = true;

        [SerializeField]
        private bool use3DMovement = true;

        [SerializeField]
        private float verticalSpeed = 1.0f;

        // ===== Private Fields =====
        private bool isDashing = false;
        private float dashCooldown = 0.0f;
        private const float DASH_COOLDOWN_TIME = 1.0f;

        private const float DEG2RAD = 0.0174532924f;
        private const float PI = 3.14159265359f;

        // ===== Lifecycle Methods =====
        public void OnStart()
        {
            Engine.InternalCalls.Log("=== PlayerMovement Started ===");
            Engine.InternalCalls.Log("EntityID: " + EntityID);

            // Ensure rigidbody exists and configure it
            Engine.InternalCalls.Entity_AddRigidBody((uint)EntityID);
            Engine.InternalCalls.Rigidbody_SetIsKinematic((uint)EntityID, false);
            Engine.InternalCalls.Rigidbody_SetUseGravity((uint)EntityID, !use3DMovement); // Disable gravity for 3D movement
            Engine.InternalCalls.Rigidbody_SetMass((uint)EntityID, 1.0f);

            Engine.InternalCalls.Log("Rigidbody configured successfully");
        }

        public void OnUpdate(float deltaTime)
        {
            if (!moveAllowed)
                return;

            // ===== Dash cooldown =====
            if (dashCooldown > 0.0f)
            {
                dashCooldown -= deltaTime;
                if (dashCooldown <= 0.0f)
                {
                    dashCooldown = 0.0f;
                    isDashing = false;
                }
            }

            // ===== Get Current Velocity =====
            Engine.Vector3 currentVel;
            Engine.InternalCalls.Rigidbody_GetVelocity((uint)EntityID, out currentVel);

            // ===== WASD Input (local space) =====
            float inputX = 0.0f;
            float inputZ = 0.0f;
            float inputY = 0.0f;

            if (Engine.Input.IsKeyPressed(Engine.KeyCode.W))
            {
                inputZ += 1.0f;
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.S))
            {
                inputZ -= 1.0f;
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.A))
            {
                inputX -= 1.0f;
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.D))
            {
                inputX += 1.0f;
            }

            // Vertical movement (Q/E for up/down)
            if (use3DMovement)
            {
                if (Engine.Input.IsKeyPressed(Engine.KeyCode.Q))
                {
                    inputY -= 1.0f;
                }
                if (Engine.Input.IsKeyPressed(Engine.KeyCode.E))
                {
                    inputY += 1.0f;
                }
            }

            bool hasInput = (inputX != 0.0f) || (inputZ != 0.0f) || (inputY != 0.0f);

            // ===== Convert local input to world-space based on player rotation =====
            Engine.Vector3 moveDirWorld = new Engine.Vector3(0.0f, 0.0f, 0.0f);
            if (hasInput)
            {
                moveDirWorld = GetMoveDirectionInWorld(inputX, inputY, inputZ);
                //InternalCalls.Log("Moving in: " + moveDirWorld.X.ToString() + ", " + moveDirWorld.Y.ToString() + ", " + moveDirWorld.Z.ToString());
            }

            // ===== Apply Movement (relative to facing direction) =====
            if (hasInput && !isDashing)
            {
                Engine.Vector3 desiredVel;

                if (use3DMovement)
                {
                    // Full 3D movement (no gravity influence)
                    desiredVel = new Engine.Vector3(
                        moveDirWorld.X * maxSpeed,
                        moveDirWorld.Y * verticalSpeed,
                        moveDirWorld.Z * maxSpeed
                    );
                }
                else
                {
                    // 2D movement with gravity
                    desiredVel = new Engine.Vector3(
                        moveDirWorld.X * maxSpeed,
                        currentVel.Y,  // preserve vertical velocity (gravity)
                        moveDirWorld.Z * maxSpeed
                    );
                }

                // Velocity change needed
                Engine.Vector3 velChange = new Engine.Vector3(
                    desiredVel.X - currentVel.X,
                    use3DMovement ? (desiredVel.Y - currentVel.Y) : 0.0f,
                    desiredVel.Z - currentVel.Z
                );

                // Apply force proportional to velocity change
                Engine.Vector3 force = new Engine.Vector3(
                    velChange.X * moveSpeed,
                    use3DMovement ? (velChange.Y * moveSpeed) : 0.0f,
                    velChange.Z * moveSpeed
                );

                Engine.InternalCalls.Rigidbody_AddForce((uint)EntityID, ref force);
            }

            // ===== Dash Mechanic (relative to facing direction) =====
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.Space) && !isDashing && dashCooldown <= 0.0f)
            {
                Engine.Vector3 dashDirWorld;

                if (hasInput)
                {
                    // Dash in the current movement direction
                    dashDirWorld = moveDirWorld;
                }
                else
                {
                    // No input: dash straight forward in the direction player is facing
                    // Local forward is (0, 0, 1)
                    dashDirWorld = GetMoveDirectionInWorld(0.0f, 0.0f, 1.0f);
                }

                Dash(dashDirWorld);
            }

            SendPositionEvent();

        }

        // ===== World-Space Movement Direction (based on player rotation) =====
        private Engine.Vector3 GetMoveDirectionInWorld(float inputX, float inputY, float inputZ)
        {
            // Local input direction
            if (inputX == 0.0f && inputY == 0.0f && inputZ == 0.0f)
                return new Engine.Vector3(0.0f, 0.0f, 0.0f);

            // Normalize local input (so diagonals aren't faster)
            float lenSq = inputX * inputX + inputY * inputY + inputZ * inputZ;
            if (lenSq > 1.0f)
            {
                float invLen = 1.0f / SimpleSqrt(lenSq);
                inputX *= invLen;
                inputY *= invLen;
                inputZ *= invLen;
            }

            // Get player rotation (Euler angles in degrees)
            Engine.Vector3 rotation = Engine.Transform.GetRotation((ulong)EntityID);
            float pitchRad = rotation.X * DEG2RAD;  // Pitch (X rotation)
            float yawRad = rotation.Y * DEG2RAD;     // Yaw (Y rotation)
            float rollRad = rotation.Z * DEG2RAD;    // Roll (Z rotation)

            // Calculate sin/cos for each axis
            float sinPitch = SimpleSin(pitchRad);
            float cosPitch = SimpleCos(pitchRad);
            float sinYaw = SimpleSin(yawRad);
            float cosYaw = SimpleCos(yawRad);
            float sinRoll = SimpleSin(rollRad);
            float cosRoll = SimpleCos(rollRad);

            // Full 3D rotation matrix (YXZ order - yaw, then pitch, then roll)
            // This transforms local space (where +Z is forward, +X is right, +Y is up)
            // to world space based on player's orientation

            // Rotate by yaw (Y axis)
            float x1 = inputX * cosYaw + inputZ * sinYaw;
            float y1 = inputY;
            float z1 = -inputX * sinYaw + inputZ * cosYaw;

            // Rotate by pitch (X axis)
            float x2 = x1;
            float y2 = y1 * cosPitch - z1 * sinPitch;
            float z2 = y1 * sinPitch + z1 * cosPitch;

            // Rotate by roll (Z axis)
            float wx = x2 * cosRoll - y2 * sinRoll;
            float wy = x2 * sinRoll + y2 * cosRoll;
            float wz = z2;

            // Normalize world-space direction
            float worldLenSq = wx * wx + wy * wy + wz * wz;
            if (worldLenSq > 0.000001f)
            {
                float invWorldLen = 1.0f / SimpleSqrt(worldLenSq);
                wx *= invWorldLen;
                wy *= invWorldLen;
                wz *= invWorldLen;
            }

            return new Engine.Vector3(wx, wy, wz);
        }

        // ===== Dash (uses world-space direction) =====
        private void Dash(Engine.Vector3 dashDirWorld)
        {
            float lenSq = dashDirWorld.X * dashDirWorld.X +
                          dashDirWorld.Y * dashDirWorld.Y +
                          dashDirWorld.Z * dashDirWorld.Z;

            if (lenSq <= 0.000001f)
                return;

            float invLen = 1.0f / SimpleSqrt(lenSq);
            Engine.Vector3 dir = new Engine.Vector3(
                dashDirWorld.X * invLen,
                0.0f,
                dashDirWorld.Z * invLen
            );

            // Apply dash impulse
            Engine.Vector3 dashImpulse = new Engine.Vector3(
                dir.X * dashForce,
                0.0f,
                dir.Z * dashForce
            );

            Engine.InternalCalls.Rigidbody_AddForce((uint)EntityID, ref dashImpulse);

            isDashing = true;
            dashCooldown = DASH_COOLDOWN_TIME;

            Engine.InternalCalls.Log("DASH!");
        }

        // ===== Simple Sine (Taylor Series Approximation) =====
        private float SimpleSin(float x)
        {
            // Normalize to [-PI, PI]
            while (x > PI) x -= 2.0f * PI;
            while (x < -PI) x += 2.0f * PI;

            float x2 = x * x;
            float x3 = x2 * x;
            float x5 = x3 * x2;
            float x7 = x5 * x2;

            return x - (x3 / 6.0f) + (x5 / 120.0f) - (x7 / 5040.0f);
        }

        // ===== Simple Cosine (Taylor Series Approximation) =====
        private float SimpleCos(float x)
        {
            // Normalize to [-PI, PI]
            while (x > PI) x -= 2.0f * PI;
            while (x < -PI) x += 2.0f * PI;

            float x2 = x * x;
            float x4 = x2 * x2;
            float x6 = x4 * x2;

            return 1.0f - (x2 / 2.0f) + (x4 / 24.0f) - (x6 / 720.0f);
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
                float diff = x - y;
                if (diff < epsilon && diff > -epsilon)
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

        // ===== Position Event Helper =====
        private void SendPositionEvent()
        {
            // Get current world position from Transform
            Engine.Vector3 pos = new Vector3 { };
            InternalCalls.Transform_GetPosition((uint)EntityID, out pos);

            string payload =
                EntityID.ToString() + "|" +
                pos.X.ToString() + "|" +
                pos.Y.ToString() + "|" +
                pos.Z.ToString();

            Engine.EventSystem.Publish("PlayerPosition", payload);
        }


    }
}