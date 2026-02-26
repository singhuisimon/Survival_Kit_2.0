using Engine;
using System;
using static Engine.Scene;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.SpriteRenderer;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// Manages the Alternate Fire HUD Widget
    /// 
    /// How it works:
    ///     - Listens to "AltChargeUpdate" events (published by PlayerWeapon when charge changes)
    ///     - Listens to "AltFireReady" events (published by PlayerWeapon when charge is full)
    ///     - Listens to "AltFireUsed" events (published by PlayerWeapon after firing / charge reset)
    /// 
    /// When ready, FillActive replaces Fill and a subtle pulse opacity plays
    /// <summary>
    
    public class AltFireUI : ScriptBehaviour
    {
        // Entity Names (Must match the tag names in the scene JSON)
        private const string ENTITY_WINDOW      = "AltFireUI_Window";
        private const string ENTITY_ICON        = "AltFireUI_Icon";
        private const string ENTITY_FILL        = "AltFireUI_Fill";
        private const string ENTITY_FILL_ACTIVE = "AltFireUI_FillActive";

        // Events (match what PlayerWeapon publishes) 
        private const string EVT_CHARGE_UPDATE = "AltChargeUpdate";  // payload = current charge int
        private const string EVT_ALT_READY     = "AltFireReady";     // payload = ""
        private const string EVT_ALT_USED      = "AltFireUsed";      // payload = ""
        private const string EVT_GAME_OVER     = "GameOver";
        private const string EVT_GAME_WIN      = "GameWin";

        // Charge settings 
        [SerializeField] private int chargeMax = 30;

        // Screen position where the widget lives (edit later for accuracy)
        [SerializeField] private float widgetX = 1355.0f;
        [SerializeField] private float widgetY = 672.0f;

        // Fill bar settings
        [SerializeField] private float fullFillWidth = 80.0f;

        // X position of the Fill entity's LEFT anchor 
        [SerializeField] private float fillAnchorX = 1315.0f;

        // Ready-pulse animation
        [SerializeField] private float pulseSpeed  = 4.0f;   // radians/sec
        [SerializeField] private float pulseMin    = 0.6f;   // minimum alpha
        [SerializeField] private float pulseMax    = 1.0f;   // maximum alpha

        // Off-screen hide position
        private const float HIDE_Y = -1000.0f;

        // Runtime state
        private uint windowId;
        private uint iconId;
        private uint fillId;
        private uint fillActiveId;

        private bool entitiesFound = false;
        private bool isReady       = false;
        private int  currentCharge = 0;
        private bool gameEnded     = false;

        private float pulseTimer = 0.0f;

        // Cached initial scale of the fill entities (set once in OnStart)
        private Vector3 fillInitialScale;
        private Vector3 fillActiveInitialScale;

        // Positions
        private Vector3 windowPos;
        private Vector3 iconPos;
        private Vector3 fillBasePos;   // position when fill is at 100 %

        public override void OnStart()
        {
            // Cache Screen positions
            windowPos    = new Vector3(widgetX,        widgetY,        0.0f);
            iconPos      = new Vector3(widgetX,        widgetY - 10.0f, 0.0f);  // slight offset, adjust as needed
            fillBasePos  = new Vector3(fillAnchorX + fullFillWidth * 0.5f, widgetY + 18.0f, 0.0f);

            // Find entities
            windowId      = SceneFindEntityByName(ENTITY_WINDOW);
            iconId        = SceneFindEntityByName(ENTITY_ICON);
            fillId        = SceneFindEntityByName(ENTITY_FILL);
            fillActiveId  = SceneFindEntityByName(ENTITY_FILL_ACTIVE);

            // Entities could not be found
            if (windowId == 0)     LogMessage("[AltFireUI] WARNING: entity not found: " + ENTITY_WINDOW);
            if (iconId == 0)       LogMessage("[AltFireUI] WARNING: entity not found: " + ENTITY_ICON);
            if (fillId == 0)       LogMessage("[AltFireUI] WARNING: entity not found: " + ENTITY_FILL);
            if (fillActiveId == 0) LogMessage("[AltFireUI] WARNING: entity not found: " + ENTITY_FILL_ACTIVE);
        
            entitiesFound = (windowId != 0);

            // Cache initial scales so we can proportially shrink the fill bar
            if (fillId != 0)       fillInitialScale       = GetScale(fillId);
            if (fillActiveId != 0) fillActiveInitialScale = GetScale(fillActiveId);

            // Subscribe to events
            Subscribe(EVT_CHARGE_UPDATE, OnChargeUpdate);
            Subscribe(EVT_ALT_READY,     OnAltReady);
            Subscribe(EVT_ALT_USED,      OnAltUsed);
            Subscribe(EVT_GAME_OVER,     OnGameEnded);
            Subscribe(EVT_GAME_WIN,      OnGameEnded);

            // Start hidden, then show in initial (empty) state
            isReady       = false;
            currentCharge = 0;
            RefreshWidget();

            LogMessage("[AltFireUI] Initialized.");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!entitiesFound || gameEnded) 
                return;

            // Pulse fill the FillActive sprite when ready so the player notices it
            if (isReady)
            {
                pulseTimer += deltaTime * pulseSpeed;
                float alpha = pulseMin + (pulseMax - pulseMin) * (0.5f + 0.5f * (float)Math.Sin(pulseTimer));

            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVT_CHARGE_UPDATE, OnChargeUpdate);
            Unsubscribe(EVT_ALT_READY,     OnAltReady);
            Unsubscribe(EVT_ALT_USED,      OnAltUsed);
            Unsubscribe(EVT_GAME_OVER,     OnGameEnded);
            Unsubscribe(EVT_GAME_WIN,      OnGameEnded);
        }

        // ───────────────────── Event callbacks ─────────────────────

        private void OnChargeUpdate(string eventName, string payload) 
        {
            if (!int.TryParse(payload, out int charge))
                return;

            currentCharge = charge;

            // Don't overwrite the ready state if it's already set
            if (!isReady)
                RefreshWidget();
        }

        private void OnAltReady(string eventName, string payload)
        {
            isReady       = true;
            currentCharge = chargeMax;
            pulseTimer    = 0.0f;
            RefreshWidget();
            LogMessage("[AltFireUI] Alt fire is READY!");
        }

        private void OnAltUsed(string eventName, string payload)
        {
            isReady       = false;
            currentCharge = 0;
            pulseTimer    = 0.0f;
            RefreshWidget();
            LogMessage("[AltFireUI] Alt fire was used – resetting charge bar.");
        }

        private void OnGameEnded(string eventName, string payload)
        {
            gameEnded = true;
        }

         // ───────────────────── Widget Rendering ─────────────────────

        private void RefreshWidget()
        {
            if (!entitiesFound)
                return;

            // Window and icon  visible during gameplay
            SetPosition(windowId, ref windowPos);
            SetPosition(iconId,   ref iconPos);

            // Update the fill bar width based on current charge
            float chargeRatio = (chargeMax > 0) ? ((float)currentCharge / (float)chargeMax) : 0.0f;
            if (chargeRatio < 0.0f) chargeRatio = 0.0f;
            if (chargeRatio > 1.0f) chargeRatio = 1.0f;

            if (isReady)
            {
                // ─────────────── Ready State ─────────────────
                // Show the bright "active" fill at full width; hide the normal fill
                ShowFillBar(fillActiveId, fillActiveInitialScale, 1.0f);
                HideFillBar(fillId);
            }
            else
            {
                // ─────────────── Charging State ─────────────────
                // Scale the normal fill bar proportionally to current charge
                ShowFillBar(fillId, fillInitialScale, chargeRatio);
                HideFillBar(fillActiveId);
            }
        }

        /// <summary>
        /// Scales fillEntityId along X proportionally and positions it so the
        /// left edge stays anchored
        /// </summary>
        
        private void ShowFillBar(uint entityId, Vector3 initialScale, float ratio)
        {
            if (entityId == 0) return;

            float newWidth = initialScale.X * ratio;
            // Guard against zero-width 
            if (newWidth < 0.01f) newWidth = 0.01f;

            Vector3 newScale = new Vector3(newWidth, initialScale.Y, initialScale.Z);
            SetScale(entityId, ref newScale);

            // Keep LEFT edge fixed: centre X = anchorX + newWidth/2
            float centreX = fillAnchorX + newWidth * 0.5f;
            Vector3 pos = new Vector3(centreX, fillBasePos.Y, fillBasePos.Z);
            SetPosition(entityId, ref pos);
        }

        private void HideFillBar(uint entityId)
        {
            if (entityId == 0) return;
            Vector3 hidePos = new Vector3(fillAnchorX, HIDE_Y, 0.0f);
            SetPosition(entityId, ref hidePos);
        }

    }
} // end of namespace Game