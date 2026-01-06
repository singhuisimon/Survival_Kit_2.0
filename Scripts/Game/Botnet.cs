// Botnet.cs
using Engine;
using System;
using static Engine.Event;
using static Engine.Log;
using static Engine.Scene;
using static Engine.Prefab;
using static Engine.Physics;
using static Engine.Rigidbody;
using static Engine.Audio;
using static Engine.Tag;

namespace Game
{
    /// <summary>
    /// Port of EnemyE004_BOTNET from Unity to custom engine script.
    /// Uses rigidbody physics via wrapped Engine functions and simple tag-based targeting.
    /// Now with look-at functionality using quaternion-only rotation.
    /// </summary>
    public class Botnet : ScriptBehaviour
    {
        // ===== Serialized Fields (Editable in Inspector) =====

        // Movement
        [SerializeField] private float acceleration = 50.0f;
        [SerializeField] private float topSpeed = 30.0f;

        // Rotation speed (how fast bot turns towards target)
        [SerializeField] private float rotateSpeed = 5.0f;
        [SerializeField] private bool enableLookAt = true; // Toggle look-at behavior

        // Explosion properties
        [SerializeField] private float blastRadius = 5.0f;

        // Status effects
        [SerializeField] private bool isStunned = false;
        [SerializeField] private float stunnedTime = 1.0f;

        // Brute-force attack (rage mode)
        [SerializeField] private float bruteForceAttackSpeed = 300.0f;

        // Target selection timing
        [SerializeField] private float minInitialTargetDelay = 0.5f;
        [SerializeField] private float maxInitialTargetDelay = 0.8f;
        [SerializeField] private float minRetargetDelay = 1.0f;
        [SerializeField] private float maxRetargetDelay = 1.5f;

        // Death explosion prefab path
        [SerializeField] private string deathExplosionPrefab = "Sources/Prefabs/BotnetExplosion.prefab";

        // ===== Private Runtime State =====

        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint targetID = INVALID_ENTITY;

        private bool isMoving = false;
        private bool isDead = false;
        private bool isExploding = false;

        // Timers
        private float stunTimer = 0.0f;
        private float chooseTargetTimer = 0.0f;
        private bool hasChosenInitialTarget = false;

        // Simple script-side RNG state (xorshift32)
        private static uint s_RngState = 0x12345678u;

        // Tags / events
        private const string TAG_PLAYER = "Player";
        private const string TAG_SEMICONDUCTOR = "SEMICONDUCTOR";
        private const string TAG_EMPLACEMENT = "EMPLACEMENT";
        private const string TAG_CORE_BARRIER = "CORE_BARRIER";
        private const string TAG_ALLIES = "ALLIES";
        private const string EVENT_BULLET_HIT = "BulletHit";
        private const string EVENT_SPAWN_DISABLE = "DisablingSpawn";

        // ===== Lifecycle =====

        public override void OnStart()
        {
            LogMessage("=== Botnet started (EntityID = " + EntityID + ") ===");

            // Seed RNG
            s_RngState ^= (uint)EntityID * 747796405u + 2891336453u;

            // Setup rigidbody (wrapped)
            EntityAddRigidBody((uint)EntityID);
            RigidbodySetIsKinematic((uint)EntityID, false);
            RigidbodySetUseGravity((uint)EntityID, false);
            RigidbodySetMass((uint)EntityID, 1.0f);

            // Reset runtime state
            isDead = false;
            isExploding = false;
            isMoving = false;
            targetID = INVALID_ENTITY;

            isStunned = false;
            stunTimer = 0.0f;

            hasChosenInitialTarget = false;
            chooseTargetTimer = RandomRangeFloat(minInitialTargetDelay, maxInitialTargetDelay);

            PhysicsEnableCollisionEvents();

            Subscribe(EVENT_BULLET_HIT, OnBulletHit);
            Subscribe(EVENT_SPAWN_DISABLE, OnSpawnDisable);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead)
                return;

            // Update stun
            if (isStunned)
            {
                UpdateStun(deltaTime);
                return;
            }

            // Target selection
            UpdateTargetSelectionTimer(deltaTime);

            // Movement and rotation
            if (isMoving && targetID != INVALID_ENTITY)
            {
                if (enableLookAt)
                    RotateTowardsTarget(deltaTime);

                MoveTowardsTarget(deltaTime);
                ClampSpeed();
            }

