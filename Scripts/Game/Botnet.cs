// Botnet.cs
using Engine;
using System;
using System.Collections.Generic;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Prefab;
using static Engine.Physics;
using static Engine.Rigidbody;
using static Engine.Audio;
using static Engine.Tag;
using static Engine.Transform;

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
        [SerializeField] private float acceleration = 200.0f;
        [SerializeField] private float topSpeed = 300.0f;

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
        //[SerializeField] 
        private string deathExplosionPrefab = "Sources/Prefabs/BotnetExplosion.prefab";
        private string hitmarkerAudioPrefab = "Sources/Prefabs/audio_hitmarker.prefab";

        // Botnet Health
        [SerializeField] private float HP = 3.0f;

        // Botnet Damage
        [SerializeField] private float blastDamage = 5.0f;

        //DEBUG
        [SerializeField] private string TARGET = "";

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
        //private const string TAG_ALLIES = "ALLIES";
        private const string TAG_ALLIES = "Gunship";
        private const string TAG_PRIMARY_BULLET = "PrimaryBullet";
        private const string TAG_SECONDARY_BULLET = "PrimaryUltBullet";
        //private const string EVENT_BULLET_HIT = "BulletHit";
        private string EVENT_BULLET_HIT = "Damage:";
        private const string EVENT_SPAWN_DISABLE = "DisablingSpawn";
        private const string EVENT_GAME_OVER = "GameOver";
        private const string EVENT_GAME_WIN = "GameWin";

        // Pause state
        private Vector3 savedVelocity = Vector3.Zero;
        private bool wasPaused = false;

        // ===== Lifecycle =====

        public override void OnStart()
        {
            LogMessage("=== Botnet started (EntityID = " + EntityID.ToString() + ") ===");

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

            // Immediate attempt (no busy-wait loops)
            chooseTargetTimer = 0.0f;
            ChooseTarget();
            hasChosenInitialTarget = true;

            if (targetID != INVALID_ENTITY)
            {
                isMoving = true;
                chooseTargetTimer = RandomRangeFloat(minRetargetDelay, maxRetargetDelay);
            }
            else
            {
                isMoving = false;
                // keep chooseTargetTimer at 0 so OnUpdate retries next frame
            }

            EVENT_BULLET_HIT += EntityID.ToString();

            Subscribe(EVENT_BULLET_HIT, OnBulletHit);
            Subscribe(EVENT_GAME_OVER, OnGameEnd);
            Subscribe(EVENT_GAME_WIN, OnGameEnd);
            //Subscribe(EVENT_SPAWN_DISABLE, OnSpawnDisable);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (isDead)
                return;

            // Handle pause - save/restore velocity
            if (GameState.IsPaused)
            {
                if (!wasPaused)
                {
                    // Just paused - save velocity and stop
                    savedVelocity = RigidbodyGetVelocity((uint)EntityID);
                    Vector3 zero = Vector3.Zero;
                    RigidbodySetVelocity((uint)EntityID, ref zero);
                    wasPaused = true;
                }
                return;
            }
            else if (wasPaused)
            {
                // Just unpaused - restore velocity
                RigidbodySetVelocity((uint)EntityID, ref savedVelocity);
                wasPaused = false;
            }

            if (isStunned)
            {
                UpdateStun(deltaTime);
                return;
            }

            UpdateTargetSelectionTimer(deltaTime);

            if (isMoving && targetID != INVALID_ENTITY)
            {
                if (enableLookAt)
                    RotateTowardsTarget(deltaTime);

                MoveTowardsTarget(deltaTime);
                ClampSpeed();
            }

            HandleCollisionTriggers();

            if (isExploding)
                Explode();
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_BULLET_HIT, OnBulletHit);
            Unsubscribe(EVENT_GAME_OVER, OnGameEnd);
            Unsubscribe(EVENT_GAME_WIN, OnGameEnd);
            //Unsubscribe(EVENT_SPAWN_DISABLE, OnSpawnDisable);
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
            // if (isDead || eventName != EVENT_BULLET_HIT)
            //     return;

            // if (!uint.TryParse(payload, out uint hitId))
            //     return;

            // if (hitId != (uint)EntityID){
            //     LogMessage("[Botnet] ID DOESN'T MATCH HIT!");
            //     return;
            // }

            // HP -= 1.0f;

            float damage = DamageSystem.ParseAmount(payload);
            HP -= damage;

            uint attackerId = DamageSystem.ParseAttackerId(payload);
            if(attackerId != INVALID_ENTITY){
                string attackerTag = TagGetTag(attackerId);
                if(attackerTag == TAG_PRIMARY_BULLET || attackerTag == TAG_SECONDARY_BULLET){
                    //instantiate the hitmarker audio
                    Vector3 spawnPos = GetPosition((uint)EntityID);
                    Quat spawnRot = GetRotation((uint)EntityID);
                    Vector3 scale = new Vector3(0.1f,0.1f,0.1f);
                    uint hitmarkerID = 0;
                    hitmarkerID = PrefabInstantiateWithTransform(hitmarkerAudioPrefab, ref spawnPos, ref spawnRot, ref scale, false);
                    if(hitmarkerID == 0){
                        LogMessage("[Botnet] Player Hit! But hitmarkerID fail to instantiate");
                    }
                }
            }

            

            LogMessage("[Botnet] CurrentBotnetHP is: " + HP.ToString());
            LogMessage("[Botnet] SUCCESS MATCH! REDUCING HEALTH!");

            if (HP <= 0.0f)
            {
                Publish("BotnetDeath", 1.ToString());
                Explode();
            }
        }

        private void OnGameEnd(string eventName, string payload){
            if(isDead){
                return;
            }

            LogMessage("[Botnet] Detect game end. Event is: " + eventName);
            isDead = true;
            isExploding = false;
            SceneDestroyEntity((uint)EntityID);
        }

        // private void OnSpawnDisable(string eventName, string payload)
        // {
        //     if (isDead || eventName != EVENT_SPAWN_DISABLE)
        //         return;

        //     if (!bool.TryParse(payload, out bool active))
        //         return;

        //     if (!active)
        //     {
        //         isDead = true;
        //         isExploding = false;

        //         LogMessage("Destroying itself as spawn is disabled");
        //         SceneDestroyEntity((uint)EntityID);
        //     }
        // }

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

                if (targetID != INVALID_ENTITY){
                    LogMessage("[Botnet] UpdateTargetSelectionTimer: targetid is invalid");
                    return;
                }

                chooseTargetTimer = 0.0f;
            }

            chooseTargetTimer -= deltaTime;
            if (chooseTargetTimer > 0.0f)
                return;

            if (!hasChosenInitialTarget)
                hasChosenInitialTarget = true;

            ChooseTarget();

            if (targetID != INVALID_ENTITY)
            {
                isMoving = true;
                chooseTargetTimer = RandomRangeFloat(minRetargetDelay, maxRetargetDelay);
            }
            else
            {
                isMoving = false;
                chooseTargetTimer = RandomRangeFloat(0.5f, 1.0f);
            }
        }

        private void ChooseTarget()
        {
            if (targetID != INVALID_ENTITY)
                return;

            int choice = RandomRangeInt(0, 4);
            //int choice = 3; // detect Gunship only for testing
            LogMessage("[Botnet] CHOICE IS: " + choice.ToString() + "for entity: " + EntityID.ToString());

            uint chosen = INVALID_ENTITY;

            switch (choice)
            {
                case 0:
                    chosen = FindFirstEntityWithTag(TAG_PLAYER);
                    TARGET = TAG_PLAYER;
                    break;
                case 1:
                    chosen = FindRandomEntityWithTag(TAG_SEMICONDUCTOR);
                    TARGET = TAG_SEMICONDUCTOR;
                    break;
                case 2:
                    chosen = FindRandomEntityWithTag(TAG_EMPLACEMENT);
                    TARGET = TAG_EMPLACEMENT;
                    break;
                case 3:
                    chosen = FindRandomEntityWithTag(TAG_ALLIES);
                    TARGET = TAG_ALLIES;
                    break;
            }

            if (chosen != INVALID_ENTITY)
            {
                targetID = chosen;
                isMoving = true;
                LogMessage("[Botnet] ChooseTarget: Botnet (EntityID = " + EntityID + ") chose target " + targetID + " (choice " + choice + ")");
            }
            else
            {
                targetID = INVALID_ENTITY;
                isMoving = false;
                LogMessage("[Botnet] ChooseTarget: Could not find a target");
            }
        }

        private void EnsureTargetStillValid()
        {
            if (targetID == INVALID_ENTITY){
                LogMessage("[Botnet] Ensure target still valid: target is invalid entity");
                return;
            }

            string tag = TagGetTag(targetID);
            if (string.IsNullOrEmpty(tag))
            {
                LogMessage("[Botnet] EnsureTargetStillValid Botnet (EntityID = " + EntityID + ") target " + targetID + " destroyed");
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

            Vector3 myPos = Transform.GetPosition(self);
            Vector3 targetPos = Transform.GetPosition(targetID);

            Vector3 toTarget = new Vector3(
                targetPos.X - myPos.X,
                targetPos.Y - myPos.Y,
                targetPos.Z - myPos.Z
            );

            // Normalize target direction
            if (!TryNormalize(ref toTarget))
                return;

            Quat currentRot = Transform.GetRotation(self);

            // IMPORTANT:
            // Set this to the botnet mesh's "forward" axis in LOCAL space.
            // If your botnet model faces +X by default, use Vector3.Right instead of Vector3.Forward.
            Vector3 localForward = Vector3.Forward;

            // Compute current forward in WORLD space from current rotation
            Vector3 currentForward = RotateVectorByQuat(currentRot, localForward);
            if (!TryNormalize(ref currentForward))
                return;

            // Delta rotation from where we're facing -> where we want to face
            Quat delta = QuaternionFromToSafe(currentForward, toTarget);

            // Absolute target orientation
            Quat targetRot = Mul(delta, currentRot);
            NormalizeQuat(ref targetRot);

            float t = SimpleMath.Clamp(rotateSpeed * deltaTime, 0.0f, 1.0f);
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
            LogMessage("[Botnet] UpdateMovement: AddingForce to botnet: " + EntityID.ToString() + "." );
            LogMessage("[Botnet] UpdateMovement: Force Added: X: " + force.X.ToString() + ",Y: " + force.Y.ToString() + ", Z: " + force.Z.ToString());
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

        // ========================================================================
        // BOTNET.CS - UPDATED COLLISION HANDLING
        // Replace your HandleCollisionTriggers() method (lines 446-481) with this:
        // ========================================================================

        private void HandleCollisionTriggers()
        {
            if (targetID == INVALID_ENTITY)
                return;

            // Query CollisionManager for what this enemy collided with
            List<uint> collisions = CollisionManager.GetEnemyCollisions((uint)EntityID);
            
            if (collisions == null || collisions.Count == 0)
                return;
            
            uint playerID = SceneFindEntityByName("Player");
            
            foreach (uint other in collisions)
            {
                // Check if hit player
                if (other == playerID)
                {
                    LogMessage("[Botnet] Botnet (EntityID = " + EntityID + ") ATTACKED the Player!");
                    Publish("BotnetAttackedPlayer", EntityID.ToString());
                }
                
                // Check if hit target
                if (other == targetID)
                {
                    LogMessage("[Botnet] Botnet (EntityID = " + EntityID + ") collided with target " + targetID);
                    
                    //Temporary measure
                    DamageSystem.DealDamage(targetID, blastDamage, (uint)EntityID);
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

            LogMessage("[Botnet] Botnet (EntityID = " + EntityID + ") exploding!");

            ApplyBlastToTag(TAG_PLAYER);
            ApplyBlastToTag(TAG_SEMICONDUCTOR);
            ApplyBlastToTag(TAG_EMPLACEMENT);
            ApplyBlastToTag(TAG_ALLIES);

            if (!string.IsNullOrEmpty(deathExplosionPrefab))
            {
                LogMessage("Prefab is not null");
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
                    LogMessage("HI FROM BOTNET WE ARE FUKED");
                    DamageSystem.DealDamage(id, blastDamage, (uint)EntityID);
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

        private static bool TryNormalize(ref Vector3 v)
        {
            float lenSq = v.X * v.X + v.Y * v.Y + v.Z * v.Z;
            if (lenSq <= 0.000001f)
                return false;

            float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
            v.X *= invLen;
            v.Y *= invLen;
            v.Z *= invLen;
            return true;
        }

        private static Vector3 Cross(in Vector3 a, in Vector3 b)
        {
            return new Vector3(
                a.Y * b.Z - a.Z * b.Y,
                a.Z * b.X - a.X * b.Z,
                a.X * b.Y - a.Y * b.X
            );
        }

        private static float Dot(in Vector3 a, in Vector3 b)
        {
            return a.X * b.X + a.Y * b.Y + a.Z * b.Z;
        }

        private static Quat Mul(in Quat a, in Quat b)
        {
            Quat r;
            r.W = a.W * b.W - a.X * b.X - a.Y * b.Y - a.Z * b.Z;
            r.X = a.W * b.X + a.X * b.W + a.Y * b.Z - a.Z * b.Y;
            r.Y = a.W * b.Y - a.X * b.Z + a.Y * b.W + a.Z * b.X;
            r.Z = a.W * b.Z + a.X * b.Y - a.Y * b.X + a.Z * b.W;
            return r;
        }

        private static Vector3 RotateVectorByQuat(in Quat q, in Vector3 v)
        {
            // v' = q * (v,0) * conj(q)
            Quat vq;
            vq.X = v.X; vq.Y = v.Y; vq.Z = v.Z; vq.W = 0.0f;

            Quat qc;
            qc.X = -q.X; qc.Y = -q.Y; qc.Z = -q.Z; qc.W = q.W;

            Quat t = Mul(q, vq);
            Quat r = Mul(t, qc);

            return new Vector3(r.X, r.Y, r.Z);
        }

        private static void NormalizeQuat(ref Quat q)
        {
            float lenSq = q.X * q.X + q.Y * q.Y + q.Z * q.Z + q.W * q.W;
            if (lenSq <= 0.000001f)
                return;

            float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
            q.X *= invLen;
            q.Y *= invLen;
            q.Z *= invLen;
            q.W *= invLen;
        }

        private static Quat QuaternionFromToSafe(Vector3 from, Vector3 to)
        {
            // Ensure both are unit
            if (!TryNormalize(ref from) || !TryNormalize(ref to))
            {
                Quat id;
                id.X = 0; id.Y = 0; id.Z = 0; id.W = 1;
                return id;
            }

            float dot = Dot(from, to);
            dot = SimpleMath.Clamp(dot, -1.0f, 1.0f);

            // Nearly identical -> identity
            if (dot > 0.9999f)
            {
                Quat id;
                id.X = 0; id.Y = 0; id.Z = 0; id.W = 1;
                return id;
            }

            // Nearly opposite -> 180 around any orthogonal axis
            if (dot < -0.9999f)
            {
                Vector3 axis = Cross(from, Vector3.Up);
                if (!TryNormalize(ref axis))
                {
                    axis = Cross(from, Vector3.Right);
                    TryNormalize(ref axis);
                }

                Quat q180;
                q180.X = axis.X;
                q180.Y = axis.Y;
                q180.Z = axis.Z;
                q180.W = 0.0f;
                return q180;
            }

            Vector3 cross = Cross(from, to);

            float s = SimpleMath.Sqrt((1.0f + dot) * 2.0f);
            float invS = 1.0f / s;

            Quat q;
            q.X = cross.X * invS;
            q.Y = cross.Y * invS;
            q.Z = cross.Z * invS;
            q.W = 0.5f * s;

            NormalizeQuat(ref q);
            return q;
        }

    }
}
