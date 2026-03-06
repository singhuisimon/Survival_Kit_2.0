using Engine;
using System;
using static Engine.Transform;
using static Engine.Prefab;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Camera;
using static Engine.Input;
using static Engine.Rigidbody;
using static Engine.Event;
using static Engine.ParticleSystem;
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
        [SerializeField("Pitch Limit (Degrees)")] private float pitchLimitDegrees = 80.0f;

        // ===== Camera Positioning =====
        [SerializeField("Camera Height Offset")] private float cameraHeightOffset = 20.0f;
        [SerializeField("Camera Distance Behind")] private float cameraDistanceBack = 30.0f;
        [SerializeField("Camera Look Distance")] private float cameraLookDistance = 1000.0f;

        // ===== Camera Movement  =====
        // Catch-up feel (bigger = snappier, smaller = more “laggy”)
        [SerializeField("Camera Follow Smooth")] private float cameraFollowSmooth = 1.8f;
        [SerializeField("Yaw Speed")] private float yawSpeed = 0.05f;
        [SerializeField("Pitch Speed")] private float pitchSpeed = 0.02f;
        [SerializeField("Spaceship Speed")] private float spaceshipSpeed = 100.0f;

        // ===== Player Rotation =====
        [SerializeField("Player Rotation Speed")] private float playerRotationSpeed = 3.5f;
        [SerializeField("Model X Rotation Offset")] private float modelXRotationOffset = 0.0f;

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
        private string EVENT_GAMEWIN = "GameWin";
        private string EVENT_GAMEEND = "GameEnd";

        //Vampirism
        private const string EVENT_WORMHOST_DEAD   = "WormHostDead";
        private const string EVENT_LOVELETTER_DEAD = "LoveLetterKilled";
        private const string EVENT_BOTNET_DEAD     = "BotnetDeath";
        private const string EVENT_KEYLOGGER_DEAD  = "KeyloggerDeath";

        private const float VAMPIRISM_WORMHOST    = 5.0f;
        private const float VAMPIRISM_LOVELETTER  = 25.0f;
        private const float VAMPIRISM_BOTNET      = 2.0f;
        private const float VAMPIRISM_KEYLOGGER   = 5.0f;

        private const string HEAL_VFX_ENTITY_NAME = "HealingVFX";

        private const string TAG_PRIMARY_BULLET   = "PrimaryBullet";
        private const string TAG_SECONDARY_BULLET = "PrimaryUltBullet";

        private bool endscene = false;

        // ======= STATE OF COLLISION / IN ENVIRONMENT
        [SerializeField] private bool inEnvironment = true;
        [SerializeField] private float countdownOOB = 5.0f;
        [SerializeField] private float originalCountdownOOB = 5.0f;

        // ===== Internal State =====
        private uint cameraEntityID = 0;
        private uint playerEntityID = 0;
        private uint healingVFXEntityID = 0;
        private Vector3 smoothCamPos = Vector3.Zero;
        private Vector3 camAimTarget = Vector3.Zero;

        // ===== Camera Direction Vectors =====
        private Vector3 cameraForward;
        private Vector3 cameraUp;
        private Vector3 cameraRight;

        // ===== Camera Rotation Radians =====
        private float yawRad = 0.0f;
        private float pitchRad = 0.0f;

        // ===== Flags =====
        private bool initialized = false;
        private bool cursorWasVisible = false;
        private bool camFollowInit = false;

        // ===== VFX =====
        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint playerHitSparksID = INVALID_ENTITY;
        private uint tempPlayerHitSparksID = INVALID_ENTITY;
        private float hitSparksTimer = 0.1f;
        private bool isHitSparks = false;
        string playerHitSparksPrefabPath = "Sources/Prefabs/HitSparksPlayer.prefab";

        // ===== Camera Shake =====
        //private bool isCameraShake = false;
        private const float camShakeTimerThreshold = 1.0f;      // Larger than the longest shake duration 
        private float camShakeTimer = camShakeTimerThreshold;   // Default is at threshold value
        //[SerializeField] private float CAMSHAKE_DamageTakenMagnitude = 0.1f;
        [SerializeField] private float CAMSHAKE_DamageTakenMagnitude = 100.0f;
        [SerializeField] private float CAMSHAKE_DamageTakenDuration = 0.1f;



        public override void OnStart()
        {

            // Kenny: Initialize values that changes overtime (PlayerHP, Countdown OOB, Environment)
            playerHP = playerOriginalHP;
            countdownOOB = originalCountdownOOB;
            inEnvironment = true;

            // Find entities
            cameraEntityID = SceneFindEntityByName(cameraName);
            playerEntityID = SceneFindEntityByName(playerName);
            healingVFXEntityID = SceneFindEntityByName(HEAL_VFX_ENTITY_NAME);

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
            RigidbodySetIsKinematic(playerEntityID, true); // Comment this if using physics

            // Setup cursor
            cursorWasVisible = IsCursorVisible();
            if (startWithCursorLocked)
            {
                SetCursorVisible(false);
            }

            Quat initialCamRot = GetRotation(cameraEntityID);
            SetRotation(playerEntityID, ref initialCamRot);

            // Set up event subscription for player damage and OOB
            EVENT_PLAYER_DAMAGE += playerEntityID.ToString();
            EVENT_PLAYER_OOB += playerEntityID.ToString();

            Subscribe(EVENT_PLAYER_DAMAGE, OnDamageReceived);
            Subscribe(EVENT_PLAYER_OOB, OnDamageReceived);
            Subscribe(EVENT_GAMEEND, OnGameEnd);
            Subscribe(EVENT_GAMEWIN, OnGameEnd);

            // Vampirism - heal player on player kills
            Subscribe(EVENT_WORMHOST_DEAD,   OnVampirismKill);
            Subscribe(EVENT_LOVELETTER_DEAD, OnVampirismKill);
            Subscribe(EVENT_BOTNET_DEAD,     OnVampirismKill);
            Subscribe(EVENT_KEYLOGGER_DEAD,  OnVampirismKill);
            
            initialized = true;
            LogMessage("[SpaceshipController] Initialized - physics in FixedUpdate, visuals in Update");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized || cameraEntityID == 0 || playerEntityID == 0)
                return;

            // Don't process input when game is paused (check global state)
            if (GameState.IsPaused)
                return;

            // Handle cursor toggle
            HandleCursorToggle();

            // Only update camera if cursor is locked
            if (!IsCursorVisible())
            {
                // Update camera rotation from mouse (VISUAL ONLY)
                UpdateCameraRotationFromMouse(deltaTime);
            }

            // For hit sparks VFX
            hitSparksTimer -= deltaTime;
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!initialized || cameraEntityID == 0 || playerEntityID == 0)
                return;

            // Don't update when game is paused
            if (GameState.IsPaused)
            {
                // Stop physics movement while paused
                Vector3 zero = Vector3.Zero;
                RigidbodySetVelocity(playerEntityID, ref zero);
                return;
            }

            // All physics/movement happens here
            HandleMovementPhysics(deltaTime);
        }

        public override void OnDestroy()
        {
            // Restore cursor state
            SetCursorVisible(cursorWasVisible);
            Unsubscribe(EVENT_PLAYER_DAMAGE, OnDamageReceived);
            Unsubscribe(EVENT_PLAYER_OOB, OnDamageReceived);
            Unsubscribe(EVENT_GAMEEND, OnGameEnd);
            Unsubscribe(EVENT_GAMEWIN, OnGameEnd);

            //unsubscribe for vampirism
            Unsubscribe(EVENT_WORMHOST_DEAD,   OnVampirismKill);
            Unsubscribe(EVENT_LOVELETTER_DEAD, OnVampirismKill);
            Unsubscribe(EVENT_BOTNET_DEAD,     OnVampirismKill);
            Unsubscribe(EVENT_KEYLOGGER_DEAD,  OnVampirismKill);
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
        private void UpdateCameraRotationFromMouse(float deltaTime)
        {
            // Get mouse delta of -1 or 1 for both dx and dy
            GetMouseDelta(out float dx, out float dy);

            // Convert to radians with delta time
            yawRad += -dx * yawSpeed * deltaTime;
            pitchRad += -dy * pitchSpeed * deltaTime;

            // Clamping for pitch to ensure it does not flip the camera
            float limit = pitchLimitDegrees * SimpleMath.DEG_TO_RAD;
            pitchRad = SimpleMath.Clamp(pitchRad, -limit, limit);

            // Convert yaw and pitch to quat
            Quat qYaw = Quat.FromAxisAngle(Vector3.Up, yawRad);
            Quat qPitch = Quat.FromAxisAngle(Vector3.Right, pitchRad);

            // Integrate yaw first, then pitch in yawed space
            Quat camRot = (qYaw * qPitch).Normalized();

            cameraForward = camRot.Forward;
            cameraRight = camRot.Right;
            cameraUp = camRot.Up;

            // Set rotation for camera
            SetRotation(cameraEntityID, ref camRot);

            // Update player rotation
            Quat playerRot = GetRotation(playerEntityID);
            Quat modelXOffset = Quat.FromAxisAngle(Vector3.Right, modelXRotationOffset * SimpleMath.DEG_TO_RAD);
            Quat targetRot = (camRot * modelXOffset).Normalized();

            // Treat playerRotationSpeed as an ANGULAR SPEED (radians/sec).
            playerRot = RotateTowards(playerRot, targetRot, playerRotationSpeed, deltaTime);
            SetRotation(playerEntityID, ref playerRot);
        }

        private void HandleMovementPhysics(float deltaTime)
        {
            // Get S and W (-1 to 1) and A and D (-1 to 1)

            // Store the value in a Vector3 move, and normalize it

            // ----------------------------
            // (a) Player movement based on CAMERA axes
            // ----------------------------
            GetCameraMoveAxes(out Vector3 camFwdFlat, out Vector3 camRightFlat, out Vector3 camUpFlat);

            Vector3 moveDir = Vector3.Zero;

            if (IsKeyPressed(KeyCode.W)) moveDir += camFwdFlat;         // forward
            if (IsKeyPressed(KeyCode.S)) moveDir -= camFwdFlat;         // backward
            if (IsKeyPressed(KeyCode.D)) moveDir += camRightFlat;       // right
            if (IsKeyPressed(KeyCode.A)) moveDir -= camRightFlat;       // left
            if (IsKeyPressed(KeyCode.Space)) moveDir += camUpFlat;      // up
            if (IsKeyPressed(KeyCode.LeftShift)) moveDir -= camUpFlat;  // down

            if (moveDir.SqrMagnitude > 1e-8f)
                moveDir = moveDir.Normalized;
            else
                moveDir = Vector3.Zero;

            // Emit particles trail from player
            emitParticles(moveDir);

            Vector3 playerPos = GetPosition(playerEntityID);
            playerPos = playerPos + moveDir * spaceshipSpeed * deltaTime;
            SetPosition(playerEntityID, ref playerPos);
            // ----------------------------
            // (b)(c) Camera "plays catch" while staying 5 above & 10 behind
            // ----------------------------

            // Use the camera's current forward (flattened) to define "behind"
            Vector3 desiredCamPos =
                playerPos
                - camFwdFlat * cameraDistanceBack
                + camUpFlat * cameraHeightOffset;

            if (!camFollowInit)
            {
                smoothCamPos = GetPosition(cameraEntityID); // start from current
                camFollowInit = true;
            }

            // Frame-rate independent smoothing (no Exp needed)
            // t = k*dt / (1 + k*dt) gives a nice "catch-up" feel
            float t = cameraFollowSmooth * deltaTime;
            t = t / (1.0f + t);

            // Compute smooth camera position
            smoothCamPos = Vector3.Lerp(smoothCamPos, desiredCamPos, t);
            
            // Apply camera shake to smooth position if needed
            if(camShakeTimer < camShakeTimerThreshold) {

                // Update camera shake timer
                camShakeTimer += deltaTime;

                // Choose a random value * right vector (X-axis) to apply for the shake
                Vector3 xShake = camRightFlat * RNG.RandFloat(-CAMSHAKE_DamageTakenMagnitude, CAMSHAKE_DamageTakenMagnitude) * deltaTime;

                // Choose a random value * up vector (Y-axis) to apply for the shake
                Vector3 yShake = camUpFlat * RNG.RandFloat(-CAMSHAKE_DamageTakenMagnitude, CAMSHAKE_DamageTakenMagnitude) * deltaTime;

                // Camera shake based on primary fire shake magnitude
                smoothCamPos = smoothCamPos + xShake + yShake;

                // Resets camera shake timer when it exceeds shake duration
                if (camShakeTimer >= CAMSHAKE_DamageTakenDuration) camShakeTimer = camShakeTimerThreshold;

            } 
            // Set camera position 
            SetPosition(cameraEntityID, ref smoothCamPos);

            // Update camera target
            Vector3 camPos = GetPosition(cameraEntityID);
            Quat camRot = GetRotation(cameraEntityID);
            Vector3 camForward = camRot.Forward;   // Engine convention (-Z)
            camAimTarget = camPos + camForward * cameraLookDistance;
            SetTarget(cameraEntityID, ref camAimTarget);

            //---------------------- DEBUG LOG --------------------------//
            //float distSmoothDesired = Vector3.Distance(smoothCamPos, desiredCamPos);
            //LogMessage("[SpaceshipController] Dist from smooth to desired: " + distSmoothDesired);
            //float distCamPlayer = Vector3.Distance(camPos, playerPos);
            //LogMessage("[SpaceshipController] Dist from camera to player: " + distCamPlayer);

            //float test = Vector3.Dot(cameraForward, camFwdFlat);
            //LogMessage("[SpaceshipController] Dot from consec camForward vectors: " + test);
        }

        private void OnDamageReceived(string eventName, string payload)
        {

            if(playerHP <= 0 || endscene){
                return;
            }

            float damage = DamageSystem.ParseAmount(payload);

            playerHP -= damage;

            // Sets camera shake timer to 0 upon damage received
            camShakeTimer = 0.0f;

            // Reset hit sparks timer and begin new cycle of hit sparks
            if (hitSparksTimer <= 0.0f)
            {
                hitSparksTimer = 0.3f;
                tempPlayerHitSparksID = playerHitSparksID;
                SceneDestroyEntity(tempPlayerHitSparksID);
                playerHitSparksID = INVALID_ENTITY;
                isHitSparks = false;
            }

            // Player hit sparks VFX
            if (playerHitSparksID == INVALID_ENTITY && isHitSparks == false)
            {
                playerHitSparksID = PrefabInstantiate(playerHitSparksPrefabPath);
                isHitSparks = true;
            }
            Vector3 playerPos = GetPosition(playerEntityID);
            Transform.SetPosition(playerHitSparksID, ref playerPos);

            LogMessage("[SPACESHIP CONTROLLER] OnDamageReceived player " + playerEntityID.ToString() + " gets damage! Health: " + playerHP.ToString() + "/" + playerOriginalHP.ToString());
            LogMessage("[SPACESHIP CONTROLLER] OnDamageReceived player from payload" + payload);
            //Send event here to ui 
            //take note playerHP is a float!
            Publish(EVENT_PLAYER_HEALTHCHANGE, playerHP.ToString());

            //Check if player is <= 0 if so send signal that you are ded
            if (playerHP <= 0)
            {
                //publish event here
                //do what you need to do here
                Publish("PlayerDead", "");  // ADD THIS
            }
        }

        private void OnGameEnd(string eventName, string payload){
            LogMessage("[Spaceship Controller] Detect game end condition: " + eventName);
            endscene = true;
        }

        private void emitParticles(Vector3 moveDir)
        {
            // Particle trail
            float currentSpeed = moveDir.Magnitude;

            // Emission rate scaling
            float maxEmissionRate = 80.0f;   // Lots of particles when fast
            float minEmissionRate = 1.0f;    // Single particle at very low speed
            float stopThreshold = 1.0f;      // Below this = complete stop

            // Calculate speed ratio (0.0 to 1.0)
            float speedRatio = SimpleMath.Clamp((currentSpeed * spaceshipSpeed) / spaceshipSpeed, 0.0f, 1.0f);

            LogMessage("speedRatio: " + speedRatio);

            float emissionRate;
            if (currentSpeed < stopThreshold)
            {
                // Nearly stopped - emit final particle then stop
                emissionRate = 0.0f;
            }
            else if (speedRatio < 0.1f) // Less than 10% speed
            {
                // Very slow - just 1-2 particles per second
                emissionRate = minEmissionRate;
            }
            else
            {
                // Scale emission rate with speed (quadratic for better feel)
                emissionRate = SimpleMath.Lerp(minEmissionRate, maxEmissionRate, speedRatio * speedRatio);
            }

            SetEmissionRate(playerEntityID, emissionRate);

            // Velocity still scales with speed for particle direction
            Vector3 exhaustVelocity = -moveDir.Normalized * 50.0f;
            SetEmitterVelocity(playerEntityID, ref exhaustVelocity);
        }

        // ========================================
        // HELPERS
        // ========================================
        // Movement axes based on the player's *visual facing* (includes modelRotationOffset)
        // Assumes engine "forward" is -Z (so yaw=0 => forward = (0,0,-1))
        private void GetCameraMoveAxes(out Vector3 forward, out Vector3 right, out Vector3 up)
        {
            // Camera rotation -> basis
            Quat camRot = GetRotation(cameraEntityID);

            // Use camera forward, but PROJECT to XZ so looking up/down doesn’t make WASD fly vertically.
            forward = camRot.Forward;
            //forward.Y = 0.0f;

            if (forward.SqrMagnitude < 1e-8f)
                forward = Vector3.Forward;
            else
                forward = forward.Normalized;

            right = camRot.Right;
            //right.Y = 0.0f;

            if (right.SqrMagnitude < 1e-8f)
                right = Vector3.Right;
            else
                right = right.Normalized;

            up = camRot.Up;
            if (up.SqrMagnitude < 1e-8f)
                up = Vector3.Up;
            else
                up = up.Normalized;
        }

        // Rotates from "current" toward "target" by at most (maxRadiansPerSec * dt).
        // Guaranteed to reach the target in finite time without an artificial snap threshold.
        private static Quat RotateTowards(Quat current, Quat target, float maxRadiansPerSec, float dt)
        {
            // Safety
            if (dt <= 0f || maxRadiansPerSec <= 0f)
                return current;

            // Make sure we take the shortest path (q and -q represent the same rotation)
            float dot = Quat.Dot(current, target);
            if (dot < 0f)
            {
                // Negate target (same rotation, opposite quaternion)
                target = new Quat(-target.W, -target.X, -target.Y, -target.Z);
                dot = -dot;
            }

            // Clamp for numeric safety
            dot = SimpleMath.Clamp(dot, -1f, 1f);

            // Angle between quaternions:
            // angle = 2 * acos(dot)
            float angle = 2f * SimpleMath.Acos(dot);

            // If already basically identical, finish exactly
            if (angle < 1e-5f)
                return target;

            float maxStep = maxRadiansPerSec * dt;

            // If we can reach this frame, land exactly on target (not a “snap threshold” — it’s physically correct)
            if (maxStep >= angle)
                return target;

            // Otherwise slerp by the fraction of the angle we can cover this frame
            float t = maxStep / angle;
            return Quat.Slerp(current, target, t).Normalized();
        }

        //vampirism
        private void OnVampirismKill(string eventName, string payload)
        {
            if (endscene || playerHP <= 0)
                return;

            // Only heal if the killing blow was a player bullet
            bool isPlayerKill = payload.Contains(TAG_PRIMARY_BULLET) || payload.Contains(TAG_SECONDARY_BULLET);
            if (!isPlayerKill)
                return;

            float healAmount = 0.0f;
            if      (eventName == EVENT_WORMHOST_DEAD)   healAmount = VAMPIRISM_WORMHOST;
            else if (eventName == EVENT_LOVELETTER_DEAD) healAmount = VAMPIRISM_LOVELETTER;
            else if (eventName == EVENT_BOTNET_DEAD)     healAmount = VAMPIRISM_BOTNET;
            else if (eventName == EVENT_KEYLOGGER_DEAD)  healAmount = VAMPIRISM_KEYLOGGER;

            if (healAmount > 0.0f)
            { 
                HealPlayer(healAmount);
                SetEmissionRate(healingVFXEntityID, 15.0f);
            }
            else
            {
                SetEmissionRate(healingVFXEntityID, 0.0f);
            }
                
        }

        private void HealPlayer(float amount)
        {
            playerHP += amount;
            if (playerHP > playerOriginalHP)
                playerHP = playerOriginalHP;

            LogMessage("[SpaceshipController] Vampirism healed player +" + amount + " | HP: " + playerHP + "/" + playerOriginalHP);
            Publish(EVENT_PLAYER_HEALTHCHANGE, playerHP.ToString());
        }

    }
}
