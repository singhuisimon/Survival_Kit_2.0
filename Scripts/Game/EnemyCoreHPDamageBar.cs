using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game{

     public class EnemyCoreHPDamageBar : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_ENEMYCORE_HEALTHCHANGE = "EnemyCore Health Change";

        // ===== Visual Settings =====
        private float barMaxWidth;  // Maximum width at 100% health (set from scene)
        private const float LERP_DURATION = 0.5f;  // Time to lerp in seconds

        // ===== State =====
        private bool initialized = false;
        private Vector3 initialPosition;  // Store initial center position
        private float initialWidth;       // Store initial width to calculate offset
        private float enemyCoreMaxHP = 600.0f;  // EnemyCore's max HP
        private float hpToWidthRatio;     // Ratio: barMaxWidth / enemyCoreMaxHP

        // ===== Lerp State =====
        private bool isLerping = false;
        private float lerpTimer = 0.0f;
        private float startWidth = 0.0f;   // Width at start of lerp
        private float targetWidth = 0.0f;  // Width to lerp to
        private float currentWidth = 0.0f; // Current lerped width

        public override void OnStart()
        {
            LogMessage("=== EnemyCoreDamageBar OnStart ===");
            LogMessage("EnemyCoreDamageBar EntityID: " + EntityID);

            // Subscribe to EnemyCore health update events
            Event.Subscribe(EVENT_ENEMYCORE_HEALTHCHANGE, OnEnemyCoreHealthChange);
            LogMessage("EnemyCoreDamageBar: Subscribed to event '" + EVENT_ENEMYCORE_HEALTHCHANGE + "'");

            // Store initial position and width
            initialPosition = Transform.GetPosition((uint)EntityID);

            // Get the actual initial scale to determine real width
            Vector3 initialScale = Transform.GetScale((uint)EntityID);
            float actualInitialWidth = initialScale.X;

            // Use the actual width from the scene as barMaxWidth
            barMaxWidth = actualInitialWidth;
            initialWidth = actualInitialWidth;
            currentWidth = actualInitialWidth;
            hpToWidthRatio = barMaxWidth / enemyCoreMaxHP;

            initialized = true;

            LogMessage("EnemyCoreDamageBar initialized:");
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
                    LogMessage("EnemyCoreDamageBar: Lerp complete at width " + currentWidth);
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

        private void OnEnemyCoreHealthChange(string eventName, string payload)
        {
            LogMessage("=== EnemyCoreDamageBar: OnEnemyCoreHealthChange CALLED ===");
            LogMessage("  Payload: '" + payload + "'");

            if (!float.TryParse(payload, out float newHP))
            {
                LogError("EnemyCoreDamageBar: Failed to parse HP from payload: " + payload);
                return;
            }

            // Clamp HP to valid range (0 - enemyCoreMaxHP)
            if (newHP < 0.0f) newHP = 0.0f;
            if (newHP > enemyCoreMaxHP) newHP = enemyCoreMaxHP;

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
            Event.Unsubscribe(EVENT_ENEMYCORE_HEALTHCHANGE, OnEnemyCoreHealthChange);

            LogMessage("=== EnemyCoreDamageBar Destroyed ===");
        }
    }
}