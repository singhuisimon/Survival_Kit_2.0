using System;
using Engine;

namespace Game
{
    /// <summary>
    /// TestScript - Combined Player Controller
    /// Handles: Movement (WASD), Camera Orbit (Mouse), Dash (Space)
    ///          Weapons (J=Primary, K=Secondary, R=Reload)
    /// </summary>
    public class TestScript
    {
        // ===== Entity Reference =====
        private int EntityID = 0;

        // ===== Camera Orbit Settings =====
        [SerializeField]
        private float orbitRadius = 7.5f;
        [SerializeField]
        private float orbitPitch = 0.25f;
        [SerializeField]
        private float orbitYaw = 0.0f;
        [SerializeField]
        private float mouseSensitivity = 0.0025f;
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
        public string PrimaryBulletPrefab = "Sources/Prefabs/PrimaryBullet.prefab";
        [SerializeField]
        public string SecondaryBulletPrefab = "Sources/Prefabs/SecondaryBullet.prefab";
        [SerializeField]
        public int PrimaryMaxAmmo = 100;
        [SerializeField]
        public float PrimaryReloadTime = 1.5f;
        [SerializeField]
        public float PrimaryFireRate = 0.3f;
        [SerializeField]
        public int SecondaryMaxAmmo = 5;
        [SerializeField]
        public float SecondaryReloadTime = 3.0f;
        [SerializeField]
        public float SecondaryFireRate = 0.8f;

        private enum WeaponType { Primary, Secondary }
        private enum WeaponState { Idle, Shooting, Reloading }

        private WeaponType currentWeapon = WeaponType.Primary;
        private WeaponState currentState = WeaponState.Idle;
        private int primaryCurrentAmmo = 100;
        private int secondaryCurrentAmmo = 5;
        private float primaryFireCooldown = 0.0f;
        private float secondaryFireCooldown = 0.0f;
        private float reloadTimer = 0.0f;
        private bool isReloading = false;
        private bool isShooting = false;

        // ===== Constants =====
        private const float DEG2RAD = 0.0174532924f;
        private const float RAD2DEG = 57.2957795f;
        private const float PI = 3.14159265359f;
        private const float HALF_PI = 1.5707963268f;

        private Engine.Vector2 previousMousePos = new Engine.Vector2(0.0f, 0.0f);

        // ===== Lifecycle Methods =====
        public void OnStart()
        {
            Engine.InternalCalls.Log("=== PlayerController Started ===");
            Engine.InternalCalls.Log("PlayerID: " + EntityID);

            // Add camera component to player entity
            Engine.InternalCalls.Entity_AddCamera((uint)EntityID);
            Engine.InternalCalls.Log("Camera component added to player");

            // Set camera as primary
            Engine.InternalCalls.Camera_SetPrimary((uint)EntityID, true);
            Engine.InternalCalls.Camera_SetEnabled((uint)EntityID, true);

            // Set initial camera properties
            Engine.InternalCalls.Camera_SetFOV((uint)EntityID, 60.0f);
            Engine.InternalCalls.Camera_SetNear((uint)EntityID, 0.1f);
            Engine.InternalCalls.Camera_SetFar((uint)EntityID, 1000.0f);

            // Add rigidbody for physics
            Engine.InternalCalls.Entity_AddRigidBody((uint)EntityID);
            Engine.InternalCalls.Rigidbody_SetIsKinematic((uint)EntityID, false);
            Engine.InternalCalls.Rigidbody_SetUseGravity((uint)EntityID, false);
            Engine.InternalCalls.Rigidbody_SetMass((uint)EntityID, 1.0f);
            Engine.InternalCalls.Log("Rigidbody configured for spaceship physics");

            // Initialize weapon ammo
            primaryCurrentAmmo = PrimaryMaxAmmo;
            secondaryCurrentAmmo = SecondaryMaxAmmo;
            currentState = WeaponState.Idle;
            currentWeapon = WeaponType.Primary;

            // Initialize mouse pos so first frame doesn't spin
            previousMousePos = Engine.Input.GetMousePosition();

            Engine.InternalCalls.Log("=== PlayerController fully initialized ===");
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

            // ===== Handle Weapon Cooldowns =====
            if (primaryFireCooldown > 0) primaryFireCooldown -= deltaTime;
            if (secondaryFireCooldown > 0) secondaryFireCooldown -= deltaTime;

            // ===== Update Reload =====
            if (isReloading)
            {
                UpdateReload(deltaTime);
                return;
            }

            // ===== Get Current Spaceship Position and Rotation =====
            Engine.Vector3 shipPos;
            Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out shipPos);

