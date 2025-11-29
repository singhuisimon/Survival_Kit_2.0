using System;
using Engine;

namespace Game
{
    /// <summary>
    /// Unified Player Controller - Flying + Shooting
    /// Original camera logic (static orbit, Tab handled by C++ engine) + Weapon system
    /// </summary>
    public class TestScript:ScriptBehaviour
    {
        // ===== Entity Reference =====
       // public int EntityID = 3;

        // ===== Camera Orbit Settings =====
        [SerializeField]
        private float orbitRadius = 7.5f;
        [SerializeField]
        private float orbitPitch = 0.25f;
        [SerializeField]
        private float orbitYaw = 0.0f;
        [SerializeField]
        private float mouseSensitivity = 0.05f;
        [SerializeField]
        private float aimHeightOffset = 2.0f;

        // ===== Movement Settings =====
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

        // ===== Weapon Settings =====
        [SerializeField]
        private string PrimaryBulletPrefab = "Sources/Prefabs/PrimaryBullet.prefab";
        [SerializeField]
        private string SecondaryBulletPrefab = "Sources/Prefabs/SecondaryBullet.prefab";
        [SerializeField]
        private int PrimaryMaxAmmo = 100;
        [SerializeField]
        private float PrimaryReloadTime = 1.5f;
        [SerializeField]
        private float PrimaryFireRate = 0.3f;
        [SerializeField]
        private int SecondaryMaxAmmo = 5;
        [SerializeField]
        private float SecondaryReloadTime = 3.0f;
        [SerializeField]
        private float SecondaryFireRate = 0.8f;

        // Weapon state
        private int primaryCurrentAmmo = 100;
        private int secondaryCurrentAmmo = 5;
        private float primaryFireCooldown = 0.0f;
        private float secondaryFireCooldown = 0.0f;
        private float reloadTimer = 0.0f;
        private bool isReloading = false;
        private enum WeaponType { Primary, Secondary }
        private WeaponType currentWeapon = WeaponType.Primary;

        // ===== Camera Reference =====
        private uint mainCameraEntityID = 0;

        // ===== Input State Tracking (for "just pressed" detection) =====
        private bool spaceWasPressed = false;
        private bool rWasPressed = false;

        // ===== Constants =====
        private const float DEG2RAD = 0.0174532924f;
        private const float RAD2DEG = 57.2957795f;
        private const float PI = 3.14159265359f;
        private const float HALF_PI = 1.5707963268f;

        private Engine.Vector2 previousMousePos = new Engine.Vector2(0.0f, 0.0f);

        // ===== Lifecycle =====
        public void OnStart()
        {
            Engine.InternalCalls.Log("=== PlayerMovement Started ===");
            Engine.InternalCalls.Log("PlayerID: " + EntityID);

            // Find the MainCamera entity
            mainCameraEntityID = Engine.InternalCalls.Scene_FindEntityByName("MainCamera");
            if (mainCameraEntityID == 0)
            {
                Engine.InternalCalls.LogWarning("MainCamera not found! Shooting direction may be incorrect.");
            }
            else
            {
                Engine.InternalCalls.Log("Found MainCamera entity: " + mainCameraEntityID);
            }

            // Camera setup
/*            Engine.InternalCalls.Entity_AddCamera((uint)EntityID);
            Engine.InternalCalls.Log("Camera component added to player");

            Engine.InternalCalls.Camera_SetPrimary((uint)EntityID, true);
            Engine.InternalCalls.Camera_SetEnabled((uint)EntityID, true);
            Engine.InternalCalls.Camera_SetFOV((uint)EntityID, 60.0f);
            Engine.InternalCalls.Camera_SetNear((uint)EntityID, 0.1f);
            Engine.InternalCalls.Camera_SetFar((uint)EntityID, 1000.0f);*/

            // Physics setup
            Engine.InternalCalls.Entity_AddRigidBody((uint)EntityID);
            Engine.InternalCalls.Rigidbody_SetIsKinematic((uint)EntityID, false);
            Engine.InternalCalls.Rigidbody_SetUseGravity((uint)EntityID, false);
            Engine.InternalCalls.Rigidbody_SetMass((uint)EntityID, 1.0f);
            Engine.InternalCalls.Log("Rigidbody configured for spaceship physics");

            // Weapon setup
            primaryCurrentAmmo = PrimaryMaxAmmo;
            secondaryCurrentAmmo = SecondaryMaxAmmo;

            Engine.InternalCalls.Log("Player initialized - Movement + Shooting ready");
            Engine.InternalCalls.Log("Controls: WASD=Move, Space=Dash, J=Primary, K=Secondary, R=Reload");
        }

