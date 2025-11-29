using System;
using Engine;

namespace Game
{
    public class PositionListener
    {
        public int EntityID;

        public void OnStart()
        {
            // Subscribe to the ScriptEvent named "PlayerPosition"
            // This hooks into the C++ event system bridge.
            EventSystem.Subscribe("PlayerPosition", OnPlayerPosition);
        }

        private void OnPlayerPosition(string name, string payload)
        {
            // Expected payload format: "entityId|x|y|z"
            string[] parts = payload.Split('|');
            if (parts.Length != 4)
            {
                InternalCalls.LogWarning("[PositionListener] Invalid payload: " + payload);
                return;
            }

            int entityId;
            float x, y, z;

            if (!int.TryParse(parts[0], out entityId) ||
                !float.TryParse(parts[1], out x) ||
                !float.TryParse(parts[2], out y) ||
                !float.TryParse(parts[3], out z))
            {
                InternalCalls.LogWarning("[PositionListener] Failed to parse payload: " + payload);
                return;
            }

            InternalCalls.Log(
                "[PositionListener] Entity " + entityId +
                " at (" + x + ", " + y + ", " + z + ")"
            );
        }
    }
}