            Engine.Vector3 shipRot = Engine.Transform.GetRotation((uint)EntityID);

            // ===== Camera Aim Target (spaceship center) =====
            Engine.Vector3 aimTarget = new Engine.Vector3(
                shipPos.X,
                shipPos.Y + aimHeightOffset,
                shipPos.Z
            );

            // ===== Handle Mouse Input for Camera Orbit =====
            Engine.Vector2 currentMousePos = Engine.Input.GetMousePosition();

            float xOffset = currentMousePos.X - previousMousePos.X;
            float yOffset = currentMousePos.Y - previousMousePos.Y;
            previousMousePos = currentMousePos;

            // Horizontal mouse (X) -> yaw: move right = turn right
            orbitYaw += xOffset * mouseSensitivity;

            // Vertical mouse (Y) -> pitch: move up = look up
            orbitPitch -= yOffset * mouseSensitivity;

            // Clamp pitch to avoid flipping
            if (orbitPitch > HALF_PI - 0.01f)
                orbitPitch = HALF_PI - 0.01f;
            if (orbitPitch < -HALF_PI + 0.01f)
                orbitPitch = -HALF_PI + 0.01f;

            // ===== Calculate Camera Position (orbit around spaceship) =====
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

            // ===== Update Camera Target to spaceship center =====
            Engine.InternalCalls.Camera_SetTarget((uint)EntityID, ref aimTarget);

            // ===== Rotate Spaceship to Face Camera Direction (Yaw Only) =====
            float normalizedYaw = orbitYaw;
            while (normalizedYaw > PI) normalizedYaw -= 2.0f * PI;
            while (normalizedYaw < -PI) normalizedYaw += 2.0f * PI;

            Engine.Vector3 newShipRot = new Engine.Vector3(
                shipRot.X,
                normalizedYaw * RAD2DEG,
                shipRot.Z
            );
            Engine.Transform.SetRotation((uint)EntityID, ref newShipRot);
            // ===== Get Input =====
            float inputX = 0.0f;
            float inputZ = 0.0f;
            float inputY = 0.0f;

