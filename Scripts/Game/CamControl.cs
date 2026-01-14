using Engine;
using System;
using static Engine.Transform;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Camera;
using static Engine.Rigidbody;

namespace Game{
    /// <Summary>
    /// Cam Control class controls a empty parent containing the player camera
    /// as well as a LookAt Object
    /// </Summary>
    public class CamControl : ScriptBehaviour{
        [SerializeField] private string lookAtName = "LookAt";
        [SerializeField] private string cameraName = "PlayerCam";
        [SerializeField] private string camControlName = "CamControl";
        [SerializeField] private string playerName = "Player";
        [SerializeField] private float mouseSensitivity = 0.002f;
        [SerializeField] private float movementSpeed = 333.0f;
        [SerializeField] private float playerFollowSpeed = 5.0f;
        [SerializeField] private float camFollowSpeed = 10.0f;
        [SerializeField] private float pitchLimitedDegree = 85.0f;
        [SerializeField] private float offsetCamHeight = 15.0f;

        private uint cameraEntityID = 0;
        private uint lookAtEntityID = 0;
        private uint camControlEntityID = 0;
        private uint playerEntityID = 0;

        private float currentPitch = 0.0f;
        private float currentYaw = 0.0f;

        public override void OnStart(){
            cameraEntityID = SceneFindEntityByName(cameraName);
            lookAtEntityID = SceneFindEntityByName(lookAtName);
            camControlEntityID = SceneFindEntityByName(camControlName);
            playerEntityID = SceneFindEntityByName(playerName);

            if(cameraEntityID == 0){
                LogMessage("[CamControl] player cam cannot be found");
            } 
            if(lookAtEntityID == 0){
                LogMessage("[CamControl] look at entity cannot be found");
            }
            if(camControlEntityID == 0){
                LogMessage("[CamControl] cam control entity cannot be found");
            }
            if(playerEntityID == 0){
                LogMessage("[CamControl] player entity cannot be found");
            }

            //Quat currentRot = GetRotation((uint)EntityID);
            Quat currentRot = GetRotation(camControlEntityID);
            Vector3 euler = currentRot.ToEuler();

            currentYaw = euler.Y;
            currentPitch = euler.X;
        }

        public override void OnUpdate(float deltaTime){
            HandleMouseRotation();
            HandleMovement();
            MovePlayerTowardLookAt(deltaTime);
            UpdateCamControlPosition(deltaTime);
            UpdateCameraTarget();
        }

        private void HandleMouseRotation(){
            Input.GetMouseDelta(out float deltaX, out float deltaY);

            currentYaw -= deltaX * mouseSensitivity;
            currentPitch -= deltaY * mouseSensitivity;

            float pitchLimitedRad = pitchLimitedDegree * SimpleMath.DEG_TO_RAD;
            currentPitch = SimpleMath.Clamp(currentPitch, -pitchLimitedRad, pitchLimitedRad);

            Quat yawRotation = Quat.FromAxisAngle(Vector3.Up, currentYaw);
            Quat pitchRotation = Quat.FromAxisAngle(Vector3.Right, currentPitch);
            Quat finalRotation = yawRotation * pitchRotation;

            SetRotation(camControlEntityID, ref finalRotation);
            LogMessage("Setting cam rot: " + finalRotation.X.ToString() + ", Y: " + 
            finalRotation.Y.ToString() + ", Z: " + finalRotation.Z.ToString() +
            ", W: " + finalRotation.W.ToString());
            //SetRotation(cameraEntityID, ref finalRotation);
        }

        private void HandleMovement(){
            if(lookAtEntityID == 0){
                LogMessage("[CamControl] returning at handlemovement");
                return;
            }

            Vector3 localMoveDir = Vector3.Zero;

            if(Input.IsKeyPressed(KeyCode.W)){
                LogMessage("W IS PRESSED");
                localMoveDir.Z -= 1.0f;
            }
            if(Input.IsKeyPressed(KeyCode.A)){
                LogMessage("A IS PRESSED");
                localMoveDir.X -= 1.0f;
            }
            if(Input.IsKeyPressed(KeyCode.S)){
                LogMessage("S IS PRESSED");
                localMoveDir.Z += 1.0f;
            }
            if(Input.IsKeyPressed(KeyCode.D)){
                LogMessage("D IS PRESSED");
                localMoveDir.X += 1.0f;
            }

            if(localMoveDir.SqrMagnitude > 0.001f){
                LogMessage("Adding force");
                localMoveDir = localMoveDir.Normalized;

                Quat camRotation = GetRotation(camControlEntityID);
                Vector3 moveDir = camRotation.Rotate(localMoveDir);

                Vector3 force = moveDir * movementSpeed;
                RigidbodyAddForce(lookAtEntityID, ref force);

                //Debugging purpose
                Quat rot = GetRotation(camControlEntityID);
                LogMessage("Rot for cam X: " + rot.X.ToString() + ", Y: " + rot.Y.ToString() + ", Z: " + rot.Z.ToString() + ", W: " + rot.W.ToString());

                Vector3 pos = GetPosition(camControlEntityID);
                LogMessage("Pos for cam X: " + pos.X.ToString() + ", Y: " + pos.Y.ToString() + ", Z: " + pos.Z.ToString());
            }
        }

        private void MovePlayerTowardLookAt(float deltaTime){
            if(playerEntityID == 0 || lookAtEntityID == 0){
                LogMessage("[CamControl] moveplayer toward look at returning");
                return;
            }

            Vector3 playerPos = GetPosition(playerEntityID);
            Vector3 targetPos = GetPosition(lookAtEntityID);

            float t = SimpleMath.Clamp(playerFollowSpeed * deltaTime, 0.0f, 1.0f);
            Vector3 newPlayerPos = Vector3.Lerp(playerPos, targetPos, t);

            SetPosition(playerEntityID, ref newPlayerPos);
        }

        private void UpdateCamControlPosition(float deltaTime){
            if(playerEntityID == 0 || cameraEntityID == 0){
                LogMessage("[CamControl] update cam control pos returning");
                return;
            }

            //works but jittery
            // Vector3 playerPos = GetPosition(playerEntityID);
            // Vector3 currentPos = GetPosition(cameraEntityID);
            // Vector3 targetpos = playerPos;
            // targetpos.Y = targetpos.Y + 15.0f;
            // targetpos.Z = targetpos.Z + 35.0f; 

            // float t = SimpleMath.Clamp(camFollowSpeed * deltaTime, 0.0f, 1.0f);
            // Vector3 newCamControlPos = Vector3.Lerp(targetpos, playerPos, t);

            // SetPosition(cameraEntityID, ref newCamControlPos);

            Vector3 playerPos = GetPosition(playerEntityID);
            Vector3 camPos = GetPosition(cameraEntityID);
            Vector3 targetpos = playerPos;
            targetpos.Y = targetpos.Y + 15.0f;
            targetpos.Z = targetpos.Z + 35.0f;

            SetPosition(cameraEntityID, ref targetpos);
        }

        private void UpdateCameraTarget(){
            if(cameraEntityID == 0 || playerEntityID == 0){
                LogMessage("[CamControl] returning at update cam target");
                return;
            }

            Vector3 playerPos = GetPosition(playerEntityID);
            playerPos.Y = playerPos.Y + offsetCamHeight;
            SetTarget(cameraEntityID, ref playerPos);
        }
    }
}