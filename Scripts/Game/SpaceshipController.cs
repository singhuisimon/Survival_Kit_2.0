using Engine;
using System;
using static Engine.Transform;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Camera;
using static Engine.Input;
using static Engine.Rigidbody;
using static Engine.Event;
using System.Collections.Specialized;

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
        //[SerializeField("Camera Entity Name")] 
        private string cameraName = "PlayerCam";
        //[SerializeField("Player Entity Name")] 
        private string playerName = "Player";

        // ===== Camera Settings =====
        [SerializeField("Mouse Sensitivity")] private float mouseSensitivity = 0.002f;
        [SerializeField("Mouse Smoothing")] private float mouseSmoothing = 2.0f;
        [SerializeField("Max Turn Speed")] private float maxTurnSpeed = 3.0f;
        [SerializeField("Pitch Limit (Degrees)")] private float pitchLimitDegrees = 65.0f;

        // ===== Movement Settings =====
        [SerializeField("Forward Speed")] private float forwardSpeed = 333.0f;
        [SerializeField("Backward Speed")] private float backwardSpeed = 200.0f;
        [SerializeField("Strafe Speed")] private float strafeSpeed = 250.0f;
        [SerializeField("Vertical Speed")] private float verticalSpeed = 200.0f;

        // ===== Strafe Dynamics =====
        [SerializeField("Strafe Acceleration")] private float strafeAcceleration = 15.0f;
        [SerializeField("Strafe Deceleration")] private float strafeDeceleration = 12.0f;

        //// ===== Camera Positioning =====
        //[SerializeField("Camera Distance Back")] private float cameraDistanceBack = 15.0f;
        ////[SerializeField("Camera Distance Back")] private float cameraDistanceBack = 30.0f;
        ////[SerializeField("Camera Height Offset")] private float cameraHeightOffset = 3.0f;
        //[SerializeField("Camera Height Offset")] private float cameraHeightOffset = 0.0f;
        //[SerializeField("Camera Follow Speed")] private float cameraFollowSpeed = 15.0f;
        //[SerializeField("Camera Rotation Speed")] private float cameraRotationSpeed = 8.0f;
        //[SerializeField("Camera Rotation Speed")] private float cameraLookHeight = 8.0f;

        // ===== Player Rotation =====
        [SerializeField("Player Rotation Speed")] private float playerRotationSpeed = 5.0f;
        [SerializeField("Model Y Rotation Offset")] private float modelYRotationOffset = -90.0f;
        [SerializeField("Model Rotation Offset")] private float modelRotationOffset = 180.0f;

        // ===== Cursor Control =====
        [SerializeField("Toggle Cursor Key")] private KeyCode toggleCursorKey = KeyCode.F3;
        [SerializeField("Start With Cursor Locked")] private bool startWithCursorLocked = true;

        [SerializeField] private float playerHP = 100.0f;
        [SerializeField] private const float playerOriginalHP = 100.0f;

        //[SerializeField] private 
        string EVENT_PLAYER_DAMAGE = "Damage:";
        //[SerializeField] 
        private string EVENT_PLAYER_HEALTHCHANGE = "Health Change";
        //[SerializeField] 
        private string EVENT_PLAYER_OOB = "Damage:";

        // ======= STATE OF COLLISION / IN ENVIRONMENT
        [SerializeField] private bool inEnvironment = true;
        [SerializeField] private float countdownOOB = 5.0f;
        [SerializeField] private float originalCountdownOOB = 5.0f;


        /*
         Default movement values
         */
        // Mouse step increments (matches your C++ ternary step style)
        [SerializeField("Yaw Step")] private float yawStep = 0.05f;
        [SerializeField("Pitch Step")] private float pitchStep = 0.02f;

        // Clamp pitch to avoid flip (HALF_PI - small epsilon)
        [SerializeField("Pitch Clamp Epsilon")] private float pitchClampEps = 0.01f;

        [SerializeField("Orbit Radius")] private float orbitRadius = 7.5f;
        [SerializeField("Model Y Offset (Degrees)")] private float modelYawOffsetDeg = 180.0f;

        [SerializeField("Pitch Clamp Deg")] private float pitchLimitDeg = 85.0f;

        private float aimTargetHeight = 4.55f;
        private float camDistanceBack = -9.24f;
        private Vector3 camAimTarget = Vector3.Zero;

        private Vector3 desiredCameraPos = Vector3.Zero;

        private Vector3 cameraForward; 
        private Vector3 cameraUp; 
        private Vector3 cameraRight; 



        // ===== Internal State =====
        private uint cameraEntityID = 0;
        private uint playerEntityID = 0;

        // Camera rotation angles
        private float pitch = 0.25f;  // Up/down (alpha)
        private float yaw = 0.0f;    // Left/right (betta) (unlimited 360)

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

        // Player rotation
        private float targetPlayerPitch = 0.0f;
        private float currentPlayerPitch = 0.0f;

        private bool initialized = false;
        private bool cursorWasVisible = false;

        public override void OnStart()
        {

            // Kenny: Initialize values that changes overtime (PlayerHP, Countdown OOB, Environment)
            playerHP = playerOriginalHP;
            countdownOOB = originalCountdownOOB;
            inEnvironment = true;

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

            // Setup physics, no gravity 
            RigidbodySetUseGravity(playerEntityID, false);

            // Initialize camera
            //InitializeCamera();

            // Setup cursor
            cursorWasVisible = IsCursorVisible();
            if (startWithCursorLocked)
            {
                SetCursorVisible(false);
            }

            // Set up event subscription for player damage and OOB
            EVENT_PLAYER_DAMAGE += playerEntityID.ToString();
            EVENT_PLAYER_OOB += playerEntityID.ToString();

            Subscribe(EVENT_PLAYER_DAMAGE, OnDamageReceived);
            Subscribe(EVENT_PLAYER_OOB, OnDamageReceived);

            initialized = true;
            LogMessage("[SpaceshipController] Initialized - physics in FixedUpdate, visuals in Update");
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

            // Camera follow player's position, player follow camera's rotation
            // Update camera position with player's position (VISUAL - smooth interpolation)
            UpdateCameraTransform(deltaTime);

            // Smoothly rotate player to face camera direction (VISUAL) (NEED TO UPDATE PITCH TOO)
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
            Unsubscribe(EVENT_PLAYER_DAMAGE, OnDamageReceived);
            Unsubscribe(EVENT_PLAYER_OOB, OnDamageReceived);
        }

        private void InitializeCamera()
        {
            // Kenny: Get player and camera position 
            Vector3 playerPos = GetPosition(playerEntityID);
            Vector3 camPos = GetPosition(cameraEntityID);

            // Kenny: Save camera position for smoothing 
            smoothCameraPosition = camPos;

            // Kenny: Update camera's aim target
            camAimTarget = new Vector3(playerPos.X, playerPos.Y + aimTargetHeight, playerPos.Z);

            // Kenny: Compute difference between player and camera to determine pitch and yaw value
            Vector3 offset = camPos - camAimTarget;

            // Kenny: Set player and camera position offset with pre-set inputs
            if (offset.Magnitude > 0.000001f)
            {
                float horizontalDist = SimpleMath.Clamp(offset.Y / offset.Magnitude, -1.0f, 1.0f);
                pitch = SimpleMath.Asin(horizontalDist);
                yaw = SimpleMath.Atan2(offset.X, offset.Z);
            }
            else
            {
                pitch = 0.25f;
                yaw = 0.0f;
            }

            // Use camera yaw as the authoritative facing direction
            targetPlayerYaw = yaw;
            currentPlayerYaw = yaw;

            targetPlayerPitch = pitch;
            currentPlayerPitch = pitch;
        }

        // ========================================
        // CURSOR CONTROL
        // ========================================
        private void HandleCursorToggle()
        {
            if (IsKeyReleased(toggleCursorKey) || IsKeyReleased(KeyCode.Tab))
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
        private Vector2 smoothedMouse = Vector2.Zero;
        private float mouseSmooth = 18.0f;
        private float pitchAccumRad = 0.0f;   // tracked pitch
        private void UpdateCameraRotationFromMouse(float deltaTime)
        {
            GetMouseDelta(out float dx, out float dy);

            //// Calculate target velocities
            //float targetYawVel = -deltaX * mouseSensitivity;
            //float targetPitchVel = -deltaY * mouseSensitivity;

            //// Smooth input
            //smoothYaw = SimpleMath.Lerp(smoothYaw, targetYawVel, mouseSmoothing);
            //smoothPitch = SimpleMath.Lerp(smoothPitch, targetPitchVel, mouseSmoothing);

            //// Clamp turn speed
            //smoothYaw = SimpleMath.Clamp(smoothYaw, -maxTurnSpeed, maxTurnSpeed);
            //smoothPitch = SimpleMath.Clamp(smoothPitch, -maxTurnSpeed, maxTurnSpeed);

            //// Update angles (yaw is UNLIMITED for 360 rotation)
            //yaw += smoothYaw * deltaTime;
            //pitch += smoothPitch * deltaTime;

            //// Normalize yaw to prevent float overflow (keep within -2π to 2π)
            //while (yaw > SimpleMath.PI * 2.0f)
            //    yaw -= SimpleMath.PI * 2.0f;
            //while (yaw < -SimpleMath.PI * 2.0f)
            //    yaw += SimpleMath.PI * 2.0f;

            //// Clamp pitch only (up/down limit)
            //float pitchLimitRadians = pitchLimitDegrees * SimpleMath.DEG_TO_RAD;
            //pitch = SimpleMath.Clamp(pitch, -pitchLimitRadians, pitchLimitRadians);

            //// Update target player yaw (horizontal only, no pitch)
            //targetPlayerYaw = yaw;



            /*---------------------------- ORIGINAL VERSION ----------------------------*/
            //// Update orbit angles only when mouse moves (and only when cursor locked)
            //// WORK ON SMOOTHING THE PITCH AND YAW
            //if (dx != 0.0f || dy != 0.0f)
            //{
            //    // yaw += (xOffset < 0) ? +0.05 : (xOffset > 0) ? -0.05 : 0
            //    if (dx < 0.0f) yaw += yawStep;
            //    else if (dx > 0.0f) yaw -= yawStep;

            //    // pitch += (yOffset > 0) ? +0.02 : (yOffset < 0) ? -0.02 : 0
            //    if (dy > 0.0f) pitch += pitchStep;
            //    else if (dy < 0.0f) pitch -= pitchStep;


            //    // Clamp pitch to avoid flipping
            //    float pitchLimitRadians = pitchLimitDegrees * SimpleMath.DEG_TO_RAD;
            //    pitch = SimpleMath.Clamp(pitch, -pitchLimitRadians, pitchLimitRadians);

            //    //float halfPi = SimpleMath.PI * 0.5f;
            //    //pitch = SimpleMath.Clamp(pitch, -halfPi + pitchClampEps, halfPi - pitchClampEps);
            //}

            ///*---------------------------- SMOOTH VERSION (BACK UP) ----------------------------*/
            // Update orbit angles only when mouse moves (and only when cursor locked)
            if (dx != 0.0f || dy != 0.0f)
            {
                // yaw += (xOffset < 0) ? +0.05 : (xOffset > 0) ? -0.05 : 0
                if (dx < 0.0f) yaw += yawStep;
                else if (dx > 0.0f) yaw -= yawStep;

                // pitch += (yOffset > 0) ? +0.02 : (yOffset < 0) ? -0.02 : 0
                if (dy > 0.0f) pitch += pitchStep;
                else if (dy < 0.0f) pitch -= pitchStep;


                //// yaw += (xOffset < 0) ? +0.05 : (xOffset > 0) ? -0.05 : 0
                //yaw += -dx * 0.05f;

                //// pitch += (yOffset > 0) ? +0.02 : (yOffset < 0) ? -0.02 : 0
                //pitch += dy * 0.02f;

                // Compute smooth version of yaw and pitch
                smoothYaw = SimpleMath.Lerp(smoothYaw, yaw, mouseSmoothing * deltaTime);
                smoothPitch = SimpleMath.Lerp(smoothPitch, pitch, mouseSmoothing * deltaTime);

                // Clamp pitch to avoid flipping
                float pitchLimitRadians = pitchLimitDegrees * SimpleMath.DEG_TO_RAD;
                smoothPitch = SimpleMath.Clamp(smoothPitch, -pitchLimitRadians, pitchLimitRadians);

                //float halfPi = SimpleMath.PI * 0.5f;
                //pitch = SimpleMath.Clamp(pitch, -halfPi + pitchClampEps, halfPi - pitchClampEps);

                string yawString = "[SpaceshipController] yaw: " + yaw;
                LogMessage(yawString);
                string pitchString = "[SpaceshipController] pitch: " + pitch;
                LogMessage(pitchString);

            }


            ///*---------------------------- SMOOTH VERSION ----------------------------*/
            //// Update orbit angles only when mouse moves (and only when cursor locked)
            //if (dx != 0.0f || dy != 0.0f)
            //{
            //    //// yaw += (xOffset < 0) ? +0.05 : (xOffset > 0) ? -0.05 : 0
            //    //if (dx < 0.0f) yaw += yawStep;
            //    //else if (dx > 0.0f) yaw -= yawStep;

            //    //// pitch += (yOffset > 0) ? +0.02 : (yOffset < 0) ? -0.02 : 0
            //    //if (dy > 0.0f) pitch += pitchStep;
            //    //else if (dy < 0.0f) pitch -= pitchStep;

            //    // Compute smooth version of yaw and pitch
            //    smoothYaw = SimpleMath.Lerp(smoothYaw, yaw, mouseSmoothing * deltaTime);
            //    smoothPitch = SimpleMath.Lerp(smoothPitch, pitch, mouseSmoothing * deltaTime);

            //    // Clamp pitch to avoid flipping
            //    float pitchLimitRadians = pitchLimitDegrees * SimpleMath.DEG_TO_RAD;
            //    smoothPitch = SimpleMath.Clamp(smoothPitch, -pitchLimitRadians, pitchLimitRadians);

            //    // Get radians from yaw and pitch
            //    float yawRad = dx * mouseSensitivity;
            //    float pitchRad = -dy * mouseSensitivity;

            //    // Update camera rotation in place using radians from yaw and pitch
            //    Quat deltaYaw = Quat.FromAxisAngle(Vector3.Up, yawRad);
            //    Quat deltaPitch = Quat.FromAxisAngle(Vector3.Right, pitchRad);
            //    Quat current = GetRotation(cameraEntityID);
            //    current = (deltaYaw * current * deltaPitch).Normalized;
            //    SetRotation(cameraEntityID, current);

            //    //string yawString = "[SpaceshipController] yaw: " + yaw;
            //    //LogMessage(yawString);
            //    //string pitchString = "[SpaceshipController] pitch: " + pitch;
            //    //LogMessage(pitchString);
            //} 

            //GetMouseDelta(out float dx, out float dy);

            //// Smooth mouse input (no Exp)
            //Vector2 raw = new Vector2(dx, dy);
            //float s = mouseSmooth * deltaTime;
            //float t = s / (1.0f + s);
            //t = SimpleMath.Clamp01(t);
            //smoothedMouse = Vector2.Lerp(smoothedMouse, raw, t);

            //// Convert to radians (your yaw sign flip is OK)
            //float deltaYawRad = -smoothedMouse.X * mouseSensitivity;
            //float desiredDeltaPitch = -smoothedMouse.Y * mouseSensitivity;

            //// --- Clamp pitch cleanly using accumulator ---
            //float limitRad = pitchLimitDeg * SimpleMath.DEG_TO_RAD;

            //float newPitch = pitchAccumRad + desiredDeltaPitch;
            //newPitch = SimpleMath.Clamp(newPitch, -limitRad, limitRad);

            //float allowedDeltaPitch = newPitch - pitchAccumRad;
            //pitchAccumRad = newPitch;

            //// --- Apply yaw then pitch ---
            //Quat current = GetRotation(cameraEntityID);

            //// 1) Yaw around WORLD up (always stable)
            //Quat deltaYaw = Quat.FromAxisAngle(Vector3.Up, deltaYawRad);
            //current = (deltaYaw * current).Normalized();

            //// 2) Pitch around CAMERA LOCAL right (in world space)
            //Vector3 localRightWorld = current.Right; // <-- IMPORTANT: NOT Vector3.Right
            //                                         // (optional safety normalize)
            //if (localRightWorld.SqrMagnitude > 1e-8f) localRightWorld = localRightWorld.Normalized;

            //Quat deltaPitch = Quat.FromAxisAngle(localRightWorld, allowedDeltaPitch);

            //// Pitch in local space: post-multiply
            //current = (current * deltaPitch).Normalized();

            //SetRotation(cameraEntityID, ref current);

        }

        // ========================================
        // UPDATE: Camera transform (visual smoothing)
        // ========================================
        private void UpdateCameraTransform(float deltaTime)
        {
            /*---------------------------- SMOOTH VERSION ----------------------------*/
            //// Rebuild direction of camera from player using yaw/pitch EVERY FRAME
            ////Vector3 dir;
            ////dir.X = SimpleMath.Cos(smoothPitch) * SimpleMath.Sin(smoothYaw);
            ////dir.Y = SimpleMath.Sin(smoothPitch);
            ////dir.Z = SimpleMath.Cos(smoothPitch) * SimpleMath.Cos(smoothYaw);
            ////dir = dir.Normalized;

            //// Get player's position to compute camera aim target
            ////Vector3 playerPos = GetPosition(playerEntityID);
            //Vector3 camPos = GetPosition(cameraEntityID);
            //Quat camRot = GetRotation(cameraEntityID);

            //Vector3 camDerivedForward = camRot.Forward;
            //camDerivedForward = camDerivedForward.Normalized;

            //camAimTarget = camPos + camDerivedForward * 10.0f;


            ////// Calculate desired camera position (behind and above player)
            ////desiredCameraPos = camAimTarget + dir * orbitRadius; // May need to update orbitRadius to accomodate damping of cam playing catch up with player
            ////SetPosition(cameraEntityID, ref desiredCameraPos);

            //// Always update camera target to the player's head/aim point
            //SetTarget(cameraEntityID, ref camAimTarget);



            ///*---------------------------- SMOOTH VERSION (BACK UP)----------------------------*/
            // Rebuild direction of camera from player using yaw/pitch EVERY FRAME
            Vector3 dir;
            dir.X = SimpleMath.Cos(smoothPitch) * SimpleMath.Sin(smoothYaw);
            dir.Y = SimpleMath.Sin(smoothPitch);
            dir.Z = SimpleMath.Cos(smoothPitch) * SimpleMath.Cos(smoothYaw);
            dir = dir.Normalized;

            // Get player's position to compute camera aim target
            Vector3 playerPos = GetPosition(playerEntityID);
            camAimTarget = new Vector3(playerPos.X, playerPos.Y + aimTargetHeight, playerPos.Z);


            // Calculate desired camera position (behind and above player)
            desiredCameraPos = camAimTarget + dir * orbitRadius; // May need to update orbitRadius to accomodate damping of cam playing catch up with player
            SetPosition(cameraEntityID, ref desiredCameraPos);

            // Always update camera target to the player's head/aim point
            SetTarget(cameraEntityID, ref camAimTarget);


            /*---------------------------- ORIGINAL VERSION ----------------------------*/
            //// Rebuild direction of camera from player using yaw/pitch EVERY FRAME
            //Vector3 dir;
            //dir.X = SimpleMath.Cos(pitch) * SimpleMath.Sin(yaw);
            //dir.Y = SimpleMath.Sin(pitch);
            //dir.Z = SimpleMath.Cos(pitch) * SimpleMath.Cos(yaw);
            //dir = dir.Normalized;

            //// Get player's position to compute camera aim target
            //Vector3 playerPos = GetPosition(playerEntityID);
            //camAimTarget = new Vector3(playerPos.X, playerPos.Y + aimTargetHeight, playerPos.Z);

            //// Calculate desired camera position (behind and above player)
            //desiredCameraPos = camAimTarget + dir * orbitRadius; // May need to update orbitRadius to accomodate damping of cam playing catch up with player
            //SetPosition(cameraEntityID, ref desiredCameraPos);

            //// Always update camera target to the player's head/aim point
            //SetTarget(cameraEntityID, ref camAimTarget);
        }

        private void UpdatePlayerRotation(float deltaTime)
        {

            /*---------------------------- SMOOTH VERSION ----------------------------*/
            //// Calculate camera forward direction from angles
            //Vector3 camForward = (camAimTarget - desiredCameraPos).Normalized;
            //Vector3 worldUp = Vector3.Up;
            //Vector3 camRight = Vector3.Cross(worldUp, camForward);
            //if (camRight.SqrMagnitude < 1e-8f)
            //    camRight = Vector3.Right;
            //else
            //    camRight = camRight.Normalized;

            //Vector3 camUp = Vector3.Cross(camForward, camRight);
            //if (camUp.SqrMagnitude < 1e-8f)
            //    camUp = worldUp;
            //else
            //    camUp = camUp.Normalized;

            //// Convert basis -> quaternion, then apply model offset
            //cameraForward = camForward;
            //cameraUp = camUp;
            //cameraRight = camRight;
            //Quat camRot = QuatFromBasis(camRight, camUp, camForward);
            //Quat modelOffset = Quat.FromAxisAngle(Vector3.Up, modelYawOffsetDeg * SimpleMath.DEG_TO_RAD);
            ////Quat camRot = GetRotation(cameraEntityID);
            //Quat playerRot = GetRotation(playerEntityID);


            //Quat playerTargetRot = (camRot * modelOffset).Normalized();
            //Quat finalPlayerRot = Quat.Slerp(playerRot, playerTargetRot, playerRotationSpeed * deltaTime);
            //SetRotation(playerEntityID, ref finalPlayerRot);

            ///*---------------------------- SMOOTH VERSION (BACK UP) ----------------------------*/
            // Calculate camera forward direction from angles
            Vector3 camForward = (camAimTarget - desiredCameraPos).Normalized;
            Vector3 worldUp = Vector3.Up;
            Vector3 camRight = Vector3.Cross(worldUp, camForward);
            if (camRight.SqrMagnitude < 1e-8f)
                camRight = Vector3.Right;
            else
                camRight = camRight.Normalized;

            Vector3 camUp = Vector3.Cross(camForward, camRight);
            if (camUp.SqrMagnitude < 1e-8f)
                camUp = worldUp;
            else
                camUp = camUp.Normalized;

            // Convert basis -> quaternion, then apply model offset
            cameraForward = camForward;
            cameraUp = camUp;
            cameraRight = camRight;
            Quat camRot = QuatFromBasis(camRight, camUp, camForward);
            Quat modelOffset = Quat.FromAxisAngle(Vector3.Up, modelYawOffsetDeg * SimpleMath.DEG_TO_RAD);
            //Quat camRot = GetRotation(cameraEntityID);
            Quat playerRot = GetRotation(playerEntityID);


            Quat playerTargetRot = (camRot * modelOffset).Normalized();
            Quat finalPlayerRot = Quat.Slerp(playerRot, playerTargetRot, playerRotationSpeed * deltaTime);
            SetRotation(playerEntityID, ref finalPlayerRot);

            /*---------------------------- ORIGINAL VERSION ----------------------------*/
            //// Calculate camera forward direction from angles
            //Vector3 camForward = (camAimTarget - desiredCameraPos).Normalized;
            //Vector3 worldUp = Vector3.Up;
            //Vector3 camRight = Vector3.Cross(worldUp, camForward);
            //if (camRight.SqrMagnitude < 1e-8f)
            //    camRight = Vector3.Right;
            //else
            //    camRight = camRight.Normalized;

            //Vector3 camUp = Vector3.Cross(camForward, camRight);
            //if (camUp.SqrMagnitude < 1e-8f)
            //    camUp = worldUp;
            //else
            //    camUp = camUp.Normalized;

            //// Convert basis -> quaternion, then apply model offset
            //Quat camRot = QuatFromBasis(camRight, camUp, camForward);
            //Quat modelOffset = Quat.FromAxisAngle(Vector3.Up, modelYawOffsetDeg * SimpleMath.DEG_TO_RAD);

            //Quat finalPlayerRot = (camRot * modelOffset).Normalized();
            //SetRotation(playerEntityID, ref finalPlayerRot);
        }


        // ========================================
        // FIXED UPDATE: Movement physics
        // ========================================
        private void HandleMovementPhysics(float deltaTime)
        {
            // Get S and W (-1 to 1) and A and D (-1 to 1)

            // Store the value in a Vector3 move, and normalize it



            //GetPlayerMoveAxes(out Vector3 moveForward, out Vector3 moveRight);

            Vector3 forwardVelocity = Vector3.Zero;
            if (IsKeyPressed(KeyCode.W))
                forwardVelocity = cameraForward * forwardSpeed;
            else if (IsKeyPressed(KeyCode.S))
                forwardVelocity = -cameraForward * backwardSpeed;

            if (IsKeyPressed(KeyCode.A))
                targetStrafeVelocity = strafeSpeed;
            else if (IsKeyPressed(KeyCode.D))
                targetStrafeVelocity = -strafeSpeed;
            else
                targetStrafeVelocity = 0.0f;

            if (targetStrafeVelocity != 0.0f)
                currentStrafeVelocity = SimpleMath.Lerp(currentStrafeVelocity, targetStrafeVelocity, strafeAcceleration * deltaTime);
            else
                currentStrafeVelocity = SimpleMath.Lerp(currentStrafeVelocity, 0.0f, strafeDeceleration * deltaTime);

            Vector3 strafeVelocity = cameraRight * currentStrafeVelocity;

            Vector3 verticalVelocity = Vector3.Zero;
            if (IsKeyPressed(KeyCode.Space))
                verticalVelocity = Vector3.Up * verticalSpeed;
            else if (IsKeyPressed(KeyCode.LeftControl))
                verticalVelocity = -Vector3.Up * verticalSpeed;

            Vector3 finalVelocity = forwardVelocity + strafeVelocity + verticalVelocity;
            RigidbodySetVelocity(playerEntityID, ref finalVelocity);


            // Settle camera position
            Vector3 camPos = GetPosition(cameraEntityID);
            Vector3 playerPos = GetPosition(playerEntityID);
            camPos = playerPos + (-cameraForward * 10.0f) + (cameraUp * 5.0f);
            SetPosition(cameraEntityID, ref camPos);

            //GetPlayerMoveAxes(out Vector3 moveForward, out Vector3 moveRight);

            //Vector3 forwardVelocity = Vector3.Zero;
            //if (IsKeyPressed(KeyCode.W))
            //    forwardVelocity = moveForward * forwardSpeed;
            //else if (IsKeyPressed(KeyCode.S))
            //    forwardVelocity = -moveForward * backwardSpeed;

            //if (IsKeyPressed(KeyCode.A))
            //    targetStrafeVelocity = -strafeSpeed;
            //else if (IsKeyPressed(KeyCode.D))
            //    targetStrafeVelocity = strafeSpeed;
            //else
            //    targetStrafeVelocity = 0.0f;

            //if (targetStrafeVelocity != 0.0f)
            //    currentStrafeVelocity = SimpleMath.Lerp(currentStrafeVelocity, targetStrafeVelocity, strafeAcceleration * deltaTime);
            //else
            //    currentStrafeVelocity = SimpleMath.Lerp(currentStrafeVelocity, 0.0f, strafeDeceleration * deltaTime);

            //Vector3 strafeVelocity = moveRight * currentStrafeVelocity;

            //Vector3 verticalVelocity = Vector3.Zero;
            //if (IsKeyPressed(KeyCode.Space))
            //    verticalVelocity = Vector3.Up * verticalSpeed;
            //else if (IsKeyPressed(KeyCode.LeftControl))
            //    verticalVelocity = -Vector3.Up * verticalSpeed;

            //Vector3 finalVelocity = forwardVelocity + strafeVelocity + verticalVelocity;
            //RigidbodySetVelocity(playerEntityID, ref finalVelocity);
        }


        private void OnDamageReceived(string eventName, string payload)
        {
            float damage = DamageSystem.ParseAmount(payload);

            playerHP -= damage;

            LogMessage("[SPACESHIP CONTROLLER] OnDamageReceived player " + playerEntityID.ToString() + " gets damage! Health: " + playerHP.ToString() + "/" + playerOriginalHP.ToString());

            //Send event here to ui 
            //take note playerHP is a float!
            Publish(EVENT_PLAYER_HEALTHCHANGE, playerHP.ToString());

            //Check if player is <= 0 if so send signal that you are ded
            if (playerHP <= 0)
            {
                //publish event here
                //do what you need to do here
            }
        }

        // ========================================
        // HELPERS
        // ========================================
        private Vector3 CalculateCameraForward()
        {
            Vector3 forward = Vector3.Zero;
            forward.X = SimpleMath.Cos(smoothPitch) * SimpleMath.Sin(smoothYaw);
            forward.Y = SimpleMath.Sin(smoothPitch);
            forward.Z = SimpleMath.Cos(smoothPitch) * SimpleMath.Cos(smoothYaw);
            return forward.Normalized;
        }

        // Movement axes based on the player's *visual facing* (includes modelRotationOffset)
        // Assumes engine "forward" is -Z (so yaw=0 => forward = (0,0,-1))
        private void GetPlayerMoveAxes(out Vector3 forward, out Vector3 right)
        {
            // --- 3) Player movement: WASD relative to player's facing direction (derived from rotation) ---
            // Mesh default forward is Z- now
            Quat rot = GetRotation(playerEntityID);

            // Forward is local -Z rotated
            forward = rot.Forward;
            if (forward.SqrMagnitude < 1e-8f) forward = Vector3.Forward;
            else forward = forward.Normalized;

            // Right vector from forward and world up
            right = Vector3.Cross(forward, Vector3.Up);
            if (right.SqrMagnitude < 1e-8f) right = Vector3.Right;
            else right = right.Normalized;
        }

        // =========================
        // Basis -> Quaternion
        // right/up/forward are treated as the basis axes (like your glm::mat3(camRight, camUp, camFwd))
        // This avoids the “rolling” artifact you were seeing with naive yaw/pitch Euler composition.
        // =========================
        private static Quat QuatFromBasis(in Vector3 right, in Vector3 up, in Vector3 forward)
        {
            // Rotation matrix with columns: [right up forward]
            float m00 = right.X, m01 = up.X, m02 = forward.X;
            float m10 = right.Y, m11 = up.Y, m12 = forward.Y;
            float m20 = right.Z, m21 = up.Z, m22 = forward.Z;

            float trace = m00 + m11 + m22;

            float x, y, z, w;

            if (trace > 0.0f)
            {
                float s = SimpleMath.Sqrt(trace + 1.0f) * 2.0f; // s = 4w
                w = 0.25f * s;
                x = (m21 - m12) / s;
                y = (m02 - m20) / s;
                z = (m10 - m01) / s;
            }
            else if (m00 > m11 && m00 > m22)
            {
                float s = SimpleMath.Sqrt(1.0f + m00 - m11 - m22) * 2.0f; // s = 4x
                w = (m21 - m12) / s;
                x = 0.25f * s;
                y = (m01 + m10) / s;
                z = (m02 + m20) / s;
            }
            else if (m11 > m22)
            {
                float s = SimpleMath.Sqrt(1.0f + m11 - m00 - m22) * 2.0f; // s = 4y
                w = (m02 - m20) / s;
                x = (m01 + m10) / s;
                y = 0.25f * s;
                z = (m12 + m21) / s;
            }
            else
            {
                float s = SimpleMath.Sqrt(1.0f + m22 - m00 - m11) * 2.0f; // s = 4z
                w = (m10 - m01) / s;
                x = (m02 + m20) / s;
                y = (m12 + m21) / s;
                z = 0.25f * s;
            }

            // Assumes Quat has (x,y,z,w) constructor. If your Quat uses different field order,
            // adjust here to match your engine's Quat definition.
            Quat q = new Quat(x, y, z, w);
            return q.Normalized();
        }

        private static Vector3 RotateVectorByQuat(in Quat q, in Vector3 v)
        {
            // q must be normalized for best results
            Vector3 qv = new Vector3(q.X, q.Y, q.Z);

            // t = 2 * cross(q.xyz, v)
            Vector3 t = Vector3.Cross(qv, v) * 2.0f;

            // v' = v + w*t + cross(q.xyz, t)
            return v + t * q.W + Vector3.Cross(qv, t);
        }

    }
}
