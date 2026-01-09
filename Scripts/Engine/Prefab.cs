using System.Runtime.CompilerServices;

namespace Engine
{
    public static class Prefab
    {
        // Register as: "Engine.Prefab::Prefab_*"
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern uint Prefab_Instantiate(string prefabPath);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern uint Prefab_InstantiateScene(string prefabPath);

        // Note: your old InternalCalls had this as `internal`; it can be private here.
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern uint Prefab_InstantiateWithTransform(string prefabPath, ref Vector3 position, ref Quat rotation, ref Vector3 scale, bool isScenePrefab);

        public static uint PrefabInstantiate(string prefabPath) => Prefab_Instantiate(prefabPath ?? string.Empty);
        public static uint PrefabInstantiateScene(string prefabPath) => Prefab_InstantiateScene(prefabPath ?? string.Empty);

        // Script-friendly wrapper (no ref at call site)
        public static uint PrefabInstantiateWithTransform(string prefabPath, ref Vector3 position, ref Quat rotation, ref Vector3 scale, bool isScenePrefab) => Prefab_InstantiateWithTransform(prefabPath ?? string.Empty, ref position, ref rotation, ref scale, isScenePrefab);

    }
}
