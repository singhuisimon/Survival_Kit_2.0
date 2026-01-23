using Engine;
using System;
using static Engine.Transform;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Camera;
using static Engine.Input;
using static Engine.Rigidbody;

namespace Game
{
    /// <summary>
    /// Clean spaceship controller with proper physics separation
    /// - Camera rotation in Update (visual)
    /// - Physics/movement in FixedUpdate
    /// - Player rotates only on Y axis (no banking/rolling)
    /// - Full 360-degree mouse rotation
    /// </summary>
    public class SpaceshipController : ScriptBehaviour
    {
        // ===== Entity References =====
        [SerializeField("Camera Entity Name")] private string cameraName = "PlayerCam";
        [SerializeField("Player Entity Name")] private string playerName = "Player";

        // ===== Camera Settings =====
        [SerializeField("Mouse Sensitivity")] private float mouseSensitivity = 15f;
        [SerializeField("Mouse Smoothing")] private float mouseSmoothing = 0.15f;
        [SerializeField("Max Turn Speed")] private float maxTurnSpeed = 3.0f;
        [SerializeField("Pitch Limit (Degrees)")] private float pitchLimitDegrees = 85.0f;

        // ===== Movement Settings =====
        [SerializeField("Forward Speed")] private float forwardSpeed = 333.0f;
        [SerializeField("Backward Speed")] private float backwardSpeed = 200.0f;
        [SerializeField("Strafe Speed")] private float strafeSpeed = 250.0f;
        [SerializeField("Vertical Speed")] private float verticalSpeed = 200.0f;

        // ===== Strafe Dynamics =====
        [SerializeField("Strafe Acceleration")] private float strafeAcceleration = 15.0f;
        [SerializeField("Strafe Deceleration")] private float strafeDeceleration = 12.0f;

        // ===== Camera Positioning =====
        [SerializeField("Camera Distance Back")] private float cameraDistanceBack = 15.0f;
        //[SerializeField("Camera Distance Back")] private float cameraDistanceBack = 30.0f;
        //[SerializeField("Camera Height Offset")] private float cameraHeightOffset = 3.0f;
        [SerializeField("Camera Height Offset")] private float cameraHeightOffset = 0.0f;
        [SerializeField("Camera Follow Speed")] private float cameraFollowSpeed = 10.0f;
        [SerializeField("Camera Rotation Speed")] private float cameraRotationSpeed = 8.0f;
        [SerializeField("Camera Rotation Speed")] private float cameraLookHeight = 8.0f;

        // ===== Player Rotation =====
        [SerializeField("Player Rotation Speed")] private float playerRotationSpeed = 5.0f;
        [SerializeField("Model Y Rotation Offset")] private float modelYRotationOffset = -90.0f;
        [SerializeField("Model Rotation Offset")] private float modelRotationOffset = 180.0f;

        // ===== Cursor Control =====
        [SerializeField("Toggle Cursor Key")] private KeyCode toggleCursorKey = KeyCode.F3;
        [SerializeField("Start With Cursor Locked")] private bool startWithCursorLocked = true;


        // ===== Internal State =====
        private uint cameraEntityID = 0;
        private uint playerEntityID = 0;

        // Camera rotation angles
        private float pitch = 0.0f;  // Up/down (clamped)
        private float yaw = 0.0f;    // Left/right (unlimited 360)

        // Smoothed input
        private float smoothYaw = 0.0f;
        private float smoothPitch = 0.0f;

        // Movement state
        private float currentStrafeVelocity = 0.0f;
        private float targetStrafeVelocity = 0.0f;
        // private Vector3 targetMovementVelocity = Vector3.Zero;

        // Camera state
        private Vector3 smoothCameraPosition = Vector3.Zero;
        //private Quat smoothCameraRotation = Quat.Identity;

        // Player rotation
        private float targetPlayerYaw = 0.0f;
        private float currentPlayerYaw = 0.0f;

        private bool initialized = false;
        private bool cursorWasVisible = false;

