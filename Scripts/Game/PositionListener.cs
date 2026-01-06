using System;
using Engine;
using static Engine.Logger;
using static Engine.Event;

namespace Game
{
    public class PositionListener
    {
        public int EntityID;

        public void OnStart()
        {
            // Subscribe to the ScriptEvent named "PlayerPosition"
            // This hooks into the C++ event system bridge.
            Subscribe("PlayerPosition", OnPlayerPosition);
        }

        private void OnPlayerPosition(string name, string payload)
        {
            // Expected payload format: "entityId|x|y|z"
            string[] parts = payload.Split('|');
            if (parts.Length != 4)
            {
                LogWarning("[PositionListener] Invalid payload: " + payload);
                return;
            }

            int entityId;
            float x, y, z;

            if (!int.TryParse(parts[0], out entityId) ||
                !float.TryParse(parts[1], out x) ||
                !float.TryParse(parts[2], out y) ||
                !float.TryParse(parts[3], out z))
            {
                LogWarning("[PositionListener] Failed to parse payload: " + payload);
                return;
            }

            LogMessage(
                "[PositionListener] Entity " + entityId +
                " at (" + x + ", " + y + ", " + z + ")"
            );
        }
    }
}
