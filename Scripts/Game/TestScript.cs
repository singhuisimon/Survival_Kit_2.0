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
        private float moveSpeed = 0.5f;
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

        // ===== Input State Tracking =====
        private bool spaceWasPressed = false;
        private bool rWasPressed = false;
        private bool initialized = false;

        // ===== Constants =====
        private const float DEG2RAD = 0.0174532924f;
        private const float RAD2DEG = 57.2957795f;
        private const float PI = 3.14159265359f;
        private const float HALF_PI = 1.5707963268f;

        public void OnStart()
        {
            Engine.InternalCalls.Log("=== PlayerMovement Started ===");
            Engine.InternalCalls.Log("PlayerID: " + EntityID);

            // Validate EntityID
            if (EntityID == 0)
            {
                Engine.InternalCalls.LogError("ERROR: EntityID is 0! Script was not properly initialized.");
                return;
            }

            // Find MainCamera using the proper InternalCall
            mainCameraEntityID = Engine.InternalCalls.Scene_FindEntityByName("MainCamera");

            if (mainCameraEntityID == 0)
            {
                Engine.InternalCalls.LogError("MainCamera not found! Make sure an entity named 'MainCamera' exists.");
                return;
            }

            Engine.InternalCalls.Log("Found MainCamera with ID: " + mainCameraEntityID);

            // Weapon setup
            primaryCurrentAmmo = PrimaryMaxAmmo;
            secondaryCurrentAmmo = SecondaryMaxAmmo;

            initialized = true;
            Engine.InternalCalls.Log("Player initialized - Movement + Shooting ready");
            Engine.InternalCalls.Log("Camera controlled by C++ - use mouse to look around");
        }

        public void OnUpdate(float deltaTime)
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

            // Get player position and rotation (set by C++)
            Engine.Vector3 playerPos;
            Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out playerPos);

            Engine.Vector3 playerRot;
            Engine.InternalCalls.Transform_GetRotation((uint)EntityID, out playerRot);

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

            // Normalize movement direction
            float moveDirLenSq = moveDir.X * moveDir.X + moveDir.Z * moveDir.Z;
            if (moveDirLenSq > 0.000001f)
            {
                float moveDirLen = SimpleSqrt(moveDirLenSq);
                moveDir.X /= moveDirLen;
                moveDir.Z /= moveDirLen;
            }

            // Apply movement if not dashing
            // THIS IS WHERE SPEED IS APPLIED - moveSpeed is set at the top (currently 0.5f)
            if (moveDirLenSq > 0.000001f && !isDashing)
            {
                Engine.Vector3 newPos = new Engine.Vector3(
                    playerPos.X + moveDir.X * moveSpeed,
                    playerPos.Y,
                    playerPos.Z + moveDir.Z * moveSpeed
                );
                Engine.InternalCalls.Transform_SetPosition((uint)EntityID, ref newPos);
            }

            // Handle dash
            bool spaceIsPressed = Engine.Input.IsKeyPressed(Engine.KeyCode.Space);
            bool spaceJustPressed = spaceIsPressed && !spaceWasPressed;
            spaceWasPressed = spaceIsPressed;

            if (spaceJustPressed && !isDashing && dashCooldown <= 0.0f)
            {
                if (moveDirLenSq > 0.000001f)
                {
                    Engine.Vector3 dashImpulse = new Engine.Vector3(
                        moveDir.X * dashForce,
                        0.0f,
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

                // Get camera position (updated by C++)
                Engine.Vector3 cameraPosition;
                Engine.InternalCalls.Transform_GetPosition(mainCameraEntityID, out cameraPosition);
                Engine.InternalCalls.Log("Camera position: " + cameraPosition.X + ", " + cameraPosition.Y + ", " + cameraPosition.Z);

                // Calculate aim target (player head position)
                float aimHeightOffset = 2.0f;
                Engine.Vector3 aimTarget = new Engine.Vector3(
                    playerPosition.X,
                    playerPosition.Y + aimHeightOffset,
                    playerPosition.Z
                );

                // Shoot direction = from camera towards aim target
                Engine.Vector3 shootDirection = new Engine.Vector3(
                    aimTarget.X - cameraPosition.X,
                    aimTarget.Y - cameraPosition.Y,
                    aimTarget.Z - cameraPosition.Z
                );

                // Normalize direction
                float dirLen = SimpleSqrt(
                    shootDirection.X * shootDirection.X +
                    shootDirection.Y * shootDirection.Y +
                    shootDirection.Z * shootDirection.Z
                );

                if (dirLen > 0.000001f)
                {
                    shootDirection.X /= dirLen;
                    shootDirection.Y /= dirLen;
                    shootDirection.Z /= dirLen;
                }
                else
                {
                    Engine.InternalCalls.LogError("Invalid shoot direction - length is zero!");
                    return;
                }

                Engine.InternalCalls.Log("Shoot direction: " + shootDirection.X + ", " + shootDirection.Y + ", " + shootDirection.Z);

                // Spawn position - in front of player
                float spawnDistance = 2.0f;
                Engine.Vector3 firingPosition = new Engine.Vector3(
                    playerPosition.X + (shootDirection.X * spawnDistance),
                    playerPosition.Y + 1.5f,
                    playerPosition.Z + (shootDirection.Z * spawnDistance)
                );

                Engine.InternalCalls.Log("Firing position: " + firingPosition.X + ", " + firingPosition.Y + ", " + firingPosition.Z);

                // Instantiate bullet
                // Calculate bullet rotation to face the shoot direction
                // Using atan2 for yaw (horizontal rotation) and asin for pitch (vertical rotation)
                float bulletYaw = SimpleAtan2(shootDirection.X, shootDirection.Z) * RAD2DEG;
                float bulletPitch = SimpleAsin(-shootDirection.Y) * RAD2DEG;  // Negative because looking down is positive pitch

                Engine.Vector3 bulletRotation = new Engine.Vector3(bulletPitch, bulletYaw, 0.0f);
                Engine.Vector3 bulletScale = new Engine.Vector3(1.0f, 1.0f, 1.0f);

                Engine.InternalCalls.Log("Bullet rotation: " + bulletRotation.X + ", " + bulletRotation.Y + ", " + bulletRotation.Z);

                // Instantiate bullet WITH TRANSFORM - this ensures position is set immediately
                uint bulletEntityID = Engine.InternalCalls.Prefab_InstantiateWithTransform(
                    prefabPath,
                    ref firingPosition,
                    ref bulletRotation,
                    ref bulletScale,
                    false  // false = entity prefab, not scene prefab
                ); if (bulletEntityID == 0)
                {
                    Engine.InternalCalls.LogError("FAILED to instantiate bullet! Prefab_Instantiate returned 0");
                    Engine.InternalCalls.LogError("Check if prefab exists at: " + prefabPath);
                    return;
                }

                Engine.InternalCalls.Log("Bullet entity created with ID: " + bulletEntityID);

                // Set bullet position
                Engine.InternalCalls.Transform_SetPosition(bulletEntityID, ref firingPosition);

                // Verify position was set
                Engine.Vector3 verifyPos;
                Engine.InternalCalls.Transform_GetPosition(bulletEntityID, out verifyPos);
                Engine.InternalCalls.Log("Bullet position set to: " + verifyPos.X + ", " + verifyPos.Y + ", " + verifyPos.Z);

                // Set bullet velocity
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

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("=== PlayerMovement Destroyed ===");
        }
    }
}