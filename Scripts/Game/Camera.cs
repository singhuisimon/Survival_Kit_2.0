using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Transform;

namespace Game
{
    public class Camera : ScriptBehaviour
    {
        [SerializeField] private string targetName = "Player";
        [SerializeField] private float followDistance = 9.24f;
        [SerializeField] private float followHeight = 4.55f;
        [SerializeField] private float followDamping = 10.0f;

        [SerializeField] private float rotateSensitivity = 4.0f;
        [SerializeField] private float rotateDamping = 20.0f;

        [SerializeField] private float minPitchDegrees = -85.0f;
        [SerializeField] private float maxPitchDegrees = 85.0f;

        [SerializeField] private bool invertX = false;
        [SerializeField] private bool invertY = true;

        private const uint INVALID_ENTITY = 0xffffffffu;

        private uint targetEntityID = INVALID_ENTITY;

        private Quat currentRotation;
        private Quat targetRotation;
        private float currentPitchRad = 0.0f;

        private bool initialized = false;

        public override void OnStart()
        {
            targetEntityID = SceneFindEntityByName(targetName);

            currentRotation = GetRotation((uint)EntityID);
            targetRotation = currentRotation;

            Vector3 forward = currentRotation.Forward;
            currentPitchRad = SimpleMath.Asin(forward.Y);

            SnapToTargetPosition();

            initialized = true;
            LogMessage("[Camera] Initialized. Following: " + targetName +
                " (ID: " + targetEntityID.ToString() + ")");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized || targetEntityID == INVALID_ENTITY)
                return;

            UpdateRotation(deltaTime);
            UpdatePosition(deltaTime);
        }

        private void UpdateRotation(float deltaTime)
        {
            float mouseDX, mouseDY;

            Input.GetMouseDelta(out mouseDX, out mouseDY);

            LogMessage("Mouse Delta X: " + mouseDX + " Mouse Delta Y: " + mouseDY);

            if (invertX) mouseDX = -mouseDX;
            if (invertY) mouseDY = -mouseDY;

            float yawDeltaDeg = mouseDX * rotateSensitivity;
            float pitchDeltaDeg = mouseDY * rotateSensitivity;

            float yawDeltaRad = yawDeltaDeg * SimpleMath.DEG_TO_RAD;
            float pitchDeltaRad = pitchDeltaDeg * SimpleMath.DEG_TO_RAD;

            if (pitchDeltaRad != 0.0f)
            {
                float minPitchRad = minPitchDegrees * SimpleMath.DEG_TO_RAD;
                float maxPitchRad = maxPitchDegrees * SimpleMath.DEG_TO_RAD;

                float newPitch = currentPitchRad + pitchDeltaRad;

                if (newPitch < minPitchRad) newPitch = minPitchRad;
                else if (newPitch > maxPitchRad) newPitch = maxPitchRad;

                pitchDeltaRad = newPitch - currentPitchRad;
                currentPitchRad = newPitch;
            }

            float sqMag = (yawDeltaRad * yawDeltaRad) + (pitchDeltaRad * pitchDeltaRad);
            if (sqMag <= 0.00000001f)
                return;

            if (yawDeltaRad != 0.0f)
            {
                Vector3 worldUp = new Vector3(0.0f, 1.0f, 0.0f);
                Quat yawQuat = Quat.FromAxisAngle(worldUp, yawDeltaRad);
                targetRotation = yawQuat * targetRotation;
            }

            if (pitchDeltaRad != 0.0f)
            {
                Vector3 rightAxis = targetRotation.Right;
                Quat pitchQuat = Quat.FromAxisAngle(rightAxis, pitchDeltaRad);
                targetRotation = pitchQuat * targetRotation;
            }

            float t = rotateDamping * deltaTime;
            if (t > 1.0f) t = 1.0f;
            else if (t < 0.0f) t = 0.0f;

            Quat newRotation = Quat.Slerp(currentRotation, targetRotation, t);
            newRotation = newRotation.Normalized;

            currentRotation = newRotation;
            SetRotation((uint)EntityID, ref currentRotation);
        }

        private void UpdatePosition(float deltaTime)
        {
            Vector3 targetPos = GetPosition(targetEntityID);
            Vector3 currentPos = GetPosition((uint)EntityID);

            Vector3 forward = currentRotation.Forward;
            Vector3 up = currentRotation.Up;

            Vector3 desiredPos = new Vector3(
                targetPos.X - forward.X * followDistance + up.X * followHeight,
                targetPos.Y - forward.Y * followDistance + up.Y * followHeight,
                targetPos.Z - forward.Z * followDistance + up.Z * followHeight
            );

            float t = followDamping * deltaTime;
            if (t > 1.0f) t = 1.0f;
            else if (t < 0.0f) t = 0.0f;

            Vector3 newPos = LerpVector3(currentPos, desiredPos, t);
            SetPosition((uint)EntityID, ref newPos);
        }

        private void SnapToTargetPosition()
        {
            if (targetEntityID == INVALID_ENTITY)
                return;

            Vector3 targetPos = GetPosition(targetEntityID);

            Vector3 forward = currentRotation.Forward;
            Vector3 up = currentRotation.Up;

            Vector3 snapPos = new Vector3(
                targetPos.X - forward.X * followDistance + up.X * followHeight,
                targetPos.Y - forward.Y * followDistance + up.Y * followHeight,
                targetPos.Z - forward.Z * followDistance + up.Z * followHeight
            );

            SetPosition((uint)EntityID, ref snapPos);
        }

        private static Vector3 LerpVector3(Vector3 a, Vector3 b, float t)
        {
            if (t <= 0.0f) return a;
            if (t >= 1.0f) return b;

            return new Vector3(
                a.X + (b.X - a.X) * t,
                a.Y + (b.Y - a.Y) * t,
                a.Z + (b.Z - a.Z) * t
            );
        }
    }
}
