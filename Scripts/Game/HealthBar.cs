using Engine;
using System;
using static Engine.Log;
using static Engine.Transform;
using static Engine.Scene;

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
            LogMessage("HealthBar started!");

            // Find Own Entity
            healthID = SceneFindEntityByName("HealthBar");
            LogMessage("HealthBar EntityID: " + healthID.ToString());

            // SETUP - Health to FULL
            health = fullHealth;

            // SETUP - Serialize Field values
            Vector3 originalScale = GetScale((uint)healthID);
            fullWidth = originalScale.X;
            currentWidth = originalScale.X;

            // SETUP - Booleans
            hasDied = false;
            immunity = false;

            // SUBSCRIBE to Events
            Event.Subscribe("SMActivated", OnRestart); // SM Activated - SetUp, Immunity OFF
            Event.Subscribe("SMDeactivated", OnResult); // SM Deactivated - Immunity ON
            Event.Subscribe("BotnetAttackedPlayer", OnBotnetAttackedPlayer);

        }

        public override void OnUpdate(float deltaTime)
        {
            // if (Input.IsKeyPressed(KeyCode.RightAlt))
            // {
            //     health = 0f;
            //     Vector3 scale = GetScale((uint)healthID);
            //     scale.X = 0;
            //     SetScale((uint)healthID, ref scale);
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
            Event.Unsubscribe("SMActivated", OnRestart); // SM Activated - SetUp, Immunity OFF
            Event.Unsubscribe("SMDeactivated", OnResult);
            Event.Unsubscribe("BotnetAttackedPlayer", OnBotnetAttackedPlayer);
        }

        private void Die()
        {
            LogMessage("Player died!");
            Event.Publish("PlayerHasDied", healthID.ToString());
        }

        private void OnBotnetAttackedPlayer(string eventName, string payload)
        {
            if (immunity == false && hasDied == false)
            {
                // Find Bot
                ulong botId = ulong.Parse(payload);
                LogMessage("UI: Botnet attacked the player! Bot ID = " + botId);

                // Deduct health
                health -= botnetAttack;

                if (health < 0f)
                {
                    health = 0f;
                }

                // Adjust length and apply to serialize field values
                float ratio = health / fullHealth;
                Vector3 scale = GetScale((uint)healthID);
                scale.X = fullWidth * ratio;
                SetScale((uint)healthID, ref scale);
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

            Vector3 scale = GetScale((uint)healthID);
            scale.X = fullWidth;
            SetScale((uint)healthID, ref scale);

            currentWidth = fullWidth;

            hasDied = false;
            immunity = false;
        }
    }
}
