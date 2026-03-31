using Engine;
using static Engine.Scene;
using static Engine.Transform;
using static Engine.MeshRenderer;

namespace Game
{
    public class FadeOutBlackObject : ScriptBehaviour
    {
        [SerializeField("PlayerCam Name")] private string playerCamName = "PlayerCam";
        [SerializeField("Fade Duration")] private float fadeDuration = 1.5f;
        [SerializeField("Start Delay")] private float startDelay = 0.5f;

        private float timer = 0.0f;
        uint playerCamID = 0;

        public override void OnStart()
        {
            playerCamID = SceneFindEntityByName(playerCamName);
            SetOpacity(EntityID, 1.0f);
        }

        public override void OnFixedUpdate(float deltaTime)
        {
            timer += deltaTime;

            if (timer < startDelay)
                return;
            Vector3 camPos = GetPosition(playerCamID);
            Quat camRot = GetRotation(playerCamID);
            SetPosition(Entity.EntityID, ref camPos);
            SetRotation(Entity.EntityID, ref camRot);
            float duration = fadeDuration <= 0.0f ? 0.0001f : fadeDuration;
            float t = (timer - startDelay) / duration;

            if (t < 0.0f) t = 0.0f;
            if (t > 1.0f) t = 1.0f;

            SetOpacity(Entity.EntityID, (1.0f - t));
        }
    }
}