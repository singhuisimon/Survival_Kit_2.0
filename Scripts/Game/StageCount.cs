using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;
using static Engine.Event;

namespace Game
{
    /// <summary>
    /// StageCount - Displays current stage as "STAGE 1"
    /// Attach this to a text entity
    /// Static display - does not change during gameplay
    /// Hides on win or lose
    /// </summary>
    public class StageCount : ScriptBehaviour
    {
        // ===== Game Over Events =====
        private const string EVENT_PLAYER_DEAD = "PlayerDead";
        private const string EVENT_CORE_DESTROYED = "CoreMotherboardDestroyed";
        private const string EVENT_TIMER_FINISHED = "TimerFinished";

        private const string ENEMY_CORE_DEATH = "EnemyCoreDeath";
        private const string GAMEWIN = "GameWin";

        // ===== Settings =====
        [SerializeField("Stage Number")]
        private int stageNumber = 1;

        public override void OnStart()
        {
            LogMessage("=== StageCount OnStart ===");
            LogMessage("StageCount EntityID: " + EntityID);

            // Subscribe to game over events
            Event.Subscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Subscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Subscribe(EVENT_TIMER_FINISHED, OnGameOver);
            Event.Subscribe(GAMEWIN, OnGameOver);

            Event.Subscribe(ENEMY_CORE_DEATH, OnGameOver);


            // Set stage text
            //SetText((uint)EntityID, "STAGE " + stageNumber);

            LogMessage("[StageCount] Initialized - displaying STAGE " + stageNumber);
        }

        private void OnGameOver(string eventName, string payload)
        {
            LogMessage("[StageCount] Game over - hiding text");
            Text.SetIsVisible((uint)EntityID, false);
        }

        public override void OnDestroy()
        {
            Event.Unsubscribe(EVENT_PLAYER_DEAD, OnGameOver);
            Event.Unsubscribe(EVENT_CORE_DESTROYED, OnGameOver);
            Event.Unsubscribe(EVENT_TIMER_FINISHED, OnGameOver);
            LogMessage("=== StageCount Destroyed ===");
        }
    }
}