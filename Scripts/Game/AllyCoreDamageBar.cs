using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;
using static Engine.SpriteRenderer;

namespace Game
{
    /// <summary>
    /// AllyCoreDamageBar - Red flash bar overlaid on the ally core's white health bar.
    /// Listens to the same "Core Health Change" event as CoreHealthBar.
    /// On damage:
    ///   1. Snaps to pre-damage width instantly and becomes visible
    ///   2. Holds (flashes red) for FLASHDURATION (0.3s)
    ///   3. Lerps down to new HP width over LERPDURATION (0.5s)
    ///   4. Hides itself once lerp completes
    /// On heal: snaps to new width and hides immediately.
    /// </summary>
    public class AllyCoreDamageBar : ScriptBehaviour
    {
        // Same event as CoreHealthBar uses
        private const string EVENTCOREHEALTHCHANGE = "Core Health Change";
        private const string EVENTCOREDESTROYED = "CoreMotherboardDestroyed";
        private const string EVENTTIMERFINISHED = "TimerFinished";
        private const string EVENTENEMYCOREDEATH = "EnemyCoreDeath";
        private const string EVENTGAMEWIN = "GameWin";

        private const float FLASHDURATION = 0.1f;
        private const float LERPDURATION = 0.1f;

        // Bar settings
        private float barMaxWidth;
        private float initialWidth;
        private float hpToWidthRatio;
        private float currentHP = 100.0f;

        // Position
        private Vector3 initialPosition;

        // State
        private bool initialized = false;

        // Flash state
        private bool isFlashing = false;
        private float flashTimer = 0.0f;

        // Lerp state
        private bool isLerping = false;
        private float lerpTimer = 0.0f;
        private float startWidth = 0.0f;
        private float targetWidth = 0.0f;
        private float currentWidth = 0.0f;

        public override void OnStart()
        {
            LogMessage("AllyCoreDamageBar: OnStart");
            LogMessage("AllyCoreDamageBar: EntityID " + EntityID);

            Event.Subscribe(EVENTCOREHEALTHCHANGE, OnCoreHealthChange);
            Event.Subscribe(EVENTCOREDESTROYED, OnGameEnd);
            Event.Subscribe(EVENTTIMERFINISHED, OnGameEnd);
            Event.Subscribe(EVENTENEMYCOREDEATH, OnGameEnd);
            Event.Subscribe(EVENTGAMEWIN, OnGameEnd);

            initialPosition = Transform.GetPosition((uint)EntityID);
            Vector3 initialScale = Transform.GetScale((uint)EntityID);

            barMaxWidth = initialScale.X;
            initialWidth = initialScale.X;
            hpToWidthRatio = barMaxWidth / 100.0f;
            currentHP = 100.0f;
            currentWidth = initialWidth;
            startWidth = initialWidth;
            targetWidth = initialWidth;

            UpdateBarVisual(currentWidth);

            // Starts hidden - only appears on damage
            SetIsVisible((uint)EntityID, false);

            initialized = true;
            LogMessage("AllyCoreDamageBar: Initialized - Max Width=" + barMaxWidth + " HP=" + currentHP);
            LogMessage("AllyCoreDamageBar: HP to Width Ratio=" + hpToWidthRatio);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized) return;
            if (GameState.IsPaused) return;

            // Phase 1: hold at pre-damage width for flash duration
            if (isFlashing)
            {
                flashTimer += deltaTime;
                if (flashTimer >= FLASHDURATION)
                {
                    isFlashing = false;
                    isLerping = true;
                    lerpTimer = 0.0f;
                    startWidth = currentWidth;
                    LogMessage("AllyCoreDamageBar: Flash done - lerping " + startWidth + " -> " + targetWidth);
                }
                return;
            }

            // Phase 2: lerp down to post-damage width
            if (isLerping)
            {
                lerpTimer += deltaTime;
                float t = lerpTimer / LERPDURATION;
                if (t >= 1.0f)
                {
                    t = 1.0f;
                    isLerping = false;
                    currentWidth = targetWidth;
                    UpdateBarVisual(currentWidth);
                    SetIsVisible((uint)EntityID, false);
                    LogMessage("AllyCoreDamageBar: Lerp complete - bar hidden");
                }
                else
                {
                    currentWidth = Lerp(startWidth, targetWidth, t);
                    UpdateBarVisual(currentWidth);
                }
            }
        }

        private void OnCoreHealthChange(string eventName, string payload)
        {
            LogMessage("AllyCoreDamageBar: OnCoreHealthChange CALLED - payload=" + payload);

            if (!float.TryParse(payload, out float newHP))
            {
                LogError("AllyCoreDamageBar: Failed to parse HP from payload: " + payload);
                return;
            }

            if (newHP < 0.0f) newHP = 0.0f;
            if (newHP > 100.0f) newHP = 100.0f;

            float newWidth = newHP * hpToWidthRatio;

            if (newHP >= currentHP)
            {
                // Healing - snap to new width and hide, no flash
                isFlashing = false;
                isLerping = false;
                currentHP = newHP;
                currentWidth = newWidth;
                UpdateBarVisual(currentWidth);
                SetIsVisible((uint)EntityID, false);
                LogMessage("AllyCoreDamageBar: Heal - snapped to " + currentWidth + " and hidden");
                return;
            }

            // Damage - snap red bar to PRE-damage width, flash, then lerp to new width
            float preDamageWidth = currentHP * hpToWidthRatio;

            LogMessage("AllyCoreDamageBar: Damage - HP " + currentHP + " -> " + newHP +
                       " | Width " + preDamageWidth + " -> " + newWidth);

            currentHP = newHP;
            targetWidth = newWidth;
            currentWidth = preDamageWidth;
            startWidth = preDamageWidth;

            UpdateBarVisual(currentWidth);
            SetIsVisible((uint)EntityID, true);

            // Begin flash phase
            isFlashing = true;
            isLerping = false;
            flashTimer = 0.0f;
            lerpTimer = 0.0f;
        }

        private void OnGameEnd(string eventName, string payload)
        {
            LogMessage("AllyCoreDamageBar: Game ended (" + eventName + ") - hiding bar");
            isFlashing = false;
            isLerping = false;
            SetIsVisible((uint)EntityID, false);
        }

        private void UpdateBarVisual(float width)
        {
            Vector3 currentScale = Transform.GetScale((uint)EntityID);
            Vector3 newScale = new Vector3(width, currentScale.Y, currentScale.Z);
            Transform.SetScale((uint)EntityID, ref newScale);

            float widthDifference = initialWidth - width;
            Vector3 newPosition = new Vector3(initialPosition.X - widthDifference, initialPosition.Y, initialPosition.Z);
            Transform.SetPosition((uint)EntityID, ref newPosition);
        }

        private float Lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENTCOREHEALTHCHANGE, OnCoreHealthChange);
            Event.Unsubscribe(EVENTCOREDESTROYED, OnGameEnd);
            Event.Unsubscribe(EVENTTIMERFINISHED, OnGameEnd);
            Event.Unsubscribe(EVENTENEMYCOREDEATH, OnGameEnd);
            Event.Unsubscribe(EVENTGAMEWIN, OnGameEnd);
            LogMessage("AllyCoreDamageBar: Destroyed");
        }
    }
}
