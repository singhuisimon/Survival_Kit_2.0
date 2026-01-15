using System;
using Engine;
using static Engine.Scene;
using static Engine.Transform;

namespace Game{
    /// <Summary>
    /// This script manages the weapon player uses
    /// </Summary>
    public class PlayerWeapon : ScriptBehaviour{
        [SerializeField] private bool primaryWeapon = true;
        [SerializeField] private bool primaryAltWeapon = true;
        [SerializeField] private int primaryWeaponAmmo = 100;
        [SerializeField] private float primaryCooldown = 0.05f;
        [SerializeField] private float primaryReloadSpeed = 1.5f;
        [SerializeField] private float primaryAltCooldown = -1.0f;
        [SerializeField] private float primaryAltReloadSpeed = -1.0f;
        [SerializeField] private float primaryAltCharge = 30.0f;

        private bool primaryReload = false;
        private bool primaryAltReload = false;
        private float primaryElapsedTime = 0.0f;

        public void OnStart(){

        }

        public void OnUpdate(float deltaTime){

            if(Input.IsMouseButtonPressed(Mouse.Left)){
                primaryWeapon = true;
                primaryAltWeapon = false;
                ShootPrimaryBullet(deltaTime);
            } else if (Input.IsMouseButtonPressed(Mouse.Right)){
                primaryWeapon = false;
                primaryAltWeapon = true;
                ShootPrimaryAltBullet(deltaTime);
            }

            if(Input.IsKeyPressed(KeyCode.R)){
                if(primaryWeapon){
                    primaryReload = true;
                } else {
                    primaryAltReload = true;
                }
            }
            
            if(primaryReload == true){
                ReloadPrimaryBullet(float deltaTime);
            }
            
        }

        private void ShootPrimaryBullet(float deltaTime){
            
        }

        private void ReloadPrimaryBullet(){
            
        }
    }
}