        public override void OnStart()
        {
            // Find entities
            cameraEntityID = SceneFindEntityByName(cameraName);
            playerEntityID = SceneFindEntityByName(playerName);

            if (cameraEntityID == 0)
            {
                LogError("[SpaceshipController] Camera not found: " + cameraName);
                return;
            }
            if (playerEntityID == 0)
            {
                LogError("[SpaceshipController] Player not found: " + playerName);
                return;
            }

            // Setup physics
            RigidbodySetUseGravity(playerEntityID, false);

            // Initialize camera
            InitializeCamera();

            // Setup cursor
            cursorWasVisible = IsCursorVisible();
            if (startWithCursorLocked)
            {
                SetCursorVisible(false);
            }

            initialized = true;
            LogMessage("[SpaceshipController] Initialized - physics in FixedUpdate, visuals in Update");
        }

        private void InitializeCamera()
        {
            Vector3 playerPos = GetPosition(playerEntityID);
            Vector3 camPos = GetPosition(cameraEntityID);

            smoothCameraPosition = camPos;

            Vector3 offset = camPos - playerPos;
            if (offset.Magnitude > 0.001f)
            {
                float horizontalDist = SimpleMath.Sqrt(offset.X * offset.X + offset.Z * offset.Z);
                pitch = SimpleMath.Atan2(offset.Y, horizontalDist);
                yaw = SimpleMath.Atan2(offset.X, offset.Z);
            }
            else
            {
                pitch = 0.0f;
                yaw = 0.0f;
            }

            // Use camera yaw as the authoritative facing direction
            targetPlayerYaw = yaw;
            currentPlayerYaw = yaw;
        }


        public override void OnUpdate(float deltaTime)
        {
            if (!initialized || cameraEntityID == 0 || playerEntityID == 0)
                return;

            // Handle cursor toggle
            HandleCursorToggle();

            // Only update camera if cursor is locked
            if (!IsCursorVisible())
            {
                // Update camera rotation from mouse (VISUAL ONLY)
                UpdateCameraRotationFromMouse(deltaTime);
            }

            // Update camera position and rotation (VISUAL - smooth interpolation)
            UpdateCameraTransform(deltaTime);

            // Smoothly rotate player to face camera direction (VISUAL)
            UpdatePlayerRotation(deltaTime);
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!initialized || cameraEntityID == 0 || playerEntityID == 0)
                return;

            // All physics/movement happens here
            HandleMovementPhysics(deltaTime);
        }

        public override void OnDestroy()
        {
            // Restore cursor state
            SetCursorVisible(cursorWasVisible);
        }

        // ========================================
        // CURSOR CONTROL
        // ========================================
        private void HandleCursorToggle()
        {
            if (IsKeyPressed(toggleCursorKey) || IsKeyPressed(KeyCode.Tab))
            {
                bool currentVisible = IsCursorVisible();
                SetCursorVisible(!currentVisible);

                if (!currentVisible)
                {
                    LogMessage("[SpaceshipController] Cursor unlocked");
                }
                else
                {
                    LogMessage("[SpaceshipController] Cursor locked");
                }
            }
        }

        // ========================================
        // UPDATE: Camera rotation (visual only)
        // ========================================
        private void UpdateCameraRotationFromMouse(float deltaTime)
        {
            GetMouseDelta(out float deltaX, out float deltaY);

            // Calculate target velocities
            float targetYawVel = -deltaX * mouseSensitivity;
            float targetPitchVel = -deltaY * mouseSensitivity;

            // Smooth input
            smoothYaw = SimpleMath.Lerp(smoothYaw, targetYawVel, mouseSmoothing);
            smoothPitch = SimpleMath.Lerp(smoothPitch, targetPitchVel, mouseSmoothing);

            // Clamp turn speed
            smoothYaw = SimpleMath.Clamp(smoothYaw, -maxTurnSpeed, maxTurnSpeed);
            smoothPitch = SimpleMath.Clamp(smoothPitch, -maxTurnSpeed, maxTurnSpeed);

            // Update angles (yaw is UNLIMITED for 360 rotation)
            yaw += smoothYaw * deltaTime;
            pitch += smoothPitch * deltaTime;

            // Normalize yaw to prevent float overflow (keep within -2π to 2π)
            while (yaw > SimpleMath.PI * 2.0f)
                yaw -= SimpleMath.PI * 2.0f;
            while (yaw < -SimpleMath.PI * 2.0f)
                yaw += SimpleMath.PI * 2.0f;

            // Clamp pitch only (up/down limit)
            float pitchLimitRadians = pitchLimitDegrees * SimpleMath.DEG_TO_RAD;
            pitch = SimpleMath.Clamp(pitch, -pitchLimitRadians, pitchLimitRadians);

            // Update target player yaw (horizontal only, no pitch)
            targetPlayerYaw = yaw;
        }

