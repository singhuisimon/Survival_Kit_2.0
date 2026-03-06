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
    /// - Camera rotation in Update
    /// - Physics movement in FixedUpdate
    /// - Player rotation applied in FixedUpdate to avoid fighting physics
    /// - Smoothed velocity changes to reduce tunneling risk
    /// </summary>
    public class SpaceshipController : ScriptBehaviour
    {
        // ===== Entity References =====
        private string cameraName = "PlayerCam";
        private string playerName = "Player";

        // ===== Camera Settings =====
        [SerializeField("Pitch Limit (Degrees)")] private float pitchLimitDegrees = 80.0f;

        // ===== Camera Positioning =====
        [SerializeField("Camera Height Offset")] private float cameraHeightOffset = 20.0f;
        [SerializeField("Camera Distance Behind")] private float cameraDistanceBack = 30.0f;
        [SerializeField("Camera Look Distance")] private float cameraLookDistance = 1000.0f;

        // ===== Camera Movement =====
        [SerializeField("Camera Follow Smooth")] private float cameraFollowSmooth = 1.8f;
        [SerializeField("Yaw Speed")] private float yawSpeed = 0.05f;
        [SerializeField("Pitch Speed")] private float pitchSpeed = 0.02f;
        [SerializeField("Spaceship Speed")] private float spaceshipSpeed = 100.0f;

        // ===== Movement Smoothing =====
        [SerializeField("Acceleration")] private float acceleration = 180.0f;
        [SerializeField("Deceleration")] private float deceleration = 240.0f;
        [SerializeField("Reverse Brake Acceleration")] private float reverseBrakeAcceleration = 520.0f;

        // ===== Player Rotation =====
        [SerializeField("Player Rotation Speed")] private float playerRotationSpeed = 3.5f;
        [SerializeField("Model X Rotation Offset")] private float modelXRotationOffset = 0.0f;

        // ===== Cursor Control =====
        [SerializeField("Toggle Cursor Key")] private KeyCode toggleCursorKey = KeyCode.F3;
        [SerializeField("Start With Cursor Locked")] private bool startWithCursorLocked = true;

        [SerializeField] private float playerHP = 100.0f;
        [SerializeField] private const float playerOriginalHP = 100.0f;

        string EVENT_PLAYER_DAMAGE = "Damage:";
        private string EVENT_PLAYER_HEALTHCHANGE = "Health Change";
        private string EVENT_PLAYER_OOB = "Damage:";
        private string EVENT_GAMEWIN = "GameWin";
        private string EVENT_GAMEEND = "GameEnd";

        //Vampirism
        private const string EVENT_WORMHOST_DEAD   = "WormHostDead";
        private const string EVENT_LOVELETTER_DEAD = "LoveLetterKilled";
        private const string EVENT_BOTNET_DEAD     = "BotnetDeath";
        private const string EVENT_KEYLOGGER_DEAD  = "KeyloggerDeath";

        private const float VAMPIRISM_WORMHOST    = 2.0f;
        private const float VAMPIRISM_LOVELETTER  = 2.0f;
        private const float VAMPIRISM_BOTNET      = 2.0f;
        private const float VAMPIRISM_KEYLOGGER   = 2.0f;

        private const string TAG_PRIMARY_BULLET   = "PrimaryBullet";
        private const string TAG_SECONDARY_BULLET = "PrimaryUltBullet";

        private bool endscene = false;

        // ===== STATE OF COLLISION / IN ENVIRONMENT
        [SerializeField] private bool inEnvironment = true;
        [SerializeField] private float countdownOOB = 5.0f;
        [SerializeField] private float originalCountdownOOB = 5.0f;

        // ===== Internal State =====
        private uint cameraEntityID = 0;
        private uint playerEntityID = 0;
        private Vector3 smoothCamPos = Vector3.Zero;
        private Vector3 camAimTarget = Vector3.Zero;

        // ===== Camera Direction Vectors =====
        private Vector3 cameraForward = Vector3.Forward;
        private Vector3 cameraUp = Vector3.Up;
        private Vector3 cameraRight = Vector3.Right;

        // ===== Camera Rotation Radians =====
        private float yawRad = 0.0f;
        private float pitchRad = 0.0f;

        // ===== Physics / Rotation State =====
        private Vector3 commandedVelocity = Vector3.Zero;
        private Quat targetPlayerRotation;

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
        private const float camShakeTimerThreshold = 1.0f;
        private float camShakeTimer = camShakeTimerThreshold;
        [SerializeField] private float CAMSHAKE_DamageTakenMagnitude = 100.0f;
        [SerializeField] private float CAMSHAKE_DamageTakenDuration = 0.1f;

        public override void OnStart()
        {
            playerHP = playerOriginalHP;
            countdownOOB = originalCountdownOOB;
            inEnvironment = true;

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

            RigidbodySetUseGravity(playerEntityID, false);
            RigidbodySetIsKinematic(playerEntityID, false);

            Vector3 zero = Vector3.Zero;
            commandedVelocity = Vector3.Zero;
            RigidbodySetVelocity(playerEntityID, ref zero);

            cursorWasVisible = IsCursorVisible();
            if (startWithCursorLocked)
            {
                SetCursorVisible(false);
            }

            Quat initialCamRot = GetRotation(cameraEntityID);
            SetRotation(playerEntityID, ref initialCamRot);
            targetPlayerRotation = initialCamRot;

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
            LogMessage("[SpaceshipController] Initialized - smoothed rigidbody movement in FixedUpdate");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized || cameraEntityID == 0 || playerEntityID == 0)
                return;

            if (GameState.IsPaused)
                return;

            HandleCursorToggle();

            if (!IsCursorVisible())
            {
                UpdateCameraRotationFromMouse(deltaTime);
            }

            hitSparksTimer -= deltaTime;
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!initialized || cameraEntityID == 0 || playerEntityID == 0)
                return;

            if (GameState.IsPaused)
            {
                commandedVelocity = Vector3.Zero;
                Vector3 zero = Vector3.Zero;
                RigidbodySetVelocity(playerEntityID, ref zero);
                return;
            }

            HandleMovementPhysics(deltaTime);
        }

        public override void OnDestroy()
        {
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

        private void HandleCursorToggle()
        {
            if (IsKeyReleased(toggleCursorKey) || IsKeyReleased(KeyCode.Tab))
            {
                bool currentVisible = IsCursorVisible();
                SetCursorVisible(!currentVisible);

                if (!currentVisible) LogMessage("[SpaceshipController] Cursor unlocked");
                else LogMessage("[SpaceshipController] Cursor locked");
            }
        }

        private void UpdateCameraRotationFromMouse(float deltaTime)
        {
            GetMouseDelta(out float dx, out float dy);

            yawRad += -dx * yawSpeed * deltaTime;
            pitchRad += -dy * pitchSpeed * deltaTime;

            float limit = pitchLimitDegrees * SimpleMath.DEG_TO_RAD;
            pitchRad = SimpleMath.Clamp(pitchRad, -limit, limit);

            Quat qYaw = Quat.FromAxisAngle(Vector3.Up, yawRad);
            Quat qPitch = Quat.FromAxisAngle(Vector3.Right, pitchRad);

            Quat camRot = (qYaw * qPitch).Normalized();

            cameraForward = camRot.Forward;
            cameraRight = camRot.Right;
            cameraUp = camRot.Up;

            SetRotation(cameraEntityID, ref camRot);

            Quat modelXOffset = Quat.FromAxisAngle(Vector3.Right, modelXRotationOffset * SimpleMath.DEG_TO_RAD);
            targetPlayerRotation = (camRot * modelXOffset).Normalized();
        }

        private void HandleMovementPhysics(float deltaTime)
        {
            GetCameraMoveAxes(out Vector3 camFwd, out Vector3 camRight, out Vector3 camUp);

            Vector3 moveDir = Vector3.Zero;

            if (IsKeyPressed(KeyCode.W)) moveDir += camFwd;
            if (IsKeyPressed(KeyCode.S)) moveDir -= camFwd;
            if (IsKeyPressed(KeyCode.D)) moveDir += camRight;
            if (IsKeyPressed(KeyCode.A)) moveDir -= camRight;
            if (IsKeyPressed(KeyCode.Space)) moveDir += camUp;
            if (IsKeyPressed(KeyCode.LeftShift)) moveDir -= camUp;

            bool hasInput = moveDir.SqrMagnitude > 1e-8f;
            if (hasInput)
                moveDir = moveDir.Normalized;
            else
                moveDir = Vector3.Zero;

            Vector3 desiredVel = moveDir * spaceshipSpeed;

            // Use actual rigidbody velocity, not only cached commanded velocity.
            Vector3 currentVel = RigidbodyGetVelocity(playerEntityID);

            float maxDelta;
            if (!hasInput)
            {
                maxDelta = deceleration * deltaTime;
            }
            else
            {
                bool reversing = false;

                if (currentVel.SqrMagnitude > 1e-8f)
                {
                    Vector3 currentDir = currentVel.Normalized;
                    float alignment = Vector3.Dot(currentDir, moveDir);
                    reversing = alignment < -0.15f;
                }

                maxDelta = (reversing ? reverseBrakeAcceleration : acceleration) * deltaTime;
            }

            commandedVelocity = MoveTowards(currentVel, desiredVel, maxDelta);
            RigidbodySetVelocity(playerEntityID, ref commandedVelocity);

            // Kill collision-induced spin.
            Vector3 zeroAngular = Vector3.Zero;
            RigidbodySetAngularVelocity(playerEntityID, ref zeroAngular);

            // Keep facing under controller control.
            Quat playerRot = GetRotation(playerEntityID);
            playerRot = RotateTowards(playerRot, targetPlayerRotation, playerRotationSpeed, deltaTime);
            SetRotation(playerEntityID, ref playerRot);

            emitParticles(commandedVelocity);

            Vector3 playerPosNow = GetPosition(playerEntityID);
            Vector3 predictedPlayerPos = playerPosNow + commandedVelocity * deltaTime;

            Vector3 desiredCamPos =
                predictedPlayerPos
                - camFwd * cameraDistanceBack
                + camUp * cameraHeightOffset;

            if (!camFollowInit)
            {
                smoothCamPos = GetPosition(cameraEntityID);
                camFollowInit = true;
            }

            float t = cameraFollowSmooth * deltaTime;
            t = t / (1.0f + t);

            smoothCamPos = Vector3.Lerp(smoothCamPos, desiredCamPos, t);

            if (camShakeTimer < camShakeTimerThreshold)
            {
                camShakeTimer += deltaTime;

                Vector3 xShake = camRight * RNG.RandFloat(-CAMSHAKE_DamageTakenMagnitude, CAMSHAKE_DamageTakenMagnitude) * deltaTime;
                Vector3 yShake = camUp * RNG.RandFloat(-CAMSHAKE_DamageTakenMagnitude, CAMSHAKE_DamageTakenMagnitude) * deltaTime;

                smoothCamPos = smoothCamPos + xShake + yShake;

                if (camShakeTimer >= CAMSHAKE_DamageTakenDuration)
                    camShakeTimer = camShakeTimerThreshold;
            }

            SetPosition(cameraEntityID, ref smoothCamPos);

            Vector3 camPos = GetPosition(cameraEntityID);
            Quat camRot = GetRotation(cameraEntityID);
            Vector3 camForward = camRot.Forward;
            camAimTarget = camPos + camForward * cameraLookDistance;
            SetTarget(cameraEntityID, ref camAimTarget);
        }

        private void OnDamageReceived(string eventName, string payload)
        {
            if (playerHP <= 0 || endscene)
                return;

            float damage = DamageSystem.ParseAmount(payload);
            playerHP -= damage;

            camShakeTimer = 0.0f;

            if (hitSparksTimer <= 0.0f)
            {
                hitSparksTimer = 0.3f;
                tempPlayerHitSparksID = playerHitSparksID;

                if (tempPlayerHitSparksID != INVALID_ENTITY)
                    SceneDestroyEntity(tempPlayerHitSparksID);

                playerHitSparksID = INVALID_ENTITY;
                isHitSparks = false;
            }

            if (playerHitSparksID == INVALID_ENTITY && isHitSparks == false)
            {
                playerHitSparksID = PrefabInstantiate(playerHitSparksPrefabPath);
                isHitSparks = true;
            }

            if (playerHitSparksID != INVALID_ENTITY)
            {
                Vector3 playerPos = GetPosition(playerEntityID);
                Transform.SetPosition(playerHitSparksID, ref playerPos);
            }

            LogMessage("[SPACESHIP CONTROLLER] OnDamageReceived player " + playerEntityID.ToString() +
                       " gets damage! Health: " + playerHP.ToString() + "/" + playerOriginalHP.ToString());
            LogMessage("[SPACESHIP CONTROLLER] OnDamageReceived player from payload" + payload);

            Publish(EVENT_PLAYER_HEALTHCHANGE, playerHP.ToString());

            if (playerHP <= 0)
            {
                Publish("PlayerDead", "");
            }
        }

        private void OnGameEnd(string eventName, string payload)
        {
            LogMessage("[Spaceship Controller] Detect game end condition: " + eventName);
            endscene = true;
        }

        private void emitParticles(Vector3 velocity)
        {
            float currentSpeed = velocity.Magnitude;

            float maxEmissionRate = 80.0f;
            float minEmissionRate = 1.0f;
            float stopThreshold = 1.0f;

            float speedRatio = 0.0f;
            if (spaceshipSpeed > 1e-5f)
                speedRatio = SimpleMath.Clamp(currentSpeed / spaceshipSpeed, 0.0f, 1.0f);

            float emissionRate;
            if (currentSpeed < stopThreshold)
            {
                emissionRate = 0.0f;
            }
            else if (speedRatio < 0.1f)
            {
                emissionRate = minEmissionRate;
            }
            else
            {
                emissionRate = SimpleMath.Lerp(minEmissionRate, maxEmissionRate, speedRatio * speedRatio);
            }

            SetEmissionRate(playerEntityID, emissionRate);

            Vector3 exhaustVelocity = Vector3.Zero;
            if (velocity.SqrMagnitude > 1e-8f)
                exhaustVelocity = -velocity.Normalized * 50.0f;

            SetEmitterVelocity(playerEntityID, ref exhaustVelocity);
        }

        private void GetCameraMoveAxes(out Vector3 forward, out Vector3 right, out Vector3 up)
        {
            Quat camRot = GetRotation(cameraEntityID);

            forward = camRot.Forward;
            if (forward.SqrMagnitude < 1e-8f) forward = Vector3.Forward;
            else forward = forward.Normalized;

            right = camRot.Right;
            if (right.SqrMagnitude < 1e-8f) right = Vector3.Right;
            else right = right.Normalized;

            up = camRot.Up;
            if (up.SqrMagnitude < 1e-8f) up = Vector3.Up;
            else up = up.Normalized;
        }

        private static Vector3 MoveTowards(Vector3 current, Vector3 target, float maxDelta)
        {
            Vector3 delta = target - current;
            float dist = delta.Magnitude;

            if (dist <= 1e-8f || dist <= maxDelta)
                return target;

            return current + (delta / dist) * maxDelta;
        }

        private static Quat RotateTowards(Quat current, Quat target, float maxRadiansPerSec, float dt)
        {
            if (dt <= 0f || maxRadiansPerSec <= 0f)
                return current;

            float dot = Quat.Dot(current, target);
            if (dot < 0f)
            {
                target = new Quat(-target.W, -target.X, -target.Y, -target.Z);
                dot = -dot;
            }

            dot = SimpleMath.Clamp(dot, -1f, 1f);

            float angle = 2f * SimpleMath.Acos(dot);

            if (angle < 1e-5f)
                return target;

            float maxStep = maxRadiansPerSec * dt;

            if (maxStep >= angle)
                return target;

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
                HealPlayer(healAmount);
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