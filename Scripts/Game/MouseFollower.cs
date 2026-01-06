using Engine;
using static Engine.Log;
using System;

namespace Game
{
    /// <summary>
    /// Makes an object follow the mouse cursor (in 2D screen space)
    /// Useful for UI elements or 2D games
    /// </summary>
    public class MouseFollower : ScriptBehaviour
    {
        public float FollowSpeed = 5.0f;
        public float ZDistance = 10.0f; // Distance from camera

        private Vector2 lastMousePos;

        public override void OnStart()
        {
            LogMessage("MouseFollower initialized");
            lastMousePos = Input.GetMousePosition();
        }

        public override void OnUpdate(float deltaTime)
        {
            Vector2 mousePos = Input.GetMousePosition();

            // Check if mouse moved
            if (Vector2.Distance(mousePos, lastMousePos) > 0.1f)
            {
                // Convert screen space to world space (simplified)
                // In a real game, you'd do proper screen-to-world conversion
                Vector3 targetPos = new Vector3(
                    (mousePos.X / 800.0f - 0.5f) * 20.0f, // Assuming 800px width
                    -(mousePos.Y / 600.0f - 0.5f) * 15.0f, // Assuming 600px height
                    ZDistance
                );

                // Smoothly interpolate to target
                Vector3 currentPos = Transform.Position;
                Vector3 newPos = Vector3.Lerp(currentPos, targetPos, FollowSpeed * deltaTime);
                Transform.Position = newPos;

                lastMousePos = mousePos;
            }

            // Optional: Click to log position
            if (Input.IsMouseButtonPressed(MouseButton.Left))
            {
                LogMessage($"Mouse clicked at screen pos: {mousePos}");
                LogMessage($"Object world pos: {Transform.Position}");
            }
        }
    }
}
