using Engine;
using System;

namespace Game
{
    public class LoveLetter : ScriptBehaviour
    {
        // ===== Movement Settings =====
        [SerializeField]
        private float moveSpeed = 25.0f;  // Movement speed

        [SerializeField]
        private float waypointReachedDistance = 5.0f;  // Distance threshold to consider waypoint reached

        [SerializeField]
        private float rotationSpeed = 3.0f;  // Quaternion slerp speed

        [SerializeField]
        private float startDelay = 3.0f;     // Delay before starting movement

        // Quaternion we want to rotate towards
        private Quat targetRotation;

        // ===== Core Health System =====
        [SerializeField]
        private int totalCores = 9;

        [SerializeField]
        private int coresAlive = 9;

        // ===== Status =====
        private bool isMoving = false;
        private bool isRotating = false;
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
        public override void OnStart()
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
            waypoints = new Engine.Vector3[7];
            waypoints[0] = new Engine.Vector3(-26.6f, -42.0f, -780.0f);
            waypoints[1] = new Engine.Vector3(-96.0f, -42.0f, -634.0f);
            waypoints[2] = new Engine.Vector3(-96.0f, -42.0f, -335.0f);
            waypoints[3] = new Engine.Vector3(-40.0f, -42.0f, -250.0f);
            waypoints[4] = new Engine.Vector3(45.0f, -42.0f, -250.0f);
            waypoints[5] = new Engine.Vector3(165.0f, -57.0f, -215.0f);
            waypoints[6] = new Engine.Vector3(165.0f, -115.0f, -131.0f);

            Engine.InternalCalls.Log("Waypoints:");
            Engine.InternalCalls.Log("  [0]: " + waypoints[0].X + ", " + waypoints[0].Y + ", " + waypoints[0].Z);
            Engine.InternalCalls.Log("  [1]: " + waypoints[1].X + ", " + waypoints[1].Y + ", " + waypoints[1].Z);
            Engine.InternalCalls.Log("  [2]: " + waypoints[2].X + ", " + waypoints[2].Y + ", " + waypoints[2].Z);
            Engine.InternalCalls.Log("  [3]: " + waypoints[3].X + ", " + waypoints[3].Y + ", " + waypoints[3].Z);
            Engine.InternalCalls.Log("  [4]: " + waypoints[4].X + ", " + waypoints[4].Y + ", " + waypoints[4].Z);
            Engine.InternalCalls.Log("  [5]: " + waypoints[5].X + ", " + waypoints[5].Y + ", " + waypoints[5].Z);
            Engine.InternalCalls.Log("  [6]: " + waypoints[6].X + ", " + waypoints[6].Y + ", " + waypoints[6].Z);

            // Start with delay
            delayTimer = startDelay;
            currentWaypoint = 0;

            Engine.InternalCalls.Log("LoveLetter initialized - waiting " + startDelay + " seconds before movement");
        }

        public override void OnUpdate(float deltaTime)
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

            if (isRotating)
            {
                RotateTowardsTarget(deltaTime);
                return;
            }

            // Move along path
            if (isMoving)
            {
                MoveTowardsWaypoint(deltaTime);
            }

            if (coresAlive <= 0)
            {
                Engine.InternalCalls.Scene_DestroyEntity((uint)EntityID);
                return;
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

            // Start at waypoint 0, rotate to face it first
            currentWaypoint = 0;
            StartRotationToWaypoint(currentWaypoint);

            Engine.InternalCalls.Log("Starting rotation to waypoint 0");
        }

        private void StartRotationToWaypoint(int waypointIndex)
        {
            // Get current position
            Engine.Vector3 currentPos;
            Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out currentPos);

            // Get target waypoint
            Engine.Vector3 targetPos = waypoints[waypointIndex];

            // Horizontal direction (ignore vertical for facing)
            Engine.Vector3 direction = new Engine.Vector3(
                targetPos.X - currentPos.X,
                0.0f,
                targetPos.Z - currentPos.Z
            );

            // Length squared in XZ plane
            float lenSq = direction.X * direction.X + direction.Z * direction.Z;
            if (lenSq <= 0.0001f)
            {
                isRotating = false;
                isMoving = true;
                return;
            }

            // Normalize horizontal direction
            float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
            direction.X *= invLen;
            direction.Z *= invLen;

            // Yaw in radians around world Y axis
            float yaw = SimpleMath.Atan2(direction.X, direction.Z);

            // Build quaternion from axis-angle (Y axis)
            Engine.Vector3 upAxis = new Engine.Vector3(0.0f, 1.0f, 0.0f);
            targetRotation = Engine.Quat.FromAxisAngle(upAxis, yaw);

            // Optional: debug log in degrees
            float yawDeg = yaw * RAD2DEG;
            Engine.InternalCalls.Log("Starting rotation to yaw (deg): " + yawDeg);

