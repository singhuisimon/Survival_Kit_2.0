using Engine;
using static Engine.Logger;
using static Engine.Event;
using static Engine.SpriteRenderer;

namespace Game
{
    /// <summary>
    /// SkipTutorialController - Attach to any entity in a level scene.
    /// Displays a toggle checkbox: unticked = play tutorial, ticked = skip tutorial.
    /// Uses the same show/hide pattern as SettingsPopup mute buttons.
    /// Tick IsLevel1 / IsLevel2 / IsLevel3 in the scene editor to identify the level.
    /// </summary>
    public class SkipTutorialController : ScriptBehaviour
    {
        [SerializeField] private bool IsLevel1 = false;
        [SerializeField] private bool IsLevel2 = false;
        [SerializeField] private bool IsLevel3 = false;

        private const string SKIPTUTORIAL_UNTICKED_NAME = "SkipTutorial";
        private const string SKIPTUTORIAL_TICKED_NAME = "SkipTutorialTicked";

        private uint skipTutorialId;
        private uint skipTutorialTickedId;

        private bool initialized = false;
        private bool wasMousePressed = false;
        private bool isTicked = false;

        public override void OnStart()
        {
            LogMessage("SkipTutorialController: OnStart");

            skipTutorialId = Scene.SceneFindEntityByName(SKIPTUTORIAL_UNTICKED_NAME);
            skipTutorialTickedId = Scene.SceneFindEntityByName(SKIPTUTORIAL_TICKED_NAME);

            if (skipTutorialId == 0) LogError("SkipTutorialController: Could not find entity: " + SKIPTUTORIAL_UNTICKED_NAME);
            if (skipTutorialTickedId == 0) LogError("SkipTutorialController: Could not find entity: " + SKIPTUTORIAL_TICKED_NAME);

            ProgressTracker.LoadProgress();

            // Read current saved state for this level
            if (IsLevel1) isTicked = ProgressTracker.SkipTutorialLevel1;
            else if (IsLevel2) isTicked = ProgressTracker.SkipTutorialLevel2;
            else if (IsLevel3) isTicked = ProgressTracker.SkipTutorialLevel3;

            UpdateVisual();

            initialized = true;
            wasMousePressed = false;

            LogMessage("SkipTutorialController: Ready - isTicked=" + isTicked +
                       " IsLevel1=" + IsLevel1 + " IsLevel2=" + IsLevel2 + " IsLevel3=" + IsLevel3);
        }

        public override void OnUpdate(float deltaTime)
        {
            if (!initialized) return;

            bool isMousePressed = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = isMousePressed && !wasMousePressed;
            wasMousePressed = isMousePressed;

            if (!mouseJustPressed) return;

            // Check click on either entity (both occupy same position)
            bool clickedUnticked = skipTutorialId != 0 && Collision2D.IsMouseCollidingWithEntity(skipTutorialId);
            bool clickedTicked = skipTutorialTickedId != 0 && Collision2D.IsMouseCollidingWithEntity(skipTutorialTickedId);

            if (!clickedUnticked && !clickedTicked) return;

            // Toggle
            isTicked = !isTicked;

            // Save to ProgressTracker
            ProgressTracker.LoadProgress();
            if (IsLevel1) ProgressTracker.SkipTutorialLevel1 = isTicked;
            else if (IsLevel2) ProgressTracker.SkipTutorialLevel2 = isTicked;
            else if (IsLevel3) ProgressTracker.SkipTutorialLevel3 = isTicked;
            else { LogError("SkipTutorialController: No level flag set!"); return; }

            ProgressTracker.SaveProgress();
            UpdateVisual();

            LogMessage("SkipTutorialController: Toggled - isTicked=" + isTicked);
            Publish("SkipTutorialToggled", isTicked ? "true" : "false");
        }

        private void UpdateVisual()
        {
            if (skipTutorialId != 0) SetIsVisible(skipTutorialId, !isTicked);
            if (skipTutorialTickedId != 0) SetIsVisible(skipTutorialTickedId, isTicked);
        }

        public override void OnDestroy()
        {
            LogMessage("SkipTutorialController: Destroyed");
        }
    }
}
        