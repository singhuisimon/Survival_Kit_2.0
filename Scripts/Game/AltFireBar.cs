using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;
using static Engine.SpriteRenderer;

namespace Game
{
    /// <summary>
    /// AltFireBar - Visual representation of alt fire charge
    /// Grey bar fills from bottom to top as player kills enemies
    /// Shows "ready" indicator when fully charged
    /// Attach this to the AltFireReady (grey bar) entity
    /// </summary>
    public class AltFireBar : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_GAIN_ULT = "GainUlt";
        private const string EVENT_ALT_FIRED = "AltFired";

        // ===== Entity Names =====
        private const string READY_INDICATOR_NAME = "AltFireReadyIndicator";

        // ===== Visual Settings =====
        private float barMaxHeight;  // Maximum height at full charge (set from scene)
        private const int MAX_CHARGE = 30;  // Max charge points

        // ===== State =====
        private bool initialized = false;
        private Vector3 initialPosition;  // Store initial center position
        private float initialHeight;  // Store initial height
        private float chargeToHeightRatio;  // Calculated: barMaxHeight / MAX_CHARGE
        private int currentCharge = 0;

        // ===== Entity IDs =====
        private uint readyIndicatorId = 0;

        public override void OnStart()
        {
            LogMessage("=== AltFireBar OnStart ===");
            LogMessage("AltFireBar EntityID: " + EntityID);

            // Find the ready indicator entity
            readyIndicatorId = Scene.SceneFindEntityByName(READY_INDICATOR_NAME);
            if (readyIndicatorId == 0)
            {
                LogError("AltFireBar: Could not find entity: " + READY_INDICATOR_NAME);
            }
            else
            {
                LogMessage("AltFireBar: Found ready indicator entity ID: " + readyIndicatorId);
            }

            // Subscribe to charge events
            Event.Subscribe(EVENT_GAIN_ULT, OnGainCharge);
            Event.Subscribe(EVENT_ALT_FIRED, OnAltFired);
            LogMessage("AltFireBar: Subscribed to events");

            // Store initial position and height
            initialPosition = Transform.GetPosition((uint)EntityID);

            // Get the actual initial scale to determine max height
            Vector3 initialScale = Transform.GetScale((uint)EntityID);
            float actualInitialHeight = initialScale.Y;

            // Use the actual height from the scene as barMaxHeight
            barMaxHeight = actualInitialHeight;
            initialHeight = actualInitialHeight;

            // Calculate ratio: barMaxHeight / MAX_CHARGE
            chargeToHeightRatio = barMaxHeight / (float)MAX_CHARGE;

            initialized = true;

            // Start empty
            currentCharge = 0;
            UpdateBarVisual(0);

            // Hide ready indicator initially
            if (readyIndicatorId != 0)
            {
                SpriteRenderer.SetIsVisible(readyIndicatorId, false);
            }

            LogMessage("AltFireBar initialized:");
            LogMessage("  Max Height (from scene): " + barMaxHeight);
            LogMessage("  Max Charge: " + MAX_CHARGE);
            LogMessage("  Charge to Height Ratio: " + chargeToHeightRatio);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized)
                return;
        }

        // ===== EVENT HANDLERS =====

        private void OnGainCharge(string eventName, string payload)
        {
            LogMessage("=== OnGainCharge CALLED ===");

            // Increment charge by 1
            currentCharge++;
            if (currentCharge > MAX_CHARGE)
                currentCharge = MAX_CHARGE;

            LogMessage("  Current Charge: " + currentCharge + "/" + MAX_CHARGE);

            // Update visual
            UpdateBarVisual(currentCharge);

            // Check if fully charged
            if (currentCharge >= MAX_CHARGE)
            {
                // Show ready indicator
                if (readyIndicatorId != 0)
                {
                    //Publish("UltCharged", UltRecharged.ToString());
                    SpriteRenderer.SetIsVisible(readyIndicatorId, true);
                    LogMessage("  ALT FIRE READY - Indicator shown");
                }
            }
        }

        private void OnAltFired(string eventName, string payload)
        {
            LogMessage("=== OnAltFired CALLED ===");
            LogMessage("  Event: " + eventName + ", Payload: " + payload);

            // Reset charge to 0
            currentCharge = 0;
            UpdateBarVisual(0);

            // Hide ready indicator
            if (readyIndicatorId != 0)
            {
                LogMessage("  Hiding ready indicator (ID: " + readyIndicatorId + ")");
                SpriteRenderer.SetIsVisible(readyIndicatorId, false);
                LogMessage("  Ready indicator hidden");
            }
            else
            {
                LogError("  Ready indicator ID is 0 - cannot hide!");
            }

            LogMessage("  Charge reset to 0");
        }

        // ===== HELPER METHODS =====

        private void UpdateBarVisual(int charge)
        {
            // Calculate volume as a 0-1 ratio
            float chargeRatio = (float)charge / (float)MAX_CHARGE;
            if (chargeRatio < 0.0f) chargeRatio = 0.0f;
            if (chargeRatio > 1.0f) chargeRatio = 1.0f;

            // Calculate new height based on charge ratio
            float newHeight = initialHeight * chargeRatio;

            LogMessage("  Charge " + charge + "/" + MAX_CHARGE + " (ratio: " + chargeRatio.ToString("F2") + ") -> Height: " + newHeight);

            // Get current scale
            Vector3 currentScale = Transform.GetScale((uint)EntityID);

            // Set height based on charge (fills bottom-to-top)
            Vector3 newScale = new Vector3(
                currentScale.X,    // Keep width
                newHeight,         // Height = initialHeight * chargeRatio
                currentScale.Z     // Keep depth
            );
            Transform.SetScale((uint)EntityID, ref newScale);

            // Adjust position to keep BOTTOM edge fixed
            // When bar shrinks, center needs to move down
            float heightDiff = initialHeight - newHeight;
            Vector3 newPosition = new Vector3(
                initialPosition.X,
                initialPosition.Y + heightDiff,  // Move up as bar shrinks (same as pause menu mixer)
                initialPosition.Z
            );
            Transform.SetPosition((uint)EntityID, ref newPosition);

            LogMessage("  Position Y offset: " + heightDiff);
        }

        public override void OnDestroy()
        {
            // Clean up event subscriptions
            Event.Unsubscribe(EVENT_GAIN_ULT, OnGainCharge);
            Event.Unsubscribe(EVENT_ALT_FIRED, OnAltFired);

            LogMessage("=== AltFireBar Destroyed ===");
        }
    }
}