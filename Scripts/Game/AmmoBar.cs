using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// AmmoBar - Visual representation of player ammo
    /// Position adjustment: Move by FULL width change (not half)
    /// Left edge = Position.X - Width (scale.X is radius, not diameter)
    /// Attach this to the ammo bar sprite entity
    /// </summary>
    public class AmmoBar : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_AMMO_CHANGE = "AmmoChange";

        // ===== Visual Settings =====
        private float barMaxWidth;  // Maximum width at full ammo (set from scene)

        // ===== State =====
        private bool initialized = false;
        private Vector3 initialPosition;  // Store initial center position
        private float initialWidth;  // Store initial width to calculate offset
        private float playerMaxAmmo = 100.0f;  // Player's actual max ammo
        private float ammoToWidthRatio = 4.0f;  // 100 ammo = 400 width, so ratio is 4

        public override void OnStart()
        {
            LogMessage("=== AmmoBar OnStart ===");
            LogMessage("AmmoBar EntityID: " + EntityID);

            // Subscribe to ammo update events from PlayerWeapon
            Event.Subscribe(EVENT_AMMO_CHANGE, OnAmmoChange);
            LogMessage("AmmoBar: Subscribed to event '" + EVENT_AMMO_CHANGE + "'");

            // Store initial position and width
            initialPosition = Transform.GetPosition((uint)EntityID);

            // Get the actual initial scale to determine real width
            Vector3 initialScale = Transform.GetScale((uint)EntityID);
            float actualInitialWidth = initialScale.X;

            // Use the actual width from the scene as barMaxWidth
            barMaxWidth = actualInitialWidth;
            initialWidth = actualInitialWidth;

            initialized = true;

            LogMessage("AmmoBar initialized:");
            LogMessage("  Max Width (from scene): " + barMaxWidth);
            LogMessage("  Initial Position X: " + initialPosition.X);
            LogMessage("  Subscribed to event: " + EVENT_AMMO_CHANGE);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized)
                return;
        }

        // ===== EVENT HANDLERS =====

        private void OnAmmoChange(string eventName, string payload)
        {
            LogMessage("=== OnAmmoChange CALLED ===");
            LogMessage("  Payload: '" + payload + "'");

            if (!float.TryParse(payload, out float currentAmmo))
            {
                LogError("AmmoBar: Failed to parse currentAmmo from payload: " + payload);
                return;
            }

            // Clamp ammo to valid range (0-100)
            if (currentAmmo < 0.0f) currentAmmo = 0.0f;
            if (currentAmmo > playerMaxAmmo) currentAmmo = playerMaxAmmo;

            // Convert ammo to width (100 ammo = 400 width)
            float currentWidth = currentAmmo * ammoToWidthRatio;

            LogMessage("  Current Ammo: " + currentAmmo + " -> Width: " + currentWidth);

            // Get current scale
            Vector3 currentScale = Transform.GetScale((uint)EntityID);

            // Set width based on converted ammo value
            Vector3 newScale = new Vector3(
                currentWidth,      // Width = Ammo * 4
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
            LogMessage("=== OnAmmoChange COMPLETE ===");
        }

        public override void OnDestroy()
        {
            // Clean up event subscriptions
            Event.Unsubscribe(EVENT_AMMO_CHANGE, OnAmmoChange);

            LogMessage("=== AmmoBar Destroyed ===");
        }
    }
}