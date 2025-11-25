using Engine;
using System;

namespace Game
{
    public class EnemyE005
    {
        // ===== Entity Reference =====
        public int EntityID;

        // ===== Serialized Fields =====
        [SerializeField]
        private bool isDead = false;

        [SerializeField]
        private float smoothTime = 0.5f;

        [SerializeField]
        private float speed = 10f;

        [SerializeField]
        private bool moving = false;

        [SerializeField]
        private bool isStunned = false;

        [SerializeField]
        private float stunnedTime = 0.5f;

        [SerializeField]
        private int payloadsDestroyed = 0;

        [SerializeField]
        private int payloadsOnBoard = 9;

        [SerializeField]
        private string pathGiven = "";

        [SerializeField]
        private float startActionsDelay = 0.5f;

        // ===== Private Fields =====
        private Engine.Vector3 velocity;
        private float stunTimer = 0.0f;
        private int[] pathEntityIDs;
        private int pathCurrent = 0;
        private float actionDelayTimer = 0.0f;

        // ===== Lifecycle Methods =====
        public void OnStart()
        {
            Engine.InternalCalls.Log("=== EnemyE005 Started ===");
            Engine.InternalCalls.Log("EntityID: " + EntityID);

            // Setup initial state
            payloadsDestroyed = 0;
            isDead = false;
            isStunned = false;
            moving = false;
            actionDelayTimer = startActionsDelay;
            velocity = new Engine.Vector3(0, 0, 0);

            PathSetup();
        }

        public void OnUpdate(float deltaTime)
        {
            // Handle delayed start
            if (actionDelayTimer > 0.0f)
            {
                actionDelayTimer -= deltaTime;
                if (actionDelayTimer <= 0.0f)
                {
                    StartPathing();
                }
                return;
            }

            // Handle stun
            if (isStunned)
            {
                stunTimer -= deltaTime;
                if (stunTimer <= 0.0f)
                {
                    isStunned = false;
                    moving = true;
                    Engine.InternalCalls.Log("EnemyE005 recovered from stun!");
                }
                return;
            }

            // Move along path
            if (moving && !isDead)
            {
                MovePayload(deltaTime);
            }
        }

        // ===== Path Management =====
        private void PathSetup()
        {
            // TODO: Find path waypoints by tag
            // Example: pathEntityIDs = FindEntitiesByTag("PATH_" + pathGiven);
            Engine.InternalCalls.Log("PathSetup called for: " + pathGiven);
        }

        private void StartPathing()
        {
            if (pathEntityIDs == null || pathEntityIDs.Length == 0)
            {
                Engine.InternalCalls.LogWarning("EnemyE005: No path waypoints set!");
                return;
            }

            // Get first waypoint position (use OUT)
            Engine.Vector3 startPos;
            Engine.InternalCalls.Transform_GetPosition((uint)pathEntityIDs[0], out startPos);

            // Set this entity's position (use REF)
            Engine.InternalCalls.Transform_SetPosition((uint)EntityID, ref startPos);

            pathCurrent = 1;
            moving = true;

            Engine.InternalCalls.Log("EnemyE005 pathing started");
        }

        private void MovePayload(float deltaTime)
        {
            if (pathEntityIDs == null || pathCurrent >= pathEntityIDs.Length)
            {
                moving = false;
                Engine.InternalCalls.Log("EnemyE005 reached end of path");
                return;
            }

            // Get current position (use OUT)
            Engine.Vector3 currentPos;
            Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out currentPos);

            // Get next waypoint position (use OUT)
            Engine.Vector3 nextPathPos;
            Engine.InternalCalls.Transform_GetPosition((uint)pathEntityIDs[pathCurrent], out nextPathPos);

            // Calculate new position
            float lerpSpeed = smoothTime * deltaTime * speed;
            Engine.Vector3 newPos = Lerp(currentPos, nextPathPos, lerpSpeed);

            // Set new position (use REF)
            Engine.InternalCalls.Transform_SetPosition((uint)EntityID, ref newPos);

            // Check if reached waypoint
            float dist = Distance(currentPos, nextPathPos);
            if (dist < 0.5f)
            {
                pathCurrent++;
                Engine.InternalCalls.Log("EnemyE005 reached waypoint " + pathCurrent);
            }
        }

        // ===== Helper Functions =====
        private Engine.Vector3 Lerp(Engine.Vector3 a, Engine.Vector3 b, float t)
        {
            // Clamp t
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            return new Engine.Vector3(
                a.X + (b.X - a.X) * t,
                a.Y + (b.Y - a.Y) * t,
                a.Z + (b.Z - a.Z) * t
            );
        }

        private float Distance(Engine.Vector3 a, Engine.Vector3 b)
        {
            float dx = b.X - a.X;
            float dy = b.Y - a.Y;
            float dz = b.Z - a.Z;
            return SimpleSqrt(dx * dx + dy * dy + dz * dz);
        }

        private float SimpleSqrt(float value)
        {
            if (value <= 0.0f) return 0.0f;
            if (value == 1.0f) return 1.0f;

            float x = value;
            float prev = 0.0f;

            // Newton-Raphson with safety
            for (int i = 0; i < 6; i++)
            {
                if (x <= 0.0f) break;
                prev = x;
                x = 0.5f * (x + value / x);

                // Converged?
                float diff = (x - prev);
                if (diff < 0.0001f && diff > -0.0001f)
                    break;
            }

            return x;
        }

        // ===== Public API =====
        public void Stun()
        {
            if (isDead) return;

            isStunned = true;
            stunTimer = stunnedTime;
            moving = false;

            Engine.InternalCalls.Log("EnemyE005 stunned for " + stunnedTime + " seconds!");
        }

        public void PayloadDestroyed()
        {
            if (isDead) return;

            payloadsDestroyed++;
            Engine.InternalCalls.Log("Payload destroyed (" + payloadsDestroyed + "/" + payloadsOnBoard + ")");

            if (payloadsDestroyed >= payloadsOnBoard)
            {
                moving = false;
                isDead = true;
                Engine.InternalCalls.Log("EnemyE005 DESTROYED: All payloads eliminated!");

                // TODO: Trigger death animation/VFX here
            }
        }

        public void SetPath(int[] waypointEntityIDs)
        {
            pathEntityIDs = waypointEntityIDs;
            Engine.InternalCalls.Log("EnemyE005 path set with " + waypointEntityIDs.Length + " waypoints");
        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("=== EnemyE005 Destroyed ===");
        }
    }
}