        // ========================================
        // UPDATE: Camera transform (visual smoothing)
        // ========================================
        private void UpdateCameraTransform(float deltaTime)
        {
            Vector3 playerPos = GetPosition(playerEntityID);

            // Calculate camera forward direction from angles
            Vector3 camForward = CalculateCameraForward();
            Vector3 camRight = Vector3.Cross(camForward, Vector3.Up).Normalized;
            Vector3 camUp = Vector3.Cross(camRight, camForward).Normalized;

            // Calculate desired camera position (behind and above player)
            Vector3 desiredCameraPos = playerPos
                - camForward * cameraDistanceBack
                + Vector3.Up * cameraHeightOffset;

            // Smooth camera position
            smoothCameraPosition = Vector3.Lerp(
                smoothCameraPosition,
                desiredCameraPos,
                cameraFollowSpeed * deltaTime
            );

            SetPosition(cameraEntityID, ref smoothCameraPosition);

            // Calculate camera look-at target (ahead of player)
            //Vector3 lookAtTarget = playerPos + camForward * 10.0f;
            Vector3 lookAtTarget = playerPos + Vector3.Up * cameraLookHeight;
            SetTarget(cameraEntityID, ref lookAtTarget);
        }

        // ========================================
        // UPDATE: Player rotation (visual smoothing)
        // ========================================
        // private void UpdatePlayerRotation(float deltaTime)
        // {
        //     // Smoothly interpolate to target yaw (only Y-axis rotation)
        //     currentPlayerYaw = SimpleMath.Lerp(
        //         currentPlayerYaw,
        //         targetPlayerYaw,
        //         playerRotationSpeed * deltaTime
        //     );

        //     // Build rotation: Y-axis only (no pitch, no roll/banking)
        //     Quat yawRotation = Quat.FromAxisAngle(Vector3.Up, currentPlayerYaw);

        //     // Apply model offset
        //     Quat modelOffset = Quat.FromAxisAngle(Vector3.Up, modelYRotationOffset * SimpleMath.DEG_TO_RAD);

        //     Quat finalRotation = (yawRotation * modelOffset).Normalized();

        //     SetRotation(playerEntityID, ref finalRotation);
        // }

        private void UpdatePlayerRotation(float deltaTime)
        {
            // Smoothly follow the camera yaw
            currentPlayerYaw = SimpleMath.Lerp(
                currentPlayerYaw,
                targetPlayerYaw,
                playerRotationSpeed * deltaTime
            );

            // Build yaw rotation (world up)
            Quat yawRotation = Quat.FromAxisAngle(Vector3.Up, currentPlayerYaw);

            // Apply ONLY the model's visual yaw offset (e.g. -90 if the mesh faces +X by default)
            Quat modelOffset = Quat.FromAxisAngle(Vector3.Up, modelYRotationOffset * SimpleMath.DEG_TO_RAD);

            Quat finalRotation = (yawRotation * modelOffset).Normalized();
            SetRotation(playerEntityID, ref finalRotation);
        }


        // // Get camera's forward direction and project to horizontal plane
        // Vector3 camForward = CalculateCameraForward();
        // Vector3 horizontalForward = new Vector3(camForward.X, 0.0f, camForward.Z);

