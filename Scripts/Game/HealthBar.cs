using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// HealthBar - Visual representation of player health
    /// Gets health from Player via events and updates width accordingly
    /// Visual settings configured in editor (position, height, max width)
    /// </summary>
    public class HealthBar : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_PLAYER_HEALTHCHANGE = "Damage:";

        // ===== Visual Settings (Set in Editor) =====
        [SerializeField]
        private float barMaxWidth = 400.0f;  // Maximum width at 100% health

        // ===== State =====
        private bool initialized = false;
        private Vector3 initialPosition;  // Store initial center position
        private Vector3 initialLeftEdge;  // Store initial left edge position (the anchor point)
        private float currentHealthPercent = 1.0f;  // Track current health percentage
        private float playerMaxHP = 100.0f;  // Player's actual max HP
        private float hpToWidthRatio = 4.0f;  // 100 HP = 400 width, so ratio is 4

        // Key press tracking to prevent multiple triggers
        private bool hKeyWasPressed = false;
        private bool kKeyWasPressed = false;
        private bool uKeyWasPressed = false;

        // TODO: For smooth lerping (when FixedUpdate is available)
        // private float currentDisplayWidth;
        // private float targetDisplayWidth;
        // [SerializeField]
        // private float lerpSpeed = 5.0f;

        public override void OnStart()
        {
            LogMessage("=== HealthBar OnStart ===");
            LogMessage("HealthBar EntityID: " + EntityID);

            // Subscribe to health update events
            Event.Subscribe(EVENT_PLAYER_HEALTHCHANGE, OnPlayerHealthChange);
            LogMessage("HealthBar: Subscribed to event '" + EVENT_PLAYER_HEALTHCHANGE + "'");
            //Subscribe("PlayerHealthUpdate", OnPlayerHealthUpdate);
            //Subscribe("SMActivated", OnGameStart);

            // Store initial position (center of the bar)
            initialPosition = Transform.GetPosition((uint)EntityID);

            // Get the actual initial scale to determine real width
            Vector3 initialScale = Transform.GetScale((uint)EntityID);
            float actualInitialWidth = initialScale.X;

            // Use the actual width from the scene as barMaxWidth
            barMaxWidth = actualInitialWidth;

            // Calculate the initial left edge position based on actual width
            // Left edge = center X - (actual width / 2)
            initialLeftEdge = new Vector3(
                initialPosition.X - (actualInitialWidth / 2.0f),
                initialPosition.Y,
                initialPosition.Z
            );

            initialized = true;

            LogMessage("HealthBar initialized:");
            LogMessage("  Max Width (from scene): " + barMaxWidth);
            LogMessage("  Initial Center Position X: " + initialPosition.X);
            LogMessage("  Initial Left Edge X: " + initialLeftEdge.X);
            LogMessage("  Distance from left edge to center: " + (initialPosition.X - initialLeftEdge.X));
            LogMessage("  Subscribed to: " + EVENT_PLAYER_HEALTHCHANGE);
            LogMessage("  Player Max HP: " + playerMaxHP + " (will update when receiving health events)");
            LogMessage("  Test Controls: H=damage(10), J=heal(10), K=damage(50), U=full heal");
        }

        public override void OnUpdate(float deltaTime)
        {
            // OnUpdate can be used for visual-only updates if needed
            // All input handling moved to OnFixedUpdate
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!initialized)
                return;

            // ===== CHEAT CODES FOR TESTING HEALTH =====
            // Press H to deal 10 damage to player (reduces by 10 units)
      /*      if (Input.IsKeyPressed(KeyCode.H))
            {
                if (!hKeyWasPressed)
                {
                    hKeyWasPressed = true;
                    Publish("PlayerDamaged", "10");
                    LogMessage("CHEAT: Dealing 10 damage to player!");

                    // Reduce health by 10 units worth
                    float damagePercent = 10.0f / barMaxWidth;
                    currentHealthPercent -= damagePercent;
                    if (currentHealthPercent < 0.0f) currentHealthPercent = 0.0f;

                    UpdateHealthBarVisual(currentHealthPercent);
                }
            }
            else
            {
                hKeyWasPressed = false;
            }*/

            // Press J to heal player 10 HP (increases by 10 units)
           

            // Press K to deal 50 damage to player (simulate botnet attack - reduces by 50 units)
            if (Input.IsKeyPressed(KeyCode.H))
            {
                if (!kKeyWasPressed)
                {
                    kKeyWasPressed = true;
                    Publish("PlayerDamaged", "50");
                    LogMessage("CHEAT: Dealing 50 damage to player!");

                    // Reduce health by 50 units worth
                    float damagePercent = 50.0f / barMaxWidth;
                    currentHealthPercent -= damagePercent;
                    if (currentHealthPercent < 0.0f) currentHealthPercent = 0.0f;

                    UpdateHealthBarVisual(currentHealthPercent);
                }
            }
            else
            {
                hKeyWasPressed = false;
            }

 

            // TODO: For smooth lerping (when FixedUpdate is available)
            // if (Math.Abs(currentDisplayWidth - targetDisplayWidth) > 0.1f)
            // {
            //     currentDisplayWidth = Lerp(currentDisplayWidth, targetDisplayWidth, lerpSpeed * deltaTime);
            //     float currentHealthPercent = currentDisplayWidth / barMaxWidth;
            //     UpdateHealthBarVisual(currentHealthPercent);
            // }
        }

        // ===== EVENT HANDLERS =====

        /// <summary>
        /// Called when player health changes (from SpaceshipController)
        /// Payload is the current HP as a string (e.g., "75.5")
        /// HP is out of 100, Width is out of 400 (4x ratio)
        /// </summary>
        private void OnPlayerHealthChange(string eventName, string payload)
        {
            LogMessage("=== OnPlayerHealthChange CALLED ===");
            LogMessage("  Event Name: " + eventName);
            LogMessage("  Payload: " + payload);

            if (!float.TryParse(payload, out float currentHP))
            {
                LogError("HealthBar: Failed to parse currentHP from payload: " + payload);
                return;
            }

            // Clamp HP to valid range (0-100)
            if (currentHP < 0.0f) currentHP = 0.0f;
            if (currentHP > playerMaxHP) currentHP = playerMaxHP;

            // Convert HP to width (100 HP = 400 width)
            float currentWidth = currentHP * hpToWidthRatio;

            LogMessage("HealthBar: Received health update");
            LogMessage("  Current HP: " + currentHP + " / " + playerMaxHP);
            LogMessage("  Converted Width: " + currentWidth + " / " + barMaxWidth);

            // Get current scale
            Vector3 currentScale = Transform.GetScale((uint)EntityID);

            // Set width based on converted HP value
            Vector3 newScale = new Vector3(
                currentWidth,      // Width = HP * 4
                currentScale.Y,    // Keep height
                currentScale.Z     // Keep depth
            );
            Transform.SetScale((uint)EntityID, ref newScale);

            // Adjust position to keep left edge fixed
            // New center = left edge + (current width / 2)
            Vector3 newPosition = new Vector3(
                initialLeftEdge.X + (currentWidth / 2.0f),
                initialLeftEdge.Y,
                initialLeftEdge.Z
            );
            Transform.SetPosition((uint)EntityID, ref newPosition);

            LogMessage("  New Center X: " + newPosition.X);
            LogMessage("  Left Edge X (fixed): " + initialLeftEdge.X);
        }

        private void OnPlayerHealthUpdate(string eventName, string payload)
        {
            // Parse "currentHealth|maxHealth"
            string[] parts = payload.Split('|');
            if (parts.Length != 2)
            {
                LogError("HealthBar: Invalid payload format: " + payload);
                return;
            }

            if (!float.TryParse(parts[0], out float currentHealth))
            {
                LogError("HealthBar: Failed to parse currentHealth: " + parts[0]);
                return;
            }

            if (!float.TryParse(parts[1], out float maxHealth))
            {
                LogError("HealthBar: Failed to parse maxHealth: " + parts[1]);
                return;
            }

            // Calculate health percentage
            float healthPercent = currentHealth / maxHealth;
            currentHealthPercent = healthPercent;

            // Update visual immediately
            UpdateHealthBarVisual(healthPercent);

            // TODO: For smooth lerping
            // targetDisplayWidth = barMaxWidth * healthPercent;
        }

        private void OnGameStart(string eventName, string payload)
        {
            LogMessage("HealthBar: Game started - waiting for health update from Player");
        }

        // ===== VISUAL UPDATE =====

        private void UpdateHealthBarVisual(float healthPercent)
        {
            // Clamp to valid range
            if (healthPercent < 0.0f) healthPercent = 0.0f;
            if (healthPercent > 1.0f) healthPercent = 1.0f;

            // Calculate current width based on health percentage
            float currentWidth = barMaxWidth * healthPercent;

            // Get current scale (editor sets the height and depth)
            Vector3 currentScale = Transform.GetScale((uint)EntityID);

            // Update width based on health, keep height and depth
            Vector3 newScale = new Vector3(
                currentWidth,      // Width scales with health
                currentScale.Y,    // Keep height from editor
                currentScale.Z     // Keep depth from editor
            );

            Transform.SetScale((uint)EntityID, ref newScale);

            // ===== POSITION ADJUSTMENT TO ANCHOR LEFT EDGE =====
            // The bar's center needs to be at: leftEdge + (currentWidth / 2)
            // This keeps the left edge fixed at initialLeftEdge

            Vector3 newPosition = new Vector3(
                initialLeftEdge.X + (currentWidth / 2.0f),  // Center = left edge + half current width
                initialLeftEdge.Y,                           // Keep Y same
                initialLeftEdge.Z                            // Keep Z same
            );

            Transform.SetPosition((uint)EntityID, ref newPosition);

            // Log for debugging
            LogMessage("HealthBar visual updated:");
            LogMessage("  Health: " + (healthPercent * 100.0f) + "%");
            LogMessage("  Current Width: " + currentWidth + " (max: " + barMaxWidth + ")");
            LogMessage("  New Center Position X: " + newPosition.X);
            LogMessage("  Left Edge X (should be fixed): " + initialLeftEdge.X);
            LogMessage("  Calculated Left Edge X: " + (newPosition.X - (currentWidth / 2.0f)));
        }

        // TODO: Uncomment when FixedUpdate is available
        // private float Lerp(float a, float b, float t)
        // {
        //     if (t < 0.0f) t = 0.0f;
        //     if (t > 1.0f) t = 1.0f;
        //     return a + (b - a) * t;
        // }

        public override void OnDestroy()
        {
            // Clean up event subscriptions
            Event.Unsubscribe(EVENT_PLAYER_HEALTHCHANGE, OnPlayerHealthChange);
            Event.Unsubscribe("PlayerHealthUpdate", OnPlayerHealthUpdate);
            Event.Unsubscribe("SMActivated", OnGameStart);

            LogMessage("=== HealthBar Destroyed ===");
        }
    }
}