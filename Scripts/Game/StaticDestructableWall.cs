// StaticDestructableWall.cs
using Engine;
using System.Collections.Generic;
using static Engine.Event;
using static Engine.Scene;

namespace Game
{
    public class StaticDestructableWall : ScriptBehaviour
    {
        [SerializeField] private float HP = 25.0f;
        [SerializeField] private float collisionDamageToPlayer = 9999.0f;

        private const string PLAYER_NAME = "Player";
        private const string DAMAGE_EVENT_PREFIX = "Damage:";

        private uint _playerId;
        private string _selfDamageEvent;
        private string _playerDamageEvent;

        private bool _wasPlayerCollidingLastFrame = false;

        public override void OnStart()
        {
            _playerId = SceneFindEntityByName(PLAYER_NAME);

            _selfDamageEvent = DAMAGE_EVENT_PREFIX + ((uint)EntityID).ToString();
            _playerDamageEvent = DAMAGE_EVENT_PREFIX + _playerId.ToString();

            Subscribe(_selfDamageEvent, OnDamaged);
        }

        public override void OnUpdate(float deltaTime)
        {
            // CollisionManager stores "entity -> environment objects it hit this frame"
            List<uint> envHits = CollisionManager.GetEnvironmentCollisions(_playerId);

            bool collidingNow = false;
            uint self = (uint)EntityID;

            if (envHits != null)
            {
                for (int i = 0; i < envHits.Count; ++i)
                {
                    if (envHits[i] == self)
                    {
                        collidingNow = true;
                        break;
                    }
                }
            }

            // Fire once on enter (instant-kill damage event)
            if (collidingNow && !_wasPlayerCollidingLastFrame)
            {
                Publish(_playerDamageEvent, collisionDamageToPlayer.ToString());
                Engine.Logger.LogMessage("Damaging Player!!!!");
            }

            _wasPlayerCollidingLastFrame = collidingNow;
        }

        public override void OnDestroy()
        {
            Unsubscribe(_selfDamageEvent, OnDamaged);
        }

        private void OnDamaged(string eventName, string payload)
        {
            float dmg = ParseDamageAmount(payload);
            HP -= dmg;

            if (HP <= 0.0f)
            {
                SceneDestroyEntity((uint)EntityID);
            }
        }

        private static float ParseDamageAmount(string payload)
        {
            // Accept "amount" or "amount|source" (common pattern)
            if (string.IsNullOrEmpty(payload))
                return 1.0f;

            int sep = payload.IndexOf('|');
            if (sep >= 0)
                payload = payload.Substring(0, sep);

            return float.Parse(payload);
        }
    }
}