        // // Handle edge case of looking straight up/down
        // if (horizontalForward.Magnitude < 0.001f)
        // {
        //     return; // Keep current rotation
        // }

        // horizontalForward = horizontalForward.Normalized;

        // // Calculate yaw from the horizontal forward vector
        // // Note: Engine forward is -Z, so we need to account for that
        // // atan2(x, -z) converts from camera space to engine space
        // float calculatedYaw = SimpleMath.Atan2(horizontalForward.X, -horizontalForward.Z);

        // // Smoothly interpolate to target yaw
        // currentPlayerYaw = SimpleMath.Lerp(
        //     currentPlayerYaw,
        //     calculatedYaw,
        //     playerRotationSpeed * deltaTime
        // );

        // // Build rotation from calculated yaw (no need for model offset with this approach)
        // Quat playerRotation = Quat.FromAxisAngle(Vector3.Up, currentPlayerYaw);

        // // Apply model offset if your spaceship model still needs it
        // Quat modelOffset = Quat.FromAxisAngle(Vector3.Up, modelYRotationOffset * SimpleMath.DEG_TO_RAD);
        // Quat finalRotation = (playerRotation * modelOffset).Normalized();

        // SetRotation(playerEntityID, ref finalRotation);


        // ========================================
        // FIXED UPDATE: Movement physics
        // ========================================
        private void HandleMovementPhysics(float deltaTime)
        {
            GetPlayerMoveAxes(out Vector3 moveForward, out Vector3 moveRight);

            Vector3 forwardVelocity = Vector3.Zero;
            if (IsKeyPressed(KeyCode.W))
                forwardVelocity = moveForward * forwardSpeed;
            else if (IsKeyPressed(KeyCode.S))
                forwardVelocity = -moveForward * backwardSpeed;

            if (IsKeyPressed(KeyCode.A))
                targetStrafeVelocity = -strafeSpeed;
            else if (IsKeyPressed(KeyCode.D))
                targetStrafeVelocity = strafeSpeed;
            else
                targetStrafeVelocity = 0.0f;

            if (targetStrafeVelocity != 0.0f)
                currentStrafeVelocity = SimpleMath.Lerp(currentStrafeVelocity, targetStrafeVelocity, strafeAcceleration * deltaTime);
            else
                currentStrafeVelocity = SimpleMath.Lerp(currentStrafeVelocity, 0.0f, strafeDeceleration * deltaTime);

            Vector3 strafeVelocity = moveRight * currentStrafeVelocity;

            Vector3 verticalVelocity = Vector3.Zero;
            if (IsKeyPressed(KeyCode.Space))
                verticalVelocity = Vector3.Up * verticalSpeed;
            else if (IsKeyPressed(KeyCode.LeftControl))
                verticalVelocity = -Vector3.Up * verticalSpeed;

            Vector3 finalVelocity = forwardVelocity + strafeVelocity + verticalVelocity;
            RigidbodySetVelocity(playerEntityID, ref finalVelocity);
        }

        // ========================================
        // HELPERS
        // ========================================
        private Vector3 CalculateCameraForward()
        {
            Vector3 forward = Vector3.Zero;
            forward.X = SimpleMath.Cos(pitch) * SimpleMath.Sin(yaw);
            forward.Y = SimpleMath.Sin(pitch);
            forward.Z = SimpleMath.Cos(pitch) * SimpleMath.Cos(yaw);
            return forward.Normalized;
        }

        // Movement axes based on the player's *visual facing* (includes modelRotationOffset)
        // Assumes engine "forward" is -Z (so yaw=0 => forward = (0,0,-1))
        private void GetPlayerMoveAxes(out Vector3 forward, out Vector3 right)
        {
            forward = CalculateCameraForward(); // includes pitch (Y)

            right = Vector3.Cross(forward, Vector3.Up);
            if (right.SqrMagnitude < 0.0001f) right = Vector3.Right;
            else right = right.Normalized;
        }
    }
}