            if (Engine.Input.IsKeyPressed(Engine.KeyCode.W)) inputZ += 1.0f;
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.S)) inputZ -= 1.0f;
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.A)) inputX -= 1.0f;
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.D)) inputX += 1.0f;

            bool hasInput = (inputX != 0.0f) || (inputZ != 0.0f) || (inputY != 0.0f);

            // ===== Convert local input to world-space based on SPACESHIP rotation =====
            Engine.Vector3 moveDirWorld = new Engine.Vector3(0.0f, 0.0f, 0.0f);
            if (hasInput)
            {
                moveDirWorld = GetMoveDirectionInWorld(inputX, inputY, inputZ, newShipRot);
            }

            // ===== Get Current Velocity =====
            Engine.Vector3 currentVel;
            Engine.InternalCalls.Rigidbody_GetVelocity((uint)EntityID, out currentVel);

            // ===== Apply Movement Forces (Spaceship Physics) =====
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

            // ===== Handle Dash =====
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.Space) && !isDashing && dashCooldown <= 0.0f)
            {
                Engine.Vector3 dashDirWorld;

                if (hasInput)
                    dashDirWorld = moveDirWorld;
                else
                    dashDirWorld = GetMoveDirectionInWorld(0.0f, 0.0f, 1.0f, newShipRot);

                PerformDash(dashDirWorld);
            }

            // ===== Handle Weapon Input =====
            HandleWeaponInput(deltaTime);
            CheckAutoReload();

            SendPositionEvent(shipPos);
        }

        // ===== WEAPON SYSTEM =====
        private void HandleWeaponInput(float deltaTime)
        {
            if (Engine.Input.IsKeyPressed(Engine.KeyCode.R))
            {
                TryReload();
                return;
            }

            bool primaryFirePressed = Engine.Input.IsKeyPressed(Engine.KeyCode.J);
            bool secondaryFirePressed = Engine.Input.IsKeyPressed(Engine.KeyCode.K);

            if (primaryFirePressed && !secondaryFirePressed)
            {
                TryShootPrimary();
            }
            else if (secondaryFirePressed && !primaryFirePressed)
            {
                TryShootSecondary();
            }
            else
            {
                isShooting = false;
                currentState = WeaponState.Idle;
            }
        }

        private void TryShootPrimary()
        {
            if (isReloading) return;
            if (primaryFireCooldown > 0) return;
            if (primaryCurrentAmmo <= 0) return;

            currentWeapon = WeaponType.Primary;
            currentState = WeaponState.Shooting;
            isShooting = true;

            ShootPrimary();
            primaryFireCooldown = PrimaryFireRate;
        }

        private void TryShootSecondary()
        {
            if (isReloading) return;
            if (secondaryFireCooldown > 0) return;
            if (secondaryCurrentAmmo <= 0) return;

            currentWeapon = WeaponType.Secondary;
            currentState = WeaponState.Shooting;
            isShooting = true;

            ShootSecondary();
            secondaryFireCooldown = SecondaryFireRate;
        }

        private void ShootPrimary()
        {
            primaryCurrentAmmo--;
            Engine.InternalCalls.Log("=================================================");
            Engine.InternalCalls.Log("PRIMARY FIRE! Ammo: " + primaryCurrentAmmo + "/" + PrimaryMaxAmmo);
            Engine.InternalCalls.Log("=================================================");
            SpawnBullet(PrimaryBulletPrefab);
        }

        private void ShootSecondary()
        {
            secondaryCurrentAmmo--;
            Engine.InternalCalls.Log("=================================================");
            Engine.InternalCalls.Log("SECONDARY FIRE! Ammo: " + secondaryCurrentAmmo + "/" + SecondaryMaxAmmo);
            Engine.InternalCalls.Log("=================================================");
            SpawnBullet(SecondaryBulletPrefab);
        }

        private float Sqrt(float x)
        {
            if (x <= 0.0f) return 0.0f;

            float guess = x;
            float epsilon = 0.00001f;

            for (int i = 0; i < 10; i++)
            {
                float newGuess = (guess + x / guess) * 0.5f;
                float diff = newGuess - guess;
                if (diff < 0) diff = -diff;

                if (diff < epsilon)
                    return newGuess;
                guess = newGuess;
            }

            return guess;
        }

        private void SpawnBullet(string prefabPath)
        {
            try
            {
                uint playerEntityID = (uint)EntityID;

                // Get player position
                Engine.Vector3 playerPosition;
                Engine.InternalCalls.Transform_GetPosition(playerEntityID, out playerPosition);

                // Get player rotation (Y is in degrees)
                Engine.Vector3 playerRot = Engine.Transform.GetRotation(playerEntityID);

                Engine.InternalCalls.Log("Player rotation Y (degrees): " + playerRot.Y);

                // Convert Y rotation to radians
                float yawRad = playerRot.Y * DEG2RAD;

                // Forward vector: (sin(yaw), 0, cos(yaw))
                // This matches standard forward-facing along +Z
                float sinYaw = SimpleSin(yawRad);
                float cosYaw = SimpleCos(yawRad);

                Engine.Vector3 playerForward = new Engine.Vector3(
                    sinYaw,
                    0.0f,
                    cosYaw
                );

                Engine.InternalCalls.Log("Player forward: " + playerForward.X + ", " + playerForward.Y + ", " + playerForward.Z);

                // Spawn position: player position + forward offset
                float spawnDistance = 2.0f;
                Engine.Vector3 firingPosition = new Engine.Vector3(
                    playerPosition.X + (playerForward.X * spawnDistance),
                    playerPosition.Y,
                    playerPosition.Z + (playerForward.Z * spawnDistance)
                );

                Engine.InternalCalls.Log("Firing position: " + firingPosition.X + ", " + firingPosition.Y + ", " + firingPosition.Z);

                // Instantiate bullet
                uint bulletEntityID = Engine.InternalCalls.Prefab_Instantiate(prefabPath);

                if (bulletEntityID == 0)
                {
                    Engine.InternalCalls.LogError("Failed to instantiate bullet prefab!");
                    return;
                }

                Engine.InternalCalls.Transform_SetPosition(bulletEntityID, ref firingPosition);

                // Bullet velocity
                float bulletSpeed = prefabPath.Contains("Primary") ? 15.0f : 10.0f;
                Engine.Vector3 velocity = new Engine.Vector3(
                    playerForward.X * bulletSpeed,
                    playerForward.Y * bulletSpeed,
                    playerForward.Z * bulletSpeed
                );

                Engine.InternalCalls.Rigidbody_SetVelocity(bulletEntityID, ref velocity);

                Engine.InternalCalls.Log("=== BULLET SPAWNED ===");
            }
            catch (Exception e)
            {
                Engine.InternalCalls.LogError("ERROR: " + e.Message);
            }
        }
    


        private void TryReload()
        {
            if (isReloading) return;
            if (isShooting) return;

            if (currentWeapon == WeaponType.Primary)
            {
                if (primaryCurrentAmmo >= PrimaryMaxAmmo)
                {
                    Engine.InternalCalls.Log("Primary weapon already full!");
                    return;
                }
                StartReload(WeaponType.Primary, PrimaryReloadTime);
            }
            else
            {
                if (secondaryCurrentAmmo >= SecondaryMaxAmmo)
                {
                    Engine.InternalCalls.Log("Secondary weapon already full!");
                    return;
                }
                StartReload(WeaponType.Secondary, SecondaryReloadTime);
            }
        }

        private void StartReload(WeaponType weapon, float reloadTime)
        {
            isReloading = true;
            currentState = WeaponState.Reloading;
            reloadTimer = reloadTime;

            string weaponName = weapon == WeaponType.Primary ? "Primary" : "Secondary";
            Engine.InternalCalls.Log("Reloading " + weaponName + " weapon...");
        }

        private void UpdateReload(float deltaTime)
        {
            reloadTimer -= deltaTime;
            if (reloadTimer <= 0.0f)
            {
                CompleteReload();
            }
        }

        private void CompleteReload()
        {
            isReloading = false;
            currentState = WeaponState.Idle;

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

        private void CheckAutoReload()
        {
            if (primaryCurrentAmmo <= 0 && currentWeapon == WeaponType.Primary && !isReloading && !isShooting)
            {
                StartReload(WeaponType.Primary, PrimaryReloadTime);
            }

            if (secondaryCurrentAmmo <= 0 && currentWeapon == WeaponType.Secondary && !isReloading && !isShooting)
            {
                StartReload(WeaponType.Secondary, SecondaryReloadTime);
            }
        }

        public int GetPrimaryAmmo() => primaryCurrentAmmo;
        public int GetSecondaryAmmo() => secondaryCurrentAmmo;
        public bool IsReloading() => isReloading;
        public bool IsShooting() => isShooting;

        // ===== Movement Helpers =====
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

            float yawRad = rotation.Y * DEG2RAD;
            float sinYaw = SimpleSin(yawRad);
            float cosYaw = SimpleCos(yawRad);

            float wx = inputX * cosYaw + inputZ * sinYaw;
            float wy = inputY;
            float wz = -inputX * sinYaw + inputZ * cosYaw;

            return new Engine.Vector3(wx, wy, wz);
        }

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

        // ===== Math Helpers =====
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
                x = 0.5f * (x + value / x);

            return x;
        }

        // ===== Control Methods =====
        public void Stop()
        {
            moveAllowed = false;
            Engine.InternalCalls.Rigidbody_Stop((uint)EntityID);
            Engine.InternalCalls.Log("Player movement stopped");
        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("=== PlayerController Destroyed ===");
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