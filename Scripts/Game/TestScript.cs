using System;
using Engine;

namespace Game
{
    public class TestScript
    {
        public int EntityID;
        private int frameCount = 0;

        public void OnStart()
        {
            Console.WriteLine("[C#] TestScript.OnStart() called!");
            Engine.InternalCalls.Log("TestScript started!");
            Engine.InternalCalls.LogWarning("This is a warning from C#!");
            Engine.InternalCalls.LogError("This is an error from C#!");
        }

        public void OnUpdate(float deltaTime)  //  MUST be exactly this signature
        {
            frameCount++;
                //            Engine.InternalCalls.Log("stesting dotnet ading");

            //Engine.InternalCalls.Log("su no hot reloading");
           if (Input.IsKeyPressed(KeyCode.W))
            {
                 //movement.Z += 1.0f;
                 Engine.InternalCalls.Log("stesting dotnet ading");


             }
            Console.WriteLine("[C#] OnUpdate frame: " + frameCount);

            if (frameCount % 60 == 0)
            {
                Console.WriteLine("[C#] OnUpdate frame: " + frameCount);
                //Engine.InternalCalls.Log("OnUpdate called from C#!");
            }
        }

        public void OnDestroy()
        {
            Engine.InternalCalls.Log("TestScript destroyed!");
        }
    }
}
