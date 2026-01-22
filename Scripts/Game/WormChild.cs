// WormChild.cs
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
using System.Numerics;

namespace Game
{
    public class WormChild : ScriptBehaviour
    {
        [SerializeField] private float targetX;
        [SerializeField] private float targetY;
        [SerializeField] private float targetZ;

        [SerializeField] private float OGscaleX;
        [SerializeField] private float OGscaleY;
        [SerializeField] private float OGscaleZ;

        private const uint INVALID_ENTITY = 0xffffffffu;
        
        private const string EVENT_HOST_SPLIT = "WormHostSplit";

        private bool hasSplit;

        public override void OnStart()
        {
            // Check own entity
            LogMessage("======= WormChild started (EntityID = " + EntityID + ") =======");

            Engine.Vector3 ownPosition = GetPosition((uint)EntityID);

            targetX = ownPosition.X;
            targetY = ownPosition.Y;
            targetZ = ownPosition.Z;

            Engine.Vector3 ownScale = GetScale((uint)EntityID);
            OGscaleX = 0.001f;
            OGscaleY = 0.001f;
            OGscaleZ = 0.001f;

            Engine.Vector3 disappearScale = new Engine.Vector3(0,0,0);
            SetScale(EntityID, ref disappearScale);

            hasSplit = false;

            Subscribe("WormHostSplit", OnHostSplit);
            LogMessage("----------------- Subscribing to WormHostSplit ----------------- ");
            
        }

        public override void OnUpdate(float deltaTime)
        {   
            if(hasSplit){

                LogMessage("Aiming at: " + targetX + ", " + targetY + ", " + targetZ);
            }
        }

        public override void OnDestroy()
        {
            Unsubscribe(EVENT_HOST_SPLIT, OnHostSplit);
        }

        private void OnHostSplit(string eventName, string payload)
        {
            Engine.Vector3 appearScale = new Engine.Vector3(OGscaleX,OGscaleY,OGscaleZ);
            SetScale(EntityID, ref appearScale);

            Engine.Vector3 aim = VectorToString(payload);
            targetX = aim.X;
            targetY = aim.Y;
            targetZ = aim.Z;

            Engine.Vector3 globalPosition = SimpleMath.LocalChildtoWorld((uint)EntityID);
            Engine.Vector3 distanceVec = aim - globalPosition;

            float yaw = SimpleMath.Atan2(distanceVec.X, distanceVec.Z);
            Engine.Vector3 upAxis = new Engine.Vector3(0.0f, 1.0f, 0.0f);
            Quat yawQ = Quat.FromAxisAngle(upAxis, yaw);

            float horizLen = SimpleMath.Sqrt(distanceVec.X*distanceVec.X + distanceVec.Z*distanceVec.Z);
            float pitch = SimpleMath.Atan2(distanceVec.Y, horizLen);

            Engine.Vector3 localRight = new Engine.Vector3(1.0f, 0.0f, 0.0f);
            Engine.Vector3 rightAxis = QuatMultiplyVec3(yawQ,  localRight);
            Quat pitchQ = Engine.Quat.FromAxisAngle(rightAxis, -pitch);

            Quat finalRotation = pitchQ * yawQ;
            SetRotation((uint)EntityID, ref finalRotation);
            
            hasSplit = true;
        }

        public Engine.Vector3 VectorToString(string payload){
            
            string[] parts = payload.Split(',');

            float x = float.Parse(parts[0]);
            float y = float.Parse(parts[1]);
            float z = float.Parse(parts[2]);

            return new Engine.Vector3(x, y, z);
        }

        public Engine.Vector3 QuatMultiplyVec3 (Quat q, Engine.Vector3 v)
        {
            float x = q.X;
            float y = q.Y;
            float z = q.Z;
            float w = q.W;

            // Cross product:
            float crossX = 2.0f * (y * v.Z - z * v.Y);
            float crossY = 2.0f * (z * v.X - x * v.Z);
            float crossZ = 2.0f * (x * v.Y - y * v.X);

            // Final rotated vector: v + w*t + cross(q.xyz, t)
            float rotatedX = v.X + w * crossX + (y * crossZ - z * crossY);
            float rotatedY = v.Y + w * crossY + (z * crossX - x * crossZ);
            float rotatedZ = v.Z + w * crossZ + (x * crossY - y * crossX);

            return new Engine.Vector3(rotatedX, rotatedY, rotatedZ);
        }
    }
    
}
