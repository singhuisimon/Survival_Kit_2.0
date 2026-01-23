using Engine;
using System;
using static Engine.Transform;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Camera;
using static Engine.Rigidbody;
using static Engine.Input;

namespace Game
{
    /// <Summary>
    /// Cam Control class controls a empty parent containing the player camera
    /// as well as a LookAt Object
    /// </Summary>
    public class CamControl : ScriptBehaviour
    {
        [SerializeField] private string lookAtName = "LookAt";
        [SerializeField] private string cameraName = "PlayerCam";
        [SerializeField] private string camControlName = "CamControl";
        [SerializeField] private string playerName = "Player";

        [SerializeField] private float mouseSensitivity = 0.002f;

        [SerializeField] private float movementSpeed = 333.0f;
        [SerializeField] private float playerFollowSpeed = 5.0f;

        [SerializeField] private float camFollowSpeed = 10.0f;
        [SerializeField] private float pitchLimitedDegree = 85.0f;
        [SerializeField] private float offsetCamHeight = 15.0f;
        [SerializeField] private float cameraDistance = 35.0f;

        // How far ahead of the player the camera "aims" based on mouse pitch/yaw.
        [SerializeField] private float cameraLookDistance = 50.0f;

        // Optional: keep, but no longer used for aiming (you can delete if you want)
        [SerializeField] private float cameraOffSet = 5.0f;

        [SerializeField] private float playerRotationSpeed = 10.0f;

        private uint cameraEntityID = 0;
        private uint lookAtEntityID = 0;
        private uint camControlEntityID = 0;
        private uint playerEntityID = 0;

        private float currentPitch = 0.0f;
        private float currentYaw = 0.0f;

        public override void OnStart()
        {
            cameraEntityID = SceneFindEntityByName(cameraName);
            lookAtEntityID = SceneFindEntityByName(lookAtName);
            camControlEntityID = SceneFindEntityByName(camControlName);
            playerEntityID = SceneFindEntityByName(playerName);

            if (cameraEntityID == 0) { LogMessage("[CamControl] player cam cannot be found"); }
            if (lookAtEntityID == 0) { LogMessage("[CamControl] look at entity cannot be found"); }
            if (camControlEntityID == 0) { LogMessage("[CamControl] cam control entity cannot be found"); }
            if (playerEntityID == 0) { LogMessage("[CamControl] player entity cannot be found"); }

            currentYaw = 0.0f;
            currentPitch = 0.0f;

            // Initialize lookAt rotation
            Quat initialRot = Quat.FromAxisAngle(Vector3.Up, currentYaw);
            if (lookAtEntityID != 0)
                SetRotation(lookAtEntityID, ref initialRot);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (cameraEntityID == 0 || lookAtEntityID == 0 || playerEntityID == 0)
                return;

            HandleMouseRotation();
            HandleMovement();

            MovePlayerTowardLookAt(deltaTime);

            // Camera must be updated from mouse yaw/pitch (NOT player forward)
            UpdateCamControlPosition(deltaTime);
            UpdateCameraTarget();

            // Rotate player to face movement direction
            RotatePlayerTowardsMovement(deltaTime);
        }

        // ----------------------------
        // Mouse: updates yaw/pitch
        // ----------------------------
        private void HandleMouseRotation()
        {
            Input.GetMouseDelta(out float deltaX, out float deltaY);

            // Fix horizontal direction: mouse right => yaw increases
            float yawChange = +deltaX * mouseSensitivity;

            // Typical: mouse up => look up (often deltaY is negative when moving up)
            float pitchChange = -deltaY * mouseSensitivity;

            currentYaw += yawChange;
            currentPitch += pitchChange;

            float pitchLimitedRad = pitchLimitedDegree * SimpleMath.DEG_TO_RAD;
            currentPitch = SimpleMath.Clamp(currentPitch, -pitchLimitedRad, pitchLimitedRad);

            // Keep your lookAt entity rotated (useful for debug / movement reference)
            Quat yawRotation = Quat.FromAxisAngle(Vector3.Up, currentYaw);
            Vector3 localRight = yawRotation.RotateVector(Vector3.Right);
            Quat pitchRotation = Quat.FromAxisAngle(localRight, currentPitch);

            Quat finalRotation = (yawRotation * pitchRotation).Normalized();
            SetRotation(lookAtEntityID, ref finalRotation);

            // IMPORTANT: remove spam logging (it will tank your frame time)
            // LogMessage($"Yaw: {currentYaw}, Pitch: {currentPitch}");
        }

        // ----------------------------
        // Movement: follows camera yaw + pitch (includes Y tilt)
        // ----------------------------
        private void HandleMovement()
        {
            float inputX = 0.0f;
            float inputZ = 0.0f;

            // W => +1 forward, S => -1 forward
            if (Input.IsKeyPressed(KeyCode.W)) inputZ += 1.0f;
            if (Input.IsKeyPressed(KeyCode.S)) inputZ -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.A)) inputX -= 1.0f;
            if (Input.IsKeyPressed(KeyCode.D)) inputX += 1.0f;

            if ((inputX * inputX + inputZ * inputZ) < 0.001f)
                return;

            // Use camera forward INCLUDING pitch (Y component)
            Vector3 camForward = GetCameraForward(); // already normalized

