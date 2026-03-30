using Engine;
using System.Collections.Generic;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Tag;

namespace Game
{
    /// <summary>
    /// Triggers the trench wall warning VO when player hits this detector.
    /// Self-destructs after triggering so it cannot replay.
    /// </summary>
    public class DetectorWallMidTrenchRun : ScriptBehaviour
    {
        private const string EVENT_TRENCH_MID_RUN_WARNING_TRIGGERED = "TrenchMidRunWarningTriggered";
        private const string PLAYER_TAG = "Player";

        private bool triggered = false;

        public override void OnStart()
        {
            LogMessage("[DetectorWallMidTrenchRun] Ready on entity " + EntityID);
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            if (GameState.IsPaused || triggered)
                return;

            CheckCollisions();
        }

        private void CheckCollisions()
        {
            List<uint> hitters = CollisionManager.GetEnvironmentHitBy((uint)EntityID);

            if (hitters == null || hitters.Count == 0)
                return;

            foreach (uint otherId in hitters)
            {
                string otherTag = TagGetTag(otherId);

                if (otherTag == PLAYER_TAG)
                {
                    TriggerOnce(otherId);
                    return;
                }
            }
        }

        private void TriggerOnce(uint playerEntityId)
        {
            if (triggered)
                return;

            triggered = true;

            Publish(EVENT_TRENCH_MID_RUN_WARNING_TRIGGERED, playerEntityId.ToString());

            LogMessage("[DetectorWallMidTrenchRun] Player detected. Published wall warning event.");

            // Self destruct so this trigger never replays
            SceneDestroyEntity((uint)EntityID);
        }

        public override void OnDestroy()
        {
        }
    }
}