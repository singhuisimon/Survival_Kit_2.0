// Botnet.cs
using Engine;
using System;

namespace Game
{
    /// <summary>
    /// Port of EnemyE004_BOTNET from Unity to custom engine script.
    /// Uses rigidbody physics via InternalCalls and simple tag-based targeting.
    /// </summary>
    public class Botnet : ScriptBehaviour
    {
        // ===== Serialized Fields (Editable in Inspector) =====

        [SerializeField]
        private float acceleration = 50.0f;

        [SerializeField]
        private float topSpeed = 30.0f;

        //[SerializeField]
        //private float rotateSpeed = 5.0f; // Currently unused - orientation not yet implemented

        // Explosion properties
        [SerializeField]
        private float blastRadius = 5.0f;

        [SerializeField]
        private int blastDamage = 10;

        //[SerializeField]
        //private float blastImpulse = 0.0f; // Not used yet (no explicit AddExplosionForce API)

        // Status effect
        [SerializeField]
        private bool isStunned = false;

        [SerializeField]
        private float stunnedTime = 1.0f;

        // Brute-force attack (rage mode)
        [SerializeField]
        private float bruteForceAttackSpeed = 300.0f;

        // Target selection timing
        [SerializeField]
        private float minInitialTargetDelay = 0.5f;

        [SerializeField]
        private float maxInitialTargetDelay = 0.8f;

        [SerializeField]
        private float minRetargetDelay = 1.0f;

        [SerializeField]
        private float maxRetargetDelay = 1.5f;

        // Death explosion prefab path (engine prefab, NOT a Unity GameObject)
        [SerializeField]
        private string deathExplosionPrefab = string.Empty;

        // ===== Private Runtime State =====

        private uint targetID = 0;
        private bool isMoving = false;
        private bool isDead = false;
        private bool isExploding = false;

        //private float boostTimerCooldown = 0.0f; // Reserved for future "boost" feature
        //private float fuseTimerCooldown = 0.0f;  // Reserved for fuse/explode delay

        private float stunTimer = 0.0f;
        private float chooseTargetTimer = 0.0f;
        private bool hasChosenInitialTarget = false;

        // Random state (simple LCG to avoid System.Random / System.Math)
        private static uint s_RngState = 0x12345678u;

        // Tags used by this enemy (match tags configured on entities)
        private const string TAG_PLAYER = "Player";
        private const string TAG_SEMICONDUCTOR = "SEMICONDUCTOR";
        private const string TAG_EMPLACEMENT = "EMPLACEMENT";
        private const string TAG_CORE_BARRIER = "CORE_BARRIER";
        private const string TAG_ALLIES = "ALLIES";

        // ===== Lifecycle =====

        public override void OnStart()
        {
            Log("=== Botnet enemy started ===");

            // Seed RNG with entity ID so different instances behave differently
            s_RngState ^= (uint)EntityID * 747796405u + 2891336453u;

            // Ensure rigidbody exists and is configured
            InternalCalls.Entity_AddRigidBody((ulong)EntityID);
            InternalCalls.Rigidbody_SetIsKinematic((ulong)EntityID, false);
            InternalCalls.Rigidbody_SetUseGravity((ulong)EntityID, false); // These enemies fly / ignore gravity
            InternalCalls.Rigidbody_SetMass((ulong)EntityID, 1.0f);

            isDead = false;
            isExploding = false;
            isMoving = false;
            targetID = 0;

            isStunned = false;
            stunTimer = 0.0f;

            //boostTimerCooldown = 0.0f;
            //fuseTimerCooldown = 0.0f;

            hasChosenInitialTarget = false;
            chooseTargetTimer = RandomRangeFloat(minInitialTargetDelay, maxInitialTargetDelay);

            // Enable collision events globally (no-op if already enabled)
            InternalCalls.Physics_EnableCollisionEvents();
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead)
                return;

            // Update stun status
            if (isStunned)
            {
                UpdateStun(deltaTime);
                // While stunned we don't move or process targeting
                return;
            }

            // Initial target selection / retargeting timer
            UpdateTargetSelectionTimer(deltaTime);

            // If we have a valid target, update movement
            if (isMoving && targetID != 0)
            {
                MoveTowardsTarget(deltaTime);
                ClampSpeed();
            }

            // Handle collision-triggered explosion
            HandleCollisionTriggers();

            if (isExploding)
            {
                Explode();
            }
        }

        // ===== Public API (called from other scripts / game logic) =====

