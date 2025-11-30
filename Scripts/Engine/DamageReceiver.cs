// DamageReceiver.cs (Engine namespace)
using System;

namespace Engine
{
    /// <summary>
    /// Component / behaviour that listens for damage events targeted at its entity
    /// and forwards them to an attached Health component.
    ///
    /// - Subscribes to "Damage:{EntityID}" on start.
    /// - On event, parses amount and calls Health.TakeDamage().
    /// </summary>
    public class DamageReceiver : ScriptBehaviour
    {
        [SerializeField("Health Component")]
        //private Health health;

        private bool subscribed = false;
        private string eventName;
        private EventSystem.ScriptEventHandler handler;

        /// <summary>
        /// Called by the engine when the script instance is created.
        /// Sets up the per-entity event name and subscribes to it.
        /// </summary>
        public override void OnStart()
        {
            // Build event name for this entity
            eventName = "Damage:" + EntityID.ToString();

            // Lazily bind handler once
            if (handler == null)
            {
                handler = OnDamageEvent;
            }

            Subscribe();
        }

        /// <summary>
        /// Called by the engine when the script instance is destroyed.
        /// Ensures we unsubscribe from the event bus.
        /// </summary>
        public override void OnDestroy()
        {
            Unsubscribe();
        }

        private void Subscribe()
        {
            if (subscribed || string.IsNullOrEmpty(eventName) || handler == null)
                return;

            EventSystem.Subscribe(eventName, handler);
            subscribed = true;
        }

        private void Unsubscribe()
        {
            if (!subscribed || string.IsNullOrEmpty(eventName) || handler == null)
                return;

            EventSystem.Unsubscribe(eventName, handler);
            subscribed = false;
        }

        /// <summary>
        /// Callback invoked by the EventSystem when a matching damage event is published.
        /// </summary>
        private void OnDamageEvent(string name, string payload)
        {
            //if (health == null)
            //    return;

            float amount = DamageSystem.ParseAmount(payload, 0.0f);
            if (amount <= 0.0f)
                return;

            //health.TakeDamage(amount);
        }
    }
}
