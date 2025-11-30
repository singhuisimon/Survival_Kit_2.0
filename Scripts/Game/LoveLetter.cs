using Engine;
using System;

namespace Game
{
    public class LoveLetter : ScriptBehaviour
    {
        // ===== Movement Settings =====
        [SerializeField]
        private float moveSpeed = 0.5f;

        [SerializeField]
        private float waypointReachedDistance = 0.5f;

        [SerializeField]
        private float startDelay = 1.0f;

        // ===== Core Health System =====
        [SerializeField]
        private int totalCores = 9;
        private int coresAlive = 9;

        // ===== Status =====
        [SerializeField]
        private bool isMoving = false;

        [SerializeField]
        private int currentWaypoint = 0;

        // ===== Hardcoded Waypoints =====
        private Engine.Vector3[] waypoints;
        private float delayTimer = 0.0f;
        private Engine.Vector3 startPosition;

        // ===== Constants =====
        private const float DEG2RAD = 0.0174532924f;
        private const float RAD2DEG = 57.2957795f;
        private const float PI = 3.14159265359f;

        private void OnAllCoresDestroyed()
        {
            Engine.InternalCalls.Log("=== ALL CORES DESTROYED ===");
            Engine.InternalCalls.Log("LoveLetter is defeated!");

            // Destroy this entity
            Engine.InternalCalls.Scene_DestroyEntity((uint)EntityID);
        }

        private void OnCoreDestroyed()
        {
            coresAlive--;
            Engine.InternalCalls.Log("Core destroyed! Cores alive: " + coresAlive + "/" + totalCores);

            if (coresAlive <= 0)
            {
                OnAllCoresDestroyed();
            }
        }

        private void OnCoreDestroyedEvent(string eventName, string payload)
        {
            // payload contains the parent entity ID that lost a core
            if (int.TryParse(payload, out int parentID) && parentID == EntityID)
            {
                OnCoreDestroyed();
            }
        }

        // ===== Lifecycle =====
        public void OnStart()
        {
            Engine.InternalCalls.Log("=== LoveLetter Started ===");
            Engine.InternalCalls.Log("EntityID: " + EntityID);
            Engine.InternalCalls.Log("Total Cores: " + totalCores);

            // Initialize cores alive
            coresAlive = totalCores;


            // Subscribe to the "CoreDestroyed" channel
            Engine.EventSystem.Subscribe("CoreDestroyed", OnCoreDestroyedEvent);
            // Get spawn position
            Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out startPosition);
            Engine.InternalCalls.Log("Spawn position: " + startPosition.X + ", " + startPosition.Y + ", " + startPosition.Z);

            // Create hardcoded waypoints
            waypoints = new Engine.Vector3[4];
            waypoints[0] = new Engine.Vector3(0.0f, 5.0f, 0.0f);      // Point 1 (right)
            waypoints[1] = new Engine.Vector3(10.0f, 5.0f, 0.0f);     // Point 2 (forward)
            waypoints[2] = new Engine.Vector3(10.0f, 5.0f, 10.0f);    // Point 3 (left)
            waypoints[3] = new Engine.Vector3(0.0f, 5.0f, 10.0f);     // Point 4 (back to start)

            Engine.InternalCalls.Log("Waypoints:");
            Engine.InternalCalls.Log("  [0]: " + waypoints[0].X + ", " + waypoints[0].Y + ", " + waypoints[0].Z);
            Engine.InternalCalls.Log("  [1]: " + waypoints[1].X + ", " + waypoints[1].Y + ", " + waypoints[1].Z);
            Engine.InternalCalls.Log("  [2]: " + waypoints[2].X + ", " + waypoints[2].Y + ", " + waypoints[2].Z);
            Engine.InternalCalls.Log("  [3]: " + waypoints[3].X + ", " + waypoints[3].Y + ", " + waypoints[3].Z);

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

            // Use spawn position as starting point, face first waypoint
            currentWaypoint = 0;
            FaceTowardsWaypoint(startPosition, waypoints[0]);
            isMoving = true;

            Engine.InternalCalls.Log("Movement started! Facing waypoint 0, will start moving");
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

