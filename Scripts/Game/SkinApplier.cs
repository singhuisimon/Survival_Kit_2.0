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

        // Skin index -> trail StartColor / base color (RGBA)
        public static readonly Vector4[] TRAIL_START_COLOR = {
            new Vector4(0.0196f, 1.0f,   0.0f, 0.2f),  // 0 = Default (green)  R:5   G:255 B:0
            new Vector4(0.0f,    0.368f,  1.0f, 0.2f),  // 1 = Blue             R:0   G:94  B:255
            new Vector4(0.776f,  0.0f,    1.0f, 0.2f),  // 2 = Purple           R:198 G:0   B:255
            new Vector4(1.0f,    0.490f,  0.0f, 0.2f)   // 3 = Rainbow          R:255 G:125 B:0
        };

        // Skin index -> trail EndColor / emission color (RGBA)
        public static readonly Vector4[] TRAIL_END_COLOR = {
            new Vector4(0.102f, 1.0f,   0.0f, 0.0f),   // 0 = Default (green)  R:26  G:255 B:0
            new Vector4(0.0f,   0.675f, 1.0f, 0.0f),   // 1 = Blue             R:0   G:172 B:255
            new Vector4(0.937f, 0.0f,   1.0f, 0.0f),   // 2 = Purple           R:239 G:0   B:255
            new Vector4(1.0f,   0.631f, 0.0f, 0.0f)    // 3 = Rainbow          R:255 G:161 B:0
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

            // Apply trail colors (TrailComponent StartColor/EndColor)
            Vector4 startColor = TRAIL_START_COLOR[skinIndex];
            Vector4 endColor = TRAIL_END_COLOR[skinIndex];
            TrailSystem.SetStartColor(playerId, ref startColor);
            TrailSystem.SetEndColor(playerId, ref endColor);
            LogMessage("SkinApplier: Trail colors set - Start(" + startColor.X + "," + startColor.Y + "," + startColor.Z + "," + startColor.W + ") End(" + endColor.X + "," + endColor.Y + "," + endColor.Z + "," + endColor.W + ")");

            // Also update particle colors to match (use alpha 1.0 for particles, not trail alpha)
            Vector4 particleMin = new Vector4(startColor.X, startColor.Y, startColor.Z, 1.0f);
            Vector4 particleMax = new Vector4(endColor.X, endColor.Y, endColor.Z, 1.0f);
            ParticleSystem.SetColorMin(playerId, ref particleMin);
            ParticleSystem.SetColorMax(playerId, ref particleMax);

            LogMessage("SkinApplier: Skin applied successfully!");
        }
    }
}
