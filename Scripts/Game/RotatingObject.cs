using Engine;
using System;
using static Engine.Log;

namespace Game
{
    /// <summary>
    /// Simple script that makes an object rotate continuously.
    /// Now uses quaternion-based rotation via Transform.RotateAxisAngle.
    /// </summary>
    public class RotatingObject : ScriptBehaviour
    {
        /// <summary>
        /// Rotation speed in degrees per second.
        /// </summary>
        public float RotationSpeed = 45.0f;

        /// <summary>
        /// Axis to rotate around (in local space).
        /// </summary>
        public Vector3 RotationAxis = Vector3.Up;

        private float totalRotation = 0.0f;

        public override void OnStart()
        {
            LogMessage($"RotatingObject started on entity {EntityID}");
        }

        public override void OnUpdate(float deltaTime)
        {
            // No rotation requested or no time has passed
            if (deltaTime <= 0.0f || Math.Abs(RotationSpeed) <= 0.0001f)
                return;

            // Normalise the axis (engine's Vector3 has a Normalized helper)
            Vector3 axis = RotationAxis.Normalized;

            // Guard against a zero-length axis
            if (Math.Abs(axis.X) < 0.0001f &&
                Math.Abs(axis.Y) < 0.0001f &&
                Math.Abs(axis.Z) < 0.0001f)
            {
                return;
            }

            // Degrees this frame
            float rotationThisFrameDeg = RotationSpeed * deltaTime;

            // Convert to radians for the quaternion API (glm-style)
            float rotationThisFrameRad = rotationThisFrameDeg * SimpleMath.DEG_TO_RAD;

            // Apply rotation using the Transform helper (quaternion under the hood)
            Transform.RotateAxisAngle(axis, rotationThisFrameRad);

            // Track accumulated rotation in degrees just for logging
            totalRotation += Math.Abs(rotationThisFrameDeg);

            // LogMessage every full revolution
            if (totalRotation >= 360.0f)
            {
                LogMessage($"Completed a full rotation! Total accumulated: {totalRotation:F2} degrees");
                totalRotation -= 360.0f;
            }
        }
    }
}
