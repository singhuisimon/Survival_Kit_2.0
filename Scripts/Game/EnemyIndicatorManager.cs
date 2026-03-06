using Engine;
using System;
using System.Collections.Generic;
using static Engine.Transform;
using static Engine.SpriteRenderer;
using static Engine.Logger;
using static Engine.Scene;
using static Engine.Prefab;

namespace Game
{

    public static class EnemyRegistry
    {
        private static readonly List<uint> s_enemies = new List<uint>();

        public static void Register(uint entityID)
        {
            if (!s_enemies.Contains(entityID))
                s_enemies.Add(entityID);
        }

        public static void Unregister(uint entityID)
        {
            s_enemies.Remove(entityID);
        }

        public static IReadOnlyList<uint> Enemies => s_enemies;
    }

    public class EnemyIndicatorManager : ScriptBehaviour
    {
        [SerializeField("Camera Entity Name")]
        private string cameraName = "PlayerCam";

        [SerializeField("Detection Radius")]
        private float detectionRadius = 2000.0f;

        [SerializeField("Screen Half Width")]
        private float screenHalfWidth = 640.0f;

        [SerializeField("Screen Half Height")]
        private float screenHalfHeight = 360.0f;

        // Radius in screen-space pixels of the indicator circle, centered at (screenHalfWidth, screenHalfHeight).
        [SerializeField("Indicator Circle Radius")]
        private float circleRadius = 260.0f;

        [SerializeField("Edge Padding")]
        private float edgePadding = 20.0f;

        // Dot product threshold for suppressing the indicator when the enemy
        // is visibly on screen.  Represents the cosine of the half-FOV angle.
        // 0.6 == 53° half-angle (suits a ~90 degree FOV). Raise toward 1.0 to make
        // the indicator disappear only when the enemy is near dead-centre.
        [SerializeField("On Screen Threshold")]
        private float onScreenThreshold = 0.58f;

        [SerializeField("Max Indicators")]
        private int maxIndicators = 8;

        [SerializeField("Indicator Prefab Path")]
        private string indicatorPrefabPath = "Sources/Prefabs/EnemyIndicator.prefab";

        private uint cameraEntityID = 0;
        private uint[] indicatorPool;
        private bool initialized = false;


        public override void OnStart()
        {
            cameraEntityID = SceneFindEntityByName(cameraName);
            if (cameraEntityID == 0)
            {
                LogError("[EnemyIndicatorManager] Camera not found: " + cameraName);
                return;
            }

            // Spawn the indicator pool from the prefab
            indicatorPool = new uint[maxIndicators];
            for (int i = 0; i < maxIndicators; i++)
            {
                indicatorPool[i] = PrefabInstantiate(indicatorPrefabPath);

                if (indicatorPool[i] == 0)
                {
                    LogError("[EnemyIndicatorManager] Failed to instantiate: " + indicatorPrefabPath);
                    return;
                }

                // Hide all indicators at startup.
                SetIsVisible(indicatorPool[i], false);
            }

            initialized = true;
            LogMessage("[EnemyIndicatorManager] Initialized with " + maxIndicators + " indicators.");
        }


        public override void OnUpdate(float deltaTime)
        {
            if (!initialized || cameraEntityID == 0)
                return;

            if (GameState.IsPaused)
            {
                HideAllIndicators();
                return;
            }

            UpdateIndicators();
        }

        public override void OnDestroy()
        {
            if (indicatorPool == null) return;
            for (int i = 0; i < indicatorPool.Length; i++)
                if (indicatorPool[i] != 0)
                    SceneDestroyEntity(indicatorPool[i]);
        }

