using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// CoreHealthBar - Visual representation of core/motherboard health
    /// Position adjustment: Move by FULL width change (not half)
    /// Left edge = Position.X - Width (scale.X is radius, not diameter)
    /// Attach this to the core health bar sprite entity
    /// NOTE: There is only one core in the game, so we use a simple global event
    /// </summary>
    public class CoreHealthBar : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_CORE_HEALTHCHANGE = "Core Health Change";

        // ===== Visual Settings =====
        private float barMaxWidth;  // Maximum width at 100% health (set from scene)

        // ===== State =====
        private bool initialized = false;
        private Vector3 initialPosition;  // Store initial center position
        private float initialWidth;  // Store initial width to calculate offset
        private float hpToWidthRatio;  // Calculated: barMaxWidth / 100

        public override void OnStart()
        {
            LogMessage("=== CoreHealthBar OnStart ===");
            LogMessage("CoreHealthBar EntityID: " + EntityID);

            // Subscribe to core health update events
            Event.Subscribe(EVENT_CORE_HEALTHCHANGE, OnCoreHealthChange);
            LogMessage("CoreHealthBar: Subscribed to event '" + EVENT_CORE_HEALTHCHANGE + "'");

            // Store initial position and width
            initialPosition = Transform.GetPosition((uint)EntityID);

            // Get the actual initial scale to determine real width
            Vector3 initialScale = Transform.GetScale((uint)EntityID);
            float actualInitialWidth = initialScale.X;

            // Use the actual width from the scene as barMaxWidth
            barMaxWidth = actualInitialWidth;
            initialWidth = actualInitialWidth;

            // Calculate ratio: Since max HP is always 100, ratio = barMaxWidth / 100
            hpToWidthRatio = barMaxWidth / 100.0f;

            initialized = true;

            LogMessage("CoreHealthBar initialized:");
            LogMessage("  Max Width (from scene): " + barMaxWidth);
            LogMessage("  Initial Position X: " + initialPosition.X);
            LogMessage("  HP to Width Ratio: " + hpToWidthRatio);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized)
                return;
        }

        // ===== EVENT HANDLERS =====

        private void OnCoreHealthChange(string eventName, string payload)
        {
            LogMessage("=== OnCoreHealthChange CALLED ===");
            LogMessage("  Payload: '" + payload + "'");

            if (!float.TryParse(payload, out float currentHP))
            {
                LogError("CoreHealthBar: Failed to parse currentHP from payload: " + payload);
                return;
            }

            // Clamp HP to valid range (0-100)
            if (currentHP < 0.0f) currentHP = 0.0f;
            if (currentHP > 100.0f) currentHP = 100.0f;

            // Convert HP to width (ratio calculated from bar width / 100)
            float currentWidth = currentHP * hpToWidthRatio;

            LogMessage("  Current Core HP: " + currentHP + " -> Width: " + currentWidth);

            // Get current scale
            Vector3 currentScale = Transform.GetScale((uint)EntityID);

            // Set width based on converted HP value
            Vector3 newScale = new Vector3(
                currentWidth,      // Width = HP * ratio
                currentScale.Y,    // Keep height
                currentScale.Z     // Keep depth
            );
            Transform.SetScale((uint)EntityID, ref newScale);

            // Adjust position to keep LEFT edge fixed
            // Position moves by FULL width difference (not half)
            float widthDifference = initialWidth - currentWidth;
            Vector3 newPosition = new Vector3(
                initialPosition.X - widthDifference,
                initialPosition.Y,
                initialPosition.Z
            );
            Transform.SetPosition((uint)EntityID, ref newPosition);

            LogMessage("  Position offset: " + widthDifference);
            LogMessage("=== OnCoreHealthChange COMPLETE ===");
        }

        public override void OnDestroy()
        {
            // Clean up event subscriptions
            Event.Unsubscribe(EVENT_CORE_HEALTHCHANGE, OnCoreHealthChange);

            LogMessage("=== CoreHealthBar Destroyed ===");
        }
    }
}