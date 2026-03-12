using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// HealthBar - Snaps DOWN instantly on damage, lerps UP slowly on heal.
    /// Mirror of DamageBar which lerps down on damage.
    /// </summary>
    public class HealthBar : ScriptBehaviour
    {
        private const string EVENT_PLAYER_HEALTHCHANGE = "Health Change";
        private const float LERP_DURATION = 0.5f;

        private float barMaxWidth;
        private bool initialized = false;
        private Vector3 initialPosition;
        private float initialWidth;
        private float playerMaxHP = 100.0f;
        private float currentHP = 100.0f;  // track actual HP
        private float hpToWidthRatio;

        // ===== Lerp State (heal only) =====
        private bool isLerping = false;
        private float lerpTimer = 0.0f;
        private float startWidth = 0.0f;
        private float targetWidth = 0.0f;
        private float currentWidth = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== HealthBar OnStart ===");
            Event.Subscribe(EVENT_PLAYER_HEALTHCHANGE, OnPlayerHealthChange);

            initialPosition = Transform.GetPosition((uint)EntityID);
            Vector3 initialScale = Transform.GetScale((uint)EntityID);

            barMaxWidth = initialScale.X;
            initialWidth = initialScale.X;
            hpToWidthRatio = barMaxWidth / playerMaxHP;

            // ===== Always reset to full HP on start =====
            currentHP = playerMaxHP;
            currentWidth = initialWidth;
            isLerping = false;
            lerpTimer = 0.0f;

            UpdateBarVisual(currentWidth);

            initialized = true;
            LogMessage("HealthBar initialized - Max Width: " + barMaxWidth + " HP: " + currentHP);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized) return;

            if (isLerping)
            {
                lerpTimer += deltaTime;
                float t = lerpTimer / LERP_DURATION;

                if (t >= 1.0f)
                {
                    t = 1.0f;
                    isLerping = false;
                    currentWidth = targetWidth;
                    LogMessage("HealthBar: Lerp complete at width " + currentWidth);
                }
                else
                {
                    currentWidth = Lerp(startWidth, targetWidth, t);
                }

                UpdateBarVisual(currentWidth);
            }
        }

        private void OnPlayerHealthChange(string eventName, string payload)
        {
            LogMessage("=== HealthBar: OnPlayerHealthChange ===");

            if (!float.TryParse(payload, out float newHP))
            {
                LogError("HealthBar: Failed to parse HP: " + payload);
                return;
            }

            if (newHP < 0.0f) newHP = 0.0f;
            if (newHP > playerMaxHP) newHP = playerMaxHP;

            float newTargetWidth = newHP * hpToWidthRatio;

            if (newTargetWidth > currentWidth)
            {
                // ===== HEAL: lerp UP =====
                startWidth = currentWidth;
                targetWidth = newTargetWidth;
                lerpTimer = 0.0f;
                isLerping = true;
                LogMessage("HealthBar: Heal - lerping " + startWidth + " -> " + targetWidth + " (HP: " + currentHP + " -> " + newHP + ")");
            }
            else
            {
                // ===== DAMAGE: snap DOWN instantly =====
                isLerping = false;
                currentWidth = newTargetWidth;
                UpdateBarVisual(currentWidth);
                LogMessage("HealthBar: Damage - snapped to " + currentWidth + " (HP: " + currentHP + " -> " + newHP + ")");
            }

            currentHP = newHP;
        }

        private void UpdateBarVisual(float width)
        {
            Vector3 currentScale = Transform.GetScale((uint)EntityID);
            Vector3 newScale = new Vector3(width, currentScale.Y, currentScale.Z);
            Transform.SetScale((uint)EntityID, ref newScale);

            float widthDifference = initialWidth - width;
            Vector3 newPosition = new Vector3(
                initialPosition.X - widthDifference,
                initialPosition.Y,
                initialPosition.Z
            );
            Transform.SetPosition((uint)EntityID, ref newPosition);
        }

        private float Lerp(float a, float b, float t) { return a + (b - a) * t; }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_PLAYER_HEALTHCHANGE, OnPlayerHealthChange);
            LogMessage("=== HealthBar Destroyed ===");
        }
    }
}
