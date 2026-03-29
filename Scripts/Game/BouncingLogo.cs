using Engine;
using static Engine.Transform;
using static Engine.Logger;

namespace Game
{
    public class BouncingLogo : ScriptBehaviour
    {
        [SerializeField] private float speedX = 120.0f;
        [SerializeField] private float speedY = 80.0f;

        private float velocityX;
        private float velocityY;

        private const float SCREEN_WIDTH = 1280.0f;
        private const float SCREEN_HEIGHT = 670.0f;
        private const float MIN_Y = 0.0f;

        public override void OnStart()
        {
            velocityX = speedX;
            velocityY = speedY;
            LogMessage("BouncingLogo: Initialized");
        }

        public override void OnUpdate(float deltaTime)
        {
            Vector3 pos = GetPosition((uint)EntityID);
            Vector3 scale = GetScale((uint)EntityID);

            float halfW = scale.X;
            float halfH = scale.Y;

            pos.X += velocityX * deltaTime;
            pos.Y += velocityY * deltaTime;

            if (pos.X - halfW <= 0.0f)
            {
                pos.X = halfW;
                velocityX = -velocityX;
            }
            else if (pos.X + halfW >= SCREEN_WIDTH)
            {
                pos.X = SCREEN_WIDTH - halfW;
                velocityX = -velocityX;
            }

            if (pos.Y - halfH <= MIN_Y)
            {
                pos.Y = MIN_Y + halfH;
                velocityY = -velocityY;
            }
            else if (pos.Y + halfH >= SCREEN_HEIGHT)
            {
                pos.Y = SCREEN_HEIGHT - halfH;
                velocityY = -velocityY;
            }

            SetPosition((uint)EntityID, ref pos);
        }
    }
}
