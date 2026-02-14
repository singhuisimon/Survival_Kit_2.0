using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// ReloadIndicator - Flashing visual feedback during weapon reload
    /// Attach this to the reload indicator sprite entity in your scene
    /// The sprite will flash on/off during reload using deltaTime
    /// </summary>
    public class ReloadIndicator : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_RELOAD_START = "ReloadStart";
        private const string EVENT_RELOAD_END = "ReloadEnd";

        // ===== Flash Settings =====
        [SerializeField] private float flashInterval = 0.15f;  // Time between flashes (seconds)

        // ===== State =====
        private bool isReloading = false;
        private float flashTimer = 0.0f;
        private bool isVisible = false;      // Current visibility state
        private Vector3 visibleScale;        // Scale when visible
        private Vector3 hiddenScale;         // Scale when hidden (zero)

        public override void OnStart()
        {
            LogMessage("=== ReloadIndicator OnStart ===");
            LogMessage("ReloadIndicator EntityID: " + EntityID);

            // Subscribe to reload events from PlayerWeapon
            Event.Subscribe(EVENT_RELOAD_START, OnReloadStart);
            Event.Subscribe(EVENT_RELOAD_END, OnReloadEnd);

            LogMessage("ReloadIndicator: Subscribed to reload events");

            // Store the initial scale as the "visible" scale
            visibleScale = Transform.GetScale((uint)EntityID);
            hiddenScale = new Vector3(0.0f, 0.0f, 0.0f);

            // Start hidden
            SetVisibility(false);

            LogMessage("ReloadIndicator initialized with flash interval: " + flashInterval);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!isReloading)
                return;

            // Accumulate time using deltaTime
            flashTimer += deltaTime;

            // Toggle visibility when timer exceeds interval
            if (flashTimer >= flashInterval)
            {
                flashTimer = 0.0f;  // Reset timer
                isVisible = !isVisible;  // Toggle state
                SetVisibility(isVisible);
            }
        }

        // ===== EVENT HANDLERS =====

        private void OnReloadStart(string eventName, string payload)
        {
            LogMessage("=== OnReloadStart CALLED ===");
            LogMessage("  Starting reload flash sequence");

            isReloading = true;
            flashTimer = 0.0f;
            isVisible = true;
            SetVisibility(true);

            LogMessage("Reload indicator started flashing");
        }

        private void OnReloadEnd(string eventName, string payload)
        {
            LogMessage("=== OnReloadEnd CALLED ===");
            LogMessage("  Ending reload flash sequence");

            isReloading = false;
            flashTimer = 0.0f;
            isVisible = false;
            SetVisibility(false);

            LogMessage("Reload indicator hidden");
        }

        // ===== HELPER METHODS =====

        private void SetVisibility(bool visible)
        {
            Vector3 targetScale = visible ? visibleScale : hiddenScale;
            Transform.SetScale((uint)EntityID, ref targetScale);

            LogMessage("  Visibility: " + (visible ? "SHOWN" : "HIDDEN"));
        }

        public override void OnDestroy()
        {
            // Clean up event subscriptions
            Event.Unsubscribe(EVENT_RELOAD_START, OnReloadStart);
            Event.Unsubscribe(EVENT_RELOAD_END, OnReloadEnd);

            LogMessage("=== ReloadIndicator Destroyed ===");
        }
    }
}