            // Handle collision-triggered explosion
            HandleCollisionTriggers();

            if (isExploding)
                Explode();
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_BULLET_HIT, OnBulletHit);
            Unsubscribe(EVENT_SPAWN_DISABLE, OnSpawnDisable);
        }

        // ===== Public API =====

        public void Stunned()
        {
            LogMessage("Botnet (EntityID = " + EntityID + ") stunned");
            isStunned = true;
            stunTimer = stunnedTime;
            RigidbodyStop((uint)EntityID);
        }

        // ===== Event Handlers =====

        private void OnBulletHit(string eventName, string payload)
        {
            if (isDead || eventName != EVENT_BULLET_HIT)
                return;

            if (!uint.TryParse(payload, out uint hitId))
                return;

            if (hitId != (uint)EntityID)
                return;

            Publish("BotnetDeath", 1.ToString());
            Explode();
        }

        private void OnSpawnDisable(string eventName, string payload)
        {
            if (isDead || eventName != EVENT_SPAWN_DISABLE)
                return;

            if (!bool.TryParse(payload, out bool active))
                return;

            if (!active)
            {
                isDead = true;
                isExploding = false;

                LogMessage("Destroying itself as spawn is disabled");
                SceneDestroyEntity((uint)EntityID);
            }
        }

        public void BruteForceAttack()
        {
            LogMessage("Botnet (EntityID = " + EntityID + ") entering brute force attack mode");

            topSpeed = bruteForceAttackSpeed;

            uint semi = FindFirstEntityWithTag(TAG_SEMICONDUCTOR);
            if (semi != INVALID_ENTITY)
            {
                targetID = semi;
                isMoving = true;
            }
        }

        // ===== Movement & Targeting =====

        private void UpdateStun(float deltaTime)
        {
            if (!isStunned)
                return;

            stunTimer -= deltaTime;
            if (stunTimer <= 0.0f)
            {
                isStunned = false;
                stunTimer = 0.0f;
                LogMessage("Botnet (EntityID = " + EntityID + ") stun ended");
            }
        }

        private void UpdateTargetSelectionTimer(float deltaTime)
        {
            if (targetID != INVALID_ENTITY)
            {
                EnsureTargetStillValid();

                if (targetID != INVALID_ENTITY)
                    return;

                chooseTargetTimer = 0.0f;
            }

            chooseTargetTimer -= deltaTime;
            if (chooseTargetTimer > 0.0f)
                return;

            ChooseTarget();

            if (!hasChosenInitialTarget)
                hasChosenInitialTarget = true;

            if (targetID != INVALID_ENTITY)
            {
                isMoving = true;
                chooseTargetTimer = RandomRangeFloat(minRetargetDelay, maxRetargetDelay);
            }
            else
            {
                chooseTargetTimer = RandomRangeFloat(0.5f, 1.0f);
            }
        }

        private void ChooseTarget()
        {
            if (targetID != INVALID_ENTITY)
                return;

            int choice = RandomRangeInt(0, 4);

            // currently forced to Player (as per your commented switch)
            uint chosen = FindFirstEntityWithTag(TAG_PLAYER);

            if (chosen != INVALID_ENTITY)
            {
                targetID = chosen;
                isMoving = true;
                LogMessage("Botnet (EntityID = " + EntityID + ") chose target " + targetID + " (choice " + choice + ")");
            }
            else
            {
                targetID = INVALID_ENTITY;
                isMoving = false;
            }
        }

        private void EnsureTargetStillValid()
        {
            if (targetID == INVALID_ENTITY)
                return;

            string tag = TagGetTag(targetID);
            if (string.IsNullOrEmpty(tag))
            {
                LogMessage("Botnet (EntityID = " + EntityID + ") target " + targetID + " destroyed");
                targetID = INVALID_ENTITY;
                isMoving = false;
                chooseTargetTimer = 0.0f;
            }
        }

        private void RotateTowardsTarget(float deltaTime)
        {
            if (targetID == INVALID_ENTITY)
                return;

            uint self = (uint)EntityID;

            // Positions
            Vector3 myPos = Transform.GetPosition(self);
            Vector3 targetPos = Transform.GetPosition(targetID);

            // Direction to target
            Vector3 toTarget = new Vector3(
                targetPos.X - myPos.X,
                targetPos.Y - myPos.Y,
                targetPos.Z - myPos.Z
            );

            // Normalize direction
            float lenSq = toTarget.X * toTarget.X +
                          toTarget.Y * toTarget.Y +
                          toTarget.Z * toTarget.Z;
            if (lenSq <= 0.0001f)
                return;

            float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
            toTarget.X *= invLen;
            toTarget.Y *= invLen;
            toTarget.Z *= invLen;

            // Engine convention: forward is +Z for identity rotation
            Vector3 forward = new Vector3(0.0f, 0.0f, 1.0f);

            // Current orientation (quat)
            Quat currentRot = Transform.GetRotation(self);

            // Desired orientation: rotate +Z to face toTarget (world-space)
            Quat targetRot = QuaternionFromTo(forward, toTarget);

            // Interpolation factor based on rotateSpeed
            float t = SimpleMath.Clamp(rotateSpeed * deltaTime, 0.0f, 1.0f);

            // Smoothly rotate towards target using nlerp (shortest path)
            Quat newRot = Nlerp(currentRot, targetRot, t);

            Transform.SetRotation(self, ref newRot);
        }

        private void MoveTowardsTarget(float deltaTime)
        {
            if (targetID == INVALID_ENTITY)
                return;

            Vector3 myPos = Transform.GetPosition((uint)EntityID);
            Vector3 targetPos = Transform.GetPosition(targetID);

            Vector3 dir = new Vector3(
                targetPos.X - myPos.X,
                targetPos.Y - myPos.Y,
                targetPos.Z - myPos.Z
            );

            float distSq = dir.X * dir.X + dir.Y * dir.Y + dir.Z * dir.Z;
            if (distSq <= 0.0001f)
                return;

            float invLen = 1.0f / SimpleMath.Sqrt(distSq);
            dir.X *= invLen;
            dir.Y *= invLen;
            dir.Z *= invLen;

            Vector3 force = new Vector3(
                dir.X * acceleration,
                dir.Y * acceleration,
                dir.Z * acceleration
            );

            RigidbodyAddForce((uint)EntityID, ref force);
        }

        private void ClampSpeed()
        {
            float speed = RigidbodyGetSpeed((uint)EntityID);
            if (speed <= topSpeed || topSpeed <= 0.0f)
                return;

            Vector3 vel = RigidbodyGetVelocity((uint)EntityID);

            float lenSq = vel.X * vel.X + vel.Y * vel.Y + vel.Z * vel.Z;
            if (lenSq <= 0.0001f)
                return;

            float currentSpeed = SimpleMath.Sqrt(lenSq);
            if (currentSpeed <= 0.0001f)
                return;

            float scale = topSpeed / currentSpeed;

            vel.X *= scale;
            vel.Y *= scale;
            vel.Z *= scale;

            RigidbodySetVelocity((uint)EntityID, ref vel);
        }

        // ===== Collision & Explosion =====

        private void HandleCollisionTriggers()
        {
            int count = PhysicsGetCollisionCount();
            if (count <= 0)
                return;

            // Only explode if we have a target
            if (targetID == INVALID_ENTITY)
                return;

            uint self = (uint)EntityID;
            uint playerID = SceneFindEntityByName("Player");

            for (int i = 0; i < count; ++i)
            {
                uint a, b;
                PhysicsGetCollisionPair(i, out a, out b);

                if (a != self && b != self)
                    continue;

                uint other = (a == self) ? b : a;

                if (other == playerID)
                {
                    LogMessage("Botnet (EntityID = " + EntityID + ") ATTACKED the Player!");
                    Publish("BotnetAttackedPlayer", EntityID.ToString());
                }

                if (other == targetID)
                {
                    LogMessage("Botnet (EntityID = " + EntityID + ") collided with target " + targetID);
                    isExploding = true;
                    break;
                }
            }
        }

        private void Explode()
        {
            if (isDead)
                return;

            isDead = true;
            isExploding = false;

            LogMessage("Botnet (EntityID = " + EntityID + ") exploding!");

            ApplyBlastToTag(TAG_PLAYER);
            ApplyBlastToTag(TAG_SEMICONDUCTOR);
            ApplyBlastToTag(TAG_EMPLACEMENT);
            ApplyBlastToTag(TAG_ALLIES);

            if (!string.IsNullOrEmpty(deathExplosionPrefab))
            {
                uint explosionID = PrefabInstantiate(deathExplosionPrefab);
                Vector3 myPos = Transform.GetPosition((uint)EntityID);
                Transform.SetPosition(explosionID, ref myPos);
                AudioPlay(explosionID);
            }

            SceneDestroyEntity((uint)EntityID);
        }

        private void ApplyBlastToTag(string tag)
        {
            uint[] entities = SceneFindEntitiesByTag(tag);
            if (entities == null || entities.Length == 0)
                return;

            Vector3 myPos = Transform.GetPosition((uint)EntityID);
            float radiusSq = blastRadius * blastRadius;

            for (int i = 0; i < entities.Length; ++i)
            {
                uint id = entities[i];
                if (id == (uint)EntityID)
                    continue;

                Vector3 targetPos = Transform.GetPosition(id);

                float dx = targetPos.X - myPos.X;
                float dy = targetPos.Y - myPos.Y;
                float dz = targetPos.Z - myPos.Z;

                float distSq = dx * dx + dy * dy + dz * dz;

                if (distSq <= radiusSq)
                {
                    // DamageSystem.DealDamage(id, blastDamage, (uint)EntityID);
                }
            }
        }

        private static Quat QuaternionFromTo(Vector3 from, Vector3 to)
        {
            float dot = from.X * to.X + from.Y * to.Y + from.Z * to.Z;

            if (dot < -0.9999f)
            {
                Quat q180;
                q180.X = 0.0f;
                q180.Y = 1.0f;
                q180.Z = 0.0f;
                q180.W = 0.0f;
                return q180;
            }

            // General case: use cross product
            Vector3 cross = new Vector3(
                from.Y * to.Z - from.Z * to.Y,
                from.Z * to.X - from.X * to.Z,
                from.X * to.Y - from.Y * to.X
            );

            float s = SimpleMath.Sqrt((1.0f + dot) * 2.0f);
            float invS = 1.0f / s;

            Quat q;
            q.X = cross.X * invS;
            q.Y = cross.Y * invS;
            q.Z = cross.Z * invS;
            q.W = 0.5f * s;

            return q;
        }

        private static Quat Nlerp(Quat a, Quat b, float t)
        {
            float dot = a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;
            if (dot < 0.0f)
            {
                b.X = -b.X;
                b.Y = -b.Y;
                b.Z = -b.Z;
                b.W = -b.W;
            }

            float invT = 1.0f - t;

            Quat result;
            result.X = a.X * invT + b.X * t;
            result.Y = a.Y * invT + b.Y * t;
            result.Z = a.Z * invT + b.Z * t;
            result.W = a.W * invT + b.W * t;

            float lenSq = result.X * result.X +
                          result.Y * result.Y +
                          result.Z * result.Z +
                          result.W * result.W;

            if (lenSq > 0.000001f)
            {
                float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
                result.X *= invLen;
                result.Y *= invLen;
                result.Z *= invLen;
                result.W *= invLen;
            }
            else
            {
                result = a;
            }

            return result;
        }

        private static uint Nextuint()
        {
            uint x = s_RngState;
            if (x == 0)
                x = 0x12345678u;

            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 5;
            s_RngState = x;
            return x;
        }

        private static int RandomRangeInt(int minInclusive, int maxExclusive)
        {
            if (maxExclusive <= minInclusive)
                return minInclusive;

            uint range = (uint)(maxExclusive - minInclusive);
            uint r = Nextuint();
            return minInclusive + (int)(r % range);
        }

        private static float RandomRangeFloat(float minInclusive, float maxInclusive)
        {
            if (maxInclusive <= minInclusive)
                return minInclusive;

            uint r = Nextuint() & 0x00FFFFFFu;
            float t = r / 16777215.0f;
            return minInclusive + (maxInclusive - minInclusive) * t;
        }

        // ===== Search Helpers =====

        private static uint FindFirstEntityWithTag(string tag)
        {
            uint[] entities = SceneFindEntitiesByTag(tag);
            if (entities == null || entities.Length == 0)
                return INVALID_ENTITY;

            return entities[0];
        }

        private static uint FindRandomEntityWithTag(string tag)
        {
            uint[] entities = SceneFindEntitiesByTag(tag);
            if (entities == null || entities.Length == 0)
                return INVALID_ENTITY;

            if (entities.Length == 1)
                return entities[0];

            int idx = RandomRangeInt(0, entities.Length);
            return entities[idx];
        }
    }
}
