using System;
using Engine;

namespace Game
{
    /// <summary>
    /// Player Controller - Movement and Shooting
    /// Camera control handled by C++ Game.cpp
    /// </summary>
    public class TestScript : ScriptBehaviour
    {
        // ===== Movement Settings =====
        [SerializeField]
        private float moveSpeed = 5.5f;
        [SerializeField]
        private bool moveAllowed = true;

        // ===== Camera Settings (for fallback) =====
        [SerializeField]
        private float aimHeightOffset = 2.0f;
        [SerializeField]
        private float orbitRadius = 7.5f;
        [SerializeField]
        private float orbitPitch = 0.25f;
        [SerializeField]
        private float orbitYaw = 0.0f;

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

        // ===== Input State Tracking =====
        private bool spaceWasPressed = false;
        private bool rWasPressed = false;
        private bool initialized = false;

        // ===== Constants =====
        private const float DEG2RAD = 0.0174532924f;
        private const float RAD2DEG = 57.2957795f;
        private const float PI = 3.14159265359f;
        private const float HALF_PI = 1.5707963268f;

        public override void OnStart()
        {
            Engine.InternalCalls.Log("=== PlayerMovement Started ===");
            Engine.InternalCalls.Log("PlayerID: " + EntityID);

            EntityID = Engine.InternalCalls.Scene_FindEntityByName("Player");
            // Find MainCamera using the proper InternalCall
            mainCameraEntityID = Engine.InternalCalls.Scene_FindEntityByName("MainCamera");

            Engine.InternalCalls.Log("Found MainCamera with ID: " + mainCameraEntityID);

            // Weapon setup
            primaryCurrentAmmo = PrimaryMaxAmmo;
            secondaryCurrentAmmo = SecondaryMaxAmmo;

            initialized = true;
            Engine.InternalCalls.Log("Player initialized - Movement + Shooting ready");
            Engine.InternalCalls.Log("Camera controlled by C++ - use mouse to look around");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!moveAllowed || !initialized)
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

            // Get player position (set by C++)
            Engine.Vector3 playerPos;
            Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out playerPos);

            // Get camera position (updated by C++)
            Engine.Vector3 cameraPosition;
            Engine.InternalCalls.Transform_GetPosition(mainCameraEntityID, out cameraPosition);

            // ===== MOVEMENT INPUT =====
            // Calculate forward direction from camera to player (horizontal plane only)
            Engine.Vector3 camToPlayer = new Engine.Vector3(
                playerPos.X - cameraPosition.X,
                0.0f,  // Ignore Y difference for horizontal movement
                playerPos.Z - cameraPosition.Z
            );

            // Normalize to get forward direction (camera looking at player)
            float camToPlayerLen = SimpleSqrt(camToPlayer.X * camToPlayer.X + camToPlayer.Z * camToPlayer.Z);
            Engine.Vector3 forward = new Engine.Vector3(0.0f, 0.0f, 0.0f);

            if (camToPlayerLen > 0.000001f)
            {
                forward.X = camToPlayer.X / camToPlayerLen;
                forward.Z = camToPlayer.Z / camToPlayerLen;
            }

            // Calculate right vector (perpendicular to forward on horizontal plane)
            Engine.Vector3 worldUp = new Engine.Vector3(0.0f, 1.0f, 0.0f);
            Engine.Vector3 right = CrossProduct(forward, worldUp);
            float rightLen = SimpleSqrt(right.X * right.X + right.Z * right.Z);
            if (rightLen > 0.000001f)
            {
                right.X /= rightLen;
                right.Z /= rightLen;
            }

            Engine.Vector3 moveDir = new Engine.Vector3(0.0f, 0.0f, 0.0f);

