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
        private const float botnetAttack = 20f;
        private uint healthID = 0;

        public override void OnStart()
        {
			Engine.InternalCalls.Log("HealthBar started!");
            healthID = InternalCalls.Scene_FindEntityByName("HealthBar");
            Engine.InternalCalls.Log("HealthBar EntityID: " + healthID.ToString());

            health = fullHealth;

            EventSystem.Subscribe("BotnetAttackedPlayer", OnBotnetAttackedPlayer);

            Vector3 originalScale = Transform.GetScale((uint)healthID);
            fullWidth = originalScale.X;
            currentWidth = originalScale.X;
        }

        public override void OnUpdate(float deltaTime)
        {
            if (Input.IsKeyPressed(KeyCode.RightAlt))
            //if (Input.IsKeyPressed(KeyCode.M)) 
            // Note to Lily: Right Alt dosen't work for me
            // so I temporarily switched it to 'M' button for testing
            {
                health = 0f;
                Vector3 scale = Transform.GetScale((uint)healthID);
                scale.X = 0;
                Transform.SetScale((uint)healthID, ref scale);
                currentWidth = scale.X;
            }

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
            Engine.InternalCalls.Log("Player died!");
            EventSystem.Unsubscribe("BotnetAttackedPlayer", OnBotnetAttackedPlayer);
            EventSystem.Publish("PlayerHasDied", healthID.ToString());

        }

        private void OnBotnetAttackedPlayer(string eventName, string payload)
        {
            ulong botId = ulong.Parse(payload);

            Engine.InternalCalls.Log("UI: Botnet attacked the player! Bot ID = " + botId);

            health -= botnetAttack;

            if (health < 0f)
            {
                health = 0f;
            }

            float ratio = health / fullHealth;
            Vector3 scale = Transform.GetScale((uint)healthID);
            scale.X = fullWidth * ratio;
            Transform.SetScale((uint)healthID, ref scale);
            currentWidth = scale.X;
        }
    }
}
