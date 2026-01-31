using System;
using Engine;
using static Engine.Logger;
using static Engine.Text;

namespace Game
{
	/// <summary>
	/// TimerUI - Displays countdown timer in "00 m : 00 s" format
	/// Attach this script to a text entity to show game timer
	/// Counts down from 5 minutes to 0
	/// </summary>
	public class Timer : ScriptBehaviour
	{
		// ===== Settings =====
		[SerializeField("Starting Time (seconds)")]
		private float startingTime = 300.0f;  // 5 minutes = 300 seconds

		// ===== State =====
		private bool initialized = false;
		private float remainingTime = 0.0f;

		public override void OnStart()
		{
			LogMessage("=== TimerUI OnStart ===");
			LogMessage("TimerUI EntityID: " + EntityID);

			// Initialize with starting time
			remainingTime = startingTime;

			// Display initial time
			UpdateTimerDisplay();

			initialized = true;

			LogMessage("[TimerUI] Initialized - Starting at " + startingTime + " seconds");
		}

		public override void OnFixedUpdate(float deltaTime)
		{
			if (!initialized)
				return;

			// Count down
			remainingTime -= (deltaTime/2);

			// Clamp to 0 (don't go negative)
			if (remainingTime < 0.0f)
				remainingTime = 0.0f;

			// Update display
			UpdateTimerDisplay();
		}

		private void UpdateTimerDisplay()
		{
			// Format time as "00 m : 00 s"
			int minutes = (int)(remainingTime / 60);
			int seconds = (int)(remainingTime % 60);
			string timeText = string.Format("{0:00} m : {1:00} s", minutes, seconds);

			// Update the text display
			SetText((uint)EntityID, timeText);
		}

		public override void OnDestroy()
		{
			LogMessage("=== TimerUI Destroyed ===");
		}
	}
}