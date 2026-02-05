using System;
using Engine;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Prefab;
using static Engine.Logger;
using static Engine.Input;
using static Engine.Rigidbody;
using static Engine.Camera;
using static Engine.Event;
using static Engine.Audio;

namespace Game
{

    /// <Summary>
    /// This script manages the weapon player uses
    /// </Summary>
    public class PlayerWeapon : ScriptBehaviour
    {

        [SerializeField] private bool shootAllowed;
        [SerializeField] private bool gameEnd;

        #region primary
        [SerializeField] private bool primaryShooting = false;  //player is currently shooting
        [SerializeField] private bool reloadingPrimary = false; //player is currently reloading
        [SerializeField] private int primaryAmmo = 0;
        [SerializeField] private int primaryAmmoMax = 100;
        [SerializeField] private float primaryReloadDelay = 1.5f; //reloading time
        [SerializeField] private float primaryShootRate = 0.05f;
        [SerializeField] private float primaryShootNext = 0.0f;
        [SerializeField] private float primarybulletSpeed = 10000.0f;

        // ===== Bullet velocity inheritance =====
        // Player is moved kinematically (SetPosition in SpaceshipController), so we estimate velocity from position delta.
        [SerializeField] private bool inheritPlayerVelocity = true;

        // If true, inherit only the component along the bullet direction (keeps shots straight).
        [SerializeField] private bool inheritAlongBulletDirectionOnly = true;

        // 1.0 = full inheritance, 0.0 = none.
        [SerializeField] private float inheritVelocityFactor = 1.0f;

        // Safety clamp for spikes (teleport / hitch / pause-unpause).
        [SerializeField] private float maxInheritedSpeed = 250.0f;

        // Guarantee bullet forward speed is at least this much faster than player forward speed.
        [SerializeField] private float minBulletOverPlayerSpeed = 100.0f;

        // Absolute minimum forward speed for the bullet (world space).
        [SerializeField] private float minWorldBulletSpeed = 300.0f;

        //[SerializeField]
        private string PrimaryBulletPrefab = "Sources/Prefabs/PrimaryBullet.prefab";
        //audio
        //vfx -> trail

        #endregion

        #region alt charge
        [SerializeField] private float chargeDelayRate = 0.5f;
        [SerializeField] private float chargeDelayNext = 0.0f;
        [SerializeField] private bool primaryAltCharging = false;
        [SerializeField] private float primaryChargeRate = 0.1f;
        [SerializeField] private float primaryChargeNext = 0.0f;
        [SerializeField] private int primaryAltCharge = 0;
        [SerializeField] private int primaryAltChargeMax = 30;

        [SerializeField] private bool primaryAltReady = false;
        [SerializeField] private float primaryAltFireAOERange = 30f;
        [SerializeField] private float primaryUltSpeed = 5000.0f;

        //[SerializeField]
        private string PrimaryUltBulletPrefab = "Sources/Prefabs/PrimaryUltBullet.prefab";

        private string ULTGAINEVENT = "GainUlt";
        private string GAMEWIN = "GameWin";
        private string GAMEOVER = "GameOver";

        //2 audio for primary alt fire
        //not sure what is layermask
        //left and right player alt vortex
        //particle system

        #endregion

        #region Vfx

        //particle -> VFX when shooting primary

        #endregion

        #region sfx
        //player hit something sfx
        //sfx alt charge
        //sfx alt ready
        //sfx swap weapon

        #endregion

        #region camera shake
        [SerializeField] private float CAMSHAKE_primaryFire = 0.05f;
        [SerializeField] private float CAMSHAKE_primaryFireDuration = 0.5f;
        [SerializeField] private float CAMSHAKE_primaryAltFire = 0.16f;
        [SerializeField] private float CAMSHAKE_primaryAltFireDuration = 0.5f;
        #endregion

        #region entity

        //[SerializeField]
        private string firingPointName = "PlayerCam";
        //[SerializeField]
        private string playerName = "Player";

        #endregion

        #region others

        [SerializeField] private float muzzleDistance = 5.0f;
        [SerializeField] private bool spawnedCharge = false;
        [SerializeField] private bool spawnedChargelastframe = false;
        private string PrimaryUltChargedPrefab = "Sources/Prefabs/Audio_Primary_Ult_Recharged.prefab";
        private string AudioWeaponReloadName = "Audio_WeaponReload";


        #endregion

        private uint reloadID = 0;
        private uint firingPointEntityID = 0;
        private uint playerEntityID = 0;
        private bool isKeyRPressedPreviously = false;
        private float elapsedTime = 0.0f;

        private float reloadFinishTime = 0.0f;
        private Vector3 bulletDirection;
        private string EVENT_AMMO_CHANGE = "AmmoChange";

