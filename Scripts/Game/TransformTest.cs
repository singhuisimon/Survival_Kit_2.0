using Engine;
using static Engine.Transform;
using static Engine.Logger;
using static Engine.Scene;

namespace Game
{
    public class TransformTest : ScriptBehaviour
    {
        [SerializeField("Test Entity Name")] private string testEntityName = "TestEntity";
        [SerializeField("Tolerance")] private float tolerance = 0.01f;

        private uint testEntityID = 0;
        private int waitFrames = 0;

        private Vector3 positionTarget;
        private Quat rotationTarget;
        private Vector3 scaleTarget;

        public override void OnStart()
        {
            testEntityID = SceneFindEntityByName(testEntityName);
            if (testEntityID == 0)
            {
                LogError("[TransformTest] Could not find: " + testEntityName);
                return;
            }

            positionTarget = new Vector3(10f, 20f, 30f);
            rotationTarget = Quat.FromAxisAngle(Vector3.Up, SimpleMath.HALF_PI);
            scaleTarget = new Vector3(2f, 3f, 4f);

            SetWorldPosition(testEntityID, ref positionTarget);
            SetWorldRotation(testEntityID, ref rotationTarget);
            SetWorldScale(testEntityID, ref scaleTarget);

            waitFrames = 2;
            LogMessage("[TransformTest] All values set, waiting 2 frames...");
        }

        public override void OnUpdate(float deltaTime)
        {
            if (waitFrames <= 0) return;
            waitFrames--;
            if (waitFrames > 0) return;

            TestWorldPosition();
            TestWorldRotation();
            TestWorldScale();
        }

        private void TestWorldPosition()
        {
            Vector3 result = GetWorldPosition(testEntityID);
            bool pass = SimpleMath.Abs(result.X - positionTarget.X) < tolerance
                     && SimpleMath.Abs(result.Y - positionTarget.Y) < tolerance
                     && SimpleMath.Abs(result.Z - positionTarget.Z) < tolerance;

            LogMessage("[TransformTest] WorldPosition: " + (pass ? "PASS" : "FAIL")
                + " | Expected: (" + positionTarget.X + ", " + positionTarget.Y + ", " + positionTarget.Z + ")"
                + " | Got: (" + result.X + ", " + result.Y + ", " + result.Z + ")");
        }

        private void TestWorldRotation()
        {
            Quat result = GetWorldRotation(testEntityID);
            float dot = Quat.Dot(result, rotationTarget);
            bool pass = SimpleMath.Abs(dot) > (1f - tolerance);

            LogMessage("[TransformTest] WorldRotation: " + (pass ? "PASS" : "FAIL")
                + " | Dot: " + dot);
        }

        private void TestWorldScale()
        {
            Vector3 result = GetWorldScale(testEntityID);
            bool pass = SimpleMath.Abs(result.X - scaleTarget.X) < tolerance
                     && SimpleMath.Abs(result.Y - scaleTarget.Y) < tolerance
                     && SimpleMath.Abs(result.Z - scaleTarget.Z) < tolerance;

            LogMessage("[TransformTest] WorldScale: " + (pass ? "PASS" : "FAIL")
                + " | Expected: (" + scaleTarget.X + ", " + scaleTarget.Y + ", " + scaleTarget.Z + ")"
                + " | Got: (" + result.X + ", " + result.Y + ", " + result.Z + ")");
        }
    }
}