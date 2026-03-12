using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// HealBar - Green bar that snaps UP instantly on heal, lerps DOWN on damage.
    /// Mirror of DamageBar. Attach to a green HealBarFill entity.
    /// Layer order managed in editor, not in code.
    /// </summary>
    public class HealBar : ScriptBehaviour
    {
        private const string EVENT_PLAYER_HEALTHCHANGE = "Health Change";
        private const float LERP_DURATION = 0.5f; // Must match HealthBar lerp duration

        private float barMaxWidth;
        private bool initialized = false;
        private Vector3 initialPosition;
        private float initialWidth;
        private float playerMaxHP = 100.0f;
        private float hpToWidthRatio;

        // ===== Lerp State (damage only) =====
        private bool isLerping = false;
        private float lerpTimer = 0.0f;
        private float startWidth = 0.0f;
        private float targetWidth = 0.0f;
        private float currentWidth = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== HealBar OnStart ===");
            Event.Subscribe(EVENT_PLAYER_HEALTHCHANGE, OnPlayerHealthChange);

            initialPosition = Transform.GetPosition((uint)EntityID);
            Vector3 initialScale = Transform.GetScale((uint)EntityID);

            barMaxWidth = initialScale.X;
            initialWidth = initialScale.X;
            currentWidth = initialScale.X;
            hpToWidthRatio = barMaxWidth / playerMaxHP;

            initialized = true;
            LogMessage("HealBar initialized - Max Width: " + barMaxWidth);
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
                    LogMessage("HealBar: Lerp complete at width " + currentWidth);
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
            LogMessage("=== HealBar: OnPlayerHealthChange ===");

            if (!float.TryParse(payload, out float newHP))
            {
                LogError("HealBar: Failed to parse HP: " + payload);
                return;
            }

            if (newHP < 0.0f) newHP = 0.0f;
            if (newHP > playerMaxHP) newHP = playerMaxHP;

            float newTargetWidth = newHP * hpToWidthRatio;

            if (newTargetWidth > currentWidth)
            {
                // ===== HEAL: snap UP instantly =====
                isLerping = false;
                currentWidth = newTargetWidth;
                UpdateBarVisual(currentWidth);
                LogMessage("HealBar: Heal - snapped to " + currentWidth);
            }
            else
            {
                // ===== DAMAGE: snap DOWN instantly =====
                isLerping = false;
                currentWidth = newTargetWidth;
                UpdateBarVisual(currentWidth);
                LogMessage("HealBar: Damage - snapped to " + currentWidth);
            }

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
            LogMessage("=== HealBar Destroyed ===");
        }
    }
}