        // ===== Estimated player velocity (for bullet inheritance) =====
        private Vector3 estimatedPlayerVelocity = Vector3.Zero;
        private Vector3 lastPlayerPos = Vector3.Zero;
        private bool hasLastPlayerPos = false;
        private bool velocityTrackerWasPaused = false;

        public override void OnStart()
        {

            //Initialize Values
            shootAllowed = true;
            gameEnd = false;
            primaryAmmo = primaryAmmoMax;
            primaryAltCharge = 0;

            firingPointEntityID = SceneFindEntityByName(firingPointName);
            playerEntityID = SceneFindEntityByName(playerName);
            reloadID = SceneFindEntityByName(AudioWeaponReloadName);

            if (firingPointEntityID == 0)
            {
                LogMessage("[PlayerWeapon] firing point entity cannot be found");
            }
            if (playerEntityID == 0)
            {
                LogMessage("[PlayerWeapon] player entity cannot be found");
            }
            if (reloadID == 0)
            {
                LogMessage("[PlayerWeapon] reload entity cannot be found");
            }

            // Init velocity tracker
            if (playerEntityID != 0)
            {
                lastPlayerPos = GetPosition(playerEntityID);
                hasLastPlayerPos = true;
                estimatedPlayerVelocity = Vector3.Zero;
            }

            Subscribe(ULTGAINEVENT, UltCharging);
            Subscribe(GAMEOVER, OnGameStateChange);
            Subscribe(GAMEWIN, OnGameStateChange);
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (playerEntityID == 0)
                return;

            if (GameState.IsPaused)
            {
                estimatedPlayerVelocity = Vector3.Zero;
                hasLastPlayerPos = false;
                velocityTrackerWasPaused = true;
                return;
            }

            if (velocityTrackerWasPaused)
            {
                // Avoid a big spike right after unpausing.
                velocityTrackerWasPaused = false;
                hasLastPlayerPos = false;
            }

            Vector3 pos = GetPosition(playerEntityID);

            if (hasLastPlayerPos && deltaTime > 1e-6f)
            {
                float invDt = 1.0f / deltaTime;
                estimatedPlayerVelocity = (pos - lastPlayerPos) * invDt;

                // Clamp inherited speed to avoid teleports/hitches producing crazy bullets.
                float spd = estimatedPlayerVelocity.Magnitude;
                if (spd > maxInheritedSpeed && spd > 1e-6f)
                {
                    estimatedPlayerVelocity = estimatedPlayerVelocity * (maxInheritedSpeed / spd);
                }
            }
            else
            {
                estimatedPlayerVelocity = Vector3.Zero;
            }

            lastPlayerPos = pos;
            hasLastPlayerPos = true;
        }

        public override void OnUpdate(float deltaTime)
        {

            elapsedTime += deltaTime;
            //primaryAltReady = true;

            if (gameEnd)
            {
                LogMessage("[CamControl] player isnt allow to shoot as game ended!");
                return;
            }

            //Cheatcode
            if (Input.IsKeyPressed(KeyCode.O))
            {
                PrimaryAltCharge_Reward();
            }
            // Don't update when game is paused
            if (GameState.IsPaused)
                return;

            elapsedTime += deltaTime;//primaryAltReady = true;

            // Check if reload finished
            if (reloadingPrimary && elapsedTime >= reloadFinishTime)
            {
                primaryAmmo = primaryAmmoMax;

                //stop ui reticle spinning reload
                //show reload ui banner
                reloadingPrimary = false;
                shootAllowed = true;
                Publish(EVENT_AMMO_CHANGE, primaryAmmo.ToString());  // ADD THIS

                LogMessage("[PlayerWeapon] Reload complete!");
            }

            if (!shootAllowed)
            {
                LogMessage("[CamControl] player isnt allow to shoot!");
                return;
            }

            //there is only primary and primary alt no secondary
            Primary_ReloadAndCharging();
            PrimaryShoot();

            //Spawn botnet
            if (Input.IsKeyReleased(KeyCode.J))
            {
                uint botnet = PrefabInstantiate("Sources/Prefabs/Enemy_Botnet.prefab");
                LogMessage("Spawning botnet");
            }

        }

        public override void OnDestroy()
        {
            Unsubscribe(ULTGAINEVENT, UltCharging);
            Unsubscribe(GAMEWIN, OnGameStateChange);
            Unsubscribe(GAMEOVER, OnGameStateChange);
        }

        private void PrimaryShoot()
        {
            if (Input.IsMouseButtonPressed(MouseButton.Left) && primaryAmmo > 0)
            {
                primaryShooting = true;
            }
            else
            {
                primaryShooting = false;
            }

            if (primaryShooting)
            {
                PrimaryFire();
            }

            if (Input.IsMouseButtonPressed(MouseButton.Right) && primaryAltReady)
            {
                PrimaryAltFire();
            }
        }

