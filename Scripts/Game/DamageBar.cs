using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// DamageBar - Red bar that smoothly lerps behind the white health bar
    /// Attach this to the DamageBarFill (red bar) entity
    /// Shows delayed damage visualization by lerping over 0.5 seconds
    /// </summary>
    public class DamageBar : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_PLAYER_HEALTHCHANGE = "Health Change";

        // ===== Visual Settings =====
        private float barMaxWidth;  // Maximum width at 100% health (set from scene)
        private const float LERP_DURATION = 0.5f;  // Time to lerp in seconds

        // ===== State =====
        private bool initialized = false;
        private Vector3 initialPosition;  // Store initial center position
        private float initialWidth;  // Store initial width to calculate offset
        private float playerMaxHP = 100.0f;  // Player's actual max HP
        private float hpToWidthRatio;  // Ratio: barMaxWidth / playerMaxHP

        // ===== Lerp State =====
        private bool isLerping = false;
        private float lerpTimer = 0.0f;
        private float startWidth = 0.0f;   // Width at start of lerp
        private float targetWidth = 0.0f;  // Width to lerp to
        private float currentWidth = 0.0f; // Current lerped width

        public override void OnStart()
        {
            LogMessage("=== DamageBar OnStart ===");
            LogMessage("DamageBar EntityID: " + EntityID);

            Event.Subscribe(EVENT_PLAYER_HEALTHCHANGE, OnPlayerHealthChange);
            LogMessage("DamageBar: Subscribed to event '" + EVENT_PLAYER_HEALTHCHANGE + "'");

            initialPosition = Transform.GetPosition((uint)EntityID);

            Vector3 initialScale = Transform.GetScale((uint)EntityID);
            float actualInitialWidth = initialScale.X;

            barMaxWidth = actualInitialWidth;
            initialWidth = actualInitialWidth;
            currentWidth = actualInitialWidth;
            hpToWidthRatio = barMaxWidth / playerMaxHP;

            // Reset visual to full width in case scene was restarted mid-damage
            UpdateBarVisual(currentWidth);

            initialized = true;

            LogMessage("DamageBar initialized:");
            LogMessage("  Max Width (from scene): " + barMaxWidth);
            LogMessage("  Initial Position X: " + initialPosition.X);
            LogMessage("  HP to Width Ratio: " + hpToWidthRatio);
            LogMessage("  Lerp Duration: " + LERP_DURATION + "s");
        }


        public override void OnUpdate(float deltaTime)
        {
            if (!initialized)
                return;

            // Update lerp if active
            if (isLerping)
            {
                lerpTimer += deltaTime;
                float t = lerpTimer / LERP_DURATION;

                if (t >= 1.0f)
                {
                    // Lerp complete
                    t = 1.0f;
                    isLerping = false;
                    currentWidth = targetWidth;
                    LogMessage("DamageBar: Lerp complete at width " + currentWidth);
                }
                else
                {
                    // Lerp between start and target
                    currentWidth = Lerp(startWidth, targetWidth, t);
                }

                // Update visual
                UpdateBarVisual(currentWidth);
            }
        }

        // ===== EVENT HANDLERS =====

        private void OnPlayerHealthChange(string eventName, string payload)
        {
            LogMessage("=== DamageBar: OnPlayerHealthChange CALLED ===");
            LogMessage("  Payload: '" + payload + "'");

            if (!float.TryParse(payload, out float newHP))
            {
                LogError("DamageBar: Failed to parse HP from payload: " + payload);
                return;
            }

            // Clamp HP to valid range (0-100)
            if (newHP < 0.0f) newHP = 0.0f;
            if (newHP > playerMaxHP) newHP = playerMaxHP;

            // Convert HP to width
            float newTargetWidth = newHP * hpToWidthRatio;

            LogMessage("  New HP: " + newHP + " -> Target Width: " + newTargetWidth);
            LogMessage("  Current Width: " + currentWidth);

            // Start new lerp (or update existing lerp target)
            startWidth = currentWidth;  // Start from wherever we currently are
            targetWidth = newTargetWidth;
            lerpTimer = 0.0f;
            isLerping = true;

            LogMessage("  Starting lerp: " + startWidth + " -> " + targetWidth);
        }

        // ===== HELPER METHODS =====

        private void UpdateBarVisual(float width)
        {
            // Get current scale
            Vector3 currentScale = Transform.GetScale((uint)EntityID);

            // Set width
            Vector3 newScale = new Vector3(
                width,
                currentScale.Y,
                currentScale.Z
            );
            Transform.SetScale((uint)EntityID, ref newScale);

            // Adjust position to keep LEFT edge fixed
            float widthDifference = initialWidth - width;
            Vector3 newPosition = new Vector3(
                initialPosition.X - widthDifference,
                initialPosition.Y,
                initialPosition.Z
            );
            Transform.SetPosition((uint)EntityID, ref newPosition);
        }

        private float Lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }

        public override void OnDestroy()
        {
            // Clean up event subscriptions
            Event.Unsubscribe(EVENT_PLAYER_HEALTHCHANGE, OnPlayerHealthChange);

            LogMessage("=== DamageBar Destroyed ===");
        }
    }
}
