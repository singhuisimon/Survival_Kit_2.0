using Engine;
using static Engine.SpriteRenderer;
using static Engine.Logger;

namespace Game
{
    public class HoverHighlight : ScriptBehaviour
    {
        // Base colour — white, fully opaque
        [SerializeField] private float baseR = 1.0f;
        [SerializeField] private float baseG = 1.0f;
        [SerializeField] private float baseB = 1.0f;
        [SerializeField] private float baseA = 1.0f;

        // Hover colour — bright yellow
        [SerializeField] private float hoverR = 1.0f;
        [SerializeField] private float hoverG = 0.85f;
        [SerializeField] private float hoverB = 0.0f;
        [SerializeField] private float hoverA = 1.0f;

        // How fast the colour transition is (units per second)
        [SerializeField] private float hoverSpeed = 8.0f;

        // Current lerp factor [0 = base, 1 = hover]
        private float _t = 0.0f;

        public override void OnStart()
        {
            SetColor((uint)EntityID, baseR, baseG, baseB, baseA);
            LogMessage("[HoverHighlight] Ready");
        }

        public override void OnUpdate(float deltaTime)
        {



            bool hovered = Collision2D.IsMouseCollidingWithEntity((uint)EntityID);

            // Drive _t toward 1 when hovered, 0 when not
            float target = hovered ? 1.0f : 0.0f;
            _t += (target - _t) * hoverSpeed * deltaTime;

            // Lerp each channel
            float r = baseR + (hoverR - baseR) * _t;
            float g = baseG + (hoverG - baseG) * _t;
            float b = baseB + (hoverB - baseB) * _t;
            float a = baseA + (hoverA - baseA) * _t;

            SetColor((uint)EntityID, r, g, b, a);
        }

        public override void OnDestroy()
        {
            LogMessage("[HoverHighlight] Destroyed");
        }
    }
}