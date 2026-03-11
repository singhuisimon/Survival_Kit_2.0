using System;
using Engine;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;

namespace Game
{
	/// <summary>
	/// GunshipNameUI - Destroys both itself and the matching world-space name entity
	/// when the gunship dies. Attach to gunshipName1..6 and set allyIndex to match.
	/// </summary>
	public class GunshipNameUI : ScriptBehaviour
	{
		[SerializeField("Ally Index")]
		private int allyIndex = 1;  // 1..6

		private string destroyEventName = "";

		public override void OnStart()
		{
			LogMessage("=== GunshipNameUI OnStart ===");
			LogMessage("[GunshipNameUI] EntityID: " + EntityID + " allyIndex=" + allyIndex);

			destroyEventName = "GunshipNameDestroy:" + allyIndex;
			Subscribe(destroyEventName, OnGunshipDied);

			LogMessage("[GunshipNameUI] Listening for: " + destroyEventName);
		}

		private void OnGunshipDied(string eventName, string payload)
		{
			LogMessage("[GunshipNameUI] Gunship " + allyIndex + " died - destroying name entity");

			// Find and destroy the world-space name entity e.g. "gunshipName1"
			string nameEntityName = "gunshipName" + allyIndex;
			uint nameEntityID = SceneFindEntityByName(nameEntityName);

			if (nameEntityID != 0 && nameEntityID != 0xffffffffu)
			{
				LogMessage("[GunshipNameUI] Found " + nameEntityName + " (ID: " + nameEntityID + ") - destroying");
				SceneDestroyEntity(nameEntityID);
			}
			else
			{
				LogMessage("[GunshipNameUI] Could not find entity: " + nameEntityName);
			}

			// Destroy this script's entity too
			SceneDestroyEntity((uint)EntityID);
		}

		public override void OnDestroy()
		{
			if (!string.IsNullOrEmpty(destroyEventName))
				Unsubscribe(destroyEventName, OnGunshipDied);

			LogMessage("=== GunshipNameUI Destroyed ===");
		}
	}
}
