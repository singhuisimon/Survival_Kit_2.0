// Copyright (C) 2024-2025 DigiPen Institute of Technology.
// Reproduction or disclosure of this file or its contents without the prior
// written consent of DigiPen Institute of Technology is prohibited.

using Engine;
using static Engine.Scene;
using static Engine.Logger;

namespace Game
{
    /// <summary>
    /// Applies the equipped skin's emissive texture and trail colors to the Player entity on level load.
    /// Reads ProgressTracker.EquippedSkin and swaps the emissive texture + trail colors accordingly.
    /// </summary>
    public class SkinApplier : ScriptBehaviour
    {
        private const string PLAYER_ENTITY_NAME = "Player";

        // Skin index -> emissive texture filename
        private static readonly string[] SKIN_TEXTURES = {
            "PlayerModel_v003_Emissive_Bright.png",  // 0 = Default
            "PlayerModel_Blue_v003_Emissive.png",     // 1 = Blue
            "PlayerModel_Purple_v003_Emissive.png",   // 2 = Purple
            "PlayerModel_Rainbow_v003_Emissive.png"   // 3 = Rainbow
        };

        // Skin index -> trail StartColor (RGBA)
        private static readonly Vector4[] TRAIL_START_COLOR = {
            new Vector4(0.0f,  1.0f, 0.24f, 0.2f),   // 0 = Default (green)
            new Vector4(0.0f,  0.3f, 1.0f,  0.2f),    // 1 = Blue
            new Vector4(0.5f,  0.0f, 1.0f,  0.2f),    // 2 = Purple
            new Vector4(1.0f,  0.0f, 0.0f,  0.2f)     // 3 = Rainbow (red start)
        };

        // Skin index -> trail EndColor (RGBA)
        private static readonly Vector4[] TRAIL_END_COLOR = {
            new Vector4(0.09f, 1.0f, 0.0f, 0.0f),    // 0 = Default (green, faded)
            new Vector4(0.0f,  0.5f, 1.0f, 0.0f),     // 1 = Blue (faded)
            new Vector4(0.7f,  0.0f, 1.0f, 0.0f),     // 2 = Purple (faded)
            new Vector4(1.0f,  1.0f, 0.0f, 0.0f)      // 3 = Rainbow (yellow end, faded)
        };

        public override void OnStart()
        {
            ProgressTracker.LoadProgress();
            int skinIndex = ProgressTracker.EquippedSkin;

            LogMessage("SkinApplier: Equipped skin index = " + skinIndex);

            // Find the Player entity
            uint playerId = SceneFindEntityByName(PLAYER_ENTITY_NAME);
            if (playerId == 0)
            {
                LogError("SkinApplier: Could not find entity: " + PLAYER_ENTITY_NAME);
                return;
            }

            // Clamp skin index to valid range
            if (skinIndex < 0 || skinIndex >= SKIN_TEXTURES.Length)
            {
                LogError("SkinApplier: Invalid skin index " + skinIndex + ", defaulting to 0");
                skinIndex = 0;
            }

            // Apply emissive texture
            string textureName = SKIN_TEXTURES[skinIndex];
            LogMessage("SkinApplier: Applying emissive texture '" + textureName + "' to Player");
            MeshRenderer.SetEmissiveTexture(playerId, textureName);

            // Only apply trail/particle colors in Level 2 (not trench_run)
            uint[] turrets = SceneFindEntitiesByTag("EnemyTurret");
            bool isLevel1 = turrets != null && turrets.Length > 0;

            if (!isLevel1)
            {
                // Apply trail colors (TrailComponent StartColor/EndColor)
                Vector4 startColor = TRAIL_START_COLOR[skinIndex];
                Vector4 endColor = TRAIL_END_COLOR[skinIndex];
                TrailSystem.SetStartColor(playerId, ref startColor);
                TrailSystem.SetEndColor(playerId, ref endColor);
                LogMessage("SkinApplier: Trail colors set - Start(" + startColor.X + "," + startColor.Y + "," + startColor.Z + "," + startColor.W + ") End(" + endColor.X + "," + endColor.Y + "," + endColor.Z + "," + endColor.W + ")");

                // Also update particle colors to match
                ParticleSystem.SetColorMin(playerId, ref startColor);
                ParticleSystem.SetColorMax(playerId, ref endColor);
            }
            else
            {
                LogMessage("SkinApplier: Level 1 detected - keeping default trail colors");
            }

            LogMessage("SkinApplier: Skin applied successfully!");
        }
    }
}
