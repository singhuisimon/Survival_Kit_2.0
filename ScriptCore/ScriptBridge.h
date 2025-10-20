#pragma once
#include "ImportExport.h"
#include <glm/glm.hpp>
#include <string>

namespace Core {

    /**
     * @brief Bridge between ScriptCore and Engine's ECS
     * This allows ScriptCore to access ECS without depending on Engine
     */
    class DLL_API ScriptBridge {
    public:
        // Function pointer types - match the engine's interface
        typedef uint32_t(*CreateEntityFunc)(const std::string&);
        typedef void(*DestroyEntityFunc)(uint32_t);
        typedef std::string(*GetEntityNameFunc)(uint32_t);

        typedef glm::vec3(*GetPositionFunc)(uint32_t);
        typedef void(*SetPositionFunc)(uint32_t, const glm::vec3&);
        typedef glm::vec3(*GetRotationFunc)(uint32_t);
        typedef void(*SetRotationFunc)(uint32_t, const glm::vec3&);
        typedef glm::vec3(*GetScaleFunc)(uint32_t);
        typedef void(*SetScaleFunc)(uint32_t, const glm::vec3&);

        // Static function pointers
        static CreateEntityFunc CreateEntity;
        static DestroyEntityFunc DestroyEntity;
        static GetEntityNameFunc GetEntityName;

        static GetPositionFunc GetPosition;
        static SetPositionFunc SetPosition;
        static GetRotationFunc GetRotation;
        static SetRotationFunc SetRotation;
        static GetScaleFunc GetScale;
        static SetScaleFunc SetScale;

        // Initialize with function pointers from the engine
        static void Initialize(
            CreateEntityFunc createEntity,
            DestroyEntityFunc destroyEntity,
            GetEntityNameFunc getEntityName,
            GetPositionFunc getPosition,
            SetPositionFunc setPosition,
            GetRotationFunc getRotation,
            SetRotationFunc setRotation,
            GetScaleFunc getScale,
            SetScaleFunc setScale
        );

        static void Shutdown();
        static bool IsInitialized();

    private:
        static bool s_Initialized;
    };

} // namespace Core