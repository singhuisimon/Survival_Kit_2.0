using System.Runtime.CompilerServices;

namespace Engine
{
    public static class Scene
    {
        // Register as: "Engine.Scene::Scene_*" (same method names)
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern uint Scene_FindEntityByName(string name);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern uint Scene_FindEntityByTag(string tag);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern uint Scene_CreateEntity(string name);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Scene_DestroyEntity(uint entity);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern uint[] Scene_FindEntitiesByTag(string tag);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Entity_AddScript(uint entity, string managedClassFullName);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Entity_AddRigidBody(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Entity_AddMeshRenderer(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Entity_AddCamera(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Entity_AddAudio(uint entityID);
        [MethodImpl(MethodImplOptions.InternalCall)] private static extern void Entity_AddTag(uint entityID);

        public static void EntityAddScript(uint entity, string managedClassFullName) => Entity_AddScript(entity, managedClassFullName ?? string.Empty);
        public static void EntityAddRigidBody(uint entityID) => Entity_AddRigidBody(entityID);
        public static void EntityAddMeshRenderer(uint entityID) => Entity_AddMeshRenderer(entityID);
        public static void EntityAddCamera(uint entityID) => Entity_AddCamera(entityID);
        public static void EntityAddAudio(uint entityID) => Entity_AddAudio(entityID);
        public static void EntityAddTag(uint entityID) => Entity_AddTag(entityID);

        public static uint SceneFindEntityByName(string name) => Scene_FindEntityByName(name ?? string.Empty);
        public static uint SceneFindEntityByTag(string tag) => Scene_FindEntityByTag(tag ?? string.Empty);
        public static uint SceneCreateEntity(string name) => Scene_CreateEntity(name ?? string.Empty);
        public static void SceneDestroyEntity(uint entity) => Scene_DestroyEntity(entity);
        public static uint[] SceneFindEntitiesByTag(string tag) => Scene_FindEntitiesByTag(tag ?? string.Empty);
    }
}
