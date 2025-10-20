#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <functional>

namespace Engine {

    // Forward declarations
    class Scene;
    class Entity;

    /**
     * @brief Interface for script engine to access ECS functionality
     * This solves the circular dependency problem by providing function pointers
     */
    class IScriptEngineInterface {
    public:
        // Entity Management
        using CreateEntityFunc = std::function<uint32_t(const std::string&)>;
        using DestroyEntityFunc = std::function<void(uint32_t)>;
        using GetEntityNameFunc = std::function<std::string(uint32_t)>;

        // Transform Component Access
        using GetPositionFunc = std::function<glm::vec3(uint32_t)>;
        using SetPositionFunc = std::function<void(uint32_t, const glm::vec3&)>;
        using GetRotationFunc = std::function<glm::vec3(uint32_t)>;
        using SetRotationFunc = std::function<void(uint32_t, const glm::vec3&)>;
        using GetScaleFunc = std::function<glm::vec3(uint32_t)>;
        using SetScaleFunc = std::function<void(uint32_t, const glm::vec3&)>;

        // Component Management
        using HasComponentFunc = std::function<bool(uint32_t, const std::string&)>;
        using AddComponentFunc = std::function<void(uint32_t, const std::string&)>;
        using RemoveComponentFunc = std::function<void(uint32_t, const std::string&)>;

        // Function pointers that will be set by the engine
        static CreateEntityFunc CreateEntity;
        static DestroyEntityFunc DestroyEntity;
        static GetEntityNameFunc GetEntityName;

        static GetPositionFunc GetPosition;
        static SetPositionFunc SetPosition;
        static GetRotationFunc GetRotation;
        static SetRotationFunc SetRotation;
        static GetScaleFunc GetScale;
        static SetScaleFunc SetScale;

        static HasComponentFunc HasComponent;
        static AddComponentFunc AddComponent;
        static RemoveComponentFunc RemoveComponent;

        // Current scene pointer (set by ScriptSystem)
        static Scene* CurrentScene;

        // Initialize all function pointers
        static void Initialize(Scene* scene);
        static void Shutdown();
    };

} // namespace Engine