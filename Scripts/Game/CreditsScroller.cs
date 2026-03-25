using Engine;
using System;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.SpriteRenderer;
using static Engine.Scene;

namespace Game
{
    /// <summary>
    /// Show the end credit Scrolling.
    /// </summary>
    public class CreditsScroller : ScriptBehaviour
    {
        private uint CreditsContentID;
        private uint SkipCreditButtonID;

        [SerializeField] private float scrollSpeed = 45.0f;
        [SerializeField] private float fadeInDuration = 1.5f;
        [SerializeField] private float endYThreshold = -1804.900f;
        [SerializeField] private float endHoldDuration = 2.0f;

        private bool  scrolling         = true;
        private bool  finished          = false;
        private float endHoldTimer      = 0.0f;

        private bool  wasMousePressed   = false;
        private bool  isFadingIn        = true;

        private float fadeInElapsed    = 0.0f;

        public override void OnStart()
        {
            LogMessage("CreditScroller: Started");
            CreditsContentID = SceneFindEntityByTag("CreditsContent");
            SkipCreditButtonID = SceneFindEntityByTag("SkipCreditButton");

            if (CreditsContentID == 0)
            {
                LogError("[CreditsScroller] Could not find entity CreditsContent: " + CreditsContentID);
            }
            if (SkipCreditButtonID == 0)
            {
                LogError("[CreditsScroller] Could not find entity SkipCreditButton: " + SkipCreditButtonID);
            }

            if (CreditsContentID != 0)
            {
                SpriteRenderer.SetColor(CreditsContentID, 1.0f, 1.0f, 1.0f, 0.0f);
            }
            
            if (SkipCreditButtonID != 0)
            {
                SetIsVisible(SkipCreditButtonID, true);
            }

            scrolling = true;
            finished = false;
            endHoldTimer = 0.0f;
            isFadingIn = true;
            fadeInElapsed = 0.0f;
            wasMousePressed = false;
            LogMessage("[CreditsScroller] Initialised – scroll speed: " + scrollSpeed);
        }

        public override void OnUpdate(float deltaTime)
        {
            bool mouseDown        = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = mouseDown && !wasMousePressed;
            wasMousePressed       = mouseDown;

            // if skip button is press 
            if (mouseJustPressed
                && SkipCreditButtonID != 0
                && Collision2D.IsMouseCollidingWithEntity(SkipCreditButtonID))
            {
                LogMessage("[CreditsScroller] Skip clicked – loading Main Menu");
                LoadMainMenu();
                return;
            }

            // auto load after credits end
            if (finished)
            {
                endHoldTimer += deltaTime;
                if (endHoldTimer >= endHoldDuration)
                {
                    LogMessage("[CreditsScroller] Credits finished – loading Main Menu");
                    LoadMainMenu();
                }
                return;
            }

            // Fade in credits content
            if (isFadingIn && CreditsContentID != 0)
            {
                fadeInElapsed += deltaTime;
                float alpha = fadeInElapsed / fadeInDuration;
                if (alpha >= 1.0f)
                {
                    alpha      = 1.0f;
                    isFadingIn = false;
                }
                SpriteRenderer.SetColor(CreditsContentID, 1.0f, 1.0f, 1.0f, alpha);
            }

            // Scroll
            if (scrolling && CreditsContentID != 0)
            {
                // Fetch current position as Vector3, subtract Y to move content
                // upward, then write back using the engine's ref overload.
                Vector3 pos = Transform.GetPosition(CreditsContentID);
                pos.Y -= scrollSpeed * deltaTime;
                Transform.SetPosition(CreditsContentID, ref pos);

                // Once Y drops to endYThreshold all content has cleared the screen
                if (pos.Y <= endYThreshold)
                {
                    LogMessage("[CreditsScroller] Reached end threshold – credits done");
                    scrolling    = false;
                    finished     = true;
                    endHoldTimer = 0.0f;

                    if (SkipCreditButtonID != 0)
                        SetIsVisible(SkipCreditButtonID, false);
                }
            }
        }

        private void LoadMainMenu()
        {
            bool ok = Scene.SceneLoadFromFile(WinCutSceneContext.MAIN_MENU_SCENE);
            if (!ok)
                LogError("[CreditsScroller] Failed to load Main Menu: "
                         + WinCutSceneContext.MAIN_MENU_SCENE);
        }

        public override void OnDestroy()
        {
            LogMessage("=== CreditsScroller Destroyed ===");
        }
    }
}