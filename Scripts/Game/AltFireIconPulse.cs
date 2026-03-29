using Engine;
using System;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// AltFireIconPulse
    /// Attach this script to the AltFire icon entity.
    /// When the AltFireReadyIndicator becomes visible (alt fire is ready),
    /// the icon pulses between its normal and enlarged size in a loop.
    /// The pulse stops as soon as AltFired is fired.
    /// </summary>
    public class AltFireIconPulse : ScriptBehaviour
    {
        // Event names — must match AltFireBar.cs
        private const string EVENT_ALTFIRED = "AltFired";
        private const string EVENT_GAINULT = "GainUlt";
        // Normal state
        // Normal state
        private const float NORMAL_POS_X = 599.200f;
        private const float NORMAL_POS_Y = 360.000f;
        private const float NORMAL_SCALE_X = 639.000f;
        private const float NORMAL_SCALE_Y = 360.000f;

        // Enlarged state (editor-confirmed values)
        private const float ENLARGED_POS_X = 555.800f;
        private const float ENLARGED_POS_Y = 328.500f;
        private const float ENLARGED_SCALE_X = 689.000f;
        private const float ENLARGED_SCALE_Y = 410.000f;



        // How long one full pulse cycle takes (seconds)
        private const float PULSE_DURATION = 0.6f;

        // State
        private bool isPulsing = false;
        private float pulseTimer = 0.0f;
        private int currentCharge = 0;
        private const int MAX_CHARGE = 30;

        public override void OnStart()
        {
            LogMessage("AltFireIconPulse: Initializing...");

            // Start at normal size/position
            ApplyNormal();

            Event.Subscribe(EVENT_GAINULT, OnGainCharge);
            Event.Subscribe(EVENT_ALTFIRED, OnAltFired);

            LogMessage("AltFireIconPulse: Ready.");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!isPulsing) return;

            pulseTimer += deltaTime;

            // Normalize timer into a 0-1 ping-pong over PULSE_DURATION
            float cycleTime = pulseTimer % PULSE_DURATION;
            float halfCycle = PULSE_DURATION * 0.5f;
            float t;

            if (cycleTime < halfCycle)
            {
                // First half: normal -> enlarged
                t = cycleTime / halfCycle;
            }
            else
            {
                // Second half: enlarged -> normal
                t = 1.0f - ((cycleTime - halfCycle) / halfCycle);
            }

            // Smooth the lerp with a sine ease
            t = SineEaseInOut(t);

            // Lerp position and scale
            float posX = Lerp(NORMAL_POS_X, ENLARGED_POS_X, t);
            float posY = Lerp(NORMAL_POS_Y, ENLARGED_POS_Y, t);
            float scaleX = Lerp(NORMAL_SCALE_X, ENLARGED_SCALE_X, t);
            float scaleY = Lerp(NORMAL_SCALE_Y, ENLARGED_SCALE_Y, t);

            Vector3 newPos = new Vector3(posX, posY, 0.0f);
            Vector3 newScale = new Vector3(scaleX, scaleY, 1.0f);


            Transform.SetPosition((uint)EntityID, ref newPos);
            Transform.SetScale((uint)EntityID, ref newScale);
        }

        private void OnGainCharge(string eventName, string payload)
        {
            currentCharge++;
            if (currentCharge > MAX_CHARGE) currentCharge = MAX_CHARGE;

            if (currentCharge >= MAX_CHARGE && !isPulsing)
            {
                StartPulse();
            }
        }

        private void OnAltFired(string eventName, string payload)
        {
            currentCharge = 0;
            StopPulse();
        }

        private void StartPulse()
        {
            isPulsing = true;
            pulseTimer = 0.0f;
            LogMessage("AltFireIconPulse: Pulse started.");
        }

        private void StopPulse()
        {
            isPulsing = false;
            pulseTimer = 0.0f;
            ApplyNormal();
            LogMessage("AltFireIconPulse: Pulse stopped, reset to normal.");
        }

        private void ApplyNormal()
        {
            Vector3 pos = new Vector3(NORMAL_POS_X, NORMAL_POS_Y, 0.0f);
            Vector3 scale = new Vector3(NORMAL_SCALE_X, NORMAL_SCALE_Y, 1.0f);
            Transform.SetPosition((uint)EntityID, ref pos);
            Transform.SetScale((uint)EntityID, ref scale);
        }


        private static float Lerp(float a, float b, float t)
        {
            return a + (b - a) * t;
        }

        private static float SineEaseInOut(float t)
        {
            return -(float)(Math.Cos(Math.PI * t) - 1.0) * 0.5f;
        }


        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_GAINULT, OnGainCharge);
            Event.Unsubscribe(EVENT_ALTFIRED, OnAltFired);
            LogMessage("AltFireIconPulse: Destroyed.");
        }
    }
}
