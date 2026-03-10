using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;
using static Engine.Input;
using static Engine.Scene;

namespace Game
{
    public class GunshipHealthBar : ScriptBehaviour
    {
        [SerializeField("Ally Index")]
        private int allyIndex = 1;

        private float currentHealth = 75.0f;
        private float maxHealth = 75.0f;

        private string healthEventName = "";
        private bool initialized = false;

        private Vector3 baseScale;
        private Vector3 initialPosition;

        public override void OnStart()
        {
            LogMessage("=== GunshipHealthBar OnStart ===");
            LogMessage("[GunshipHealthBar] EntityID: " + EntityID + " allyIndex=" + allyIndex);

            // Read editor-set scale and position as base reference
            baseScale = GetScale((uint)EntityID);
            initialPosition = GetPosition((uint)EntityID);

            healthEventName = "GunshipHealthChanged:" + allyIndex;
            Subscribe(healthEventName, OnHealthChanged);

            UpdateBarScale();

            initialized = true;
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized)
                return;

            // DEBUG: Press 9 to send damage event
            if (Input.IsKeyPressed(KeyCode.D9))
            {
                LogMessage("[GunshipHealthBar] DEBUG - sending damage event for allyIndex " + allyIndex);
                Publish("DebugDamageGunship:" + allyIndex, "10");
            }
        }

        private void OnHealthChanged(string eventName, string payload)
        {
            ParseHealthPayload(payload, out currentHealth, out maxHealth);
            LogMessage("[GunshipHealthBar] (" + allyIndex + ") Health: " + currentHealth + "/" + maxHealth);

            // Destroy healthbar when gunship is dead
            if (currentHealth <= 0.0f)
            {
                LogMessage("[GunshipHealthBar] Health zero - destroying bar entity");
                SceneDestroyEntity((uint)EntityID);
                return;
            }

            UpdateBarScale();
        }

        private void UpdateBarScale()
        {
            float t = (maxHealth > 0.0f) ? (currentHealth / maxHealth) : 0.0f;
            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            float newWidth = baseScale.X * t;

            // Shift position to keep left edge fixed
            float widthDifference = baseScale.X - newWidth;
            Vector3 newPosition = new Vector3(
                initialPosition.X - widthDifference,
                initialPosition.Y,
                initialPosition.Z
            );
            SetPosition((uint)EntityID, ref newPosition);

            // READ current scale first, then ONLY change X
            Vector3 currentScale = GetScale((uint)EntityID);
            Vector3 newScale = new Vector3(newWidth, currentScale.Y, currentScale.Z);
            SetScale((uint)EntityID, ref newScale);
        }


        private void ParseHealthPayload(string payload, out float current, out float max)
        {
            current = 0.0f;
            max = 0.0f;

            if (string.IsNullOrEmpty(payload))
                return;

            string[] parts = payload.Split('|');
            if (parts.Length >= 1) float.TryParse(parts[0], out current);
            if (parts.Length >= 2) float.TryParse(parts[1], out max);
        }

        public override void OnDestroy()
        {
            if (!string.IsNullOrEmpty(healthEventName))
                Unsubscribe(healthEventName, OnHealthChanged);

            LogMessage("=== GunshipHealthBar Destroyed ===");
        }
    }
}
