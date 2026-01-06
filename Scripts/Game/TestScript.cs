using System;
using Engine;
using static Engine.Transform;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Rigidbody;
using static Engine.Prefab;
using static Engine.Audio;

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
        private string PlayerHitSoundPrefab = "Sources/Prefabs/PlayerHit.prefab";
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

        private float shootCooldownTimer = 0.5f;

        // ===== Camera Reference =====
        private uint mainCameraEntityID = 0;

        // ===== Input State Tracking =====
        private bool spaceWasPressed = false;
        private bool rWasPressed = false;
        private bool initialized = false;

        // ===== Constants (for math helpers) =====
        private const float PI = 3.14159265359f;
        private const float HALF_PI = 1.5707963268f;

        // ===== Constant string for event ======
        private const string BULLETHITENEMY = "BulletHitEnemy";

        public override void OnStart()
        {
            LogMessage("=== PlayerMovement Started ===");
            LogMessage("PlayerID: " + EntityID);

            EntityID = SceneFindEntityByName("Player");
            // Find MainCamera using the proper InternalCall
            mainCameraEntityID = SceneFindEntityByName("MainCamera");

            LogMessage("Found MainCamera with ID: " + mainCameraEntityID);

            // Weapon setup
            primaryCurrentAmmo = PrimaryMaxAmmo;
            secondaryCurrentAmmo = SecondaryMaxAmmo;

            initialized = true;
            LogMessage("Player initialized - Movement + Shooting ready");
            LogMessage("Camera controlled by C++ - use mouse to look around");

            Subscribe(BULLETHITENEMY, PlayBulletHit);
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
            Vector3 playerPos;
            playerPos = GetPosition((uint)EntityID);

            // Get camera position (updated by C++)
            Vector3 cameraPos;
            cameraPos = GetPosition(mainCameraEntityID);

            // ===== MOVEMENT INPUT (FROM DOCUMENT 16 - FULL 3D) =====
            // Calculate forward direction from camera to player (FULL 3D)
            Vector3 camToPlayer = new Vector3(
                playerPos.X - cameraPos.X,
                playerPos.Y - cameraPos.Y,  // ? Includes Y for 3D movement
                playerPos.Z - cameraPos.Z
            );

            // Normalize to get forward direction (camera looking at player)
            float camToPlayerLen = SimpleSqrt(camToPlayer.X * camToPlayer.X +
                camToPlayer.Y * camToPlayer.Y +
                camToPlayer.Z * camToPlayer.Z);
            Vector3 forward = new Vector3(0.0f, 0.0f, 0.0f);

            if (camToPlayerLen > 0.000001f)
            {
                forward.X = camToPlayer.X / camToPlayerLen;
                forward.Y = camToPlayer.Y / camToPlayerLen;
                forward.Z = camToPlayer.Z / camToPlayerLen;
            }

            // Calculate right vector (perpendicular to forward)
            Vector3 worldUp = new Vector3(0.0f, 1.0f, 0.0f);
            Vector3 right = CrossProduct(forward, worldUp);
            float rightLen = SimpleSqrt(right.X * right.X + right.Y * right.Y + right.Z * right.Z);
            if (rightLen > 0.000001f)
            {
                right.X /= rightLen;
                right.Y /= rightLen;
                right.Z /= rightLen;
            }

            Vector3 moveDir = new Vector3(0.0f, 0.0f, 0.0f);

            // Movement controls - relative to camera view (FULL 3D)
            if (Input.IsKeyPressed(KeyCode.W))  // W = Move away from camera (forward)
            {
                moveDir.X += forward.X;
                moveDir.Y += forward.Y;
                moveDir.Z += forward.Z;

                LogMessage("Send help pls");
            }
            if (Input.IsKeyPressed(KeyCode.S))  // S = Move toward camera (backward)
            {
                moveDir.X -= forward.X;
                moveDir.Y -= forward.Y;
                moveDir.Z -= forward.Z;
            }
            if (Input.IsKeyPressed(KeyCode.A))  // A = Move left relative to camera
            {
                moveDir.X -= right.X;
                moveDir.Y -= right.Y;
                moveDir.Z -= right.Z;
            }
            if (Input.IsKeyPressed(KeyCode.D))  // D = Move right relative to camera
            {
                moveDir.X += right.X;
                moveDir.Y += right.Y;
                moveDir.Z += right.Z;
            }

            // Normalize movement direction
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
                Vector3 newPos = new Vector3(
                    playerPos.X + moveDir.X * moveSpeed,
                    playerPos.Y + moveDir.Y * moveSpeed,
                    playerPos.Z + moveDir.Z * moveSpeed
                );
                SetPosition((uint)EntityID, ref newPos);
            }

            // Handle dash (FULL 3D)
            bool spaceIsPressed = Input.IsKeyPressed(KeyCode.Space);
            bool spaceJustPressed = spaceIsPressed && !spaceWasPressed;
            spaceWasPressed = spaceIsPressed;

            if (spaceJustPressed && !isDashing && dashCooldown <= 0.0f)
            {
                if (moveDirLenSq > 0.000001f)
                {
                    Vector3 dashImpulse = new Vector3(
                        moveDir.X * dashForce,
                        moveDir.Y * dashForce,
                        moveDir.Z * dashForce
                    );
                    RigidbodyAddForce((uint)EntityID, ref dashImpulse);
                    isDashing = true;
                    dashCooldown = DASH_COOLDOWN_TIME;
                    LogMessage("DASH!");
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
            if (shootCooldownTimer > 0.0f)
                shootCooldownTimer -= deltaTime;

            if (Input.IsMouseButtonPressed(MouseButton.Left) && shootCooldownTimer <= 0.0f)
            {
                shootCooldownTimer = 0.1f;
                ShootBullet();
            }

            //if (Input.IsKeyPressed(KeyCode.K))
            //{
            //    TryShootSecondary();
            //}

            bool rIsPressed = Input.IsKeyPressed(KeyCode.R);
            bool rJustPressed = rIsPressed && !rWasPressed;
            rWasPressed = rIsPressed;

            if (rJustPressed)
            {
                TryReload();
            }
        }

        private void ShootBullet()
        {
            // Player position & forward
            Vector3 playerPos = Transform.GetPosition(EntityID);
            Quat playerRot = Transform.GetRotation(EntityID);

            Vector3 forward = playerRot.Right;      // or
                                                    // Vector3 forward  = -playerRot.Right;  // if it goes backwards

            // Tune these as needed
            float spawnDistance = 1.5f;
            float bulletForce = 100.0f;

            // Spawn position in front of the player
            Vector3 spawnPos = new Vector3(
                playerPos.X + forward.X * spawnDistance,
                playerPos.Y + forward.Y * spawnDistance,
                playerPos.Z + forward.Z * spawnDistance
            );

            // Create bullet entity
            uint bulletID = SceneCreateEntity("PrimaryBullet");
            if (bulletID == 0)
                return;

            // Set transform
            Transform.SetPosition(bulletID, ref spawnPos);
            Transform.SetRotation(bulletID, ref playerRot);

            // Add Rigidbody and shoot it
            EntityAddRigidBody(bulletID);

            Vector3 force = new Vector3(
                forward.X * bulletForce,
                forward.Y * bulletForce,
                forward.Z * bulletForce
            );
            RigidbodyAddForce(bulletID, ref force);

            // Optional: visuals + script
            EntityAddMeshRenderer(bulletID);
            EntityAddScript(bulletID, "Game.PrimaryBullet");
            LogMessage("Creating bullet with ID: " + bulletID);
        }

        //private void TryShootPrimary()
        //{
        //    if (isReloading || primaryFireCooldown > 0 || primaryCurrentAmmo <= 0)
        //        return;

        //    currentWeapon = WeaponType.Primary;
        //    primaryCurrentAmmo--;
        //    primaryFireCooldown = PrimaryFireRate;

        //    LogMessage("PRIMARY FIRE! Ammo: " + primaryCurrentAmmo + "/" + PrimaryMaxAmmo);
        //    SpawnBullet(PrimaryBulletPrefab, 10.0f);
        //}

        //private void TryShootSecondary()
        //{
        //    if (isReloading || secondaryFireCooldown > 0 || secondaryCurrentAmmo <= 0)
        //        return;

        //    currentWeapon = WeaponType.Secondary;
        //    secondaryCurrentAmmo--;
        //    secondaryFireCooldown = SecondaryFireRate;

        //    LogMessage("SECONDARY FIRE! Ammo: " + secondaryCurrentAmmo + "/" + SecondaryMaxAmmo);
        //    SpawnBullet(SecondaryBulletPrefab, 30.0f);
        //}

        //private void SpawnBullet(string prefabPath, float bulletSpeed)
        //{
        //    try
        //    {
        //        LogMessage("=== SPAWNING BULLET ===");
        //        LogMessage("Prefab path: " + prefabPath);
        //        LogMessage("Bullet speed: " + bulletSpeed);

        //        // Get player position
        //        Vector3 playerPosition;
        //        GetPosition((uint)EntityID, out playerPosition);
        //        LogMessage("Player position: " + playerPosition.X + ", " + playerPosition.Y + ", " + playerPosition.Z);

        //        // Calculate aimTarget - this is where the camera is looking at (approx)
        //        Vector3 aimTarget = new Vector3(
        //            playerPosition.X,
        //            playerPosition.Y + aimHeightOffset,
        //            playerPosition.Z
        //        );

        //        // Get the ACTUAL MainCamera position (updated by C++ with mouse movement)
        //        Vector3 cameraPos;
        //        if (mainCameraEntityID != 0)
        //        {
        //            GetPosition(mainCameraEntityID, out cameraPos);
        //        }
        //        else
        //        {
        //            // Fallback: use calculated camera position if MainCamera not found
        //            LogWarning("MainCamera not found, using fallback position");
        //            float cosPitch = SimpleCos(orbitPitch);
        //            float sinPitch = SimpleSin(orbitPitch);
        //            float cosYaw = SimpleCos(orbitYaw);
        //            float sinYaw = SimpleSin(orbitYaw);

        //            cameraPos = new Vector3(
        //                aimTarget.X + cosPitch * sinYaw * orbitRadius,
        //                aimTarget.Y + sinPitch * orbitRadius,
        //                aimTarget.Z + cosPitch * cosYaw * orbitRadius
        //            );
        //        }

        //        LogMessage("Camera position: " + cameraPos.X + ", " + cameraPos.Y + ", " + cameraPos.Z);

        //        // Calculate direction FROM camera TO aimTarget (where camera is looking)
        //        Vector3 cameraToAimTarget = new Vector3(
        //            aimTarget.X - cameraPos.X,
        //            aimTarget.Y - cameraPos.Y,
        //            aimTarget.Z - cameraPos.Z
        //        );

        //        // Use the FULL 3D direction for shooting
        //        Vector3 shootDirection = cameraToAimTarget;

        //        // Normalize the full 3D vector
        //        float lenSq = shootDirection.X * shootDirection.X +
        //                      shootDirection.Y * shootDirection.Y +
        //                      shootDirection.Z * shootDirection.Z;

        //        if (lenSq > 0.0001f)
        //        {
        //            float invLen = 1.0f / SimpleSqrt(lenSq);
        //            shootDirection = new Vector3(
        //                shootDirection.X * invLen,
        //                shootDirection.Y * invLen,
        //                shootDirection.Z * invLen
        //            );
        //        }
        //        else
        //        {
        //            LogError("Invalid shoot direction - length is zero!");
        //            return;
        //        }

        //        LogMessage("Shoot direction: " + shootDirection.X + ", " + shootDirection.Y + ", " + shootDirection.Z);

        //        // Spawn position - along the full 3D shoot direction
        //        float spawnDistance = 1.5f;  // Distance in front of player

        //        Vector3 firingPosition = new Vector3(
        //            playerPosition.X + (shootDirection.X * spawnDistance),
        //            playerPosition.Y + aimHeightOffset,
        //            playerPosition.Z + (shootDirection.Z * spawnDistance)
        //        );

        //        LogMessage("Firing position: " + firingPosition.X + ", " + firingPosition.Y + ", " + firingPosition.Z);

        //        // Compute bullet rotation as a pure quaternion that looks along shootDirection
        //        Vector3 worldUp = new Vector3(0.0f, 1.0f, 0.0f);
        //        Quat bulletRotationQuat = LookRotation(shootDirection, worldUp);

        //        Vector3 bulletScale = new Vector3(0.4f, 0.4f, 0.2f);
        //        if (prefabPath == SecondaryBulletPrefab)
        //        {
        //            bulletScale = new Vector3(0.3f, 0.20f, 0.3f);
        //        }

        //        // Instantiate bullet WITH TRANSFORM (quat-based rotation)
        //        uint bulletEntityID = Prefab_InstantiateWithTransform(
        //            prefabPath,
        //            ref firingPosition,
        //            ref bulletRotationQuat,
        //            ref bulletScale,
        //            false  // false = entity prefab, not scene prefab
        //        );
        //        if (bulletEntityID == 0)
        //        {
        //            LogError("FAILED to instantiate bullet! Prefab_InstantiateWithTransform returned 0");
        //            LogError("Check if prefab exists at: " + prefabPath);
        //            return;
        //        }

        //        LogMessage("Bullet entity created with ID: " + bulletEntityID);

        //        // Ensure bullet uses quaternion rotation as the authoritative orientation
        //        Transform.SetRotation(bulletEntityID, ref bulletRotationQuat);

        //        // Set bullet position
        //        SetPosition(bulletEntityID, ref firingPosition);

        //        // Verify position was set
        //        Vector3 verifyPos;
        //        GetPosition(bulletEntityID, out verifyPos);
        //        LogMessage("Bullet position set to: " + verifyPos.X + ", " + verifyPos.Y + ", " + verifyPos.Z);

        //        // Set velocity with FULL 3D direction (this allows up/down shooting)
        //        Vector3 velocity = new Vector3(
        //            shootDirection.X * bulletSpeed,
        //            shootDirection.Y * bulletSpeed,
        //            shootDirection.Z * bulletSpeed
        //        );

        //        LogMessage("Setting bullet velocity: " + velocity.X + ", " + velocity.Y + ", " + velocity.Z);

        //        // Check if bullet has rigidbody
        //        if (EntityHasRigidBody(bulletEntityID))
        //        {
        //            Rigidbody_SetVelocity(bulletEntityID, ref velocity);

        //            // Verify velocity was set
        //            Vector3 verifyVel;
        //            Rigidbody_GetVelocity(bulletEntityID, out verifyVel);
        //            LogMessage("Bullet velocity verified: " + verifyVel.X + ", " + verifyVel.Y + ", " + verifyVel.Z);
        //        }
        //        else
        //        {
        //            LogError("Bullet has NO RIGIDBODY! Velocity not set.");
        //            LogError("Make sure your bullet prefab has a RigidbodyComponent!");
        //        }

        //        LogMessage("=== BULLET SPAWN COMPLETE ===");
        //    }
        //    catch (Exception e)
        //    {
        //        LogError("EXCEPTION spawning bullet: " + e.Message);
        //    }
        //}

        private void TryReload()
        {
            if (isReloading) return;

            if (currentWeapon == WeaponType.Primary && primaryCurrentAmmo < PrimaryMaxAmmo)
            {
                isReloading = true;
                reloadTimer = PrimaryReloadTime;
                LogMessage("Reloading Primary...");
            }
            else if (currentWeapon == WeaponType.Secondary && secondaryCurrentAmmo < SecondaryMaxAmmo)
            {
                isReloading = true;
                reloadTimer = SecondaryReloadTime;
                LogMessage("Reloading Secondary...");
            }
        }

        private void CompleteReload()
        {
            isReloading = false;
            if (currentWeapon == WeaponType.Primary)
            {
                primaryCurrentAmmo = PrimaryMaxAmmo;
                LogMessage("Primary reloaded!");
            }
            else
            {
                secondaryCurrentAmmo = SecondaryMaxAmmo;
                LogMessage("Secondary reloaded!");
            }
        }

        private void SendPositionEvent(Vector3 pos)
        {
            string payload =
                EntityID.ToString() + "|" +
                pos.X.ToString() + "|" +
                pos.Y.ToString() + "|" +
                pos.Z.ToString();

            Publish("PlayerPosition", payload);
        }

        // ===== QUAT HELPERS =====

        // Builds a quaternion that rotates the "forward" axis of the object to match `forward`,
        // using `up` as the approximate up direction (similar to LookRotation in other engines).
        private Quat LookRotation(Vector3 forward, Vector3 up)
        {
            // Normalize forward
            float fLenSq = forward.X * forward.X + forward.Y * forward.Y + forward.Z * forward.Z;
            if (fLenSq < 1e-8f)
            {
                // Degenerate forward; return identity
                Quat id = new Quat();
                id.X = 0.0f;
                id.Y = 0.0f;
                id.Z = 0.0f;
                id.W = 1.0f;
                return id;
            }
            float fInvLen = 1.0f / SimpleSqrt(fLenSq);
            Vector3 f = new Vector3(
                forward.X * fInvLen,
                forward.Y * fInvLen,
                forward.Z * fInvLen
            );

            // Orthonormal basis: right = normalize(cross(up, forward)), newUp = cross(forward, right)
            Vector3 r = CrossProduct(up, f);
            float rLenSq = r.X * r.X + r.Y * r.Y + r.Z * r.Z;
            if (rLenSq < 1e-8f)
            {
                // up and forward are colinear; choose arbitrary orthogonal up
                r = CrossProduct(new Vector3(0.0f, 1.0f, 0.0f), f);
                rLenSq = r.X * r.X + r.Y * r.Y + r.Z * r.Z;
                if (rLenSq < 1e-8f)
                {
                    r = CrossProduct(new Vector3(1.0f, 0.0f, 0.0f), f);
                    rLenSq = r.X * r.X + r.Y * r.Y + r.Z * r.Z;
                }
            }
            float rInvLen = 1.0f / SimpleSqrt(rLenSq);
            r = new Vector3(r.X * rInvLen, r.Y * rInvLen, r.Z * rInvLen);

            Vector3 u = CrossProduct(f, r);

            // Build rotation matrix from basis (right, up, forward) as columns
            float m00 = r.X; float m01 = u.X; float m02 = f.X;
            float m10 = r.Y; float m11 = u.Y; float m12 = f.Y;
            float m20 = r.Z; float m21 = u.Z; float m22 = f.Z;

            float trace = m00 + m11 + m22;
            Quat q = new Quat();

            if (trace > 0.0f)
            {
                float s = SimpleSqrt(trace + 1.0f) * 2.0f; // s = 4 * qw
                q.W = 0.25f * s;
                q.X = (m21 - m12) / s;
                q.Y = (m02 - m20) / s;
                q.Z = (m10 - m01) / s;
            }
            else if (m00 > m11 && m00 > m22)
            {
                float s = SimpleSqrt(1.0f + m00 - m11 - m22) * 2.0f; // s = 4 * qx
                q.W = (m21 - m12) / s;
                q.X = 0.25f * s;
                q.Y = (m01 + m10) / s;
                q.Z = (m02 + m20) / s;
            }
            else if (m11 > m22)
            {
                float s = SimpleSqrt(1.0f + m11 - m00 - m22) * 2.0f; // s = 4 * qy
                q.W = (m02 - m20) / s;
                q.X = (m01 + m10) / s;
                q.Y = 0.25f * s;
                q.Z = (m12 + m21) / s;
            }
            else
            {
                float s = SimpleSqrt(1.0f + m22 - m00 - m11) * 2.0f; // s = 4 * qz
                q.W = (m10 - m01) / s;
                q.X = (m02 + m20) / s;
                q.Y = (m12 + m21) / s;
                q.Z = 0.25f * s;
            }

            return q;
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

        private Vector3 CrossProduct(Vector3 a, Vector3 b)
        {
            return new Vector3(
                a.Y * b.Z - a.Z * b.Y,
                a.Z * b.X - a.X * b.Z,
                a.X * b.Y - a.Y * b.X
            );
        }

        public void Stop()
        {
            moveAllowed = false;
            RigidbodyStop((uint)EntityID);
            LogMessage("Player stopped");
        }

        public override void OnDestroy()
        {
            LogMessage("=== PlayerMovement Destroyed ===");
            Unsubscribe(BULLETHITENEMY, PlayBulletHit);
        }

        #region event handlers

        private void PlayBulletHit(string eventName, string payload)
        {
            if (!bool.TryParse(payload, out bool hitenemy))
            {
                return;
            }
            if (hitenemy)
            {
                //Spawn Instantiate a prefab with the sound and play it
                if (!string.IsNullOrEmpty(PlayerHitSoundPrefab))
                {
                    uint hitSoundID = PrefabInstantiate(PlayerHitSoundPrefab);
                    Vector3 myPos = GetPosition((uint)EntityID);
                    SetPosition(hitSoundID, ref myPos);
                    AudioPlay(hitSoundID);

                    LogMessage("Player Hit Sound Prefab was instantiated");
                }
            }
        }

        #endregion
    }
}