        /// <summary>
        /// Apply a stun effect to this enemy for stunnedTime seconds.
        /// Movement stops while stunned.
        /// </summary>
        public void Stunned()
        {
            Log("Botnet stunned");
            isStunned = true;
            stunTimer = stunnedTime;

            // Stop movement immediately
            InternalCalls.Rigidbody_Stop((ulong)EntityID);
        }

        /// <summary>
        /// Brute force / rage mode: dramatically increase top speed and
        /// force target to SEMICONDUCTOR.
        /// </summary>
        public void BruteForceAttack()
        {
            Log("Botnet entering brute force attack mode");

            topSpeed = bruteForceAttackSpeed;

            uint semi = FindFirstEntityWithTag(TAG_SEMICONDUCTOR);
            if (semi != 0)
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
                Log("Botnet stun ended");
            }
        }

        private void UpdateTargetSelectionTimer(float deltaTime)
        {
            chooseTargetTimer -= deltaTime;
            if (chooseTargetTimer <= 0.0f)
            {
                ChooseTarget();

                if (!hasChosenInitialTarget)
                {
                    hasChosenInitialTarget = true;
                }

                // Schedule next retarget if current target becomes invalid later
                chooseTargetTimer = RandomRangeFloat(minRetargetDelay, maxRetargetDelay);
            }

            // If current target died / was removed, attempt to get a new one on next tick
            if (targetID == 0 && hasChosenInitialTarget)
            {
                chooseTargetTimer = 0.0f;
            }
        }

        private void ChooseTarget()
        {
            // 0: PLAYER
            // 1: SEMICONDUCTOR
            // 2: EMPLACEMENT
            // 3: random ALLIES
            int choice = RandomRangeInt(0, 4);

            uint chosen = 0;

            switch (choice)
            {
                case 0:
                    chosen = FindFirstEntityWithTag(TAG_PLAYER);
                    break;

                case 1:
                    chosen = FindFirstEntityWithTag(TAG_SEMICONDUCTOR);
                    break;

                case 2:
                    chosen = FindFirstEntityWithTag(TAG_EMPLACEMENT);
                    break;

                case 3:
                    chosen = FindRandomEntityWithTag(TAG_ALLIES);
                    break;
            }

            if (chosen != 0)
            {
                targetID = chosen;
                isMoving = true;
                Log("Botnet chose target " + targetID + " (choice " + choice + ")");
            }
            else
            {
                // Couldn't find a target of this type - try again later
                targetID = 0;
                isMoving = false;
                Log("Botnet failed to find target for choice " + choice + ")");
            }
        }

        private void MoveTowardsTarget(float deltaTime)
        {
            if (targetID == 0)
                return;

            // Get positions
            Vector3 myPos = Transform.Position;

            Vector3 targetPos;
            InternalCalls.Transform_GetPosition(targetID, out targetPos);

            // Direction to target
            Vector3 dir = new Vector3(
                targetPos.X - myPos.X,
                targetPos.Y - myPos.Y,
                targetPos.Z - myPos.Z
            );

            float distSq = dir.X * dir.X + dir.Y * dir.Y + dir.Z * dir.Z;
            if (distSq <= 0.0001f)
                return;

            // Normalize direction
            float invLen = 1.0f / SimpleSqrt(distSq);
            dir.X *= invLen;
            dir.Y *= invLen;
            dir.Z *= invLen;

            // Apply acceleration along direction
            Vector3 force = new Vector3(
                dir.X * acceleration,
                dir.Y * acceleration,
                dir.Z * acceleration
            );

            InternalCalls.Rigidbody_AddForce((ulong)EntityID, ref force);
        }

        private void ClampSpeed()
        {
            float speed = InternalCalls.Rigidbody_GetSpeed((ulong)EntityID);
            if (speed <= topSpeed || topSpeed <= 0.0f)
                return;

            // Pull velocity back to topSpeed
            Vector3 vel;
            InternalCalls.Rigidbody_GetVelocity((ulong)EntityID, out vel);

            float lenSq = vel.X * vel.X + vel.Y * vel.Y + vel.Z * vel.Z;
            if (lenSq <= 0.0001f)
                return;

            float currentSpeed = SimpleSqrt(lenSq);
            if (currentSpeed <= 0.0001f)
                return;

            float scale = topSpeed / currentSpeed;

            vel.X *= scale;
            vel.Y *= scale;
            vel.Z *= scale;

            InternalCalls.Rigidbody_SetVelocity((ulong)EntityID, ref vel);
        }

        // ===== Collision & Explosion =====

        private void HandleCollisionTriggers()
        {
            int count = InternalCalls.Physics_GetCollisionCount();
            if (count <= 0)
                return;

            uint self = (uint)EntityID;

            for (int i = 0; i < count; ++i)
            {
                uint a, b;
                InternalCalls.Physics_GetCollisionPair(i, out a, out b);

                if (a != self && b != self)
                    continue;

                uint other = (a == self) ? b : a;

                string tag = InternalCalls.Tag_GetTag(other);
                if (tag == TAG_PLAYER ||
                    tag == TAG_SEMICONDUCTOR ||
                    tag == TAG_EMPLACEMENT ||
                    tag == TAG_CORE_BARRIER ||
                    tag == TAG_ALLIES)
                {
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

            Log("Botnet exploding!");

            // Approximate Unity's Physics.OverlapSphere by checking all
            // entities of relevant tags and applying damage if within radius.
            ApplyBlastToTag(TAG_PLAYER);
            ApplyBlastToTag(TAG_SEMICONDUCTOR);
            ApplyBlastToTag(TAG_EMPLACEMENT);
            ApplyBlastToTag(TAG_ALLIES);

            // Spawn death explosion prefab if provided
            if (!string.IsNullOrEmpty(deathExplosionPrefab))
            {
                uint explosionID = InternalCalls.Prefab_Instantiate(deathExplosionPrefab);

                // Move spawned prefab to this enemy's position
                Vector3 myPos = Transform.Position;
                InternalCalls.Transform_SetPosition(explosionID, ref myPos);
            }

            // Destroy this entity
            InternalCalls.Scene_DestroyEntity((uint)EntityID);
        }

        private void ApplyBlastToTag(string tag)
        {
            uint[] entities = InternalCalls.Scene_FindEntitiesByTag(tag);
            if (entities == null || entities.Length == 0)
                return;

            Vector3 myPos = Transform.Position;

            for (int i = 0; i < entities.Length; ++i)
            {
                uint id = entities[i];
                if (id == (uint)EntityID)
                    continue;

                Vector3 targetPos;
                InternalCalls.Transform_GetPosition(id, out targetPos);

                float dx = targetPos.X - myPos.X;
                float dy = targetPos.Y - myPos.Y;
                float dz = targetPos.Z - myPos.Z;

                float distSq = dx * dx + dy * dy + dz * dz;
                float radiusSq = blastRadius * blastRadius;

                if (distSq <= radiusSq)
                {
                    // Here is where you'd apply damage to the entity.
                    // The engine currently doesn't expose a generic "damage" API
                    // from C#, so we simply log for now.
                    Log($"Blast would hit entity {id} (tag {tag}) for {blastDamage} damage");
                }
            }
        }

        // ===== Helpers =====

        private static int RandomRangeInt(int minInclusive, int maxExclusive)
        {
            if (maxExclusive <= minInclusive)
                return minInclusive;

            s_RngState = 1664525u * s_RngState + 1013904223u;
            uint range = (uint)(maxExclusive - minInclusive);
            return minInclusive + (int)(s_RngState % range);
        }

        private static float RandomRangeFloat(float minInclusive, float maxInclusive)
        {
            if (maxInclusive <= minInclusive)
                return minInclusive;

            s_RngState = 1664525u * s_RngState + 1013904223u;
            // Take upper 24 bits to build a [0,1) float
            uint r = (s_RngState >> 8) & 0x00FFFFFFu;
            float t = r / 16777215.0f; // 2^24 - 1

            return minInclusive + (maxInclusive - minInclusive) * t;
        }

        private static float SimpleSqrt(float value)
        {
            if (value <= 0.0f)
                return 0.0f;
            if (value == 1.0f)
                return 1.0f;

            // Newton-Raphson method
            float x = value;
            float y = 1.0f;
            const float epsilon = 0.0001f;

            for (int i = 0; i < 4; ++i)
            {
                y = 0.5f * (x + value / x);
                float diff = x - y;
                if (diff < epsilon && diff > -epsilon)
                    break;
                x = y;
            }

            return y;
        }

        private static uint FindFirstEntityWithTag(string tag)
        {
            uint[] entities = InternalCalls.Scene_FindEntitiesByTag(tag);
            if (entities == null || entities.Length == 0)
                return 0;

            return entities[0];
        }

        private static uint FindRandomEntityWithTag(string tag)
        {
            uint[] entities = InternalCalls.Scene_FindEntitiesByTag(tag);
            if (entities == null || entities.Length == 0)
                return 0;

            if (entities.Length == 1)
                return entities[0];

            int idx = RandomRangeInt(0, entities.Length);
            return entities[idx];
        }
    }
}