        public void OnUpdate(float deltaTime)
        {
            if (!moveAllowed)
                return;

            // Update cooldowns
            if (dashCooldown > 0.0f)
            {
                dashCooldown -= deltaTime;
                if (dashCooldown <= 0.0f)
                {
                    dashCooldown = 0.0f;
                    isDashing = false;
                }
            }

            if (primaryFireCooldown > 0) primaryFireCooldown -= deltaTime;
            if (secondaryFireCooldown > 0) secondaryFireCooldown -= deltaTime;

            // Get player state
            Engine.Vector3 shipPos;
            Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out shipPos);

            Engine.Vector3 shipRot;
            Engine.InternalCalls.Transform_GetRotation((uint)EntityID, out shipRot);

            // Camera orbit and aim target
            Engine.Vector3 aimTarget = new Engine.Vector3(
                shipPos.X,
                shipPos.Y + aimHeightOffset,
                shipPos.Z
            );

            // Camera orbit angles remain static (no mouse control in C# script)
            // Tab key is handled by C++ engine to toggle editor camera

            // Clamp pitch
            if (orbitPitch > HALF_PI - 0.01f) orbitPitch = HALF_PI - 0.01f;
            if (orbitPitch < -HALF_PI + 0.01f) orbitPitch = -HALF_PI + 0.01f;

            // Calculate camera position
            float cosPitch = SimpleCos(orbitPitch);
            float sinPitch = SimpleSin(orbitPitch);
            float cosYaw = SimpleCos(orbitYaw);
            float sinYaw = SimpleSin(orbitYaw);

            Engine.Vector3 orbitDir = new Engine.Vector3(
                cosPitch * sinYaw,
                sinPitch,
                cosPitch * cosYaw
            );

            Engine.Vector3 cameraPos = new Engine.Vector3(
                aimTarget.X + orbitDir.X * orbitRadius,
                aimTarget.Y + orbitDir.Y * orbitRadius,
                aimTarget.Z + orbitDir.Z * orbitRadius
            );

            // Update camera target
            //Engine.InternalCalls.Camera_SetTarget((uint)EntityID, ref aimTarget);

            // Rotate player to face camera yaw
            Engine.Vector3 newShipRot = new Engine.Vector3(
                shipRot.X,
                orbitYaw * RAD2DEG,
                shipRot.Z
            );
            Engine.InternalCalls.Transform_SetRotation((uint)EntityID, ref newShipRot);

            // Handle weapon input
            HandleWeaponInput(deltaTime);

            // Handle movement input
            float inputX = 0.0f;
            float inputZ = 0.0f;

