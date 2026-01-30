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

namespace Game{

    /// <Summary>
    /// This script manages the weapon player uses
    /// </Summary>
    public class PlayerWeapon : ScriptBehaviour{

        [SerializeField] private bool shootAllowed;

        #region primary
        [SerializeField] private bool primaryShooting = false;  //player is currently shooting
        [SerializeField] private bool reloadingPrimary = false; //player is currently reloading
        [SerializeField] private int primaryAmmo = 0;
        [SerializeField] private int primaryAmmoMax = 100;
        [SerializeField] private float primaryReloadDelay = 1.5f; //reloading time
        [SerializeField] private float primaryShootRate = 0.05f; 
        [SerializeField] private float primaryShootNext = 0.0f;
        [SerializeField] private float primarybulletSpeed = 1000.0f;

        [SerializeField] private string PrimaryBulletPrefab = "Sources/Prefabs/PrimaryBullet.prefab";
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
        [SerializeField] private float primaryUltSpeed = 500.0f;

        [SerializeField] private string PrimaryUltBulletPrefab = "Sources/Prefabs/PrimaryUltBullet.prefab";

        private string ULTGAINEVENT = "GainUlt";

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

        [SerializeField] private string firingPointName = "PlayerCam";
        [SerializeField] private string playerName = "Player";

        #endregion

        #region others
        
        [SerializeField] private float muzzleDistance = 5.0f;

        #endregion

        private uint firingPointEntityID = 0;
        private uint playerEntityID = 0;
        private bool isKeyRPressedPreviously = false;
        private float elapsedTime = 0.0f;

        private float reloadFinishTime = 0.0f;
        private Vector3 bulletDirection;

        public override void OnStart(){

            //Initialize Values
            shootAllowed = true;
            primaryAmmo = primaryAmmoMax;
            primaryAltCharge = 0;

            firingPointEntityID = SceneFindEntityByName(firingPointName);
            playerEntityID = SceneFindEntityByName(playerName);

            if(firingPointEntityID == 0){
                LogMessage("[CamControl] firing point entity cannot be found");
            }
            if(playerEntityID == 0){
                LogMessage("[CamControl] player entity cannot be found");
            }

            Subscribe(ULTGAINEVENT, UltCharging);
        }

        public override void OnUpdate(float deltaTime){

            elapsedTime += deltaTime;primaryAltReady = true;

            // Check if reload finished
            if (reloadingPrimary && elapsedTime >= reloadFinishTime)
            {
                primaryAmmo = primaryAmmoMax;
                //stop ui reticle spinning reload
                //show reload ui banner
                reloadingPrimary = false;
                shootAllowed = true;
                LogMessage("[PlayerWeapon] Reload complete!");
            }

            //Cheatcode
            if(Input.IsKeyPressed(KeyCode.O)){
                PrimaryAltCharge_Reward();
            }
            
            if(!shootAllowed){
                LogMessage("[CamControl] player isnt allow to shoot!");
                return;
            }

            //there is only primary and primary alt no secondary
            Primary_ReloadAndCharging();
            PrimaryShoot();

            //Spawn botnet
            if(Input.IsKeyReleased(KeyCode.J)){
                uint botnet = PrefabInstantiate("Sources/Prefabs/Enemy_Botnet.prefab");
                LogMessage("Spawning botnet");
            }
        }

        public override void OnDestroy(){
            Unsubscribe(ULTGAINEVENT, UltCharging);
        }

