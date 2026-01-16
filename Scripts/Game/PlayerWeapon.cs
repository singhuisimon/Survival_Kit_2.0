using System;
using Engine;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.Prefab;
using static Engine.Logger;
using static Engine.Input;
using static Engine.Rigidbody;

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
        [SerializeField] private float bulletOffset = 10.0f;

        [SerializeField] private string PrimaryBulletPrefab = "Sources/Prefabs/PrimaryBullet.prefab";

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

        #endregion

        #region entity

        [SerializeField] private string lookAtName = "LookAt";
        [SerializeField] private string playerName = "Player";

        #endregion

        private uint lookAtEntityID = 0;
        private uint playerEntityID = 0;
        private bool isKeyRPressedPreviously = false;
        private float elapsedTime = 0.0f;

        private float reloadFinishTime = 0.0f;

        // [SerializeField] private bool primaryWeapon = true;
        // [SerializeField] private bool primaryAltWeapon = true;
        // [SerializeField] private int primaryWeaponAmmo = 100;
        // [SerializeField] private float primaryCooldown = 0.05f;
        // [SerializeField] private float primaryReloadSpeed = 1.5f;
        // [SerializeField] private float primaryAltCooldown = -1.0f;
        // [SerializeField] private float primaryAltReloadSpeed = -1.0f;
        // [SerializeField] private float primaryAltCharge = 30.0f;

        // private bool weaponReload = false;
        // private bool primaryAltReload = false;
        // private float primaryElapsedTime = 0.0f;
        // private float primaryElapsedReload = 0.0f;

        public override void OnStart(){
            //Initialize Values
            shootAllowed = true;
            primaryAmmo = primaryAmmoMax;
            primaryAltCharge = primaryAltChargeMax;

            lookAtEntityID = SceneFindEntityByName(lookAtName);
            playerEntityID = SceneFindEntityByName(playerName);

            if(lookAtEntityID == 0){
                LogMessage("[CamControl] look at entity cannot be found");
            }
            if(playerEntityID == 0){
                LogMessage("[CamControl] player entity cannot be found");
            }
        }

        public override void OnUpdate(float deltaTime){

            elapsedTime += deltaTime;

            // Check if reload finished
            if (reloadingPrimary && elapsedTime >= reloadFinishTime)
            {
                primaryAmmo = primaryAmmoMax;
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


            // if(Input.IsMouseButtonPressed(Mouse.Left)){
            //     primaryWeapon = true;
            //     primaryAltWeapon = false;
            //     ShootPrimaryBullet(deltaTime);
            // } else if (Input.IsMouseButtonPressed(Mouse.Right)){
            //     primaryWeapon = false;
            //     primaryAltWeapon = true;
            //     ShootPrimaryAltBullet(deltaTime);
            // }

            // if(Input.IsKeyPressed(KeyCode.R)){
            //     weaponReload = true;
            // }
            
            // //Update the reload timing should it be true
            // if(weaponReload == true){
            //     primaryElapsedReload += deltaTime;
            // }


            // //Check if reload time has passed if so reload
            // if(primaryElapsedRelaod >= primaryReloadSpeed){
            //     primaryWeaponAmmo = 100;
            //     primaryElapsedReload = 0;
            //     primaryReload = false;
            // }
            
        }

        private void PrimaryShoot(){
            if(Input.IsMouseButtonPressed(MouseButton.Left) && primaryAmmo > 0){
                primaryShooting = true;
            } else {
                primaryShooting = false;
            }

            if(primaryShooting){
                PrimaryFire();
            }

            if(Input.IsMouseButtonPressed(MouseButton.Right) && primaryAltReady){
                PrimaryAltFire();
            }
        }

        private void Primary_ReloadAndCharging(){
            //Primary - Reload and Charging Alt when holding 'R' key
            Primary_ReloadingAndCharging();

            //Primary Auto reloads when ammo reaches 0
            Primary_AutoReload();

            //Alt Charging - Primary
            Primary_AltCharging();
        }

        #region PRIMARY
        private void Primary_ReloadingAndCharging(){
            if((Input.IsKeyPressed(KeyCode.R) && !primaryShooting) && !isKeyRPressedPreviously){
                isKeyRPressedPreviously = true;
                chargeDelayNext = elapsedTime + chargeDelayRate;
            } else if(Input.IsKeyPressed(KeyCode.R) && isKeyRPressedPreviously){
                if(elapsedTime > chargeDelayNext){
                    primaryAltCharging = true;
                }
            } else if(Input.IsKeyReleased(KeyCode.R)){
                isKeyRPressedPreviously = false;
                if(!primaryAltCharging && !reloadingPrimary && primaryAmmo < primaryAmmoMax){
                    //update UI here

                    //reload
                    PrimaryReload(primaryReloadDelay);
                    

                    //play sound effects here
                }
            } else {
                primaryAltCharging = false;
            }
        }

        #endregion

        #region reloading

        private void Primary_AutoReload(){
            if(!primaryAltCharging && !reloadingPrimary && primaryAmmo <= 0){
                //update UI

                //reload
                PrimaryReload(primaryReloadDelay);

                //SFX
            }
        }

        private void PrimaryAltCharging(){
            if(primaryAltCharging && !primaryShooting){
                if(primaryAltCharge < primaryAltChargeMax){
                    PrimaryCharge();
                }
                else {
                    LogMessage("[PlayerWeapon] Alt Charge Full");
                    primaryAltReady = true;

                    //play SFX
                }
            } else if(primaryAltCharge >= primaryAltChargeMax){
                primaryAltReady = true;
            }
        }

        private void PrimaryReload(float delay){
            reloadingPrimary = true;
            shootAllowed = false;

            //yield return new Delay primaryReloadDelay - TODO
            reloadFinishTime = elapsedTime + delay;

            primaryAmmo = primaryAmmoMax;

            //delay for 0.5f - TODO

            // remove temporary
            // reloadingPrimary = false;
            // primaryAltCharging = false;
            // shootAllowed = true;
        }

        private void Primary_AltCharging(){
            if(primaryAltCharging && !primaryShooting){
                if(primaryAltCharge < primaryAltChargeMax){
                    PrimaryCharge();
                }
                else{
                    LogMessage("[PlayerWeapon] alt charge full");
                    primaryAltReady = true;

                    //SFX
                }
            } else if (primaryAltCharge >= primaryAltChargeMax){
                primaryAltReady = true;
            }
        }

        private void PrimaryFire(){
           
            if(elapsedTime > primaryShootNext){

                 if(lookAtEntityID == 0 || playerEntityID == 0){
                    LogMessage("[PlayerWeapon] returning at PrimaryFire");
                    return;
                }

                Quat rotation = GetRotation(playerEntityID);
                Vector3 playerPos = GetPosition(playerEntityID);
                Vector3 velocity = RigidbodyGetVelocity(lookAtEntityID);
                Vector3 targetOffset = Vector3.Zero;

                if(velocity.SqrMagnitude > 0.1f){
                    Vector3 moveDir = velocity.Normalized;
                    Vector3 perpendicular = Vector3.Cross(moveDir, Vector3.Up);
                    targetOffset = perpendicular * bulletOffset;
                }

                
                Vector3 forward = rotation.Forward;
                Vector3 bulletPos = playerPos + (forward * bulletOffset);
                Vector3 scale = new Vector3(0.1f, 0.1f, 0.1f);
                
                //Instantiate at the firing position & rotation);
                //Vector3
                uint bulletID = 0;
                bulletID = PrefabInstantiateWithTransform(PrimaryBulletPrefab, ref bulletPos, ref rotation, ref scale, false);
                if(bulletID == 0){
                    LogMessage("[PlayerWeapon] Primary Fire bulletID fail to instantiate");
                }
                //TODO NEED DO PRIMARY BULLET SCRIPT
                //EntityAddScript(bulletID, "Game.PrimaryBullet");
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

        private void PrimaryAltFire(){
            //TODO:
            // SPAWN PREFAB -> IN SCRIPT FOR IT ENSURE IF COLLIDED DEAL DAMAGE.
            primaryAltCharge = 0;
            primaryAltReady = false;
        }

        private void PrimaryCharge(){
            if(elapsedTime > primaryChargeNext){
                if(primaryAmmo > 0){
                    primaryAmmo -= 1;
                    primaryAltCharge += 1;

                    //SFX for altcharge

                    primaryChargeNext = elapsedTime + primaryChargeRate;
                }
            }
        }

        private void PrimaryAltCharge_Reward(){
            if(primaryAltCharge < primaryAltChargeMax){
                ++primaryAltCharge;
            } else {
                primaryAltReady = true;
                LogMessage("[PlayerWeapon] AltCharge full from reward!!!");
            }
        }

        #endregion

    }
}