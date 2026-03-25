// Copyright (C) 2026 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

namespace Game
{
    /// <summary>
    /// Persists across SceneLoadFromFile calls – static memory is never wiped
    /// by a scene load. Mirrors the same pattern as LevelSelectController.IsLevel2Selected.
    ///
    /// Usage – in your game scene's win trigger, before loading the cutscene:
    ///
    ///   Level 1 win:
    ///     WinCutSceneContext.NextScene    = WinCutSceneContext.LEVEL2_SCENE;
    ///     WinCutSceneContext.IsFinalLevel = false;
    ///     Scene.SceneLoadFromFile(WinCutSceneContext.CUTSCENE_SCENE);
    ///
    ///   Level 2 win:
    ///     WinCutSceneContext.NextScene    = WinCutSceneContext.LEVEL3_SCENE;
    ///     WinCutSceneContext.IsFinalLevel = false;
    ///     Scene.SceneLoadFromFile(WinCutSceneContext.CUTSCENE_SCENE);
    ///
    ///   Level 3 win:
    ///     WinCutSceneContext.NextScene    = WinCutSceneContext.MAIN_MENU_SCENE;
    ///     WinCutSceneContext.IsFinalLevel = true;
    ///     Scene.SceneLoadFromFile(WinCutSceneContext.CUTSCENE_SCENE);
    /// </summary>
    public static class WinCutSceneContext
    {
        // Set both of these before loading the cutscene
        public static string NextScene    = MAIN_MENU_SCENE;
        public static bool   IsFinalLevel = false;

        public static string FinalScore = "0";
        public static string FinalTime  = "00 m : 00 s";

        // Scene paths
        public const string CUTSCENE_SCENE  = "Resources/Sources/Scenes/WinCutScene.json";
        public const string LEVEL1_SCENE    = "Resources/Sources/Scenes/trench_run.json";
        public const string LEVEL2_SCENE    = "Resources/Sources/Scenes/level2_graphic_card.json";
        public const string LEVEL3_SCENE    = "Resources/Sources/Scenes/level3_motherboard.json";
        public const string MAIN_MENU_SCENE = "Resources/Sources/Scenes/MainMenu.json";
        public const string CREDITS_END_SCENE = "Resources/Sources/Scenes/EndCreditTransition.json";
    }
}