        private void Primary_ReloadAndCharging()
        {
            //Primary - Reload and Charging Alt when holding 'R' key
            Primary_ReloadingAndCharging();

            //Primary Auto reloads when ammo reaches 0
            Primary_AutoReload();

            //Alt Charging - Primary
            //Change to events
            //Primary_AltCharging();
        }

        #region PRIMARY
        private void Primary_ReloadingAndCharging()
        {
            if ((Input.IsKeyPressed(KeyCode.R) && !primaryShooting) && !isKeyRPressedPreviously)
            {
                isKeyRPressedPreviously = true;
                chargeDelayNext = elapsedTime + chargeDelayRate;
            }
            else if (Input.IsKeyPressed(KeyCode.R) && isKeyRPressedPreviously)
            {
                //this is the charge delay function in unity
                if (elapsedTime > chargeDelayNext)
                {
                    primaryAltCharging = true;
                }
            }
            else if (Input.IsKeyReleased(KeyCode.R))
            {
                isKeyRPressedPreviously = false;
                if (!primaryAltCharging && !reloadingPrimary && primaryAmmo < primaryAmmoMax)
                {
                    //update UI here

                    //reload
                    PrimaryReload(primaryReloadDelay);


                    //play sound effects here
                }
            }
            else
            {
                primaryAltCharging = false;
            }
        }

        #endregion

        private void Primary_AutoReload()
        {
            if (!primaryAltCharging && !reloadingPrimary && primaryAmmo <= 0)
            {
                //update UI

                //reload
                PrimaryReload(primaryReloadDelay);

                //SFX
                if(reloadID != 0){
                    LogMessage("[PlayerWeapon] Playing audio right now");
                    AudioPlay(reloadID);
                }
            }
        }

        private void UltCharging(string eventName, string payload)
        {
            if (primaryAltReady || eventName != ULTGAINEVENT)
                return;

            if (!int.TryParse(payload, out int gainAmount))
                return;

            if (primaryAltCharge < primaryAltChargeMax)
            {
                primaryAltCharge += gainAmount;
            }

            if (primaryAltCharge >= primaryAltChargeMax)
            {
                //play sfx -> notify player ult is ready
                //spawn the prefab here
                PrefabInstantiate(PrimaryUltChargedPrefab);

                primaryAltReady = true;

                LogMessage("[PlayerWeapon] AltCharge is full");
            }

            //add in bullet hit audio here i guess - Amanda
        }

        private void PrimaryFire()
        {

            if (elapsedTime > primaryShootNext)
            {
                if (!CalculateFiringPos(out Vector3 bulletSpawnPos, out Vector3 bulletDirection, out Quat bulletRot))
                {
                    LogMessage("[PlayerWeapon] Failed to calculate firing position");
                    return;
                }

                Vector3 scale = new Vector3(0.025f, 0.025f, 0.025f);

                uint bulletID = 0;
                bulletID = PrefabInstantiateWithTransform(PrimaryBulletPrefab, ref bulletSpawnPos, ref bulletRot, ref scale, false);
                if (bulletID == 0)
                {
                    LogMessage("[PlayerWeapon] Primary Fire bulletID fail to instantiate");
                }

                // NEW: bullet velocity inherits (estimated) player velocity
                Vector3 bulletVel = ComputeBulletVelocity(bulletDirection, primarybulletSpeed);
                RigidbodySetVelocity(bulletID, ref bulletVel);

                primaryAmmo -= 1;
                Publish(EVENT_AMMO_CHANGE, primaryAmmo.ToString());

                primaryShootNext = elapsedTime + primaryShootRate;

                if (primaryAmmo <= 0)
                {
                    //change the ui to run out of ammo here
                }
            }
        }

        //this by right not void but rather IEnumerator
        //return things like how long till this script resume.
        private void PrimaryReload(float delay)
        {
            reloadingPrimary = true;
            shootAllowed = false;

            reloadFinishTime = elapsedTime + delay;

            LogMessage("[PlayerWeapon] Reloading... will finish in " + delay + " seconds");

            primaryAltCharging = false;
        }