            // Movement controls - relative to camera view
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.W))  // W = Move away from camera (forward)
            {
                moveDir.X += forward.X;
                moveDir.Z += forward.Z;
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.S))  // S = Move toward camera (backward)
            {
                moveDir.X -= forward.X;
                moveDir.Z -= forward.Z;
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.A))  // A = Move left relative to camera
            {
                moveDir.X -= right.X;
                moveDir.Z -= right.Z;
            }
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.D))  // D = Move right relative to camera
            {
                moveDir.X += right.X;
                moveDir.Z += right.Z;
            }

            // Normalize movement direction (FULL 3D)
            float moveDirLenSq = moveDir.X * moveDir.X + moveDir.Y * moveDir.Y + moveDir.Z * moveDir.Z;
            if (moveDirLenSq > 0.000001f)
            {
                float moveDirLen = SimpleSqrt(moveDirLenSq);
                moveDir.X /= moveDirLen;
                moveDir.Y /= moveDirLen;
                moveDir.Z /= moveDirLen;
            }

            // Apply movement if not dashing (FULL 3D FLIGHT)
            if (moveDirLenSq > 0.000001f && !isDashing)
            {
                Engine.Vector3 newPos = new Engine.Vector3(
                    playerPos.X + moveDir.X * moveSpeed,
                    playerPos.Y + moveDir.Y * moveSpeed,
                    playerPos.Z + moveDir.Z * moveSpeed
                );
                Engine.InternalCalls.Transform_SetPosition((uint)EntityID, ref newPos);
            }

            // Handle dash (FULL 3D)
            bool spaceIsPressed = Engine.Input.IsKeyPressed(Engine.KeyCode.Space);
            bool spaceJustPressed = spaceIsPressed && !spaceWasPressed;
            spaceWasPressed = spaceIsPressed;

            if (spaceJustPressed && !isDashing && dashCooldown <= 0.0f)
            {
                if (moveDirLenSq > 0.000001f)
                {
                    Engine.Vector3 dashImpulse = new Engine.Vector3(
                        moveDir.X * dashForce,
                        moveDir.Y * dashForce,
                        moveDir.Z * dashForce
                    );
                    Engine.InternalCalls.Rigidbody_AddForce((uint)EntityID, ref dashImpulse);
                    isDashing = true;
                    dashCooldown = DASH_COOLDOWN_TIME;
                    Engine.InternalCalls.Log("DASH!");
                }
            }

            // ===== WEAPON INPUT =====
            HandleWeaponInput(deltaTime);

            // ===== RELOAD =====
            if (isReloading)
            {
                reloadTimer -= deltaTime;
                if (reloadTimer <= 0.0f)
                {
                    CompleteReload();
                }
            }

            // Send position event
            SendPositionEvent(playerPos);
        }

        // ===== WEAPON SYSTEM =====
        private void HandleWeaponInput(float deltaTime)
        {
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.J))
            {
                TryShootPrimary();
            }

            if (Engine.Input.IsKeyPressed(Engine.KeyCode.K))
            {
                TryShootSecondary();
            }

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
            if (isReloading || primaryFireCooldown > 0 || primaryCurrentAmmo <= 0)
                return;

            currentWeapon = WeaponType.Primary;
            primaryCurrentAmmo--;
            primaryFireCooldown = PrimaryFireRate;

            Engine.InternalCalls.Log("PRIMARY FIRE! Ammo: " + primaryCurrentAmmo + "/" + PrimaryMaxAmmo);
            SpawnBullet(PrimaryBulletPrefab, 50.0f);
        }

        private void TryShootSecondary()
        {
            if (isReloading || secondaryFireCooldown > 0 || secondaryCurrentAmmo <= 0)
                return;

            currentWeapon = WeaponType.Secondary;
            secondaryCurrentAmmo--;
            secondaryFireCooldown = SecondaryFireRate;

            Engine.InternalCalls.Log("SECONDARY FIRE! Ammo: " + secondaryCurrentAmmo + "/" + SecondaryMaxAmmo);
            SpawnBullet(SecondaryBulletPrefab, 30.0f);
        }

        private void SpawnBullet(string prefabPath, float bulletSpeed)
        {
            try
            {
                Engine.InternalCalls.Log("=== SPAWNING BULLET ===");
                Engine.InternalCalls.Log("Prefab path: " + prefabPath);
                Engine.InternalCalls.Log("Bullet speed: " + bulletSpeed);

                // Get player position
                Engine.Vector3 playerPosition;
                Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out playerPosition);
                Engine.InternalCalls.Log("Player position: " + playerPosition.X + ", " + playerPosition.Y + ", " + playerPosition.Z);

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
                    Engine.InternalCalls.LogWarning("MainCamera not found, using fallback position");
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

                Engine.InternalCalls.Log("Camera position: " + cameraPosition.X + ", " + cameraPosition.Y + ", " + cameraPosition.Z);

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
                else
                {
                    Engine.InternalCalls.LogError("Invalid shoot direction - length is zero!");
                    return;
                }

                Engine.InternalCalls.Log("Shoot direction: " + shootDirection.X + ", " + shootDirection.Y + ", " + shootDirection.Z);

                // Spawn position - FOLLOWS THE ANGLE (full 3D offset)
                float spawnDistance = 1.5f;  // Distance in front of player

                // Spawn bullet along the full 3D shoot direction
                Engine.Vector3 firingPosition = new Engine.Vector3(
                    playerPosition.X + (shootDirection.X * spawnDistance),
                    playerPosition.Y + aimHeightOffset,
                    playerPosition.Z + (shootDirection.Z * spawnDistance)
                );

                Engine.InternalCalls.Log("Firing position: " + firingPosition.X + ", " + firingPosition.Y + ", " + firingPosition.Z);

                // Calculate bullet rotation to face the shoot direction using quaternions
                // Compute yaw and pitch in radians from the 3D shoot direction
                float bulletYawRad = SimpleAtan2(shootDirection.X, shootDirection.Z);
                float bulletPitchRad = SimpleAsin(-shootDirection.Y);

                // Build quaternion from Euler angles in radians (pitch, yaw, roll)
                Engine.Vector3 bulletEulerRad = new Engine.Vector3(bulletPitchRad, bulletYawRad, 0.0f);
                Engine.Quat bulletRotationQuat = Engine.Quat.FromEuler(bulletEulerRad);

                // Convert quaternion back to Euler degrees as a helper for prefab instantiation
                Engine.Vector3 bulletEulerFromQuatRad = bulletRotationQuat.ToEuler();
                Engine.Vector3 bulletRotationDeg = new Engine.Vector3(
                    bulletEulerFromQuatRad.X * RAD2DEG,
                    bulletEulerFromQuatRad.Y * RAD2DEG,
                    bulletEulerFromQuatRad.Z * RAD2DEG
                );

                Engine.Vector3 bulletScale = new Engine.Vector3(0.4f, 0.4f, 0.2f);

                if (prefabPath == SecondaryBulletPrefab)
                {
                    bulletScale = new Engine.Vector3(0.3f, 0.20f, 0.3f);
                }

                Engine.InternalCalls.Log("Bullet rotation (deg): " + bulletRotationDeg.X + ", " + bulletRotationDeg.Y + ", " + bulletRotationDeg.Z);

                // Instantiate bullet WITH TRANSFORM - this ensures position is set immediately
                uint bulletEntityID = Engine.InternalCalls.Prefab_InstantiateWithTransform(
                    prefabPath,
                    ref firingPosition,
                    ref bulletRotationDeg,
                    ref bulletScale,
                    false  // false = entity prefab, not scene prefab
                );
                if (bulletEntityID == 0)
                {
                    Engine.InternalCalls.LogError("FAILED to instantiate bullet! Prefab_Instantiate returned 0");
                    Engine.InternalCalls.LogError("Check if prefab exists at: " + prefabPath);
                    return;
                }

                Engine.InternalCalls.Log("Bullet entity created with ID: " + bulletEntityID);

                // Ensure bullet uses quaternion rotation as the authoritative orientation
                Engine.Transform.SetRotation(bulletEntityID, ref bulletRotationQuat);

                // Set bullet position
                Engine.InternalCalls.Transform_SetPosition(bulletEntityID, ref firingPosition);

                // Verify position was set
                Engine.Vector3 verifyPos;
                Engine.InternalCalls.Transform_GetPosition(bulletEntityID, out verifyPos);
                Engine.InternalCalls.Log("Bullet position set to: " + verifyPos.X + ", " + verifyPos.Y + ", " + verifyPos.Z);

                // Set velocity with FULL 3D direction (this allows up/down shooting)
                Engine.Vector3 velocity = new Engine.Vector3(
                    shootDirection.X * bulletSpeed,
                    shootDirection.Y * bulletSpeed,
                    shootDirection.Z * bulletSpeed
                );

                Engine.InternalCalls.Log("Setting bullet velocity: " + velocity.X + ", " + velocity.Y + ", " + velocity.Z);

                // Check if bullet has rigidbody
                if (Engine.InternalCalls.EntityHasRigidBody(bulletEntityID))
                {
                    Engine.InternalCalls.Rigidbody_SetVelocity(bulletEntityID, ref velocity);

                    // Verify velocity was set
                    Engine.Vector3 verifyVel;
                    Engine.InternalCalls.Rigidbody_GetVelocity(bulletEntityID, out verifyVel);
                    Engine.InternalCalls.Log("Bullet velocity verified: " + verifyVel.X + ", " + verifyVel.Y + ", " + verifyVel.Z);
                }
                else
                {
                    Engine.InternalCalls.LogError("Bullet has NO RIGIDBODY! Velocity not set.");
                    Engine.InternalCalls.LogError("Make sure your bullet prefab has a RigidbodyComponent!");
                }

                Engine.InternalCalls.Log("=== BULLET SPAWN COMPLETE ===");
            }
            catch (Exception e)
            {
                Engine.InternalCalls.LogError("EXCEPTION spawning bullet: " + e.Message);
            }
        }

        private void TryReload()
        {
            if (isReloading) return;

            if (currentWeapon == WeaponType.Primary && primaryCurrentAmmo < PrimaryMaxAmmo)
            {
                isReloading = true;
                reloadTimer = PrimaryReloadTime;
                Engine.InternalCalls.Log("Reloading Primary...");
            }
            else if (currentWeapon == WeaponType.Secondary && secondaryCurrentAmmo < SecondaryMaxAmmo)
            {
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

        private void SendPositionEvent(Engine.Vector3 pos)
        {
            string payload =
                EntityID.ToString() + "|" +
                pos.X.ToString() + "|" +
                pos.Y.ToString() + "|" +
                pos.Z.ToString();

            Engine.EventSystem.Publish("PlayerPosition", payload);
        }

        // ===== MATH HELPERS =====
        private float SimpleSin(float x)
        {
            while (x > PI) x -= 2.0f * PI;
            while (x < -PI) x += 2.0f * PI;
            float x2 = x * x;
            float x3 = x2 * x;
            float x5 = x3 * x2;
            return x - (x3 / 6.0f) + (x5 / 120.0f);
        }

        private float SimpleCos(float x)
        {
            while (x > PI) x -= 2.0f * PI;
            while (x < -PI) x += 2.0f * PI;
            float x2 = x * x;
            float x4 = x2 * x2;
            return 1.0f - (x2 / 2.0f) + (x4 / 24.0f);
        }

        private float SimpleSqrt(float value)
        {
            if (value <= 0.0f) return 0.0f;
            float x = value;
            for (int i = 0; i < 3; i++)
                x = 0.5f * (x + value / x);
            return x;
        }

        private float SimpleAsin(float x)
        {
            if (x <= -1.0f) return -HALF_PI;
            if (x >= 1.0f) return HALF_PI;

            float x2 = x * x;
            float x3 = x2 * x;
            float x5 = x3 * x2;
            float x7 = x5 * x2;

            return x + (x3 / 6.0f) + (3.0f * x5 / 40.0f) + (5.0f * x7 / 112.0f);
        }

        private float SimpleAtan2(float y, float x)
        {
            if (x == 0.0f)
            {
                if (y > 0.0f) return HALF_PI;
                if (y < 0.0f) return -HALF_PI;
                return 0.0f;
            }

            float absX = x < 0.0f ? -x : x;
            float absY = y < 0.0f ? -y : y;
            float a = absY < absX ? absY / absX : absX / absY;
            float s = a * a;
            float r = ((-0.0464964749f * s + 0.15931422f) * s - 0.327622764f) * s * a + a;

            if (absY > absX)
                r = HALF_PI - r;
            if (x < 0.0f)
                r = PI - r;
            if (y < 0.0f)
                r = -r;

            return r;
        }

        private Engine.Vector3 CrossProduct(Engine.Vector3 a, Engine.Vector3 b)
        {
            return new Engine.Vector3(
                a.Y * b.Z - a.Z * b.Y,
                a.Z * b.X - a.X * b.Z,
                a.X * b.Y - a.Y * b.X
            );
        }

        public void Stop()
        {
            moveAllowed = false;
            Engine.InternalCalls.Rigidbody_Stop((uint)EntityID);
            Engine.InternalCalls.Log("Player stopped");
        }

        public override void OnDestroy()
        {
            Engine.InternalCalls.Log("=== PlayerMovement Destroyed ===");
        }
    }
}