        private void UpdateIndicators()
        {
            Vector3 camPos = GetPosition(cameraEntityID);

            // Derive camera basis from the camera's current rotation,
            // mirroring what SpaceshipController does in UpdateCameraRotationFromMouse.
            Quat camRot = GetRotation(cameraEntityID);
            Vector3 camFwd = camRot.Forward;   // -Z in engine convention
            Vector3 camRight = camRot.Right;
            Vector3 camUp = camRot.Up;

            IReadOnlyList<uint> enemies = EnemyRegistry.Enemies;

            int indicatorIndex = 0;

            for (int e = 0; e < enemies.Count && indicatorIndex < maxIndicators; e++)
            {
                uint enemyID = enemies[e];
                if (enemyID == 0) continue;

                Vector3 enemyPos = GetPosition(enemyID);
                Vector3 toEnemy = enemyPos - camPos;
                float distance = toEnemy.Magnitude;

                // Sphere cull
                if (distance > detectionRadius) continue;
                if (distance < 1e-4f) continue;

                Vector3 dir = toEnemy.Normalized;

                // Project direction onto camera's 2D screen axes
                // sx/sy are in [-1, 1] and represent where in camera-space the
                // enemy sits.  We normalise to get a pure direction for clamping.
                float sx = Vector3.Dot(dir, camRight);
                float sy = Vector3.Dot(dir, camUp);
                float sz = Vector3.Dot(dir, camFwd);   // positive = in front

                // Check raw dot products BEFORE any bias is applied.
                // If the enemy is in front and within the angular FOV threshold,
                // it is visible — hide the indicator.
                if (sz > 0.0f && SimpleMath.Abs(sx) < onScreenThreshold && SimpleMath.Abs(sy) < onScreenThreshold)
                {
                    // Ensure the slot's indicator is hidden and skip.
                    if (indicatorPool[indicatorIndex] != 0)
                        SetIsVisible(indicatorPool[indicatorIndex], false);
                    continue;
                }

                // When sz < 0 the enemy is behind the camera.  sx/sy alone are
                // near-zero for enemies directly behind, causing the normalisation
                // below to be unstable (indicator jumps to top).
                // We inject a downward bias proportional to how far behind the
                // enemy is.  This means:
                //   directly behind  -> bottom
                //   behind + right   -> bottom-right, resolves to right edge
                //   behind + left    -> bottom-left,  resolves to left edge
                if (sz < 0.0f)
                    sy -= SimpleMath.Abs(sz);

                float len2D = SimpleMath.Sqrt(sx * sx + sy * sy);
                if (len2D < 1e-4f) len2D = 1e-4f;
                sx /= len2D;
                sy /= len2D;

                // Map to screen edge, inset by padding
                // Scale the unit direction to touch the nearest screen edge.
                // Dividing by the max of the absolute components gives us the
                // screen-box intersection (the "infinity norm" scale).
                float absX = SimpleMath.Abs(sx) / (screenHalfWidth - edgePadding);
                float absY = SimpleMath.Abs(sy) / (screenHalfHeight - edgePadding);
                //float scale = 1.0f / SimpleMath.Max(absX, absY);

                float screenX = screenHalfWidth + sx * circleRadius;
                float screenY = screenHalfHeight - sy * circleRadius;

                // Alpha by proximity
                // Full opacity at distance 0, transparent at detectionRadius.
                float alpha = 1.0f - (distance / detectionRadius);
                alpha = SimpleMath.Clamp(alpha, 0.0f, 1.0f);

                // Apply to indicator sprite
                uint indicatorID = indicatorPool[indicatorIndex];
                if (indicatorID == 0) { indicatorIndex++; continue; }

                // Position: the sprite lives in the UI orthographic plane,
                // so X/Y map directly to screen-space units.  Z can stay 0.
                Vector3 spritePos = new Vector3(screenX, screenY, 0.0f);
                SetPosition(indicatorID, ref spritePos);

                // White tint, alpha fades with distance.
                SetColor(indicatorID, 1.0f, 1.0f, 1.0f, alpha);
                SetIsVisible(indicatorID, true);

                indicatorIndex++;
            }

            // Hide any unused indicators in the pool.
            for (int i = indicatorIndex; i < maxIndicators; i++)
            {
                if (indicatorPool[i] != 0)
                    SetIsVisible(indicatorPool[i], false);
            }

        }

        private void HideAllIndicators()
        {
            if (indicatorPool == null) return;
            for (int i = 0; i < indicatorPool.Length; i++)
                if (indicatorPool[i] != 0)
                    SetIsVisible(indicatorPool[i], false);
        }
    }
}