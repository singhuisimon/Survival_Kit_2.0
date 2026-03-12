#pragma once
#include "../Utility/Types.h"
namespace Engine {

    enum class ComponentTypeID : u32
    {
        None = 0,
        Transform = 1,
        MeshRenderer = 2,
        Camera = 3,
        Light = 4,
        ParticleSystem = 5,
        RigidBody = 6,
        Script = 8,
        Audio = 9,
        Listerner = 10,
        Animator = 11,
        ReverbZone = 12,
        BehaviourTree = 13,
        SpriteRenderer = 14,
        Text = 15,
        Trail = 16,
        Beam = 17,
        Prefab = 100,
        Tag = 101,

    };
    /**
     * @brief Centralized component registration
     * @details Call RegisterAllComponents() at startup to register all component types
     */
    class ComponentRegistry {
    public:
        /**
         * @brief Register all component types with the reflection system
         * @details Must be called before any serialization operations
         */
        static void RegisterAllComponents();

      
    };

} // namespace Engine