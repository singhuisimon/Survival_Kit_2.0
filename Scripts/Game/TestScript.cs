using System;
using Engine;

namespace Game
{
    public class TestScript
    {
        public int EntityID;
        private int frameCount = 0;
        private uint playerID = 99;
        private float fireCD = 0.0f;
        private float fireTimer = 0.1f;


        [SerializeField]
        private int health = 100;

        [SerializeField("Run Speed")]
        private float speed = 10.0f;

        [SerializeField]
        private bool isActive = true;

        [SerializeField]
        private string characterName = "Player";

        public void OnStart()
        {
            Engine.InternalCalls.Log("TestScript started!");
        }

        public void OnUpdate(float deltaTime)
        {
            frameCount++;
            if (playerID == 99)
            {

                playerID = Engine.InternalCalls.Scene_FindEntityByName("Player");

            }

            fireCD -= deltaTime;
            //Engine.InternalCalls.Log(string.Concat("FireCD: ", fireCD.ToString()));

            if (Engine.Input.IsKeyPressed(Engine.KeyCode.Enter) && fireCD <= 0)
            {
                fireCD = fireTimer;

                // Create bullet ent
                uint bullet = Engine.InternalCalls.Scene_CreateEntity("Bullet");

                // Add script to bullet
                Engine.InternalCalls.Entity_AddScript(bullet, "Game.Projectile");
                Engine.InternalCalls.Entity_AddRigidBody(bullet);
                Engine.Vector3 v3 = default;
                Engine.Vector3 spawn = new Engine.Vector3(v3.X, v3.Y, v3.Z + 0.5f);
                Engine.InternalCalls.Transform_SetPosition(bullet, ref spawn);
                Engine.Vector3 vel = new Engine.Vector3(0, 0, 1400f);
                Engine.InternalCalls.Rigidbody_SetVelocity(bullet, ref vel);
                Engine.InternalCalls.Log("Firing Bullet!");
            }
        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("TestScript destroyed!");
        }
    }
}