            if (Engine.Input.IsKeyPressed(Engine.KeyCode.W)) inputZ += 1.0f;
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.S)) inputZ -= 1.0f;
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.A)) inputX -= 1.0f;
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.D)) inputX += 1.0f;

            bool hasInput = (inputX != 0.0f) || (inputZ != 0.0f);

            // Convert to world space
            Engine.Vector3 moveDirWorld = new Engine.Vector3(0.0f, 0.0f, 0.0f);
            if (hasInput)
            {
                moveDirWorld = GetMoveDirectionInWorld(inputX, 0.0f, inputZ, newShipRot);
            }

            // Get velocity
            Engine.Vector3 currentVel;
            Engine.InternalCalls.Rigidbody_GetVelocity((uint)EntityID, out currentVel);

            // Apply movement forces
            if (hasInput && !isDashing)
            {
                Engine.Vector3 desiredVel = new Engine.Vector3(
                    moveDirWorld.X * maxSpeed,
                    moveDirWorld.Y * maxSpeed,
                    moveDirWorld.Z * maxSpeed
                );

                Engine.Vector3 velChange = new Engine.Vector3(
                    desiredVel.X - currentVel.X,
                    desiredVel.Y - currentVel.Y,
                    desiredVel.Z - currentVel.Z
                );

                Engine.Vector3 force = new Engine.Vector3(
                    velChange.X * moveSpeed,
                    velChange.Y * moveSpeed,
                    velChange.Z * moveSpeed
                );

                Engine.InternalCalls.Rigidbody_AddForce((uint)EntityID, ref force);
            }

            // Handle dash - manual "just pressed" detection
            bool spaceIsPressed = Engine.Input.IsKeyPressed(Engine.KeyCode.Space);
            bool spaceJustPressed = spaceIsPressed && !spaceWasPressed;
            spaceWasPressed = spaceIsPressed;

            if (spaceJustPressed && !isDashing && dashCooldown <= 0.0f)
            {
                if (hasInput)
                {
                    PerformDash(moveDirWorld);
                }
            }

            // Send position event
            SendPositionEvent(shipPos);

            // Handle reload
            if (isReloading)
            {
                reloadTimer -= deltaTime;
                if (reloadTimer <= 0.0f)
                {
                    CompleteReload();
                }
            }
        }

        // ===== WEAPON SYSTEM =====

        private void HandleWeaponInput(float deltaTime)
        {
            // Shooting
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.J))
            {
                TryShootPrimary();
            }

            if (Engine.Input.IsKeyPressed(Engine.KeyCode.K))
            {
                TryShootSecondary();
            }

            // Reload - manual "just pressed" detection
            bool rIsPressed = Engine.Input.IsKeyPressed(Engine.KeyCode.R);
            bool rJustPressed = rIsPressed && !rWasPressed;
            rWasPressed = rIsPressed;

            if (rJustPressed)
            {
                TryReload();
            }
        }

        private void TryShootPrimary()
        {
            if (isReloading) return;
            if (primaryFireCooldown > 0) return;
            if (primaryCurrentAmmo <= 0) return;

            currentWeapon = WeaponType.Primary;
            primaryCurrentAmmo--;
            primaryFireCooldown = PrimaryFireRate;

            Engine.InternalCalls.Log("PRIMARY FIRE! Ammo: " + primaryCurrentAmmo + "/" + PrimaryMaxAmmo);
            SpawnBullet(PrimaryBulletPrefab);
        }

        private void TryShootSecondary()
        {
            if (isReloading) return;
            if (secondaryFireCooldown > 0) return;
            if (secondaryCurrentAmmo <= 0) return;

            currentWeapon = WeaponType.Secondary;
            secondaryCurrentAmmo--;
            secondaryFireCooldown = SecondaryFireRate;

            Engine.InternalCalls.Log("SECONDARY FIRE! Ammo: " + secondaryCurrentAmmo + "/" + SecondaryMaxAmmo);
            SpawnBullet(SecondaryBulletPrefab);
        }

        private void SpawnBullet(string prefabPath)
        {
            try
            {
                // Get player position
                Engine.Vector3 playerPosition;
                Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out playerPosition);

                // Calculate aimTarget - this is where the camera is looking at
                Engine.Vector3 aimTarget = new Engine.Vector3(
                    playerPosition.X,
                    playerPosition.Y + aimHeightOffset,
                    playerPosition.Z
                );

                // Get the ACTUAL MainCamera position (updated by C++ with mouse movement)
                Engine.Vector3 cameraPosition;
                if (mainCameraEntityID != 0)
                {
                    Engine.InternalCalls.Transform_GetPosition(mainCameraEntityID, out cameraPosition);
                }
                else
                {
                    // Fallback: use calculated camera position if MainCamera not found
                    float cosPitch = SimpleCos(orbitPitch);
                    float sinPitch = SimpleSin(orbitPitch);
                    float cosYaw = SimpleCos(orbitYaw);
                    float sinYaw = SimpleSin(orbitYaw);

                    cameraPosition = new Engine.Vector3(
                        aimTarget.X + cosPitch * sinYaw * orbitRadius,
                        aimTarget.Y + sinPitch * orbitRadius,
                        aimTarget.Z + cosPitch * cosYaw * orbitRadius
                    );
                }

                // Calculate direction FROM camera TO aimTarget (where camera is looking)
                Engine.Vector3 cameraToAimTarget = new Engine.Vector3(
                    aimTarget.X - cameraPosition.X,
                    aimTarget.Y - cameraPosition.Y,
                    aimTarget.Z - cameraPosition.Z
                );

                // Use the FULL 3D direction for shooting
                Engine.Vector3 shootDirection = cameraToAimTarget;

                // Normalize the full 3D vector
                float lenSq = shootDirection.X * shootDirection.X +
                              shootDirection.Y * shootDirection.Y +
                              shootDirection.Z * shootDirection.Z;

                if (lenSq > 0.0001f)
                {
                    float invLen = 1.0f / SimpleSqrt(lenSq);
                    shootDirection = new Engine.Vector3(
                        shootDirection.X * invLen,
                        shootDirection.Y * invLen,
                        shootDirection.Z * invLen
                    );
                }

                // Spawn position - balanced distance and lower height
                float spawnDistance = 1.5f;  // Far enough to clear player collider
                float bulletHeightOffset = 1.0f;  // Lower than aimTarget but not too low

                // Calculate horizontal-only direction for spawn position offset
                Engine.Vector3 horizontalDirection = new Engine.Vector3(
                    shootDirection.X,
                    0.0f,
                    shootDirection.Z
                );

                // Normalize horizontal direction
                float horizLenSq = horizontalDirection.X * horizontalDirection.X +
                                   horizontalDirection.Z * horizontalDirection.Z;

                if (horizLenSq > 0.0001f)
                {
                    float horizInvLen = 1.0f / SimpleSqrt(horizLenSq);
                    horizontalDirection = new Engine.Vector3(
                        horizontalDirection.X * horizInvLen,
                        0.0f,
                        horizontalDirection.Z * horizInvLen
                    );
                }

                // Spawn in front of player at lower height
                Engine.Vector3 firingPosition = new Engine.Vector3(
                    playerPosition.X + (horizontalDirection.X * spawnDistance),
                    playerPosition.Y + bulletHeightOffset,
                    playerPosition.Z + (horizontalDirection.Z * spawnDistance)
                );

                // Instantiate
                uint bulletEntityID = Engine.InternalCalls.Prefab_Instantiate(prefabPath);
                if (bulletEntityID == 0)
                {
                    Engine.InternalCalls.LogError("Failed to instantiate bullet!");
                    return;
                }

                Engine.InternalCalls.Transform_SetPosition(bulletEntityID, ref firingPosition);

                // Set velocity with FULL 3D direction (this allows up/down shooting)
                float bulletSpeed = prefabPath.Contains("Primary") ? 50.0f : 30.0f;
                Engine.Vector3 velocity = new Engine.Vector3(
                    shootDirection.X * bulletSpeed,
                    shootDirection.Y * bulletSpeed,
                    shootDirection.Z * bulletSpeed
                );

                Engine.InternalCalls.Rigidbody_SetVelocity(bulletEntityID, ref velocity);
            }
            catch (Exception e)
            {
                Engine.InternalCalls.LogError("Error spawning bullet: " + e.Message);
            }
        }

        private void TryReload()
        {
            if (isReloading) return;

            if (currentWeapon == WeaponType.Primary)
            {
                if (primaryCurrentAmmo >= PrimaryMaxAmmo) return;
                isReloading = true;
                reloadTimer = PrimaryReloadTime;
                Engine.InternalCalls.Log("Reloading Primary...");
            }
            else
            {
                if (secondaryCurrentAmmo >= SecondaryMaxAmmo) return;
                isReloading = true;
                reloadTimer = SecondaryReloadTime;
                Engine.InternalCalls.Log("Reloading Secondary...");
            }
        }

        private void CompleteReload()
        {
            isReloading = false;
            if (currentWeapon == WeaponType.Primary)
            {
                primaryCurrentAmmo = PrimaryMaxAmmo;
                Engine.InternalCalls.Log("Primary reloaded!");
            }
            else
            {
                secondaryCurrentAmmo = SecondaryMaxAmmo;
                Engine.InternalCalls.Log("Secondary reloaded!");
            }
        }

        // ===== MOVEMENT =====

        private Engine.Vector3 GetMoveDirectionInWorld(float inputX, float inputY, float inputZ, Engine.Vector3 rotation)
        {
            if (inputX == 0.0f && inputY == 0.0f && inputZ == 0.0f)
                return new Engine.Vector3(0.0f, 0.0f, 0.0f);

            float lenSq = inputX * inputX + inputY * inputY + inputZ * inputZ;
            if (lenSq > 1.0f)
            {
                float invLen = 1.0f / SimpleSqrt(lenSq);
                inputX *= invLen;
                inputY *= invLen;
                inputZ *= invLen;
            }

            float pitchRad = rotation.X * DEG2RAD;
            float yawRad = rotation.Y * DEG2RAD;
            float rollRad = rotation.Z * DEG2RAD;

            float sinPitch = SimpleSin(pitchRad);
            float cosPitch = SimpleCos(pitchRad);
            float sinYaw = SimpleSin(yawRad);
            float cosYaw = SimpleCos(yawRad);
            float sinRoll = SimpleSin(rollRad);
            float cosRoll = SimpleCos(rollRad);

            float x1 = inputX * cosYaw + inputZ * sinYaw;
            float y1 = inputY;
            float z1 = -inputX * sinYaw + inputZ * cosYaw;

            float x2 = x1;
            float y2 = y1 * cosPitch - z1 * sinPitch;
            float z2 = y1 * sinPitch + z1 * cosPitch;

            float wx = x2 * cosRoll - y2 * sinRoll;
            float wy = x2 * sinRoll + y2 * cosRoll;
            float wz = z2;

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

        private void PerformDash(Engine.Vector3 dashDirWorld)
        {
            float lenSq = dashDirWorld.X * dashDirWorld.X +
                          dashDirWorld.Y * dashDirWorld.Y +
                          dashDirWorld.Z * dashDirWorld.Z;

            if (lenSq <= 0.000001f) return;

            float invLen = 1.0f / SimpleSqrt(lenSq);
            Engine.Vector3 dir = new Engine.Vector3(
                dashDirWorld.X * invLen,
                dashDirWorld.Y * invLen,
                dashDirWorld.Z * invLen
            );

            Engine.Vector3 dashImpulse = new Engine.Vector3(
                dir.X * dashForce,
                dir.Y * dashForce,
                dir.Z * dashForce
            );

            Engine.InternalCalls.Rigidbody_AddForce((uint)EntityID, ref dashImpulse);

            isDashing = true;
            dashCooldown = DASH_COOLDOWN_TIME;
            Engine.InternalCalls.Log("DASH!");
        }

        // ===== MATH =====

        private float SimpleSin(float x)
        {
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

        // ===== EVENTS =====

        private void SendPositionEvent(Engine.Vector3 shipPos)
        {
            string payload =
                EntityID.ToString() + "|" +
                shipPos.X.ToString() + "|" +
                shipPos.Y.ToString() + "|" +
                shipPos.Z.ToString();

            Engine.EventSystem.Publish("PlayerPosition", payload);
        }

        public void Stop()
        {
            moveAllowed = false;
            Engine.InternalCalls.Rigidbody_Stop((uint)EntityID);
            Engine.InternalCalls.Log("Player stopped");
        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("=== PlayerMovement Destroyed ===");
        }
    }
}