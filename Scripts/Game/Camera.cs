using Engine;

namespace Game
{
    /// <summary>
    /// Third-person camera controller.
    /// - Follows a target entity (e.g., Player)
    /// - Rotates around the target using quaternion math only
    /// - Clamps pitch between configurable min/max angles
    /// </summary>
    public class Camera : ScriptBehaviour
    {
        // ===== Target / Follow Settings =====

        [SerializeField]
        private string targetName = "Player";   // Entity name to follow

        [SerializeField]
        private float followDistance = 9.24f;   // How far behind the target
        [SerializeField]
        private float followHeight = 4.55f;   // How high above the target

        [SerializeField]
        private float followDamping = 10.0f;   // Position smoothing factor

        // ===== Rotation Settings =====

        [SerializeField]
        private float rotateSensitivity = 4.0f; // Degrees per input unit

        [SerializeField]
        private float rotateDamping = 20.0f;    // Rotation smoothing factor

        [SerializeField]
        private float minPitchDegrees = -85.0f; // Clamp range for pitch
        [SerializeField]
        private float maxPitchDegrees = 85.0f;

        [SerializeField]
        private bool invertX = false;
        [SerializeField]
        private bool invertY = true;

        // ===== Internal state =====

        private const ulong INVALID_ENTITY = 0xffffffffu;

        private ulong targetEntityID = INVALID_ENTITY;

        // We track orientation purely in quaternions
        private Quat currentRotation;   // What is currently applied to camera
        private Quat targetRotation;    // Desired rotation after input

        // Pitch accumulator in radians (for clamping)
        private float currentPitchRad = 0.0f;

        private bool initialized = false;

        public override void OnStart()
        {
            // Find the target entity by name
            targetEntityID = InternalCalls.Scene_FindEntityByName(targetName);

            // Grab initial rotation from the camera entity
            currentRotation = Transform.GetRotation((ulong)EntityID);
            targetRotation = currentRotation;

            // Derive an initial pitch estimate from the forward vector (quat-only)
            Vector3 forward = currentRotation.Forward;
            // pitch ~ arcsin(Y) -> angle above horizontal in radians
            currentPitchRad = SimpleMath.Asin(forward.Y);

            // Snap position once at start so camera is in the right place
            SnapToTargetPosition();

            initialized = true;
            InternalCalls.Log("[Camera] Initialized. Following: " + targetName +
                                    " (ID: " + targetEntityID.ToString() + ")");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized || targetEntityID == INVALID_ENTITY)
                return;

            // 1) Handle rotation from input (quat only)
            UpdateRotation(deltaTime);

            // 2) Handle smoothed follow of the target
            UpdatePosition(deltaTime);
        }

        // ==========================
        // Rotation (Quaternion-only)
        // ==========================

