using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    public class EnemyCoreHPBar : ScriptBehaviour
    {
        private const string EVENT_ENEMYCORE_HEALTHCHANGE = "EnemyCore Health Change";
        private const float MaxHealth = 100.0f;

        private bool initialized = false;
        private Vector3 initialPosition;
        private float initialWidth;
        private float hpToWidthRatio;

        public override void OnStart()
        {
            LogMessage("=== EnemyCoreHPBar OnStart ===");

            Event.Subscribe(EVENT_ENEMYCORE_HEALTHCHANGE, OnHealthChange);

            initialPosition = Transform.GetPosition((uint)EntityID);
            Vector3 initialScale = Transform.GetScale((uint)EntityID);
            initialWidth = initialScale.X;
            hpToWidthRatio = initialWidth / MaxHealth;

            initialized = true;
            LogMessage("EnemyCoreHPBar initialized. MaxWidth: " + initialWidth + " Ratio: " + hpToWidthRatio);
        }

        private void OnHealthChange(string eventName, string payload)
        {
            if (!initialized) return;

            if (!float.TryParse(payload, out float currentHP))
            {
                LogError("[EnemyCoreHPBar] Failed to parse HP: " + payload);
                return;
            }

            if (currentHP < 0.0f) currentHP = 0.0f;
            if (currentHP > MaxHealth) currentHP = MaxHealth;

            float currentWidth = currentHP * hpToWidthRatio;

            Vector3 currentScale = Transform.GetScale((uint)EntityID);
            Vector3 newScale = new Vector3(currentWidth, currentScale.Y, currentScale.Z);
            Transform.SetScale((uint)EntityID, ref newScale);

            float widthDifference = initialWidth - currentWidth;
            Vector3 newPosition = new Vector3(
                initialPosition.X - widthDifference,
                initialPosition.Y,
                initialPosition.Z
            );
            Transform.SetPosition((uint)EntityID, ref newPosition);

            LogMessage("[EnemyCoreHPBar] HP: " + currentHP + " -> Width: " + currentWidth);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_ENEMYCORE_HEALTHCHANGE, OnHealthChange);
        }
    }
}