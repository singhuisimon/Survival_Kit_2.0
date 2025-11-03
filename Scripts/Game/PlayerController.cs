using Engine;
using System;

namespace Game
{
    /// <summary>
    /// Example player controller script
    /// Attach this to a player entity to enable WASD movement
    /// </summary>
    public class PlayerController : ScriptBehaviour
    {
        // Configurable properties (can be set from inspector in the future)
        public float MoveSpeed = 5.0f;
        public float RotationSpeed = 100.0f;

        public override void OnStart()
        {
            Log("PlayerController started!");
           // Log($"Entity ID: {EntityID}");
            //Log($"Initial Position: {Transform.Position}");
        }

        public override void OnUpdate(float deltaTime)
        {
            HandleMovement(deltaTime);
           // HandleRotation(deltaTime);
        }

        private void HandleMovement(float deltaTime)
        {
            Vector3 movement = Vector3.Zero;

            // WASD movement
            if (Input.IsKeyPressed(KeyCode.W)) {
                //movement.Z += 1.0f;
                Engine.InternalCalls.Log("su no hot reloading");


            }
            //movement.Z += 1.0f;
            if (Input.IsKeyPressed(KeyCode.S))
                //movement.Z -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.A))
               // movement.X -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.D))
               // movement.X += 1.0f;

            // Vertical movement
         //   if (Input.IsKeyPressed(KeyCode.Space))
         //       movement.Y += 1.0f;
         //   if (Input.IsKeyPressed(KeyCode.LeftShift))
        //        movement.Y -= 1.0f;

            // Normalize and apply speed
            if (movement.SqrMagnitude > 0.01f)
            {
     //           movement = movement.Normalized * MoveSpeed * deltaTime;
      //          Transform.Translate(movement);
            }
        }

        private void HandleRotation(float deltaTime)
        {
            Vector3 rotation = Vector3.Zero;

            // Q/E for rotation around Y axis (yaw)
            if (Input.IsKeyPressed(KeyCode.Q))
                rotation.Y -= RotationSpeed * deltaTime;
            if (Input.IsKeyPressed(KeyCode.E))
                rotation.Y += RotationSpeed * deltaTime;

            if (rotation.SqrMagnitude > 0.01f)
            {
                Transform.Rotate(rotation);
            }
        }

        public override void OnDestroy()
        {
            Log("PlayerController destroyed!");
        }
    }
}
