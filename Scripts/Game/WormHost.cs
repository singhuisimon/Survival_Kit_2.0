// WormHost.cs
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
    public class WormHost : ScriptBehaviour
    {
        [SerializeField] private bool isStationary = false;
        [SerializeField] private bool hasSplit = false;

        [SerializeField] private float speed;
        [SerializeField] private float stationaryTimer;
        [SerializeField] private float timer;

        [SerializeField] private float targetX;
        [SerializeField] private float targetY;
        [SerializeField] private float targetZ;

        private const uint INVALID_ENTITY = 0xffffffffu;
        
        private uint playerID = INVALID_ENTITY;
        private const string TAG_PLAYER = "Player";

        public override void OnStart()
        {
            // Declare WormHost Started; Check own entity
            LogMessage("======= WormHost started (EntityID = " + EntityID + ") =======");
            
            // Find Player Target
            playerID = SceneFindEntityByName(TAG_PLAYER);
            LogMessage("======= playerID: " + playerID.ToString() + " =======");

            // Set Worm Host's modes
            isStationary = false;
            hasSplit = false;

            // Set Speed and Timers (Serialize Field Value)
            speed = 1.5f;
            stationaryTimer = 4.0f;
            timer = 0.0f;

            // Set Target Location (Serialize Field Values) as Its Own
            Vector3 ownPosition = GetPosition((uint)EntityID);
            targetX = ownPosition.X;
            targetY = ownPosition.Y;
            targetZ = ownPosition.Z;
        }

        public override void OnUpdate(float deltaTime)
        {
            if(hasSplit){
                return;
            }

            // Get direction vector with regards to updated player position
            // TODO: Find allies/core
            Vector3 ownPosition = GetPosition((uint)EntityID);
            Vector3 targetPosition = GetPosition((uint)playerID);
            targetX = targetPosition.X;
            targetY = targetPosition.Y;
            targetZ = targetPosition.Z;

            Vector3 direction = targetPosition - ownPosition;

            // Get magnitude of distance
            float magnitude = SimpleMath.Sqrt(direction.X * direction.X + direction.Y * direction.Y + direction.Z * direction.Z);
    
            // If WormHost is near/at target
            if(magnitude < 5.0f){
                isStationary = true;
            }

            if(isStationary){ // If near/at target
                
                timer += deltaTime;

                LogMessage("======= WormHost EntityID " + EntityID + "; time: " + timer + " =======");

                if(timer >= stationaryTimer){

                    LogMessage("======= WormHost EntityID " + EntityID + " start splitting =======");
                    OnSplit();
                }

            } else { // If not at target
                
                // Normalise direction
                float inverseMag = 1.0f / magnitude;
                Vector3 normDirection = direction * inverseMag;

                // Calculate and set new position
                Vector3 newPosition = ownPosition + normDirection * speed * deltaTime;
                SetPosition(EntityID, ref newPosition);

            }
        }

        public void OnSplit()
        {
            if (hasSplit) return;
            hasSplit = true;

            // Disappear for now
            Vector3 newScale = new Vector3(0,0,0);
            SetScale(EntityID, ref newScale);
            
            Event.Publish("WormHostSplit", VectorToString(GetPosition((uint)EntityID)));
            LogMessage("======= WormHostSplit EntityID " + EntityID + "; Position: " + VectorToString(GetPosition((uint)EntityID)) + " =======");
        }

        public string VectorToString(Vector3 vec){
            
            string concat = vec.X.ToString() + "," + vec.Y.ToString() + "," + vec.Z.ToString();
            return concat;
        }
    }
    
}
