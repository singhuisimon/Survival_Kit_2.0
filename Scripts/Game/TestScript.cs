using System;
using Engine;

namespace Game
{
    /// <summary>
    /// Spaceship player controller with third-person camera orbit
    /// - Spaceship entity has visual model (visible in camera)
    /// - Camera orbits around spaceship (third-person view)
    /// - Player rotates to face camera direction (yaw only)
    /// - WASD applies forces relative to spaceship's current rotation
    /// - Proper 3D rotation-based movement
    /// </summary>
    public class TestScript
    {
        // ===== Entity Reference =====
        public int EntityID = 3;                 // Player spaceship entity

        // ===== Camera Orbit Settings =====
        [SerializeField]
        private float orbitRadius = 7.5f;
        [SerializeField]
        private float orbitPitch = 0.25f;  // Initial pitch angle (radians)
        [SerializeField]
        private float orbitYaw = 0.0f;     // Initial yaw angle (radians)
        [SerializeField]
        private float mouseSensitivity = 0.05f;
        [SerializeField]
        private float aimHeightOffset = 2.0f;  // Player head position

        // ===== Movement Settings (Force-Based) =====
        [SerializeField]
        private float moveSpeed = 1.0f;
        [SerializeField]
        private float maxSpeed = 1.0f;
        [SerializeField]
        private bool moveAllowed = true;

        // ===== Dash Settings =====
        [SerializeField]
        private float dashForce = 2.0f;
        private bool isDashing = false;
        private float dashCooldown = 0.0f;
        private const float DASH_COOLDOWN_TIME = 1.0f;

        // ===== Constants =====
        private const float DEG2RAD = 0.0174532924f;
        private const float RAD2DEG = 57.2957795f;
        private const float PI = 3.14159265359f;
        private const float HALF_PI = 1.5707963268f;

        // ===== Previous mouse position (for delta tracking) =====
        private Engine.Vector2 previousMousePos = new Engine.Vector2(0.0f, 0.0f);

        // ===== Lifecycle Methods =====
        public void OnStart()
        {
            Engine.InternalCalls.Log("=== PlayerMovement Started ===");
            Engine.InternalCalls.Log("PlayerID: " + EntityID);

            // Add camera component to player entity
            Engine.InternalCalls.Entity_AddCamera((ulong)EntityID);
            Engine.InternalCalls.Log("Camera component added to player");

            // Set camera as primary
            Engine.InternalCalls.Camera_SetPrimary((ulong)EntityID, true);
            Engine.InternalCalls.Camera_SetEnabled((ulong)EntityID, true);

            // Set initial camera properties
            Engine.InternalCalls.Camera_SetFOV((ulong)EntityID, 60.0f);
            Engine.InternalCalls.Camera_SetNear((ulong)EntityID, 0.1f);
            Engine.InternalCalls.Camera_SetFar((ulong)EntityID, 1000.0f);

            // Add rigidbody for physics
            Engine.InternalCalls.Entity_AddRigidBody((ulong)EntityID);
            Engine.InternalCalls.Rigidbody_SetIsKinematic((ulong)EntityID, false);
            Engine.InternalCalls.Rigidbody_SetUseGravity((ulong)EntityID, false);
            Engine.InternalCalls.Rigidbody_SetMass((ulong)EntityID, 1.0f);
            Engine.InternalCalls.Log("Rigidbody configured for spaceship physics");
        }

        public void OnUpdate(float deltaTime)
        {
            if (!moveAllowed)
                return;

            // ===== Handle Dash Cooldown =====
            if (dashCooldown > 0.0f)
            {
                dashCooldown -= deltaTime;
                if (dashCooldown <= 0.0f)
                {
                    dashCooldown = 0.0f;
                    isDashing = false;
                }
            }

            // ===== Get Current Spaceship Position and Rotation =====
            // THIS is the spaceship body position - DO NOT MODIFY
            Engine.Vector3 shipPos;
            Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out shipPos);

            Engine.Vector3 shipRot;
            Engine.Transform.GetRotation_Native((ulong)EntityID, out shipRot);

            // ===== Camera Aim Target (spaceship center) =====
            Engine.Vector3 aimTarget = new Engine.Vector3(
                shipPos.X,
                shipPos.Y + aimHeightOffset,
                shipPos.Z
            );

            // ===== Handle Mouse Input for Camera Orbit =====
            // TODO: If your Input system has GetMousePosition, get current mouse pos
            // For now, we'll keep orbit angles static

            // Example: Manual mouse delta calculation (if you have GetMousePosition)
            // Engine.Vector2 currentMousePos = Engine.Input.GetMousePosition();
            // float xOffset = currentMousePos.X - previousMousePos.X;
            // float yOffset = currentMousePos.Y - previousMousePos.Y;
            // previousMousePos = currentMousePos;
            // 
            // if (xOffset != 0.0f || yOffset != 0.0f) {
            //     orbitYaw += (xOffset < 0.0f) ? mouseSensitivity : (xOffset > 0.0f ? -mouseSensitivity : 0.0f);
            //     orbitPitch += (yOffset > 0.0f) ? -mouseSensitivity : (yOffset < 0.0f ? mouseSensitivity : 0.0f);
            // }

            // Clamp pitch to avoid flipping
            if (orbitPitch > HALF_PI - 0.01f)
                orbitPitch = HALF_PI - 0.01f;
            if (orbitPitch < -HALF_PI + 0.01f)
                orbitPitch = -HALF_PI + 0.01f;

