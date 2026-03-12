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
    public class DestructableWall : ScriptBehaviour
    {
        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint playerID = INVALID_ENTITY;
        private const string TAG_PLAYER = "Player";

        private const string TAG_PRIMARY_BULLET = "PrimaryBullet";
        private const string TAG_SECONDARY_BULLET = "PrimaryUltBullet";
        private string hitmarkerAudioPrefab = "Sources/Prefabs/audio_hitmarker.prefab";
        private string playerKillPrefab = "Sources/Prefabs/audio_Player_Kill.prefab";

        // Health
        [SerializeField] private float health = 5.0f;

        // Events
        private string EVENT_BULLET_HIT = "Damage:";

        private bool isDestroyed = false;
        private Vector3 wallPos;

        // Lifecycle
        public override void OnStart()
        {
            // Find both player and gunship
            playerID = SceneFindEntityByName(TAG_PLAYER);
            LogMessage("======= playerID: " + playerID + " =======");

            RigidbodySetIsKinematic(EntityID, true);

            EVENT_BULLET_HIT += EntityID.ToString();
            Subscribe(EVENT_BULLET_HIT, OnBulletHit);

            wallPos = Transform.GetPosition(EntityID);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (GameState.IsPaused || isDestroyed) return;
            if (playerID == INVALID_ENTITY) return;

            // Vector3 playerPos = Transform.GetPosition(playerID);
            // if (playerPos.X <= wallPos.X + 4.0f)
            // {
            //     playerPos.X = wallPos.X + 4.0f;
            //     Transform.SetPosition(playerID, ref playerPos);
            // }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_BULLET_HIT, OnBulletHit);
        }

        // Combat
        private void OnBulletHit(string eventName, string payload)
        {
            Vector3 emptyVec = new Vector3(0, 0, 0);
            RigidbodySetAngularVelocity(EntityID, ref emptyVec);

            health -= 1.0f;
            LogMessage("DestructableWall hit! Health: " + health);

            uint attackerId = DamageSystem.ParseAttackerId(payload);
            if(attackerId != INVALID_ENTITY && health > 0.0f){
                string attackerTag = TagGetTag(attackerId);
                if(attackerTag == TAG_PRIMARY_BULLET || attackerTag == TAG_SECONDARY_BULLET){
                    
                    uint hitmarkerID = 0;
                    hitmarkerID = PrefabInstantiate(hitmarkerAudioPrefab);
                    if(hitmarkerID == 0){
                        LogMessage("[DestructableWall] Player Hit! But hitmarkerID fail to instantiate");
                    }
                }
            }

            if (health <= 0)
            {
                isDestroyed = true;
                uint playerkillID = 0;
                playerkillID = PrefabInstantiate(playerKillPrefab);
                if(playerkillID == 0){
                    LogMessage("[DestructableWall] Player Kill DestructableWall! But playerkillID fail to instantiate");
                }
                Publish("DestructableWallDestroyed", EntityID.ToString());
                SceneDestroyEntity(EntityID);
            }
        }

    }
}