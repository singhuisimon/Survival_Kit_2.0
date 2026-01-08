using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;

namespace Game{
    public class PlayerCamera : ScriptBehaviour{

        [SerializeField] public bool moveAllowed = false;
        [SerializeField] private bool clampControl = true;

        [SerializeField] private float trackingTime = 100.0f;
        [SerializeField] private string playerName = "Player";

        //these 2 need replace!!! Vector2 doesn't exist in the "SerializeField"
        //[SerializeField] Vector2 rotateCamera;
        //[SerializeField] Vector2 rotateCamera_YClamp;

        [SerializeField] private float rotateCamera_X;
        [SerializeField] private float rotateCamera_Y;

        //IN DEGREE
        [SerializeField] private float rotateCamera_YClampMin = -85.0f;
        [SerializeField] private float rotateCamera_YClampMax = 85.0f;

        [SerializeField] float rotateSensitivity;
        [SerializeField] float rotateDamping;

        private const uint INVALID_ENTITY = 0xffffffffu;
        private uint playerEntityID = INVALID_ENTITY;

        private const float fixedUpdateRate = 0.02f; //2 miliseconds
        private float elapsedRate = 0.0f;
        private bool fixedUpdate = false;

        public override void OnStart(){

            //need request for function to hide mouse

            moveAllowed = true;
            elapsedRate = 0.02f; //so that camera can move immediately

            //find player 
            playerEntityID = SceneFindEntityByName(playerName);
        }

        public override void OnUpdate(float deltaTime){

            elapsedRate += deltaTime;
            
            if(elapsedRate >= fixedUpdateRate){
                elapsedRate = 0.0f; //reset the elapsed rate for next
                fixedUpdate = true;
            }

            if(moveAllowed && clampControl){
                if(fixedUpdate){
                    MovePlayerCamera();
                    fixedUpdate = false;
                }

                RotatePlayerCamera(deltaTime);
            }
        }

        void MovePlayerCamera(){
            if(playerEntityID != INVALID_ENTITY){
                Vector3 currentPos = GetPosition((uint)EntityID);
                Vector3 playerPos = GetPosition(playerEntityID);
                Vector3 transformz = camSlerp(currentPos, playerPos, trackingTime * fixedUpdateRate);

                SetPosition((uint)EntityID, ref transformz);
            }
        }

        void RotatePlayerCamera(float deltaTime){
            Vector2 mouseDelta;
            Input.GetMouseDelta(out mouseDelta.X, out mouseDelta.Y);

            //DOUBLE CHECK DO I DO IT IN RAD OR DEG. BECAUSE ROTATECAM_YCLAMP IS IN DEG
            //IN RAD

            // float yawDelta = mouseDelta.X * rotateSensitivity;
            // float pitchDelta = mouseDelta.Y * rotateSensitivity;

            // yawDelta *= DEG_TO_RAD;
            // pitchDelta *= DEG_TO_RAD;

            // rotateCamera_X -= yawDelta;
            // rotateCamera_Y += pitchDelta;

            // rotateCamera_Y = SimpleMath.Clamp(rotateCamera_Y, rotateCamera_YClampMin, rotateCamera_YClampMax);
            
            // Vector3 targetEuler = new Vector3(-rotateCamera_Y, rotateCamera_X, 0.0f);

            // Quat targetRot = Quat.FromEuler(targetEuler);

            // float t = rotateDamping * deltaTime;
            // Quat currentRot = GetRotation((uint)EntityID);

            // Quat smoothed = Quat.Slerp(currentRot, targetRot, t);

            // SetRotation((uint)EntityID, ref smoothed);

            //IN DEG

            rotateCamera_X -= mouseDelta.X * rotateSensitivity;
            rotateCamera_Y += mouseDelta.Y * rotateSensitivity;

            rotateCamera_Y = SimpleMath.Clamp(rotateCamera_Y, rotateCamera_YClampMin, rotateCamera_YClampMax);

            float yawRad = rotateCamera_X * SimpleMath.DEG_TO_RAD;
            float pitchRad = rotateCamera_Y * SimpleMath.DEG_TO_RAD;

            Vector3 eulerRad = new Vector3(-pitchRad, yawRad, 0.0f); //pitch, yaw, roll

            Quat targetRot = Quat.FromEuler(eulerRad);

            float t = rotateDamping * deltaTime;
            Quat currentRot = GetRotation((uint)EntityID);

            Quat smoothed = Quat.Slerp(currentRot, targetRot, t);

            SetRotation((uint)EntityID, ref smoothed);
        }

        //functions to aid
        private Vector3 camSlerp(Vector3 start, Vector3 end, float t){

            // Clamp t to [0,1]
            t = SimpleMath.Clamp(t, 0.0f, 1.0f);

            Vector3 a = start.Normalized;
            Vector3 b = end.Normalized;

            float dot = SimpleMath.Clamp(Vector3.Dot(a, b), -1.0f, 1.0f);
            float theta = (SimpleMath.PI * 0.5f) - SimpleMath.Asin(dot);

            //if angle is small, fallback to lerp
            if(theta < 0.0001f){
                return Vector3.Lerp(start, end, t);
            }

            //float sinTheta = (float)Math.Sin(theta);
            float sinTheta = Sine(theta);

            // float w1 = (float)Math.Sin((1.0f - t) * theta) / sinTheta;
            // float w2 = (float)Math.Sin(t * theta) / sinTheta;
            float w1 = Sine((1.0f - t) * theta) / sinTheta;
            float w2 = Sine(t * theta) / sinTheta;

            return (a * w1) + (b * w2);
        }

        private float Sine(float x)
        {
            // Normalize to [-PI, PI]
            while (x > SimpleMath.PI) x -= 2.0f * SimpleMath.PI;
            while (x < -SimpleMath.PI) x += 2.0f * SimpleMath.PI;

            // Taylor series approximation
            float x2 = x * x;
            float x3 = x2 * x;
            float x5 = x3 * x2;
            float x7 = x5 * x2;

            return x - (x3 / 6.0f) + (x5 / 120.0f) - (x7 / 5040.0f);
        }

    }
}