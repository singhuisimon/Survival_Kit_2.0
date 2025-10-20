using ScriptAPI;
using System;

public class PlayerController : Script
{
    private float moveSpeed = 5.0f;
    private float jumpForce = 10.0f;
    private bool isGrounded = true;

    public override void Update()
    {
        // Get transform
        var transform = GetTransformComponent();

        // Simple movement example
        float deltaTime = 0.016f; // ~60 FPS, you'll want to pass this from engine

        // Move the entity
        transform.X += moveSpeed * deltaTime;

        // Simple oscillation for testing
        transform.Y = (float)Math.Sin(DateTime.Now.Ticks * 0.0000001) * 2.0f + 5.0f;

        // Log position occasionally
        if (DateTime.Now.Millisecond < 10)
        {
            Console.WriteLine($"PlayerController - Position: ({transform.X:F2}, {transform.Y:F2}, {transform.Z:F2})");
        }
    }
}

