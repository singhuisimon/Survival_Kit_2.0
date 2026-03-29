// // Copyright (C) 2026 DigiPen Institute of Technology.
// // Reproduction or disclosure of this file or its contents without the prior
// // written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using System;
using static Engine.Scene;
using static Engine.Event;
using static Engine.Audio;
using static Engine.Prefab;
using static Engine.Logger;
using static Engine.Transform;
using static Engine.SpriteRenderer;

namespace Game
{
    /// <summary>
    /// Handles transition from game screen to this cut scene
    /// Detects ANY keyboard or mouse button press to trigger transition
    /// </summary>
    public class WinCutScenePopUp : ScriptBehaviour
    {
        // Entity IDs
        private uint Level2CutScene0103ID;
        private uint Level2CutScene0204ID;
        private uint Level2CutScene05ID;
        private uint Level2CutScene06ID;
        private uint Level2CutScene07ID;
        private uint Level2CutScene08ID;
        private uint CutSceneSkipButtonID;
        private uint WinPopUpID;
        private uint NextLevelButtonID;
        private uint MainMenuButtonID;
        private uint CreditEndButtonID;
        private uint WinPopUpFinalID;
        private uint MouseUIAudioID;

        // // ScenePath
        // private string mainMenuscenePath = "Resources/Sources/Scenes/MainMenu.json";
        // private string level2Path = "Resources/Sources/Scenes/level2_graphic_card.json";

        // Prefab Audio Path
        private const string audiosmack = "Sources/Prefabs/Audio_Smack_Impact.prefab";
        private const string audiochime = "Sources/Prefabs/Audio_Com_Chime.prefab";
        private const string audiowin3 = "Sources/Prefabs/Audio_VO_Lvl3_Win.prefab";
        private const string audiowin2 = "Sources/Prefabs/Audio_Win_Level2_VO.prefab";
        private const string audiowinTrench = "Sources/Prefabs/Audio_VO_TrenchWin.prefab";
        private const string audioskip = "Sources/Prefabs/Audio_Spam_Skip_Click.prefab";

        // Frame sequence
        private uint[] frameSequence = new uint[8];

        private static readonly float[] FRAME_DURATIONS = 
        {
            1.0f, // frame 1 0103
            0.5f, // frame 2 0204
            1.0f, // frame 3 0103
            1.5f, // frame 4 0204
            1.0f, // frame 5 05
            2.20f, // frame 6 06
            1.50f, // frame 7 07
            1.0f // frame 8 08
        };

        private const int FRAME_COUNT    = 8;
        private const int LAST_FRAME_IDX = FRAME_COUNT - 1;

        // Runtime state
        private bool  entitiesReady    = false;
        private int   currentFrame     = 0;
        private float frameTimer       = 0.0f;
        private bool  cutsceneDone     = false;
        private bool  skipped          = false;
        private bool  wasMousePressed  = false;
        private bool  popupVisible     = false;

        // Delayed transition
        private bool   pendingTransition     = false;
        private string pendingScene          = "";
        private float  transitionTimer       = 0.0f;
        private const float TRANSITION_DELAY = 1.0f;

