using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;
using static Engine.SpriteRenderer;

namespace Game
{
    public class DamageBar : ScriptBehaviour
    {
        private const string EVENT_HEALING = "HealthBarHealing";
        private const string EVENT_DAMAGED = "HealthBarDamaged";
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";
        private const string EVENT_TIMER_FINISHED = "TimerFinished";
        private const string EVENT_ENEMY_CORE_DEATH = "EnemyCoreDeath";
        private const string EVENT_GAME_WIN = "GameWin";

        private const float LERP_DURATION = 0.5f;

        private float barMaxWidth;
        private bool initialized = false;
        private Vector3 initialPosition;
        private float initialWidth;
        private float playerMaxHP = 100.0f;
        private float currentHP = 100.0f;
        private float hpToWidthRatio;

        private bool isLerping = false;
        private float lerpTimer = 0.0f;
        private float startWidth = 0.0f;
        private float targetWidth = 0.0f;
        private float currentWidth = 0.0f;

        public override void OnStart()
        {
            LogMessage("=== DamageBar OnStart ===");
            Event.Subscribe(EVENT_HEALING, OnHealing);
            Event.Subscribe(EVENT_DAMAGED, OnDamaged);
            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameEnd);
            Event.Subscribe(EVENT_ENEMY_CORE_DEATH, OnGameEnd);
            Event.Subscribe(EVENT_GAME_WIN, OnGameEnd);

            initialPosition = Transform.GetPosition((uint)EntityID);
            Vector3 initialScale = Transform.GetScale((uint)EntityID);

            barMaxWidth = initialScale.X;
            initialWidth = initialScale.X;
            hpToWidthRatio = barMaxWidth / playerMaxHP;

            currentHP = playerMaxHP;
            currentWidth = initialWidth;
            isLerping = false;
            lerpTimer = 0.0f;
            startWidth = initialWidth;
            targetWidth = initialWidth;

            UpdateBarVisual(currentWidth);
            SetIsVisible((uint)EntityID, true);

            initialized = true;
            LogMessage("DamageBar initialized - Max Width: " + barMaxWidth + " HP: " + currentHP);
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
                    LogMessage("DamageBar: Lerp complete at width " + currentWidth);
                }
                else
                {
                    currentWidth = Lerp(startWidth, targetWidth, t);
                }

                UpdateBarVisual(currentWidth);
            }
        }

        private void OnDamaged(string eventName, string payload)
        {
            if (!float.TryParse(payload, out float newHP)) return;
            if (newHP < 0.0f) newHP = 0.0f;
            if (newHP > playerMaxHP) newHP = playerMaxHP;

            float newTargetWidth = newHP * hpToWidthRatio;

            // ===== DAMAGE: hold current width, lerp down =====
            startWidth = currentWidth;
            targetWidth = newTargetWidth;
            lerpTimer = 0.0f;
            isLerping = true;
            currentHP = newHP;

            SetIsVisible((uint)EntityID, true);
            LogMessage("DamageBar: Damage - lerping " + startWidth + " -> " + targetWidth);
        }

        private void OnHealing(string eventName, string payload)
        {
            if (!float.TryParse(payload, out float newHP)) return;
            if (newHP < 0.0f) newHP = 0.0f;
            if (newHP > playerMaxHP) newHP = playerMaxHP;

            // ===== HEAL: snap instantly, hide red bar =====
            isLerping = false;
            currentWidth = newHP * hpToWidthRatio;
            currentHP = newHP;
            UpdateBarVisual(currentWidth);
            SetIsVisible((uint)EntityID, false);
            LogMessage("DamageBar: Heal - snapped and hidden");
        }

        private void OnGameEnd(string eventName, string payload)
        {
            LogMessage("DamageBar: Game ended (" + eventName + ") - hiding bar");
            isLerping = false;
            SetIsVisible((uint)EntityID, false);
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
            Event.Unsubscribe(EVENT_HEALING, OnHealing);
            Event.Unsubscribe(EVENT_DAMAGED, OnDamaged);
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameEnd);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameEnd);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameEnd);
            Event.Unsubscribe(EVENT_ENEMY_CORE_DEATH, OnGameEnd);
            Event.Unsubscribe(EVENT_GAME_WIN, OnGameEnd);
            LogMessage("=== DamageBar Destroyed ===");
        }
    }
}
