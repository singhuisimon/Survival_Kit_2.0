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
        // ===== Visual Settings (Set in Editor) =====
        [SerializeField]
        private float barMaxWidth = 200.0f;  // Maximum width at 100% health

        // ===== State =====
        private bool initialized = false;

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
            //Subscribe("PlayerHealthUpdate", OnPlayerHealthUpdate);
            //Subscribe("SMActivated", OnGameStart);
            barMaxWidth = 200.0f;
            initialized = true;

            LogMessage("HealthBar initialized:");
            LogMessage("  Max Width: " + barMaxWidth);
            LogMessage("  Listening for PlayerHealthUpdate events");
            LogMessage("  Test Controls: H=damage(10), J=heal(10), K=damage(50), U=full heal");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized)
                return;

            // ===== CHEAT CODES FOR TESTING HEALTH =====
            // Press H to deal 10 damage to player
            if (Input.IsKeyPressed(KeyCode.H))
            {
                Publish("PlayerDamaged", "10");
                LogMessage("CHEAT: Dealing 10 damage to player!");
                if (barMaxWidth <= 0) return;
                barMaxWidth -= 20;
                Vector3 currentScale = GetScale((uint)EntityID);
                Vector3 newScale = new Vector3(
         barMaxWidth,      // Width scales with health
         currentScale.Y,    // Keep height from editor
         currentScale.Z     // Keep depth from editor
     );

                SetScale((uint)EntityID, ref newScale);

            }

            // Press J to heal player 10 HP
            if (Input.IsKeyPressed(KeyCode.J))
            {
                Publish("PlayerHealed", "10");
                LogMessage("CHEAT: Healing player 10 HP!");
            }

            // Press K to deal 50 damage to player (simulate botnet attack)
            if (Input.IsKeyPressed(KeyCode.K))
            {
                Publish("PlayerDamaged", "50");
                LogMessage("CHEAT: Dealing 50 damage to player!");
            }

            // Press U to reset player to full health
            if (Input.IsKeyPressed(KeyCode.U))
            {
                Publish("PlayerHealed", "1000");  // Heal a lot to ensure full health
                LogMessage("CHEAT: Resetting player to full health!");
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

            // Calculate current width
            float currentWidth = barMaxWidth * healthPercent;

            // Get current scale (editor sets the height and depth)
            Vector3 currentScale = GetScale((uint)EntityID);

            // Update width based on health, keep height and depth
            Vector3 newScale = new Vector3(
                currentWidth,      // Width scales with health
                currentScale.Y,    // Keep height from editor
                currentScale.Z     // Keep depth from editor
            );

            SetScale((uint)EntityID, ref newScale);

            // Log for debugging
            LogMessage("HealthBar visual updated:");
            LogMessage("  Health: " + (healthPercent * 100.0f) + "%");
            LogMessage("  Width: " + currentWidth + " (max: " + barMaxWidth + ")");
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
            Unsubscribe("PlayerHealthUpdate", OnPlayerHealthUpdate);
            Unsubscribe("SMActivated", OnGameStart);

            LogMessage("=== HealthBar Destroyed ===");
        }
    }
}