            // Right stays "horizontal strafe" relative to world up
            Vector3 camRight = Vector3.Cross(camForward, Vector3.Up);
            if (camRight.SqrMagnitude < 0.0001f)
                camRight = new Vector3(1.0f, 0.0f, 0.0f);
            else
                camRight = camRight.Normalized;

            Vector3 moveDir = (camForward * inputZ) + (camRight * inputX);
            if (moveDir.SqrMagnitude > 0.001f)
                moveDir = moveDir.Normalized;

            Vector3 force = moveDir * movementSpeed;
            RigidbodyAddForce(lookAtEntityID, ref force);
        }


        private void MovePlayerTowardLookAt(float deltaTime)
        {
            Vector3 playerPos = GetPosition(playerEntityID);
            Vector3 targetPos = GetPosition(lookAtEntityID);

            float t = SimpleMath.Clamp(playerFollowSpeed * deltaTime, 0.0f, 1.0f);
            Vector3 newPlayerPos = Vector3.Lerp(playerPos, targetPos, t);

            SetPosition(playerEntityID, ref newPlayerPos);
        }

        // ----------------------------
        // Camera position: 3rd-person behind player using MOUSE YAW
        // ----------------------------
        private void UpdateCamControlPosition(float deltaTime)
        {
            if (playerEntityID == 0 || cameraEntityID == 0)
                return;

            Vector3 playerPos = GetPosition(playerEntityID);
            Vector3 pivot = playerPos + Vector3.Up * offsetCamHeight;

            Vector3 camForward = GetCameraForward();
            Vector3 camRight = Vector3.Cross(camForward, Vector3.Up).Normalized;

            // Orbit behind pivot using BOTH yaw + pitch
            Vector3 targetCameraPos = pivot - (camForward * cameraDistance) + (camRight * cameraOffSet);

            Vector3 currentCameraPos = GetPosition(cameraEntityID);
            float t = SimpleMath.Clamp(camFollowSpeed * deltaTime, 0.0f, 1.0f);
            Vector3 newCameraPos = Vector3.Lerp(currentCameraPos, targetCameraPos, t);

            SetPosition(cameraEntityID, ref newCameraPos);
        }


        // ----------------------------
        // Camera target: uses MOUSE YAW + PITCH (so mouse up => look up)
        // ----------------------------
        private void UpdateCameraTarget()
        {
            if (cameraEntityID == 0)
                return;

            Vector3 camPos = GetPosition(cameraEntityID);
            Vector3 camForward = GetCameraForward();

            // Look along the forward ray
            Vector3 targetPos = camPos + camForward * cameraLookDistance;
            SetTarget(cameraEntityID, ref targetPos);
        }


        // ----------------------------
        // Player yaw: face movement direction
        // ----------------------------
        private void RotatePlayerTowardsMovement(float deltaTime)
        {
            Vector3 velocity = RigidbodyGetVelocity(lookAtEntityID);
            if (velocity.SqrMagnitude < 0.1f)
                return;

            // Yaw that makes forward (-Z) align with velocity direction:
            // forward(yaw) = (sin(yaw), 0, -cos(yaw))
            float targetYaw = SimpleMath.Atan2(velocity.X, -velocity.Z);

            Quat targetRotation = Quat.FromAxisAngle(Vector3.Up, targetYaw);
            Quat currentRotation = GetRotation(playerEntityID);

            float t = SimpleMath.Clamp(playerRotationSpeed * deltaTime, 0.0f, 1.0f);
            Quat newRotation = Quat.Slerp(currentRotation, targetRotation, t);

            SetRotation(playerEntityID, ref newRotation);
        }

        // ============================
        // Helpers
        // ============================
        // Engine convention assumed by your input code:
        // yaw = 0 => forward = (0,0,-1), right = (1,0,0)
        private static void GetYawAxes(float yaw, out Vector3 forward, out Vector3 right)
        {
            float s = SimpleMath.Sin(yaw);
            float c = SimpleMath.Cos(yaw);

            forward = new Vector3(s, 0.0f, -c);
            right = new Vector3(c, 0.0f, s);
        }

        // yaw=0,pitch=0 => forward is (0,0,-1) (matches your W using Z -= 1)
        private Vector3 GetCameraForward()
        {
            float cosPitch = SimpleMath.Cos(currentPitch);
            float sinPitch = SimpleMath.Sin(currentPitch);
            float sinYaw = SimpleMath.Sin(currentYaw);
            float cosYaw = SimpleMath.Cos(currentYaw);

            Vector3 f = new Vector3(
                sinYaw * cosPitch,
                sinPitch,
                -cosYaw * cosPitch
            );

            return (f.SqrMagnitude > 0.0001f) ? f.Normalized : new Vector3(0.0f, 0.0f, -1.0f);
        }

        private void GetYawAxes(out Vector3 forward, out Vector3 right)
        {
            float sinYaw = SimpleMath.Sin(currentYaw);
            float cosYaw = SimpleMath.Cos(currentYaw);

            forward = new Vector3(sinYaw, 0.0f, -cosYaw);
            right = new Vector3(cosYaw, 0.0f, sinYaw);

            forward = forward.Normalized;
            right = right.Normalized;
        }

    }
}
