using Engine;
using System;

namespace Game
{
    public class SecondaryBullet : ScriptBehaviour
    {
        [SerializeField]
        public float ProjectileSpeed = 800.0f;

        [SerializeField]
        public float ProjectileLifetime = 5.0f;

        [SerializeField]
        public float Damage = 25.0f;

        [SerializeField]
        public string VortexPrefabPath = "Resources/Prefabs/VortexAOE.json";

        private float elapsedTime = 0.0f;

        public override void OnStart()
        {
            Log("SecondaryBullet spawned successfully! EntityID: " + EntityID);
        }

        public override void OnUpdate(float deltaTime)
        {
            elapsedTime += deltaTime;

            // Auto-destroy after lifetime expires (shorter than primary)
            if (elapsedTime >= ProjectileLifetime)
            {
                Log("SecondaryBullet lifetime expired, destroying...");
                InternalCalls.Scene_DestroyEntity((uint)EntityID);
                return;
            }

            // Check for collisions
            CheckCollisions();
        }

        private void CheckCollisions()
        {
            int collisionCount = InternalCalls.Physics_GetCollisionCount();

            for (int i = 0; i < collisionCount; i++)
            {
                InternalCalls.Physics_GetCollisionPair(i, out uint entityA, out uint entityB);

                if (entityA == EntityID || entityB == EntityID)
                {
                    uint otherEntity = (entityA == EntityID) ? entityB : entityA;
                    OnHitEnemy(otherEntity);
                    return;
                }
            }
        }

        public void OnHitEnemy(uint targetEntityID)
        {
            Log("SecondaryBullet hit entity " + targetEntityID + "! Spawning Vortex AOE...");

            // Spawn Vortex AOE at impact location
            SpawnVortexAOE();

            // Destroy this bullet
            InternalCalls.Scene_DestroyEntity((uint)EntityID);
        }

        private void SpawnVortexAOE()
        {
            try
            {
                // Get current bullet position before destroying
                Vector3 currentPosition;
                InternalCalls.Transform_GetPosition((uint)EntityID, out currentPosition);

                Log("Spawning Vortex at: " + currentPosition.X + ", " + currentPosition.Y + ", " + currentPosition.Z);

                // Instantiate vortex prefab
                uint vortexEntityID = InternalCalls.Prefab_Instantiate(VortexPrefabPath);

                if (vortexEntityID != 0)
                {
                    // Set vortex position to bullet's impact location
                    InternalCalls.Transform_SetPosition(vortexEntityID, ref currentPosition);

                    Log("Spawned Vortex AOE successfully (Entity ID: " + vortexEntityID + ")");
                }
                else
                {
                    LogError("Failed to instantiate Vortex prefab: " + VortexPrefabPath);
                    LogError("Make sure the prefab exists and path is correct");
                }
            }
            catch (Exception e)
            {
                LogError("Exception spawning Vortex AOE: " + e.Message);
                LogError("Stack trace: " + e.StackTrace);
            }
        }

        public override void OnDestroy()
        {
            Log("SecondaryBullet destroyed");
        }
    }
}