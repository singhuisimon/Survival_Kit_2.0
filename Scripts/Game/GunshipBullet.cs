using Engine;
using System;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Physics;
using static Engine.Tag;
using static Engine.Event;

namespace Game 
{

    /// <summary>
    /// 
    /// 
    /// </summary>

    public class GunshipBullet : ScriptBehaviour 
    {

        // Settings
        [SerializeField] private float projectileLifetime = 2.0f;  
        [SerializeField] private float damage = 1.0f;              
        
        // Tags this bullet can damage (Add on to this for future enemies)
        [SerializeField] private string[] targetTags = { "botnet", "loveletter", "adware" };
        
        // This bullet's tag (should match your prefab)
        [SerializeField] private string[] bulletTags = { "GunshipBullet" };
        
        // Private States
        private float elapsedTime = 0.0f;
        private bool hasHit = false;

        public override void OnStart() 
        {
            // Enable collision detection
            PhysicsEnableCollisionEvents();
            
            LogMessage("Gunship bullet spawned (ID: " + EntityID + ")");
        }

        public override void OnUpdate(float deltaTime)
        {
            // Hit Something (Collide with something)
            if (hasHit)
                return;
            
            // Update lifetime
            elapsedTime += deltaTime;
            
            // Destroy if lifetime expired
            if (elapsedTime >= projectileLifetime)
            {
                SceneDestroyEntity((uint)EntityID);
                return;
            }
            
            // Check for collisions with enemies
            CheckCollisions();
        }        

        public override void OnDestroy()
        {
            // Cleanup
        }

        // Collision Detection

        private void CheckCollisions()
        {
            // Get all collisions this frame
            int collisionCount = PhysicsGetCollisionCount();
            
            if (collisionCount <= 0)
                return;
            
            uint bulletID = (uint)EntityID;
            
            // Check each collision
            for (int i = 0; i < collisionCount; i++)
            {
                PhysicsGetCollisionPair(i, out uint entityA, out uint entityB);
                
                // Checking if the bullet is involved with the collision
                if (entityA != bulletID && entityB != bulletID)
                    continue;
                
                uint otherEntity = (entityA == bulletID) ? entityB : entityA;
                
                // Check if it's a valid target (enemy)
                string tag = TagGetTag(otherEntity);
                
                if (IsTargetTag(tag))
                {
                    // Hit the Enemy Target
                    OnHitTarget(otherEntity);
                    return;
                }
            }
        }

        // Check if a tag is one of our target tags
        private bool IsTargetTag(string tag)
        {
            if (string.IsNullOrEmpty(tag) || targetTags == null)
                return false;
            
            for (int i = 0; i < targetTags.Length; i++)
            {
                if (!string.IsNullOrEmpty(targetTags[i]) && 
                    string.Equals(tag, targetTags[i], StringComparison.OrdinalIgnoreCase))
                {
                    return true;
                }
            }
            
            return false;
        }

        // Hit Handlings

        private void OnHitTarget(uint targetID)
        {
            if (hasHit)
                return;
            
            hasHit = true;
            
            LogMessage("Gunship bullet hit target: " + targetID);
            
            // Deal damage to the target
            DealDamage(targetID);
            
            // Publish events
            Publish("BulletHit", targetID.ToString());
            Publish("BulletHitEnemy", true.ToString());
            
            // Destroy bullet
            SceneDestroyEntity((uint)EntityID);
        }
        
        private void DealDamage(uint targetID)
        {
            // Use DamageSystem if available
            DamageSystem.DealDamage(targetID, damage, (uint)EntityID);
            
            string eventName = "EnemyDamage:" + targetID.ToString();
            Publish(eventName, damage.ToString());
        }

    }   
}   // end of namespace Game