        private void PrimaryShoot(){
            if(Input.IsMouseButtonPressed(MouseButton.Left) && primaryAmmo > 0)
            {
                primaryShooting = true;
            } 
            else 
            {
                primaryShooting = false;
            }

            if(primaryShooting)
            {
                PrimaryFire();
            }

            if(Input.IsMouseButtonPressed(MouseButton.Right) && primaryAltReady)
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
            if((Input.IsKeyPressed(KeyCode.R) && !primaryShooting) && !isKeyRPressedPreviously)
            {
                isKeyRPressedPreviously = true;
                chargeDelayNext = elapsedTime + chargeDelayRate;
            } 
            else if(Input.IsKeyPressed(KeyCode.R) && isKeyRPressedPreviously)
            {
                //this is the charge delay function in unity
                if(elapsedTime > chargeDelayNext){
                    primaryAltCharging = true;
                }
            } 
            else if(Input.IsKeyReleased(KeyCode.R)){
                isKeyRPressedPreviously = false;
                if(!primaryAltCharging && !reloadingPrimary && primaryAmmo < primaryAmmoMax)
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
            if(!primaryAltCharging && !reloadingPrimary && primaryAmmo <= 0)
            {
                //update UI

                //reload
                PrimaryReload(primaryReloadDelay);

                //SFX
            }
        }

        // private void Primary_AltCharging()
        // {
        //     if(primaryAltCharging && !primaryShooting)
        //     {
        //         if(primaryAltCharge < primaryAltChargeMax)
        //         {
        //             PrimaryCharge();
        //         }
        //         else {
        //             LogMessage("[PlayerWeapon] Alt Charge Full");
        //             primaryAltReady = true;

        //             //play SFX ->notify player ult is ready?
        //         }
        //     } 
        //     else if(primaryAltCharge >= primaryAltChargeMax)
        //     {
        //         primaryAltReady = true;
        //     }
        // }

        private void UltCharging(string eventName, string payload)
        {
            if (primaryAltReady || eventName != ULTGAINEVENT)
                return;

            if (!int.TryParse(payload, out int gainAmount))
                return;

            if(primaryAltCharge < primaryAltChargeMax){
                primaryAltCharge += gainAmount;
            }

            if(primaryAltCharge >= primaryAltChargeMax){
                //play sfx -> notify player ult is ready
                //publish event here

                primaryAltReady = true;

                LogMessage("[PlayerWeapon] AltCharge is full");
            }

            //add in bullet hit audio here i guess - Amanda
        }

        private void PrimaryFire(){
           
            if(elapsedTime > primaryShootNext)
            {
                
                //testing claude theory for now so commenting - Amanda 26/1 4.37pm
                //put the below under the calculate function (TODO for claude)
                //  if(firingPointEntityID == 0 || playerEntityID == 0){
                //     LogMessage("[PlayerWeapon] returning at PrimaryFire");
                //     return;
                // }

                // //Instantiate at the firing position & rotation);
                // Vector3 firingPoint = GetPosition(firingPointEntityID);
                // Vector3 firingTarget = GetTarget(firingPointEntityID);
                // Vector3 bulletDirection = (firingTarget - firingPoint).Normalized;

                // Vector3 playerPos = GetPosition(playerEntityID);
                // Quat playerRot = GetRotation(playerEntityID);

                // Vector3 playerForward = playerRot.Forward;
                // //Vector3 bulletSpawnPos = playerPos + (playerForward * muzzleDistance);

                // //NEW
                // Vector3 offset = playerRot.Right * 5.0f;
                // Vector3 bulletSpawnPos = playerPos + offset;

                // //DEBUGGING
                // Vector3 forward = playerRot.Forward;
                // Vector3 right = playerRot.Right;
                // Vector3 up = playerRot.Up;

                // LogMessage("PlayerForward: X:" + forward.X + ", Y: " + forward.Y + ", Z: " + forward.Z);
                // LogMessage("PlayerRight: X:" + right.X + ", Y: " + right.Y + ", Z: " + right.Z);
                // LogMessage("PlayerUp: X:" + up.X + ", Y: " + up.Y + ", Z: " + up.Z);


                // Quat bulletRot = SimpleMath.LookRotation(bulletDirection, Vector3.Up);

                // FIX #1: Use the separated calculation function
                if(!CalculateFiringPos(out Vector3 bulletSpawnPos, out Vector3 bulletDirection, out Quat bulletRot))
                {
                    LogMessage("[PlayerWeapon] Failed to calculate firing position");
                    return;
                }

                //End of where the calculate firing pos 
                
                Vector3 scale = new Vector3(0.1f, 0.1f, 0.1f);

                uint bulletID = 0;
                bulletID = PrefabInstantiateWithTransform(PrimaryBulletPrefab, ref bulletSpawnPos, ref bulletRot, ref scale, false);
                if(bulletID == 0){
                    LogMessage("[PlayerWeapon] Primary Fire bulletID fail to instantiate");
                    //return; //comment this for debugging temp
                }


                Vector3 bulletVel = bulletDirection * primarybulletSpeed;
                RigidbodySetVelocity(bulletID, ref bulletVel);

                primaryAmmo -= 1;

                //vfx

                primaryShootNext = elapsedTime + primaryShootRate;

                //cameraShake
                //add function to adjust the target of the camera or the position whichever looks more real

                if(primaryAmmo <= 0){
                    //change the ui to run out of ammo here
                }
                
            }
        }

        //this by right not void but rather IEnumerator
        //return things like how long till this script resume.
        private void PrimaryReload(float delay){
            reloadingPrimary = true;
            shootAllowed = false;

            //yield return new Delay primaryReloadDelay - TODO
            reloadFinishTime = elapsedTime + delay;

            LogMessage("[PlayerWeapon] Reloading... will finish in " + delay + " seconds");

            primaryAltCharging = false;

            //delay for 0.5f - TODO

            // testing claude theory so commenting these out for now
            //primaryAmmo = primaryAmmoMax;
            //reloadingPrimary = false;
            //primaryAltCharging = false;
            //shootAllowed = true;
        }

        private void PrimaryAltFire(){
            //TODO:
            //Check with LiXiang on how does this work by tonight 21/1/26
            // Calculate firing position
            if(!CalculateFiringPos(out Vector3 bulletSpawnPos, out Vector3 bulletDirection, out Quat bulletRot))
            {
                LogMessage("[PlayerWeapon] Failed to calculate firing position for Primary Alt Fire");
                return;
            }
            
            // Scale for ult bullet (might want to make it bigger than regular bullets)
            Vector3 scale = new Vector3(5.0f, 5.0f, 5.0f);

            // Spawn the ult bullet
            uint ultBulletID = PrefabInstantiateWithTransform(PrimaryUltBulletPrefab, ref bulletSpawnPos, ref bulletRot, ref scale, false);
            if(ultBulletID == 0){
                LogMessage("[PlayerWeapon] Primary Alt Fire bulletID fail to instantiate");
                return;
            }

            // Set velocity for the ult bullet
            Vector3 bulletVel = bulletDirection * primaryUltSpeed;
            RigidbodySetVelocity(ultBulletID, ref bulletVel);

            LogMessage("[PlayerWeapon] Primary Alt Fire launched!");

            // TODO: Camera shake for ult fire (use CAMSHAKE_primaryAltFire)
            // TODO: SFX for ult fire
            // TODO: VFX for ult fire
            
            //reset the values here
            primaryAltCharge = 0;
            primaryAltReady = false;
        }

        // private void PrimaryCharge()
        // {
        //     if(elapsedTime > primaryChargeNext)
        //     {
        //         if(primaryAmmo > 0)
        //         {
        //             primaryAmmo -= 1;
        //             primaryAltCharge += 1;

        //             //SFX for altcharge

        //             primaryChargeNext = elapsedTime + primaryChargeRate;

        //             LogMessage("[PlayerWeapon] Charging... " + primaryAltCharge + "/" + primaryAltChargeMax);
        //         }
        //     }
        // }

        private void PrimaryAltCharge_Reward()
        {
            if(primaryAltCharge < primaryAltChargeMax)
            {
                ++primaryAltCharge;
            } 
            else 
            {
                primaryAltReady = true;
                LogMessage("[PlayerWeapon] AltCharge full from reward!!!");
            }
        }

        private bool CalculateFiringPos(out Vector3 bulletSpawnPos, out Vector3 bulletDirection, out Quat bulletRot){
            if(firingPointEntityID == 0 || playerEntityID == 0){
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

            // Calculate spawn position (offset to the right of the player)
            Vector3 offset = playerRot.Right * 5.0f;
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

    }
}