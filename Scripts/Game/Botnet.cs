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

        // Movement
        [SerializeField]
        private float acceleration = 50.0f;

        [SerializeField]
        private float topSpeed = 30.0f;

        // In Unity this controls how fast the bot turns towards the target.
        // Rotation/orientation is not yet wired in this engine-side port.
        [SerializeField]
        private float rotateSpeed = 5.0f;

        // Explosion properties
        [SerializeField]
        private float blastRadius = 5.0f;

        [SerializeField]
        private int blastDamage = 10;

        // In Unity this was used for AddExplosionForce, which isn't exposed here yet.
        // Kept for completeness but not used.
        [SerializeField]
        private float blastImpulse = 0.0f;

        // Status effects
        [SerializeField]
        private bool isStunned = false;

        [SerializeField]
        private float stunnedTime = 1.0f;

        // Brute-force attack (rage mode)
        [SerializeField]
        private float bruteForceAttackSpeed = 300.0f;

        // Target selection timing (Unity used Invoke + Random.Range)
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

        // Health component – damage system will drive this down to 0
        [SerializeField("Health Component")]
        private Health health;

        // ===== Private Runtime State =====

        // Sentinel for "no entity" – 0 is a VALID ECS id in your engine
        private const uint INVALID_ENTITY = 0xffffffffu;

        // Target this bot is currently chasing.
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

        // Tags used by this enemy (match tags configured on entities)
        private const string TAG_PLAYER = "Player";
        private const string TAG_SEMICONDUCTOR = "SEMICONDUCTOR";
        private const string TAG_EMPLACEMENT = "EMPLACEMENT";
        private const string TAG_CORE_BARRIER = "CORE_BARRIER";
        private const string TAG_ALLIES = "ALLIES";

        // ===== Lifecycle =====

        public override void OnStart()
        {
            Log("=== Botnet started (EntityID = " + EntityID + ") ===");

            // Seed RNG with entity ID so different instances behave differently
            s_RngState ^= (uint)EntityID * 747796405u + 2891336453u;

            // Ensure rigidbody exists and is configured
            InternalCalls.Entity_AddRigidBody((uint)EntityID);
            InternalCalls.Rigidbody_SetIsKinematic((uint)EntityID, false);
            InternalCalls.Rigidbody_SetUseGravity((uint)EntityID, false); // Botnets fly / ignore gravity
            InternalCalls.Rigidbody_SetMass((uint)EntityID, 1.0f);

            // Reset runtime state
            isDead = false;
            isExploding = false;
            isMoving = false;
            targetID = INVALID_ENTITY;

            isStunned = false;
            stunTimer = 0.0f;

            hasChosenInitialTarget = false;
            chooseTargetTimer = RandomRangeFloat(minInitialTargetDelay, maxInitialTargetDelay);

            // Enable collision events globally (no-op if already enabled)
            InternalCalls.Physics_EnableCollisionEvents();
        }

        public override void OnUpdate(float deltaTime)
        {
            // Death driven by Health + damage system
            if (!isDead && health != null && health.IsDead)
            {
                Explode();
                return;
            }

            if (isDead)
                return;

            // Update stun status. While stunned we do not move or target.
            if (isStunned)
            {
                UpdateStun(deltaTime);
                return;
            }

            // Target selection / lock-on logic
            UpdateTargetSelectionTimer(deltaTime);

            // If we have a valid target, update movement
            if (isMoving && targetID != INVALID_ENTITY)
            {
                MoveTowardsTarget(deltaTime);
                ClampSpeed();
            }

            // Handle collision-triggered explosion (kamikaze behaviour)
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
            Log("Botnet (EntityID = " + EntityID + ") stunned");
            isStunned = true;
            stunTimer = stunnedTime;

            // Stop movement immediately
            InternalCalls.Rigidbody_Stop((uint)EntityID);
        }

        /// <summary>
        /// Brute force / rage mode: dramatically increase top speed and
        /// force target to SEMICONDUCTOR. (IBruteForceAttack in Unity)
        /// </summary>
        public void BruteForceAttack()
        {
            Log("Botnet (EntityID = " + EntityID + ") entering brute force attack mode");

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
                Log("Botnet (EntityID = " + EntityID + ") stun ended");
            }
        }

        /// <summary>
        /// Target selection logic.
        /// - If we already have a valid target, stay locked and only check if it still exists.
        /// - If we lose our target or never had one, use the timer to pick a new one.
        /// </summary>
        private void UpdateTargetSelectionTimer(float deltaTime)
        {
            // If we already have a target, just verify that it still exists.
            if (targetID != INVALID_ENTITY)
            {
                EnsureTargetStillValid();

                // Still valid? Stay locked on, do NOT re-randomise.
                if (targetID != INVALID_ENTITY)
                    return;

                // Target vanished -> schedule immediate retarget.
                chooseTargetTimer = 0.0f;
            }

            // At this point we have no target; run the selection timer.
            chooseTargetTimer -= deltaTime;
            if (chooseTargetTimer > 0.0f)
                return;

            ChooseTarget();

            if (!hasChosenInitialTarget)
                hasChosenInitialTarget = true;

            if (targetID != INVALID_ENTITY)
            {
                // Successfully locked onto something.
                isMoving = true;
                chooseTargetTimer = RandomRangeFloat(minRetargetDelay, maxRetargetDelay);
            }
            else
            {
                // Still nothing to chase – retry after a short delay.
                chooseTargetTimer = RandomRangeFloat(0.5f, 1.0f);
            }
        }

        private void ChooseTarget()
        {
            // If for some reason we still have a target, don't change it here.
            if (targetID != INVALID_ENTITY)
                return;

            // 0: Player, 1: SEMICONDUCTOR, 2: EMPLACEMENT, 3: random ALLIES
            int choice = RandomRangeInt(0, 4);

            uint chosen = INVALID_ENTITY;

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

            if (chosen != INVALID_ENTITY)
            {
                targetID = chosen;
                isMoving = true;
                Log("Botnet (EntityID = " + EntityID + ") chose target " + targetID + " (choice " + choice + ")");
            }
            else
            {
                targetID = INVALID_ENTITY;
                isMoving = false;
                Log("Botnet (EntityID = " + EntityID + ") failed to find target for choice " + choice + ")");
            }
        }

        /// <summary>
        /// Checks if current target is still valid. If destroyed / removed,
        /// schedule a new ChooseTarget().
        /// </summary>
        private void EnsureTargetStillValid()
        {
            if (targetID == INVALID_ENTITY)
                return;

            // Heuristic: if tag lookup returns null/empty, assume entity is gone.
            string tag = InternalCalls.Tag_GetTag(targetID);
            if (string.IsNullOrEmpty(tag))
            {
                Log("Botnet (EntityID = " + EntityID + ") target " + targetID + " appears to be destroyed, scheduling new target");
                targetID = INVALID_ENTITY;
                isMoving = false;
                chooseTargetTimer = 0.0f; // pick a new target on the next update
            }
        }

        /// <summary>
        /// Step 1: Get my ID -> EntityID
        /// Step 2: Get targetID -> targetID
        /// Step 3: Move towards target using rigid body (AddForce)
        /// </summary>
        private void MoveTowardsTarget(float deltaTime)
        {
            if (targetID == INVALID_ENTITY)
                return;

            // Self position via Transform static helper
            Vector3 myPos = Transform.GetPosition((uint)EntityID);

            // Target position via Transform static helper
            Vector3 targetPos = Transform.GetPosition((uint)targetID);

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

            // Apply acceleration along direction via rigidbody
            Vector3 force = new Vector3(
                dir.X * acceleration,
                dir.Y * acceleration,
                dir.Z * acceleration
            );

            InternalCalls.Rigidbody_AddForce((uint)EntityID, ref force);

            // Optional rotation: if you want to face the target you can
            // compute a yaw/pitch and use Transform.SetRotation here.
        }

        private void ClampSpeed()
        {
            float speed = InternalCalls.Rigidbody_GetSpeed((uint)EntityID);
            if (speed <= topSpeed || topSpeed <= 0.0f)
                return;

            // Pull velocity back to topSpeed
            Vector3 vel;
            InternalCalls.Rigidbody_GetVelocity((uint)EntityID, out vel);

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

            InternalCalls.Rigidbody_SetVelocity((uint)EntityID, ref vel);
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

                // We only care about collisions involving this bot
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

            Log("Botnet (EntityID = " + EntityID + ") exploding!");

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
                Vector3 myPos = Transform.GetPosition((uint)EntityID);
                Transform.SetPosition((uint)explosionID, ref myPos);
            }

            // Destroy this entity (Unity: EnemyHealth.TakeDamage(9999) + Destroy(gameObject))
            InternalCalls.Scene_DestroyEntity((uint)EntityID);
        }

        private void ApplyBlastToTag(string tag)
        {
            uint[] entities = InternalCalls.Scene_FindEntitiesByTag(tag);
            if (entities == null || entities.Length == 0)
                return;

            Vector3 myPos = Transform.GetPosition((uint)EntityID);
            float radiusSq = blastRadius * blastRadius;

            for (int i = 0; i < entities.Length; ++i)
            {
                uint id = entities[i];
                if (id == (uint)EntityID)
                    continue;

                Vector3 targetPos = Transform.GetPosition((uint)id);

                float dx = targetPos.X - myPos.X;
                float dy = targetPos.Y - myPos.Y;
                float dz = targetPos.Z - myPos.Z;

                float distSq = dx * dx + dy * dy + dz * dz;

                if (distSq <= radiusSq)
                {
                    // Apply damage through the event-driven damage system.
                    // Any entity that has a DamageReceiver + Health attached
                    // will consume this event and reduce its health.
                    DamageSystem.DealDamage(id, blastDamage, (uint)EntityID);
                }
            }
        }

        // ===== RNG Helpers (xorshift32) =====

        private static uint NextUInt()
        {
            uint x = s_RngState;
            if (x == 0)
                x = 0x12345678u; // avoid zero lock

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
            uint r = NextUInt();
            return minInclusive + (int)(r % range);
        }

        private static float RandomRangeFloat(float minInclusive, float maxInclusive)
        {
            if (maxInclusive <= minInclusive)
                return minInclusive;

            uint r = NextUInt() & 0x00FFFFFFu; // 24 bits
            float t = r / 16777215.0f;        // [0,1)
            return minInclusive + (maxInclusive - minInclusive) * t;
        }

        // ===== Math & Search Helpers =====

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
                return INVALID_ENTITY;

            return entities[0];
        }

        private static uint FindRandomEntityWithTag(string tag)
        {
            uint[] entities = InternalCalls.Scene_FindEntitiesByTag(tag);
            if (entities == null || entities.Length == 0)
                return INVALID_ENTITY;

            if (entities.Length == 1)
                return entities[0];

            int idx = RandomRangeInt(0, entities.Length);
            return entities[idx];
        }
    }
}
