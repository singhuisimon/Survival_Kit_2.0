using System;
using Engine;
using static Engine.Logger;
using static Engine.SpriteRenderer;
using static Engine.Input;

namespace Game
{
    /// <summary>
    /// Hitmarker - Shows visual feedback when player hits an enemy
    /// Attach this script to the hit marker sprite entity itself
    /// </summary>
    public class Hitmarker : ScriptBehaviour
    {
        // ===== Settings =====
        [SerializeField("Hit Marker Display Time")]
        private float displayTime = 0.15f;  // How long to show the hit marker

        // ===== State =====
        private bool initialized = false;
        private float currentDisplayTimer = 0.0f;
        private bool isShowing = false;

        public override void OnStart()
        {
            LogMessage("=== Hitmarker OnStart ===");
            LogMessage("Hitmarker EntityID: " + EntityID);

            // Start with hit marker hidden
            SetIsVisible((uint)EntityID, false);

            initialized = true;

            LogMessage("[Hitmarker] Initialized");
            LogMessage("  Display Time: " + displayTime + "s");
            LogMessage("  TEST: Press H to manually show hit marker");
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!initialized)
            {
                // Start with hit marker hidden
                SetIsVisible((uint)EntityID, false);
                return;
            }


            // TEST: Press H to manually toggle hit marker for testing
     /*       if (IsKeyPressed(KeyCode.H))
            {
                LogMessage("[Hitmarker] TEST: H key pressed - showing hit marker");
                ShowHitMarker();
            }*/

            // Check if any player projectiles hit enemies this frame
            bool hitDetected = CheckForHits();

            if (hitDetected)
            {
                ShowHitMarker();
            }

            // Update timer if showing
            if (isShowing)
            {
                currentDisplayTimer -= deltaTime;

                if (currentDisplayTimer <= 0.0f)
                {
                    HideHitMarker();
                }
            }
        }

        private bool CheckForHits()
        {
            // Check if collision manager has detected any bullet that hit targetable / destroyable (e.g. enemies / destructable wall)
            return CollisionManager.HasAnyPlayerProjectileValidHits();
        }

        private void ShowHitMarker()
        {
            if (!isShowing)
            {
                SetIsVisible((uint)EntityID, true);
                isShowing = true;
                LogMessage("[Hitmarker] HIT!");
            }

            // Reset timer (in case multiple hits happen quickly)
            currentDisplayTimer = displayTime;
        }

        private void HideHitMarker()
        {
            SetIsVisible((uint)EntityID, false);
            isShowing = false;
        }

        public override void OnDestroy()
        {
            // Make sure hit marker is hidden on cleanup
            SetIsVisible((uint)EntityID, false);

            LogMessage("=== Hitmarker Destroyed ===");
        }
    }
}