            // Get target waypoint
            Engine.Vector3 targetPos = waypoints[currentWaypoint];

            // Calculate direction
            Engine.Vector3 direction = new Engine.Vector3(
                targetPos.X - currentPos.X,
                targetPos.Y - currentPos.Y,
                targetPos.Z - currentPos.Z
            );

            // Calculate distance
            float distance = CalculateDistance(direction, new Engine.Vector3(0, 0, 0));

            // Check if reached waypoint
            if (distance < waypointReachedDistance)
            {
                Engine.InternalCalls.Log("Reached waypoint " + currentWaypoint);
                currentWaypoint++;

                // Face next waypoint if available
                if (currentWaypoint < waypoints.Length)
                {
                    FaceTowardsWaypoint(currentPos, waypoints[currentWaypoint]);
                    Engine.InternalCalls.Log("Now facing waypoint " + currentWaypoint);
                }
                return;
            }

            // Normalize direction
            if (distance > 0.0f)
            {
                direction.X /= distance;
                direction.Y /= distance;
                direction.Z /= distance;
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

        /// <summary>
        /// Rotate entity to face target position
        /// </summary>
        private void FaceTowardsWaypoint(Engine.Vector3 fromPos, Engine.Vector3 toPos)
        {
            // Calculate direction to target
            Engine.Vector3 direction = new Engine.Vector3(
                toPos.X - fromPos.X,
                0.0f,  // Keep Y rotation only (horizontal)
                toPos.Z - fromPos.Z
            );

            // Calculate yaw angle (rotation around Y axis)
            float yaw = SimpleMath.Atan2(direction.X, direction.Z);

            // Get current rotation
            Engine.Vector3 currentRot = Engine.Transform.GetRotation((uint)EntityID);

            // Set new rotation (keep pitch and roll, update yaw)
            Engine.Vector3 newRot = new Engine.Vector3(
                currentRot.X,
                yaw * RAD2DEG,
                currentRot.Z
            );

            Engine.Transform.SetRotation((uint)EntityID, ref newRot);

            Engine.InternalCalls.Log("Rotated to face waypoint. Yaw: " + (yaw * RAD2DEG));
        }

        /// <summary>
        /// Called by core sub-entities when they die
        /// </summary>


        /// <summary>
        /// Called when all cores are destroyed
        /// </summary>
   

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

// Simple math helper for Atan2
// Simple math helper class
public static class SimpleMath
{
    private const float PI = 3.14159265359f;
    private const float PI_2 = PI / 2.0f;
    private const float PI_4 = PI / 4.0f;

    public const float RAD_TO_DEG = 57.2957795f;
    public const float DEG_TO_RAD = 0.0174532924f;

    public static float Atan2(float y, float x)
    {
        if (x > 0.0f)
            return ArcTan(y / x);

        if (x < 0.0f && y >= 0.0f)
            return ArcTan(y / x) + PI;

        if (x < 0.0f && y < 0.0f)
            return ArcTan(y / x) - PI;

        if (x == 0.0f && y > 0.0f)
            return PI_2;

        if (x == 0.0f && y < 0.0f)
            return -PI_2;

        return 0.0f;
    }

    public static float Asin(float x)
    {
        // Clamp to [-1, 1]
        if (x > 1.0f) x = 1.0f;
        if (x < -1.0f) x = -1.0f;

        // Approximation using Taylor series
        float x2 = x * x;
        float x3 = x2 * x;
        float x5 = x3 * x2;
        float x7 = x5 * x2;

        return x + (x3 / 6.0f) + (3.0f * x5 / 40.0f) + (15.0f * x7 / 336.0f);
    }

    public static float Clamp(float value, float min, float max)
    {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    public static float Sqrt(float value)
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

    private static float ArcTan(float x)
    {
        float x2 = x * x;
        float x3 = x2 * x;
        float x5 = x3 * x2;
        float x7 = x5 * x2;
        float x9 = x7 * x2;

        return x - (x3 / 3.0f) + (x5 / 5.0f) - (x7 / 7.0f) + (x9 / 9.0f);
    }
}