        public override void OnStart()
        {
            LogMessage("WinCutSceneTransition: Started");
            LogMessage("WinCutScenePopUp: Started – next scene: "
                       + WinCutSceneContext.NextScene
                       + " | final level: " + WinCutSceneContext.IsFinalLevel);
                       
            Level2CutScene0204ID = SceneFindEntityByTag("Level2CutScene0204");
            Level2CutScene07ID = SceneFindEntityByTag("Level2CutScene07");
            Level2CutScene08ID = SceneFindEntityByTag("Level2CutScene08");
            Level2CutScene06ID = SceneFindEntityByTag("Level2CutScene06");
            Level2CutScene05ID = SceneFindEntityByTag("Level2CutScene05");
            Level2CutScene0103ID = SceneFindEntityByTag("Level2CutScene0103");
            CutSceneSkipButtonID = SceneFindEntityByTag("CutSceneSkipButton");
            WinPopUpID = SceneFindEntityByTag("WinPopUpLvl12");
            NextLevelButtonID = SceneFindEntityByTag("NextLevelButton");
            MainMenuButtonID = SceneFindEntityByTag("MainMenuButton");
            CreditEndButtonID = SceneFindEntityByTag("CreditEndButton");
            WinPopUpFinalID = SceneFindEntityByTag("WinPopUpFinal");
            MouseUIAudioID = SceneFindEntityByName("MouseUIClick");

            LogMessage("WinCutSceneTransition: Found entity Level2CutScene0204 "+  Level2CutScene0204ID);
            LogMessage("WinCutSceneTransition: Found entity Level2CutScene07 "+ Level2CutScene07ID);
            LogMessage("WinCutSceneTransition: Found entity Level2CutScene06 "+ Level2CutScene06ID);
            LogMessage("WinCutSceneTransition: Found entity Level2CutScene05 "+ Level2CutScene05ID);
            LogMessage("WinCutSceneTransition: Found entity Level2CutScene0103 "+ Level2CutScene0103ID);
            LogMessage("WinCutSceneTransition: Found entity CutSceneSkipButton "+ Level2CutScene0103ID);
            LogMessage("WinCutSceneTransition: Found entity WinPopUpLvl12 "+ WinPopUpID);
            LogMessage("WinCutSceneTransition: Found entity NextLevelButton "+ NextLevelButtonID);
            LogMessage("WinCutSceneTransition: Found entity MainMenuButton "+ MainMenuButtonID);
            LogMessage("WinCutSceneTransition: Found entity MainMenuButtonFinal "+ CreditEndButtonID);
            LogMessage("WinCutSceneTransition: Found entity WinPopUpFinal "+ WinPopUpFinalID);
            LogMessage("WinCutSceneTransition: Found entity Level2CutScene08 "+ Level2CutScene08ID);
            LogMessage("WinCutSceneTransition: Found entity CreditEndButton "+ CreditEndButtonID);
            LogMessage("WinCutSceneTransition: Found entity MouseUIAudio "+ MouseUIAudioID);


            // If any entity is missing
            if (Level2CutScene0103ID  == 0 || Level2CutScene0204ID  == 0 ||
                Level2CutScene05ID    == 0 || Level2CutScene06ID    == 0 ||
                Level2CutScene07ID    == 0 || CutSceneSkipButtonID  == 0 ||
                WinPopUpID            == 0 || NextLevelButtonID     == 0 ||
                MainMenuButtonID      == 0 || CreditEndButtonID == 0 ||
                WinPopUpFinalID       == 0 || Level2CutScene08ID    == 0 ||
                CreditEndButtonID     == 0 || MouseUIAudioID == 0)
            {
                LogError("WinCutScenePopUp: One or more entities not found – aborting");
                return;
            }


            // Frame mapping change
            frameSequence[0] = Level2CutScene0103ID;   // frame 1
            frameSequence[1] = Level2CutScene0204ID;   // frame 2
            frameSequence[2] = Level2CutScene0103ID;   // frame 3 (reuses 0103)
            frameSequence[3] = Level2CutScene0204ID;   // frame 4 (reuses 0204)
            frameSequence[4] = Level2CutScene05ID;     // frame 5
            frameSequence[5] = Level2CutScene06ID;     // frame 6
            frameSequence[6] = Level2CutScene07ID;     // frame 7
            frameSequence[7] = Level2CutScene08ID;     // frame 8

            entitiesReady = true;

            // Initial state: Hide everthing but show frame 1 and skip button
            SetIsVisible(CutSceneSkipButtonID,  false);
            SetIsVisible(WinPopUpID,            false);
            SetIsVisible(NextLevelButtonID,     false);
            SetIsVisible(MainMenuButtonID,      false);
            SetIsVisible(CreditEndButtonID, false);
            SetIsVisible(WinPopUpFinalID, false);
            HideAllFrames();

            ShowFrame(0);
            //ShowSkipButton();
            SetIsVisible(CutSceneSkipButtonID, true);
           
            currentFrame = 0;
            frameTimer   = 0.0f;
        }

        public override void OnUpdate(float deltaTime )
        {
            if (!entitiesReady)
            {
                return;
            }

            // Delayed scene transition
            if (pendingTransition)
            {
                transitionTimer += deltaTime;
                if (transitionTimer >= TRANSITION_DELAY)
                {
                    bool ok = Scene.SceneLoadFromFile(pendingScene);
                    if (!ok) LogError("WinCutScenePopUp: Failed to load scene: " + pendingScene);
                }
                return;
            }

            // Input 
            bool mouseDown        = Input.IsMouseButtonPressed(MouseButton.Left);
            bool mouseJustPressed = mouseDown && !wasMousePressed;
            wasMousePressed       = mouseDown;


            // Popup button handling
            if (popupVisible && mouseJustPressed)
            {
                // Next Level (non-final levels only)
                if (!WinCutSceneContext.IsFinalLevel
                    && Collision2D.IsMouseCollidingWithEntity(NextLevelButtonID))
                {
                    AudioPlay(MouseUIAudioID);
                    LogMessage("WinCutScenePopUp: Next Level clicked  "
                               + WinCutSceneContext.NextScene);
                    pendingScene      = WinCutSceneContext.NextScene;
                    pendingTransition = true;
                    return;
                }
                // Main Menu – normal layout (non-final levels)
                if (!WinCutSceneContext.IsFinalLevel
                    && Collision2D.IsMouseCollidingWithEntity(MainMenuButtonID))
                {
                    AudioPlay(MouseUIAudioID);
                    LogMessage("WinCutScenePopUp: Main Menu clicked (normal)");
                    pendingScene      = WinCutSceneContext.MAIN_MENU_SCENE;
                    pendingTransition = true;
                    return;
                }

                // Main Menu – final level layout
                if (WinCutSceneContext.IsFinalLevel
                    && Collision2D.IsMouseCollidingWithEntity(CreditEndButtonID))
                {
                    AudioPlay(MouseUIAudioID);
                    LogMessage("WinCutScenePopUp: Credit End clicked (Credit)");
                    pendingScene      = WinCutSceneContext.CREDITS_END_SCENE;
                    pendingTransition = true;
                    return;
                }
            }
            // Skip 
            if (!cutsceneDone && !skipped && mouseJustPressed)
            {
                if (Collision2D.IsMouseCollidingWithEntity(CutSceneSkipButtonID))
                {
                    LogMessage("WinCutScenePopUp: Skip pressed");
                    //Instantiate Audio
                    PrefabInstantiate(audioskip);
                    SkipToLastFrame();
                    return;
                }
            }

            // Frame timer
            if (!cutsceneDone)
            {
                frameTimer += deltaTime;
                if (frameTimer >= FRAME_DURATIONS[currentFrame])
                    AdvanceFrame();
            }
        }

