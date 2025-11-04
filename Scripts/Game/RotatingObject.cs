using Engine;
using System;

namespace Game
{
    /// <summary>
    /// Simple script that makes an object rotate continuously
    /// </summary>
    public class RotatingObject : ScriptBehaviour
    {
        public float RotationSpeed = 45.0f; // Degrees per second
        public Vector3 RotationAxis = Vector3.Up;

        private float totalRotation = 0.0f;

        public override void OnStart()
        {
            Log($"RotatingObject started on entity {EntityID}");
        }

        public override void OnUpdate(float deltaTime)
        {
            // Calculate rotation for this frame
            float rotationThisFrame = RotationSpeed * deltaTime;
            totalRotation += rotationThisFrame;

            // Apply rotation based on axis
            Vector3 currentRotation = Transform.Rotation;
            
            if (Math.Abs(RotationAxis.X) > 0.5f)
                currentRotation.X += rotationThisFrame;
            if (Math.Abs(RotationAxis.Y) > 0.5f)
                currentRotation.Y += rotationThisFrame;
            if (Math.Abs(RotationAxis.Z) > 0.5f)
                currentRotation.Z += rotationThisFrame;

            Transform.Rotation = currentRotation;

            // Log every 360 degrees
            if (totalRotation >= 360.0f)
            {
                Log($"Completed a full rotation! Total: {totalRotation} degrees");
                totalRotation -= 360.0f;
            }
        }
    }
}
