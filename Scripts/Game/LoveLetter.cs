using Engine;
using System;

namespace Game
{
    public class LoveLetter
    {
        // ===== Entity Reference =====
        public int EntityID = 0;

        // ===== Serialized Waypoint Entity IDs (Edit in Inspector) =====
        [SerializeField]
        private int waypoint0ID = 3;

        [SerializeField]
        private int waypoint1ID = 4;

        [SerializeField]
        private int waypoint2ID = 5;

        [SerializeField]
        private int waypoint3ID = 6;

        // ===== Movement Settings =====
        [SerializeField]
        private float moveSpeed = 2.0f;

        [SerializeField]
        private float waypointReachedDistance = 0.5f;

        [SerializeField]
        private float startDelay = 1.0f;

        // ===== Status =====
        [SerializeField]
        private bool isMoving = false;

        [SerializeField]
        private int currentWaypoint = 0;

        // ===== Private Fields =====
        private int[] waypoints;
        private float delayTimer = 0.0f;

        // ===== Lifecycle =====
        public void OnStart()
        {
            Engine.InternalCalls.Log("=== LoveLetter Started ===");
            Engine.InternalCalls.Log("EntityID: " + EntityID);

            // Build waypoint array
            waypoints = new int[4];
            waypoints[0] = waypoint0ID;
            waypoints[1] = waypoint1ID;
            waypoints[2] = waypoint2ID;
            waypoints[3] = waypoint3ID;

            // Log waypoints
            Engine.InternalCalls.Log("Waypoints: " + waypoint0ID + ", " + waypoint1ID + ", " + waypoint2ID + ", " + waypoint3ID);

            // Validate waypoints
            if (waypoint0ID == 0 || waypoint1ID == 0 || waypoint2ID == 0 || waypoint3ID == 0)
            {
                Engine.InternalCalls.LogWarning("LoveLetter: Some waypoints not set!");
                return;
            }

            // Start with delay
            delayTimer = startDelay;
            currentWaypoint = 0;

            Engine.InternalCalls.Log("LoveLetter initialized - waiting " + startDelay + " seconds before movement");
        }

        public void OnUpdate(float deltaTime)
        {
            // Handle start delay
            if (delayTimer > 0.0f)
            {
                delayTimer -= deltaTime;

                if (delayTimer <= 0.0f)
                {
                    Engine.InternalCalls.Log("Delay finished - calling StartMoving");
                    StartMoving();
                }
                return;
            }

            // Move along path
            if (isMoving)
            {
                MoveTowardsWaypoint(deltaTime);
            }
        }

        // ===== Movement System =====
        private void StartMoving()
        {
            Engine.InternalCalls.Log("=== StartMoving called ===");

            if (waypoints == null || waypoints.Length == 0)
            {
                Engine.InternalCalls.LogWarning("No waypoints!");
                return;
            }

            // Get waypoint 0 position
            Engine.Vector3 startPos;
            Engine.InternalCalls.Transform_GetPosition((uint)waypoints[0], out startPos);

            Engine.InternalCalls.Log("Teleporting to waypoint 0: " + startPos.X + ", " + startPos.Y + ", " + startPos.Z);

            // Teleport to first waypoint
            Engine.InternalCalls.Transform_SetPosition((uint)EntityID, ref startPos);

            currentWaypoint = 1;
            isMoving = true;

            Engine.InternalCalls.Log("Movement started! Targeting waypoint 1");
        }

        private void MoveTowardsWaypoint(float deltaTime)
        {
            // Check if finished path
            if (currentWaypoint >= waypoints.Length)
            {
                isMoving = false;
                Engine.InternalCalls.Log("Reached end of path!");
                return;
            }

            // Get current position
            Engine.Vector3 currentPos;
            Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out currentPos);

            // Get target waypoint position
            Engine.Vector3 targetPos;
            Engine.InternalCalls.Transform_GetPosition((uint)waypoints[currentWaypoint], out targetPos);

            // Calculate direction
            Engine.Vector3 direction = new Engine.Vector3(
                targetPos.X - currentPos.X,
                targetPos.Y - currentPos.Y,
                targetPos.Z - currentPos.Z
            );

            // Calculate distance
            float distance = CalculateDistance(currentPos, targetPos);

            // Check if reached
            if (distance < waypointReachedDistance)
            {
                Engine.InternalCalls.Log("Reached waypoint " + currentWaypoint);
                currentWaypoint++;
                return;
            }

            // Normalize direction
            float dirLength = CalculateDistance(direction, new Engine.Vector3(0, 0, 0));
            if (dirLength > 0.0f)
            {
                direction.X /= dirLength;
                direction.Y /= dirLength;
                direction.Z /= dirLength;
            }
            else
            {
                return;
            }

            // Calculate movement
            Engine.Vector3 movement = new Engine.Vector3(
                direction.X * moveSpeed * deltaTime,
                direction.Y * moveSpeed * deltaTime,
                direction.Z * moveSpeed * deltaTime
            );

            // Calculate new position
            Engine.Vector3 newPos = new Engine.Vector3(
                currentPos.X + movement.X,
                currentPos.Y + movement.Y,
                currentPos.Z + movement.Z
            );

            // Apply new position
            Engine.InternalCalls.Transform_SetPosition((uint)EntityID, ref newPos);
        }

        // ===== Helper Functions =====
        private float CalculateDistance(Engine.Vector3 a, Engine.Vector3 b)
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

            for (int i = 0; i < 6; i++)
            {
                if (x <= 0.0f) break;
                x = 0.5f * (x + value / x);
            }

            return x;
        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("=== LoveLetter Destroyed ===");
        }
    }
}
