using Engine;
using System;

namespace Game
{
    public class HealthBar : ScriptBehaviour
    {
        [SerializeField]
        private float health;

        [SerializeField]
        private float fullWidth = 5.5f;

        [SerializeField]
        private float currentWidth = 1f;

        // Constant default values
        private const float fullHealth = 100f;
        private const float botnetAttack = 20f;

        // Own Entity ID
        private uint healthID = 0;

        // Booleans
        private bool immunity = false;
        private bool hasDied = false;

        public override void OnStart()
        {
            // Declare HealthBar Started
            Engine.InternalCalls.Log("HealthBar started!");

            // Find Own Entity
            healthID = InternalCalls.Scene_FindEntityByName("HealthBar");
            Engine.InternalCalls.Log("HealthBar EntityID: " + healthID.ToString());

            // SETUP - Health to FULL
            health = fullHealth;

            // SETUP - Serialize Field values
            Vector3 originalScale = Transform.GetScale((uint)healthID);
            fullWidth = originalScale.X;
            currentWidth = originalScale.X;

            // SETUP - Booleans
            hasDied = false;
            immunity = false;

            // SUBSCRIBE to Events
            EventSystem.Subscribe("SMActivated", OnRestart); // SM Activated - SetUp, Immunity OFF
            EventSystem.Subscribe("SMDeactivated", OnResult); // SM Deactivated - Immunity ON
            EventSystem.Subscribe("BotnetAttackedPlayer", OnBotnetAttackedPlayer);

        }

        public override void OnUpdate(float deltaTime)
        {
            // if (Input.IsKeyPressed(KeyCode.RightAlt))
            // {
            //     health = 0f;
            //     Vector3 scale = Transform.GetScale((uint)healthID);
            //     scale.X = 0;
            //     Transform.SetScale((uint)healthID, ref scale);
            //     currentWidth = scale.X;
            // }

            if (health <= 0f && hasDied == false)
            {
                Die();
                hasDied = true;
            }
        }

        public override void OnDestroy()
        {
            // UNSUBSCRIBE to Events
            EventSystem.Unsubscribe("SMActivated", OnRestart); // SM Activated - SetUp, Immunity OFF
            EventSystem.Unsubscribe("SMDeactivated", OnResult);
            EventSystem.Unsubscribe("BotnetAttackedPlayer", OnBotnetAttackedPlayer);
        }

        private void Die()
        {
            Engine.InternalCalls.Log("Player died!");
            EventSystem.Publish("PlayerHasDied", healthID.ToString());
        }

        private void OnBotnetAttackedPlayer(string eventName, string payload)
        {
            if (immunity == false && hasDied == false)
            {
                // Find Bot
                ulong botId = ulong.Parse(payload);
                Engine.InternalCalls.Log("UI: Botnet attacked the player! Bot ID = " + botId);

                // Deduct health
                health -= botnetAttack;

                if (health < 0f)
                {
                    health = 0f;
                }

                // Adjust length and apply to serialize field values
                float ratio = health / fullHealth;
                Vector3 scale = Transform.GetScale((uint)healthID);
                scale.X = fullWidth * ratio;
                Transform.SetScale((uint)healthID, ref scale);
                currentWidth = scale.X;
            }
        }

        private void OnResult(string eventName, string payload)
        {
            immunity = true;
        }

        private void OnRestart(string eventName, string payload)
        {
            health = fullHealth;

            Vector3 scale = Transform.GetScale((uint)healthID);
            scale.X = fullWidth;
            Transform.SetScale((uint)healthID, ref scale);

            currentWidth = fullWidth;

            hasDied = false;
            immunity = false;
        }
    }
}