            // ===== Calculate Camera Position (orbit around spaceship) =====
            // Spherical coordinates to Cartesian
            float cosPitch = SimpleCos(orbitPitch);
            float sinPitch = SimpleSin(orbitPitch);
            float cosYaw = SimpleCos(orbitYaw);
            float sinYaw = SimpleSin(orbitYaw);

            // Direction from aim point
            Engine.Vector3 orbitDir = new Engine.Vector3(
                cosPitch * sinYaw,
                sinPitch,
                cosPitch * cosYaw
            );

            // Camera position (orbit radius away from aim target)
            Engine.Vector3 cameraPos = new Engine.Vector3(
                aimTarget.X + orbitDir.X * orbitRadius,
                aimTarget.Y + orbitDir.Y * orbitRadius,
                aimTarget.Z + orbitDir.Z * orbitRadius
            );

            // ===== Update Camera Target to spaceship center =====
            // Camera looks at aimTarget from cameraPos (calculated above for reference)
            Engine.InternalCalls.Camera_SetTarget((ulong)EntityID, ref aimTarget);

            // ===== Rotate Spaceship to Face Camera Direction (Yaw Only) =====
            // Calculate rotation to face the camera's yaw direction
            Engine.Vector3 newShipRot = new Engine.Vector3(
                shipRot.X,
                orbitYaw * RAD2DEG,  // Convert yaw to degrees for rotation
                shipRot.Z
            );
            Engine.Transform.SetRotation_Native((ulong)EntityID, ref newShipRot);

            // ===== Get Input =====
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

            bool hasInput = (inputX != 0.0f) || (inputZ != 0.0f) || (inputY != 0.0f);

            // ===== Convert local input to world-space based on SPACESHIP rotation =====
            Engine.Vector3 moveDirWorld = new Engine.Vector3(0.0f, 0.0f, 0.0f);
            if (hasInput)
            {
                moveDirWorld = GetMoveDirectionInWorld(inputX, inputY, inputZ, newShipRot);
            }

            // ===== Get Current Velocity =====
            Engine.Vector3 currentVel;
            Engine.InternalCalls.Rigidbody_GetVelocity((ulong)EntityID, out currentVel);

            // ===== Apply Movement Forces (Spaceship Physics) =====
            if (hasInput && !isDashing)
            {
                // Desired velocity in movement direction
                Engine.Vector3 desiredVel = new Engine.Vector3(
                    moveDirWorld.X * maxSpeed,
                    moveDirWorld.Y * maxSpeed,
                    moveDirWorld.Z * maxSpeed
                );

                // Calculate velocity change needed
                Engine.Vector3 velChange = new Engine.Vector3(
                    desiredVel.X - currentVel.X,
                    desiredVel.Y - currentVel.Y,
                    desiredVel.Z - currentVel.Z
                );

                // Apply force proportional to velocity change
                Engine.Vector3 force = new Engine.Vector3(
                    velChange.X * moveSpeed,
                    velChange.Y * moveSpeed,
                    velChange.Z * moveSpeed
                );

                Engine.InternalCalls.Rigidbody_AddForce((ulong)EntityID, ref force);
            }

            // ===== Handle Dash =====
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
                    dashDirWorld = GetMoveDirectionInWorld(0.0f, 0.0f, 1.0f, newShipRot);
                }

                PerformDash(dashDirWorld);
            }

            SendPositionEvent(shipPos);
        }

        // ===== World-Space Movement Direction (based on spaceship rotation) =====
        private Engine.Vector3 GetMoveDirectionInWorld(float inputX, float inputY, float inputZ, Engine.Vector3 rotation)
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

            // Use spaceship rotation (in degrees) - convert to radians
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
            // to world space based on spaceship's orientation

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

        // ===== Dash Implementation =====
        private void PerformDash(Engine.Vector3 dashDirWorld)
        {
            float lenSq = dashDirWorld.X * dashDirWorld.X +
                          dashDirWorld.Y * dashDirWorld.Y +
                          dashDirWorld.Z * dashDirWorld.Z;

            if (lenSq <= 0.000001f)
                return;

            float invLen = 1.0f / SimpleSqrt(lenSq);
            Engine.Vector3 dir = new Engine.Vector3(
                dashDirWorld.X * invLen,
                dashDirWorld.Y * invLen,
                dashDirWorld.Z * invLen
            );

            // Apply dash impulse
            Engine.Vector3 dashImpulse = new Engine.Vector3(
                dir.X * dashForce,
                dir.Y * dashForce,
                dir.Z * dashForce
            );

            Engine.InternalCalls.Rigidbody_AddForce((ulong)EntityID, ref dashImpulse);

            isDashing = true;
            dashCooldown = DASH_COOLDOWN_TIME;
            Engine.InternalCalls.Log("DASH!");
        }

        // ===== Math Helpers =====
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

        private float SimpleSqrt(float value)
        {
            if (value <= 0.0f) return 0.0f;
            if (value == 1.0f) return 1.0f;

            float x = value;
            for (int i = 0; i < 3; i++)
            {
                x = 0.5f * (x + value / x);
            }

            return x;
        }

        // ===== Control Methods =====
        public void Stop()
        {
            moveAllowed = false;
            Engine.InternalCalls.Rigidbody_Stop((ulong)EntityID);
            Engine.InternalCalls.Log("Player movement stopped");
        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("=== PlayerMovement Destroyed ===");
        }

        // ===== Event Helper =====
        private void SendPositionEvent(Engine.Vector3 shipPos)
        {
            string payload =
                EntityID.ToString() + "|" +
                shipPos.X.ToString() + "|" +
                shipPos.Y.ToString() + "|" +
                shipPos.Z.ToString();

            Engine.EventSystem.Publish("PlayerPosition", payload);
        }
    }
}