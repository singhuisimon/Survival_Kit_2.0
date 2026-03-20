using Engine;
using System.Collections.Generic;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Rigidbody;
using static Engine.Audio;

namespace Game
{
    /// <summary>
    /// Primary weapon bullet.
    /// - Uses CollisionManager for valid enemy hits
    /// - Also listens for Damage:<EntityID> so environment/hazard scripts can kill it
    ///   without needing CollisionManager participation
    /// - After impact, becomes inert immediately but destroys on the following update,
    ///   so attacker metadata can still be resolved by other scripts for one full frame.
    /// </summary>
    public class PrimaryBullet : ScriptBehaviour
    {
        [SerializeField]
        public float ProjectileLifetime = 2.0f;

        [SerializeField]
        public float Damage = 1.0f;

        [SerializeField]
        public int UltRecharged = 1;

        private float elapsedTime = 0.0f;
        private Vector3 savedVelocity = Vector3.Zero;
        private bool wasPaused = false;

        private bool hit = false;

        // Frame-based deferred destroy state:
        // 1) queue on hit
        // 2) arm on next update
        // 3) destroy on the update after that
        private bool destroyQueued = false;
        private bool destroyArmed = false;

        private string environmentDamageEvent = "Damage:";

        public override void OnStart()
        {
            environmentDamageEvent = "Damage:" + EntityID.ToString();
            Event.Subscribe(environmentDamageEvent, OnEnvironmentHit);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (GameState.IsPaused)
            {
                if (!wasPaused)
                {
                    savedVelocity = RigidbodyGetVelocity((uint)EntityID);
                    Vector3 zero = Vector3.Zero;
                    RigidbodySetVelocity((uint)EntityID, ref zero);
                    wasPaused = true;
                }
                return;
            }
            else if (wasPaused)
            {
                RigidbodySetVelocity((uint)EntityID, ref savedVelocity);
                wasPaused = false;
            }

            elapsedTime += deltaTime;

            if (!hit && elapsedTime >= ProjectileLifetime)
            {
                SceneDestroyEntity((uint)EntityID);
                return;
            }

            if (destroyArmed)
            {
                SceneDestroyEntity((uint)EntityID);
                return;
            }

            if (destroyQueued)
            {
                destroyQueued = false;
                destroyArmed = true;
                return;
            }
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (GameState.IsPaused)
                return;

            if (!hit)
                CheckCollisions();
        }

        private void CheckCollisions()
        {
            // Enemy / valid target hits still come from CollisionManager.
            List<uint> hits = CollisionManager.GetPlayerProjectileHits((uint)EntityID);

            if (hits == null || hits.Count == 0)
                return;

            foreach (uint targetId in hits)
            {
                OnBulletHit((uint)EntityID, targetId);

                // Stop after the first valid hit so we do not double-process in the same frame.
                if (hit)
                    return;
            }
        }

        private void OnBulletHit(uint bulletEntityID, uint targetEntityID)
        {
            if (hit)
                return;

            DamageSystem.DealDamage(targetEntityID, Damage, bulletEntityID);

            Publish("BulletHit", targetEntityID.ToString());
            Publish("BulletHitEnemy", true.ToString());
            Publish("GainUlt", UltRecharged.ToString());

            LogMessage(
                "BulletHit: target=" + targetEntityID +
                " from bullet=" + bulletEntityID +
                " damage=" + Damage
            );

            BeginDeferredDestroy();
        }

        private void OnEnvironmentHit(string eventName, string payload)
        {
            if (hit)
                return;

            LogMessage("[PrimaryBullet] Environment hit received for bullet " + EntityID.ToString());
            BeginDeferredDestroy();
        }

        private void BeginDeferredDestroy()
        {
            hit = true;

            // Stop movement immediately so the bullet is logically dead now.
            Vector3 zero = Vector3.Zero;
            RigidbodySetVelocity((uint)EntityID, ref zero);

            // Do not destroy on the very next update.
            // Instead, survive that update and destroy on the following one.
            destroyQueued = true;
            destroyArmed = false;
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(environmentDamageEvent, OnEnvironmentHit);
        }
    }
}