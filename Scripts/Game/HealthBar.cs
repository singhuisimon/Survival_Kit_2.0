using Engine;
using System;

namespace Game
{
    public class HealthBar : ScriptBehaviour
    {
        [SerializeField]
        private float health;

        [SerializeField]
        private float fullWidth = 1f;

        [SerializeField]
        private float currentWidth = 1f;

        private const float fullHealth = 100f;
        private const float botnetAttack = 10f;

        public override void OnStart()
        {
			Engine.InternalCalls.Log("HealthBar started!");
            Engine.InternalCalls.Log("EntityID: " + EntityID);

            health = fullHealth;

            EventSystem.Subscribe("BotnetAttackedPlayer", OnBotnetAttackedPlayer);

            Vector3 originalScale = Transform.GetScale((uint)EntityID);
            fullWidth = originalScale.X;
            currentWidth = originalScale.X;
        }

        public override void OnUpdate(float deltaTime)
        {
            if (health <= 0f)
            {
                Die();
            }
        }

        public override void OnDestroy()
        {
            EventSystem.Unsubscribe("BotnetAttackedPlayer", OnBotnetAttackedPlayer);
        }

        private void Die()
        {
            Log("Player died!");

            EventSystem.Unsubscribe("BotnetAttackedPlayer", OnBotnetAttackedPlayer);

        }

        private void OnBotnetAttackedPlayer(string eventName, string payload)
        {
            ulong botId = ulong.Parse(payload);

            Log("UI: Botnet attacked the player! Bot ID = " + botId);

            health -= botnetAttack;

            if (health < 0f)
            {
                health = 0f;
            }

            currentWidth = fullWidth * (health / fullHealth);
            Vector3 scale = Transform.GetScale((uint)EntityID);
            scale.X = currentWidth;
            Transform.SetScale((uint)EntityID, ref scale);
        }
    }
}