            // Start rotating, stop moving for now
            isRotating = true;
            isMoving = false;
        }

        private void RotateTowardsTarget(float deltaTime)
        {
            // Get current rotation as quaternion
            Engine.Quat currentRot = Engine.Transform.GetRotation((uint)EntityID);

            // Ensure we take the shortest path by fixing the sign of the dot
            float dot = Engine.Quat.Dot(currentRot, targetRotation);
            if (dot < 0.0f)
            {
                // Flip target to stay on the same hemisphere
                targetRotation = new Engine.Quat(
                    -targetRotation.X,
                    -targetRotation.Y,
                    -targetRotation.Z,
                    -targetRotation.W
                );
                dot = -dot;
            }


            // If we are already very close, snap to final rotation
            const float DOT_THRESHOLD = 0.9995f; // ~ < 1 degree apart
            if (dot > DOT_THRESHOLD)
            {
                Engine.Transform.SetRotation((uint)EntityID, ref targetRotation);

                isRotating = false;
                isMoving = true;

                Engine.InternalCalls.Log("Rotation complete! Starting movement to waypoint " + currentWaypoint);
                return;
            }


            float t = rotationSpeed * deltaTime;
            if (t > 1.0f)
                t = 1.0f;

            // Slerp towards target quaternion
            Engine.Quat newRot;
            InternalCalls.Quat_Slerp(ref currentRot, ref targetRotation, t, out newRot);
            //Engine.InternalCalls.Log("works");

            Engine.Transform.SetRotation((uint)EntityID, ref newRot);

        }

        private void MoveTowardsWaypoint(float deltaTime)
        {
            // Get current position
            Engine.Vector3 currentPos;
            Engine.InternalCalls.Transform_GetPosition((uint)EntityID, out currentPos);

            // Get target waypoint
            Engine.Vector3 targetPos = waypoints[currentWaypoint];

            // Calculate direction (FULL 3D for vertical movement)
            Engine.Vector3 direction = new Engine.Vector3(
                targetPos.X - currentPos.X,
                targetPos.Y - currentPos.Y,
                targetPos.Z - currentPos.Z
            );

            // Calculate distance
            float distance = SimpleMath.Sqrt(
                direction.X * direction.X +
                direction.Y * direction.Y +
                direction.Z * direction.Z
            );

            // Check if reached waypoint
            if (distance < waypointReachedDistance)
            {
                Engine.InternalCalls.Log("Reached waypoint " + currentWaypoint);

                // Move to next waypoint
                currentWaypoint++;

                // Check if this was the FINAL waypoint
                if (currentWaypoint >= waypoints.Length)
                {
                    Engine.InternalCalls.Log("=== FINAL WAYPOINT REACHED ===");
                    Engine.InternalCalls.Log("LoveLetter self-destructing...");
                    Engine.InternalCalls.Scene_DestroyEntity((uint)EntityID);
                    return;
                }

                // Not final waypoint, continue to next one
                StartRotationToWaypoint(currentWaypoint);
                Engine.InternalCalls.Log("Starting rotation to waypoint " + currentWaypoint);

                return;
            }

            // Normalize direction
            if (distance > 0.0001f)
            {
                direction.X /= distance;
                direction.Y /= distance;
                direction.Z /= distance;
            }
            else
            {
                return;
            }

            // Calculate movement (FULL 3D)
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
        /// Instantly rotate entity to face target position (horizontal Y-only).
        /// </summary>
        private void FaceTowardsWaypoint(Engine.Vector3 fromPos, Engine.Vector3 toPos)
        {
            // Horizontal direction
            Engine.Vector3 direction = new Engine.Vector3(
                toPos.X - fromPos.X,
                0.0f,
                toPos.Z - fromPos.Z
            );

            float lenSq = direction.X * direction.X + direction.Z * direction.Z;
            if (lenSq <= 0.0001f)
                return;

            float invLen = 1.0f / SimpleMath.Sqrt(lenSq);
            direction.X *= invLen;
            direction.Z *= invLen;

            // Compute yaw (radians) and create quaternion around Y axis
            float yaw = SimpleMath.Atan2(direction.X, direction.Z);
            Engine.Vector3 upAxis = new Engine.Vector3(0.0f, 1.0f, 0.0f);

            Engine.Quat lookRot = Engine.Quat.FromAxisAngle(upAxis, yaw);
            Engine.Transform.SetRotation((uint)EntityID, ref lookRot);

            Engine.InternalCalls.Log("Rotated to face waypoint. Yaw (deg): " + (yaw * RAD2DEG));
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

        public override void OnDestroy()
        {
            Engine.EventSystem.Unsubscribe("CoreDestroyed", OnCoreDestroyedEvent);
            Engine.InternalCalls.Log("=== LoveLetter Destroyed ===");
        }
    }
}

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
        // Handle special cases first
        if (x == 0.0f)
        {
            if (y > 0.0f) return PI_2;
            if (y < 0.0f) return -PI_2;
            return 0.0f;
        }

        // Calculate the absolute angle
        float absX = x < 0.0f ? -x : x;
        float absY = y < 0.0f ? -y : y;

        // Use the smaller ratio for better accuracy
        float ratio;
        bool useReciprocal = absY > absX;

        if (useReciprocal)
            ratio = absX / absY;
        else
            ratio = absY / absX;

        // Rational approximation for atan (more stable than Taylor series)
        float ratio2 = ratio * ratio;
        float result = ratio / (1.0f + 0.28f * ratio2);  // Simple rational approximation

        if (useReciprocal)
            result = PI_2 - result;

        // Adjust for quadrant
        if (x < 0.0f)
            result = PI - result;
        if (y < 0.0f)
            result = -result;

        return result;
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

    public static float Abs(float value)
    {
        return value < 0.0f ? -value : value;
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
