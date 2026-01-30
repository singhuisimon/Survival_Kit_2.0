using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// HealthBar - Visual representation of player health
    /// Position adjustment: Move by FULL width change (not half)
    /// Left edge = Position.X - Width (scale.X is radius, not diameter)
    /// </summary>
    public class HealthBar : ScriptBehaviour
    {
        // ===== Event Names =====
        private const string EVENT_PLAYER_HEALTHCHANGE = "Health Change";

        // ===== Visual Settings (Set in Editor) =====
        private float barMaxWidth;  // Maximum width at 100% health (set from scene)

        // ===== State =====
        private bool initialized = false;
        private Vector3 initialPosition;  // Store initial center position
        private float initialWidth;  // Store initial width to calculate offset
        private float playerMaxHP = 100.0f;  // Player's actual max HP
        private float hpToWidthRatio = 4.0f;  // 100 HP = 400 width, so ratio is 4

        // Key press tracking to prevent multiple triggers per press
        private bool hKeyWasPressed = false;

        public override void OnStart()
        {
            LogMessage("=== HealthBar OnStart ===");
            LogMessage("HealthBar EntityID: " + EntityID);

            // Subscribe to health update events
            Event.Subscribe(EVENT_PLAYER_HEALTHCHANGE, OnPlayerHealthChange);
            LogMessage("HealthBar: Subscribed to event '" + EVENT_PLAYER_HEALTHCHANGE + "'");

            // Store initial position and width
            initialPosition = Transform.GetPosition((uint)EntityID);

            // Get the actual initial scale to determine real width
            Vector3 initialScale = Transform.GetScale((uint)EntityID);
            float actualInitialWidth = initialScale.X;

            // Use the actual width from the scene as barMaxWidth
            barMaxWidth = actualInitialWidth;
            initialWidth = actualInitialWidth;

            initialized = true;

            LogMessage("HealthBar initialized:");
            LogMessage("  Max Width (from scene): " + barMaxWidth);
            LogMessage("  Initial Position X: " + initialPosition.X);
            LogMessage("  Subscribed to event: " + EVENT_PLAYER_HEALTHCHANGE);
            LogMessage("  Test: Press H to damage (-20 width)");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized)
                return;

            // ===== TEST CODE - H KEY FOR TESTING =====
            // Press H to deal damage (reduce width by 20)
            if (Input.IsKeyPressed(KeyCode.H))
            {
                if (!hKeyWasPressed)
                {
                    hKeyWasPressed = true;
                    LogMessage("=== H KEY - DAMAGE 20 ===");

                    // Get current scale and position
                    Vector3 currentScale = Transform.GetScale((uint)EntityID);
                    Vector3 currentPosition = Transform.GetPosition((uint)EntityID);
                    float currentWidth = currentScale.X;

                    float widthChange = 20;  // Amount to decrease

                    // Reduce width by 20
                    float newWidth = currentWidth - widthChange;
                    if (newWidth < 0) newWidth = 0;

                    // Update scale
                    Vector3 newScale = new Vector3(
                        newWidth,          // Reduced width
                        currentScale.Y,    // Keep height
                        currentScale.Z     // Keep depth
                    );
                    Transform.SetScale((uint)EntityID, ref newScale);

                    // Move center LEFT by FULL width decrease to keep left edge fixed
                    Vector3 newPosition = new Vector3(
                        currentPosition.X - widthChange,
                        currentPosition.Y,
                        currentPosition.Z
                    );
                    Transform.SetPosition((uint)EntityID, ref newPosition);

                    LogMessage("  Width: " + currentWidth + " -> " + newWidth);
                    LogMessage("  Position moved left by: " + widthChange);
                }
            }
            else
            {
                hKeyWasPressed = false;
            }
        }

        // ===== EVENT HANDLERS =====

        private void OnPlayerHealthChange(string eventName, string payload)
        {
            LogMessage("=== OnPlayerHealthChange CALLED ===");
            LogMessage("  Payload: '" + payload + "'");

            if (!float.TryParse(payload, out float currentHP))
            {
                LogError("HealthBar: Failed to parse currentHP from payload: " + payload);
                return;
            }

            // Clamp HP to valid range (0-100)
            if (currentHP < 0.0f) currentHP = 0.0f;
            if (currentHP > playerMaxHP) currentHP = playerMaxHP;

            // Convert HP to width (100 HP = 400 width)
            float currentWidth = currentHP * hpToWidthRatio;

            LogMessage("  Current HP: " + currentHP + " -> Width: " + currentWidth);

            // Get current scale
            Vector3 currentScale = Transform.GetScale((uint)EntityID);

            // Set width based on converted HP value
            Vector3 newScale = new Vector3(
                currentWidth,      // Width = HP * 4
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
            LogMessage("=== OnPlayerHealthChange COMPLETE ===");
        }

        public override void OnDestroy()
        {
            // Clean up event subscriptions
            Event.Unsubscribe(EVENT_PLAYER_HEALTHCHANGE, OnPlayerHealthChange);

            LogMessage("=== HealthBar Destroyed ===");
        }
    }
}