        private void AdvanceFrame()
        {
            if (currentFrame >= LAST_FRAME_IDX)
            {
                OnCutsceneComplete();
                return;
            }

             // Only hide if the next frame uses a different entity
            if (frameSequence[currentFrame] != frameSequence[currentFrame + 1])
                HideFrame(currentFrame);

            currentFrame++;
            ShowFrame(currentFrame);

            PlayAudio();

            frameTimer = 0.0f;

            LogMessage("WinCutScenePopUp: Frame " + (currentFrame + 1));

        }

        private void SkipToLastFrame()
        {
            skipped = true;
            HideFrame(currentFrame);
            currentFrame = LAST_FRAME_IDX;
            ShowFrame(LAST_FRAME_IDX);
            //HideSkipButton();
            SetIsVisible(CutSceneSkipButtonID, false);
            OnCutsceneComplete();
        }

        private void OnCutsceneComplete()
        {
            cutsceneDone = true;
            SetIsVisible(CutSceneSkipButtonID, false);
            //SetIsVisible(WinPopUpID, true);


            if (WinCutSceneContext.IsFinalLevel)
            {
                // Final level – centred Main Menu button only
                LogMessage("WinCutScenePopUp: Final level popup");
                SetIsVisible(NextLevelButtonID,     false);
                SetIsVisible(MainMenuButtonID,      false);
                SetIsVisible(WinPopUpID, false);
                SetIsVisible(WinPopUpFinalID, true);
                SetIsVisible(CreditEndButtonID, true);

                //Instantiate the win VO for level 3
                PrefabInstantiate(audiowin3);
            }
            else
            {
                // Non-final level – Next Level + Main Menu side by side
                LogMessage("WinCutScenePopUp: Normal popup");
                SetIsVisible(NextLevelButtonID,     true);
                SetIsVisible(WinPopUpID, true);
                SetIsVisible(MainMenuButtonID,      true);
                SetIsVisible(CreditEndButtonID, false);

                //Checking if it was trench run prev
                if(WinCutSceneContext.NextScene == WinCutSceneContext.LEVEL2_SCENE)
                {
                    //Instantiate win VO for trench
                    PrefabInstantiate(audiowinTrench);
                } 
                //Checking if it was level2 prev
                else if (WinCutSceneContext.NextScene == WinCutSceneContext.LEVEL3_SCENE)
                {
                    //Instantiate win VO for level2
                    PrefabInstantiate(audiowin2);
                }

            }
            //LogMessage("WinCutScenePopUp: Cutscene complete – hook up win popup here");
            popupVisible = true;
            // ShowWinPopup();
            // Scene.SceneLoadFromFile("...");
            Publish("WinScreenShow", "");
        }

        private void ShowFrame(int index)
        {
            //Vector3 pos = new Vector3(SCREEN_CENTER_X, SCREEN_CENTER_Y, FRAME_Z);
            SetIsVisible(frameSequence[index], true);
        }

        private void HideFrame(int index)
        {
            //Vector3 pos = new Vector3(SCREEN_CENTER_X, OFFSCREEN_Y, FRAME_Z);
            SetIsVisible(frameSequence[index], false);
        }

        private void HideAllFrames()
        {
            for (int i = 0; i < FRAME_COUNT; i++)
            {
                bool alreadyHidden = false;
                for (int j = 0; j < i; j++)
                {
                    if (frameSequence[j] == frameSequence[i])
                    {
                        alreadyHidden = true;
                        break;
                    }
                }
                if (!alreadyHidden)
                    HideFrame(i);
            }
        }

        private void PlayAudio()
        {
            if(currentFrame == 1 || currentFrame == 3)
            {
                //instantiate the sound
                PrefabInstantiate(audiosmack);
            }

            if(currentFrame == 6)
            {
                //instantiate the sound
                PrefabInstantiate(audiochime);
            }
        }

        public override void OnDestroy()
        {
            LogMessage("WinCutScenePopUp: Destroyed");
        }
    }
}