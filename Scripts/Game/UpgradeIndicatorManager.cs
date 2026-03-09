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

    public static class ModuleRegistry
    {
        private static readonly List<uint> s_modules = new List<uint>();

        public static void Register(uint entityID)
        {
            if (!s_modules.Contains(entityID))
                s_modules.Add(entityID);
        }

        public static void Unregister(uint entityID)
        {
            s_modules.Remove(entityID);
        }

        public static IReadOnlyList<uint> Modules => s_modules;
    }

    public class UpgradeIndicatorManager : ScriptBehaviour
    {
        [SerializeField("Camera Entity Name")]
        private string cameraName = "PlayerCam";

        [SerializeField("Detection Radius")]
        private float detectionRadius = 150000.0f;

        [SerializeField("Screen Half Width")]
        private float screenHalfWidth = 640.0f;

        [SerializeField("Screen Half Height")]
        private float screenHalfHeight = 360.0f;

        [SerializeField("Indicator Circle Radius")]
        private float circleRadius = 260.0f;

        [SerializeField("Edge Padding")]
        private float edgePadding = 20.0f;

        [SerializeField("On Screen Threshold")]
        private float onScreenThreshold = 0.58f;

        [SerializeField("Max Indicators")]
        private int maxIndicators = 8;

        [SerializeField("Indicator Prefab Path")]
        private string indicatorPrefabPath = "Sources/Prefabs/UpgradeIndicator.prefab";

        private uint cameraEntityID = 0;
        private uint[] indicatorPool;
        private bool initialized = false;

        public override void OnStart()
        {
            cameraEntityID = SceneFindEntityByName(cameraName);
            if (cameraEntityID == 0)
            {
                LogError("[UpgradeModuleIndicator] Camera not found: " + cameraName);
                return;
            }

            indicatorPool = new uint[maxIndicators];
            for (int i = 0; i < maxIndicators; i++)
            {
                indicatorPool[i] = PrefabInstantiate(indicatorPrefabPath);

                if (indicatorPool[i] == 0)
                {
                    LogError("[UpgradeModuleIndicator] Failed to instantiate: " + indicatorPrefabPath);
                    return;
                }

                SetIsVisible(indicatorPool[i], false);
            }

            initialized = true;
            LogMessage("[UpgradeModuleIndicator] Initialized with " + maxIndicators + " indicators.");
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

            Quat camRot = GetRotation(cameraEntityID);
            Vector3 camFwd = camRot.Forward;
            Vector3 camRight = camRot.Right;
            Vector3 camUp = camRot.Up;

            IReadOnlyList<uint> modules = ModuleRegistry.Modules;

            int indicatorIndex = 0;

            for (int m = 0; m < modules.Count && indicatorIndex < maxIndicators; m++)
            {
                uint moduleID = modules[m];
                if (moduleID == 0) continue;

                Vector3 modulePos = GetPosition(moduleID);
                Vector3 toModule = modulePos - camPos;
                float distance = toModule.Magnitude;

                if (distance > detectionRadius) continue;
                if (distance < 1e-4f) continue;

                Vector3 dir = toModule.Normalized;

                float sx = Vector3.Dot(dir, camRight);
                float sy = Vector3.Dot(dir, camUp);
                float sz = Vector3.Dot(dir, camFwd);

                if (sz > 0.0f)
                    sy += sz;
                else
                    sy -= SimpleMath.Abs(sz);

                float len2D = SimpleMath.Sqrt(sx * sx + sy * sy);
                if (len2D < 1e-4f) len2D = 1e-4f;
                sx /= len2D;
                sy /= len2D;

                float absX = SimpleMath.Abs(sx) / (screenHalfWidth - edgePadding);
                float absY = SimpleMath.Abs(sy) / (screenHalfHeight - edgePadding);

                float screenX = screenHalfWidth + sx * circleRadius;
                float screenY = screenHalfHeight - sy * circleRadius;

                float alpha = 1.0f;

                uint indicatorID = indicatorPool[indicatorIndex];
                if (indicatorID == 0) { indicatorIndex++; continue; }

                Vector3 spritePos = new Vector3(screenX, screenY, 0.0f);
                SetPosition(indicatorID, ref spritePos);

                SetColor(indicatorID, 0.0f, 1.0f, 0.0f, alpha);
                SetIsVisible(indicatorID, true);

                indicatorIndex++;
            }

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