        private void PrimaryAltFire()
        {
            // Calculate firing position
            if (!CalculateFiringPos(out Vector3 bulletSpawnPos, out Vector3 bulletDirection, out Quat bulletRot))
            {
                LogMessage("[PlayerWeapon] Failed to calculate firing position for Primary Alt Fire");
                return;
            }

            // Scale for ult bullet (might want to make it bigger than regular bullets)
            Vector3 scale = new Vector3(5.0f, 5.0f, 5.0f);

            // Spawn the ult bullet
            uint ultBulletID = PrefabInstantiateWithTransform(PrimaryUltBulletPrefab, ref bulletSpawnPos, ref bulletRot, ref scale, false);
            if (ultBulletID == 0)
            {
                LogMessage("[PlayerWeapon] Primary Alt Fire bulletID fail to instantiate");
                return;
            }

            // NEW: ult bullet velocity inherits (estimated) player velocity
            Vector3 bulletVel = ComputeBulletVelocity(bulletDirection, primaryUltSpeed);
            RigidbodySetVelocity(ultBulletID, ref bulletVel);

            LogMessage("[PlayerWeapon] Primary Alt Fire launched!");

            //reset the values here
            primaryAltCharge = 0;
            primaryAltReady = false;
        }

        private void PrimaryAltCharge_Reward()
        {
            if (primaryAltCharge < primaryAltChargeMax)
            {
                ++primaryAltCharge;
            }
            else
            {
                PrefabInstantiate(PrimaryUltChargedPrefab);
                primaryAltReady = true;
                LogMessage("[PlayerWeapon] AltCharge full from reward!!!");
            }
        }

        private void OnGameStateChange(string eventName, string payload)
        {
            LogMessage("[PlayerWeapon] Game State change detected. State is currently: " + eventName);
            gameEnd = false;
            shootAllowed = false;
        }

        private bool CalculateFiringPos(out Vector3 bulletSpawnPos, out Vector3 bulletDirection, out Quat bulletRot)
        {
            if (firingPointEntityID == 0 || playerEntityID == 0)
            {
                LogMessage("[PlayerWeapon] CalculateFiringPos - entities not found");
                bulletSpawnPos = Vector3.Zero;
                bulletDirection = Vector3.Zero;
                bulletRot = Quat.Identity;
                return false;
            }

            // Get firing point (camera) position and target
            Vector3 firingPoint = GetPosition(firingPointEntityID);
            Vector3 firingTarget = GetTarget(firingPointEntityID);
            bulletDirection = (firingTarget - firingPoint).Normalized;

            // Get player position and rotation
            Vector3 playerPos = GetPosition(playerEntityID);
            Quat playerRot = GetRotation(playerEntityID);

            // Calculate spawn position at gun tips
            float forwardOffset = 4.0f;
            float sideOffset = -0.1f;
            float heightOffset = 0.0f;

            Vector3 offset = (playerRot.Forward * forwardOffset) +
                             (playerRot.Right * sideOffset) +
                             (playerRot.Up * heightOffset);
            bulletSpawnPos = playerPos + offset;

            // Calculate bullet rotation (pointing in firing direction)
            bulletRot = SimpleMath.LookRotation(bulletDirection, Vector3.Up);

            // Debug logging
            Vector3 forward = playerRot.Forward;
            Vector3 right = playerRot.Right;
            Vector3 up = playerRot.Up;

            LogMessage("PlayerForward: X:" + forward.X + ", Y: " + forward.Y + ", Z: " + forward.Z);
            LogMessage("PlayerRight: X:" + right.X + ", Y: " + right.Y + ", Z: " + right.Z);
            LogMessage("PlayerUp: X:" + up.X + ", Y: " + up.Y + ", Z: " + up.Z);

            return true;
        }

        // Build projectile velocity as: muzzleSpeed along bulletDirection + inherited player velocity.
        // Guarantee: bullet forward speed always exceeds player forward speed by minBulletOverPlayerSpeed.
        private Vector3 ComputeBulletVelocity(Vector3 bulletDirection, float muzzleSpeed)
        {
            Vector3 dir = bulletDirection;

            if (dir.SqrMagnitude < 1e-8f)
                dir = Vector3.Forward;
            else
                dir = dir.Normalized;

            Vector3 inherit = Vector3.Zero;

            if (inheritPlayerVelocity && inheritVelocityFactor != 0.0f)
            {
                if (inheritAlongBulletDirectionOnly)
                {
                    float along = Vector3.Dot(estimatedPlayerVelocity, dir);
                    inherit = dir * along;
                }
                else
                {
                    inherit = estimatedPlayerVelocity;
                }

                inherit = inherit * inheritVelocityFactor;
            }

            Vector3 vel = (dir * muzzleSpeed) + inherit;

            float playerForwardSpeed = Vector3.Dot(estimatedPlayerVelocity, dir);
            float requiredForward = playerForwardSpeed + minBulletOverPlayerSpeed;

            if (requiredForward < minWorldBulletSpeed)
                requiredForward = minWorldBulletSpeed;

            float currentForward = Vector3.Dot(vel, dir);
            if (currentForward < requiredForward)
            {
                vel = vel + (dir * (requiredForward - currentForward));
            }

            return vel;
        }

    }
}
