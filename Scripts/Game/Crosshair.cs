using System;
using Engine;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.Event;

namespace Game
{
    public class Crosshair : ScriptBehaviour
    {
        private bool isReloading = false;
        private float currentAngleZ = 0.0f;

        private const string EVENT_RELOAD_START = "ReloadStart";
        private const string EVENT_RELOAD_END = "ReloadEnd";

        public override void OnStart()
        {
            LogMessage("[Crosshair] OnStart | EntityID: " + EntityID);
            Event.Subscribe(EVENT_RELOAD_START, OnReloadStart);
            Event.Subscribe(EVENT_RELOAD_END, OnReloadEnd);
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (!isReloading)
            {
                return;


            }


            currentAngleZ += 360.0f * (deltaTime / 0.5f);
            LogMessage("[Crosshair] spinning | Z: " + currentAngleZ);

            float angleRad = currentAngleZ * (float)(Math.PI / 180.0);
            Quat newRot = RotateAxisAngle(Vector3.Forward, angleRad);
            SetRotation((uint)EntityID, ref newRot);
        }

        private void OnReloadStart(string eventName, string payload)
        {
            // ONLY set flags here, no Transform calls
            LogMessage("[Crosshair] OnReloadStart FIRED");
            currentAngleZ = 0.0f;
            isReloading = true;
        }

        private void OnReloadEnd(string eventName, string payload)
        {
            // ONLY set flags here, no Transform calls
            LogMessage("[Crosshair] OnReloadEnd FIRED");
            isReloading = false;
            currentAngleZ = 0.0f;
            Quat newRot = RotateAxisAngle(Vector3.Forward, 0);

            SetRotation((uint)EntityID, ref newRot);

        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_RELOAD_START, OnReloadStart);
            Event.Unsubscribe(EVENT_RELOAD_END, OnReloadEnd);
        }
    }
}