        private void UpdateRotation(float deltaTime)
        {
            // Get mouse delta from internal call
            float mouseDX, mouseDY;
            InternalCalls.Input_GetMouseDelta(out mouseDX, out mouseDY);
            InternalCalls.Log("Mouse Delta X: " + mouseDX + "Mouse Delta Y: " + mouseDY); 

            if (invertX) mouseDX = -mouseDX;
            if (invertY) mouseDY = -mouseDY;

            // Convert input to angle deltas in radians
            float yawDeltaDeg = mouseDX * rotateSensitivity;
            float pitchDeltaDeg = mouseDY * rotateSensitivity;

            float yawDeltaRad = yawDeltaDeg * SimpleMath.DEG_TO_RAD;
            float pitchDeltaRad = pitchDeltaDeg * SimpleMath.DEG_TO_RAD;

            // Clamp pitch using an accumulator (no Euler conversions)
            if (pitchDeltaRad != 0.0f)
            {
                float minPitchRad = minPitchDegrees * SimpleMath.DEG_TO_RAD;
                float maxPitchRad = maxPitchDegrees * SimpleMath.DEG_TO_RAD;

                float newPitch = currentPitchRad + pitchDeltaRad;

                if (newPitch < minPitchRad)
                {
                    newPitch = minPitchRad;
                }
                else if (newPitch > maxPitchRad)
                {
                    newPitch = maxPitchRad;
                }

                // Adjust delta to respect clamp
                pitchDeltaRad = newPitch - currentPitchRad;
                currentPitchRad = newPitch;
            }

            // Early-out if essentially no input (no System.Math)
            float sqMag = (yawDeltaRad * yawDeltaRad) + (pitchDeltaRad * pitchDeltaRad);
            if (sqMag <= 0.00000001f)
                return;

            Quat oldRotation = targetRotation;

            // 1) Yaw around global up axis
            if (yawDeltaRad != 0.0f)
            {
                Vector3 worldUp = new Vector3(0.0f, 1.0f, 0.0f);
                Quat yawQuat = Quat.FromAxisAngle(worldUp, yawDeltaRad);
                targetRotation = yawQuat * targetRotation;
            }

            // 2) Pitch around camera's local right axis
            if (pitchDeltaRad != 0.0f)
            {
                Vector3 rightAxis = targetRotation.Right;
                Quat pitchQuat = Quat.FromAxisAngle(rightAxis, pitchDeltaRad);
                targetRotation = pitchQuat * targetRotation;
            }

            // Smooth between currentRotation and targetRotation using Slerp
            float t = rotateDamping * deltaTime;
            if (t > 1.0f) t = 1.0f;
            else if (t < 0.0f) t = 0.0f;

            Quat newRotation = Quat.Slerp(currentRotation, targetRotation, t);
            // Normalize to avoid drift
            newRotation = newRotation.Normalized;

            currentRotation = newRotation;
            Transform.SetRotation((ulong)EntityID, ref currentRotation);
        }

        // ==================
        // Position / Follow
        // ==================

        private void UpdatePosition(float deltaTime)
        {
            Vector3 targetPos = Transform.GetPosition(targetEntityID);
            Vector3 currentPos = Transform.GetPosition((ulong)EntityID);

            // Use camera's orientation (quats only) to compute offset
            Vector3 forward = currentRotation.Forward;
            Vector3 up = currentRotation.Up;

            // desiredPos = target - forward * distance + up * height
            Vector3 desiredPos = new Vector3(
                targetPos.X - forward.X * followDistance + up.X * followHeight,
                targetPos.Y - forward.Y * followDistance + up.Y * followHeight,
                targetPos.Z - forward.Z * followDistance + up.Z * followHeight
            );

            // Smoothly interpolate current -> desired
            float t = followDamping * deltaTime;
            if (t > 1.0f) t = 1.0f;
            else if (t < 0.0f) t = 0.0f;

            Vector3 newPos = LerpVector3(currentPos, desiredPos, t);
            Transform.SetPosition((ulong)EntityID, ref newPos);
        }

        private void SnapToTargetPosition()
        {
            if (targetEntityID == INVALID_ENTITY)
                return;

            Vector3 targetPos = Transform.GetPosition(targetEntityID);

            Vector3 forward = currentRotation.Forward;
            Vector3 up = currentRotation.Up;

            Vector3 snapPos = new Vector3(
                targetPos.X - forward.X * followDistance + up.X * followHeight,
                targetPos.Y - forward.Y * followDistance + up.Y * followHeight,
                targetPos.Z - forward.Z * followDistance + up.Z * followHeight
            );

            Transform.SetPosition((ulong)EntityID, ref snapPos);
        }

        // =====================
        // Small helper methods
        // =====================

        private static Vector3 LerpVector3(Vector3 a, Vector3 b, float t)
        {
            if (t <= 0.0f) return a;
            if (t >= 1.0f) return b;

            return new Vector3(
                a.X + (b.X - a.X) * t,
                a.Y + (b.Y - a.Y) * t,
                a.Z + (b.Z - a.Z) * t
            );
        }
    }
}
