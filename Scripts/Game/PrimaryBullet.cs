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
    /// - Also listens for Damage:&lt;EntityID&gt; so environment/hazard scripts can kill it
    ///   without needing CollisionManager participation
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
        private bool audioplayed = false;

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

            if (!audioplayed)
            {
                AudioPlay((uint)EntityID);
                audioplayed = true;
            }

            elapsedTime += deltaTime;

            if (!hit && elapsedTime >= ProjectileLifetime)
            {
                SceneDestroyEntity((uint)EntityID);
                return;
            }

            if (hit)
            {
                SceneDestroyEntity((uint)EntityID);
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
            DamageSystem.DealDamage(targetEntityID, Damage, bulletEntityID);

            Publish("BulletHit", targetEntityID.ToString());
            Publish("BulletHitEnemy", true.ToString());
            Publish("GainUlt", UltRecharged.ToString());

            LogMessage(
                "BulletHit: target=" + targetEntityID +
                " from bullet=" + bulletEntityID +
                " damage=" + Damage
            );

            AudioStop((uint)EntityID);
            hit = true;
        }

        private void OnEnvironmentHit(string eventName, string payload)
        {
            if (hit)
                return;

            LogMessage("[PrimaryBullet] Environment hit received for bullet " + EntityID.ToString());

            AudioStop((uint)EntityID);
            hit = true;
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(environmentDamageEvent, OnEnvironmentHit);
        }
    }
}