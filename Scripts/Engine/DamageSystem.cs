// DamageSystem.cs (Engine namespace)
using System;

namespace Engine
{
    /// <summary>
    /// Static helper to route damage through the event system.
    /// Scripts call DealDamage, DamageReceiver listens and applies it to Health.
    /// </summary>
    public static class DamageSystem
    {
        /// <summary>
        /// Publish a damage event targeted at a specific entity.
        /// Any DamageReceiver on that entity will consume it.
        /// </summary>
        public static void DealDamage(uint targetEntityId, float amount, uint attackerEntityId = 0)
        {
            if (amount <= 0.0f || targetEntityId == 0)
                return;

            // Simple "name=value;..." payload. Extend as needed.
            string payload = "amount=" + amount.ToString() + ";attacker=" + attackerEntityId.ToString();

            // Use per-entity event names so receivers can subscribe cheaply.
            string eventName = "Damage:" + targetEntityId.ToString();

            Event.Publish(eventName, payload);
        }

        /// <summary>
        /// Parse "amount=" from the payload string.
        /// Very small helper to avoid pulling in JSON just for damage.
        /// </summary>
        public static float ParseAmount(string payload, float defaultValue = 0.0f)
        {
            if (string.IsNullOrEmpty(payload))
                return defaultValue;

            // Expect "amount=10;attacker=5" style.
            string[] parts = payload.Split(';');
            for (int i = 0; i < parts.Length; ++i)
            {
                string part = parts[i];
                if (part.StartsWith("amount=", StringComparison.OrdinalIgnoreCase))
                {
                    string value = part.Substring("amount=".Length);
                    if (float.TryParse(value, out float result))
                        return result;
                }
            }

            return defaultValue;
        }
    }
}
