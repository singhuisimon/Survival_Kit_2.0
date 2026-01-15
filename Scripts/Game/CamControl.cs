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

        [SerializeField] private float cameraDistance = 35.0f;

        [SerializeField] private float cameraOffSet = 5.0f;
        [SerializeField] private float playerRotationSpeed = 10.0f;

        private uint cameraEntityID = 0;
        private uint lookAtEntityID = 0;
        private uint camControlEntityID = 0;
        private uint playerEntityID = 0;

        private float currentPitch = 0.0f;
        private float currentYaw = 0.0f;

        private Vector3 currentCameraOffset = Vector3.Zero;

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
            //Quat currentRot = GetRotation(camControlEntityID);
            //Quat currentRot = GetRotation(lookAtEntityID);
            //Vector3 euler = currentRot.ToEuler();

            //currentYaw = euler.Y;
            //currentPitch = euler.X;
            currentYaw = 0.0f;
            currentPitch = 0.0f;

            // Also set the entity's rotation to identity to match
            Quat initialRot = Quat.FromAxisAngle(Vector3.Up, currentYaw);
            SetRotation(lookAtEntityID, ref initialRot);
        }

        public override void OnUpdate(float deltaTime){
            HandleMouseRotation();
            HandleMovement();
            MovePlayerTowardLookAt(deltaTime);
            UpdateCamControlPosition(deltaTime);
            RotatePlayerTowardsMovement(deltaTime);
            UpdateCameraTarget();
        }

        private void HandleMouseRotation(){
            Input.GetMouseDelta(out float deltaX, out float deltaY);

            if (deltaX != 0 || deltaY != 0) {
                LogMessage("Mouse Delta - X: " + deltaX + ", Y: " + deltaY);
            }

            float yawChange = -deltaX * mouseSensitivity;
            float pitchChange = -deltaY * mouseSensitivity;

            currentYaw += yawChange;
            currentPitch += pitchChange;

            float pitchLimitedRad = pitchLimitedDegree * SimpleMath.DEG_TO_RAD;
            currentPitch = SimpleMath.Clamp(currentPitch, -pitchLimitedRad, pitchLimitedRad);

            Quat yawRotation = Quat.FromAxisAngle(Vector3.Up, currentYaw);

            Vector3 localRight = yawRotation.Rotate(Vector3.Right);

            Quat pitchRotation = Quat.FromAxisAngle(localRight, currentPitch);
            Quat finalRotation = yawRotation * pitchRotation;

            //SetRotation(camControlEntityID, ref finalRotation);
            SetRotation(lookAtEntityID, ref finalRotation);
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

                //Quat camRotation = GetRotation(camControlEntityID);
                Quat camRotation = GetRotation(lookAtEntityID);
                //Vector3 camForward = camRotation.Forward;
                //LogMessage("[CamControl] Camera control forward for handle movment: X: " +
                //camForward.X + ", Y:" + camForward.Y + ", Z: " + camForward.Z);

                Vector3 moveDir = camRotation.Rotate(localMoveDir);

                LogMessage("[CamControl] local move dir for handle movment: X: " +
                localMoveDir.X + ", Y:" + localMoveDir.Y + ", Z: " + localMoveDir.Z);

                Vector3 force = moveDir * movementSpeed;
                RigidbodyAddForce(lookAtEntityID, ref force);

                //Debugging purpose
                //Quat rot = GetRotation(camControlEntityID);
                //LogMessage("Rot for cam X: " + rot.X.ToString() + ", Y: " + rot.Y.ToString() + ", Z: " + rot.Z.ToString() + ", W: " + rot.W.ToString());

                //Vector3 pos = GetPosition(camControlEntityID);
                //LogMessage("Pos for cam X: " + pos.X.ToString() + ", Y: " + pos.Y.ToString() + ", Z: " + pos.Z.ToString());
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

            //works but doesn't position itself behind the player at all time)
            // Vector3 playerPos = GetPosition(playerEntityID);
            // Vector3 camPos = GetPosition(cameraEntityID);
            // Vector3 targetpos = playerPos;
            // targetpos.Y = targetpos.Y + 15.0f;
            // targetpos.Z = targetpos.Z + 35.0f;

            // SetPosition(cameraEntityID, ref targetpos);

            //trying claude method
            Quat playerRot = GetRotation(playerEntityID);
            Vector3 playerForward = playerRot.Forward;
            Vector3 behindDirection = -playerForward;
            Vector3 playerPos = GetPosition(playerEntityID);
            Vector3 cameraOffSet = behindDirection * cameraDistance;
            cameraOffSet.Y += offsetCamHeight;

            Vector3 targetCameraPos = playerPos + cameraOffSet;

            // Optional: Smooth camera movement to avoid snapping
            Vector3 currentCameraPos = GetPosition(cameraEntityID);
            float t = SimpleMath.Clamp(camFollowSpeed * deltaTime, 0.0f, 1.0f);
            Vector3 newCameraPos = Vector3.Lerp(currentCameraPos, targetCameraPos, t);

            SetPosition(cameraEntityID, ref newCameraPos);
        }

        private void RotatePlayerTowardsMovement(float deltaTime){
            if(playerEntityID == 0 || lookAtEntityID == 0){
                return;
            }

            Vector3 velocity = RigidbodyGetVelocity(lookAtEntityID);
            
            if(velocity.SqrMagnitude < 0.1f){
                return;
            }

            float targetYaw = -SimpleMath.Atan2(velocity.X, -velocity.Z);
            // Create target rotation (rotation around Y axis)
            Quat targetRotation = Quat.FromAxisAngle(Vector3.Up, targetYaw);
            
            // Get current player rotation
            Quat currentRotation = GetRotation(playerEntityID);

            float t = SimpleMath.Clamp(playerRotationSpeed * deltaTime, 0.0f, 1.0f);
            Quat newRotation = Quat.Slerp(currentRotation, targetRotation, t);
            SetRotation(playerEntityID, ref newRotation);
        }

        private void UpdateCameraTarget(){
            if(cameraEntityID == 0 || playerEntityID == 0){
                LogMessage("[CamControl] returning at update cam target");
                return;
            }

            Vector3 playerPos = GetPosition(playerEntityID);

            Vector3 velocity = RigidbodyGetVelocity(lookAtEntityID);
            Vector3 targeOffset = Vector3.Zero;
            if(velocity.SqrMagnitude > 0.1f){
                Vector3 moveDir = velocity.Normalized;

                Vector3 perpendicular = Vector3.Cross(moveDir, Vector3.Up);
                targeOffset = perpendicular * cameraOffSet;
            }

            Vector3 targetPos = playerPos;
            targetPos.Y += offsetCamHeight;
            targetPos = targetPos + currentCameraOffset;

            //Vector3 playerPos = GetPosition(playerEntityID);
            //playerPos.Y = playerPos.Y + offsetCamHeight;
            //SetTarget(cameraEntityID, ref playerPos);
            SetTarget(cameraEntityID, ref targetPos);
        }
    }
}