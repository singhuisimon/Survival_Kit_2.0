using System;
using Engine;
using static Engine.Transform;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Rigidbody;
using static Engine.Prefab;
using static Engine.Audio;
using static Engine.Collision2D;

namespace Game
{
    public class QuickTest : ScriptBehaviour
    {
        void OnStart()
        {
            LogMessage("This script is running at entity id " + EntityID);
        }

        void OnUpdate(float elapsedTime)
        {
            if (Collision2D.IsMouseCollidingWithEntity(EntityID))
            {
                LogMessage("Yayy");
            }
            else
            {
                LogMessage("Booo");
            }
        }

        void OnDestroy()
        {

        }
    }
}