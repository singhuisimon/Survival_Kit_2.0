using Engine;
using System;
using System.Collections.Generic;
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
    public class DecorWall : ScriptBehaviour
    {

        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint playerID = INVALID_ENTITY;
        private const string TAG_PLAYER = "Player";

        // Lifecycle
        public override void OnStart()
        {
            playerID = SceneFindEntityByName(TAG_PLAYER);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (GameState.IsPaused) return;
            if (playerID == INVALID_ENTITY) return;
            CheckCollisions();
        }

        private void CheckCollisions()
        {
            // Get all entities that hit this environment object this frame
            List<uint> collisions = CollisionManager.GetEnvironmentHitBy((uint)EntityID);
            
            if (collisions == null || collisions.Count == 0)
                return;
            
            foreach (uint other in collisions)
            {
                string otherTag = TagGetTag(other);
                // Check if hit player
                if (otherTag == "PrimaryBullet")
                {
                    SceneDestroyEntity(other);
                }
            }
        }

    }
}