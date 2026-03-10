using Engine;
using System;
using static Engine.Event;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Prefab;
using static Engine.Physics;
using static Engine.Rigidbody;
using static Engine.Audio;
using static Engine.Tag;
using static Engine.Transform;

namespace Game
{
    public class IndestructableWall : ScriptBehaviour
    {
        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint playerID = INVALID_ENTITY;
        private const string TAG_PLAYER = "Player";

        // Events
        private string EVENT_FIVE_TURRETS_DESTROYED = "FiveTurretsDestroyed";

        private bool isDestroyed = false;
        private Vector3 wallPos;

        // Lifecycle
        public override void OnStart()
        {
            playerID = SceneFindEntityByName(TAG_PLAYER);
            LogMessage("======= playerID: " + playerID + " =======");

            RigidbodySetIsKinematic(EntityID, true);
            wallPos = Transform.GetPosition(EntityID);

            Subscribe(EVENT_FIVE_TURRETS_DESTROYED, OnFiveTurretDestroyed);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (GameState.IsPaused || isDestroyed) return;
            if (playerID == INVALID_ENTITY) return;

            Vector3 playerPos = Transform.GetPosition(playerID);
            if (playerPos.X <= wallPos.X + 4.0f)
            {
                playerPos.X = wallPos.X + 4.0f;
                Transform.SetPosition(playerID, ref playerPos);
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_FIVE_TURRETS_DESTROYED, OnFiveTurretDestroyed);
        }

        private void OnFiveTurretDestroyed(string eventName, string payload)
        {
            isDestroyed = true;
            SceneDestroyEntity(EntityID);
        }

    }
}