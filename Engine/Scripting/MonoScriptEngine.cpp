#include "MonoScriptEngine.h"
#include "../Utility/Logger.h"
#include "../ECS/Scene.h"
#include "../ECS/Entity.h"
#include "../ECS/Components.h"
#include "../Core/input.h"
#include "Core/Application.h"

// Mono headers
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/attrdefs.h>
#include <windows.h>  // Add at top

#include <filesystem>
#include <iostream>


namespace Engine {


    MonoScriptEngine& MonoScriptEngine::GetInstance() {
        static MonoScriptEngine instance;
        return instance;
    }


    void MonoScriptEngine::Initialize(const std::string& assemblyPath) {
        LOG_INFO("Initializing Mono Script Engine...");

        m_AssemblyPath = assemblyPath;

        // Get the exe directory on Windows
        WCHAR exePath[MAX_PATH] = { 0 };
        GetModuleFileNameW(NULL, exePath, MAX_PATH);

        std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();
        std::filesystem::path monoLibPath = exeDir / "mono" / "lib";

        std::string monoLibPathStr = monoLibPath.generic_string();
        mono_set_assemblies_path(monoLibPathStr.c_str());

        LOG_INFO("[Mono] Assembly path set to: ", monoLibPathStr);

        // Verify mono/lib exists
        if (!std::filesystem::exists(monoLibPath)) {
            LOG_ERROR("[Mono] ERROR: mono/lib not found at: ", monoLibPathStr);
            LOG_ERROR("[Mono] Make sure Mono runtime is in the build output directory");
        }

        // Initialize Mono JIT
        m_RootDomain = mono_jit_init("EngineRuntime");
        if (!m_RootDomain) {
            LOG_ERROR("Failed to initialize Mono JIT");
            return;
        }

        // Create app domain
        m_AppDomain = mono_domain_create_appdomain(const_cast<char*>("EngineAppDomain"), nullptr);
        if (!m_AppDomain) {
            LOG_ERROR("Failed to create Mono app domain");
            return;
        }
        mono_domain_set(m_AppDomain, true);

        // Load assembly (only if it exists)
        if (!assemblyPath.empty() && std::filesystem::exists(assemblyPath)) {
            LoadAssembly(assemblyPath);
        }
        else {
            LOG_WARNING("Assembly not found: ", assemblyPath);
            LOG_WARNING("Mono initialized but no scripts will be loaded");
            LOG_WARNING("ScriptComponents will be ignored");
        }

        // Register internal calls (C++ functions callable from C#)
        RegisterInternalCalls();

        LOG_INFO("Mono Script Engine initialized");

    }


    void MonoScriptEngine::Shutdown() {
        LOG_INFO("Shutting down Mono Script Engine...");
        
        UnloadAssembly();
        
        if (m_AppDomain) {
            mono_domain_set(m_RootDomain, false);
            mono_domain_unload(m_AppDomain);
            m_AppDomain = nullptr;
        }

        if (m_RootDomain) {
            mono_jit_cleanup(m_RootDomain);
            m_RootDomain = nullptr;
        }

        LOG_INFO("Mono Script Engine shut down");
    }

    void MonoScriptEngine::LoadAssembly(const std::string& path) {
        LOG_INFO("Loading C# assembly: ", path);

        if (!std::filesystem::exists(path)) {
            LOG_ERROR("Assembly file not found: ", path);
            return;
        }

        // Load assembly from file
        m_AppAssembly = mono_domain_assembly_open(m_AppDomain, path.c_str());
        if (!m_AppAssembly) {
            LOG_ERROR("Failed to load assembly: ", path);
            return;
        }

        m_AppImage = mono_assembly_get_image(m_AppAssembly);
        if (!m_AppImage) {
            LOG_ERROR("Failed to get assembly image");
            return;
        }

        LOG_INFO("Assembly loaded successfully");
    }

    void MonoScriptEngine::UnloadAssembly() {
        m_ClassCache.clear();
        m_AppImage = nullptr;
        m_AppAssembly = nullptr;
    }

    void MonoScriptEngine::ReloadAssembly() {
        LOG_INFO("Reloading assembly...");
        UnloadAssembly();
        
        // Recreate app domain
        mono_domain_set(m_RootDomain, false);
        mono_domain_unload(m_AppDomain);
        m_AppDomain = mono_domain_create_appdomain(const_cast<char*>("EngineAppDomain"), nullptr);
        mono_domain_set(m_AppDomain, true);
        
        LoadAssembly(m_AssemblyPath);
        RegisterInternalCalls();
    }

    MonoClass* MonoScriptEngine::GetScriptClass(const std::string& className) {
        // Check if app image is loaded
        if (!m_AppImage) {
            LOG_ERROR("Cannot get script class '", className, "': Assembly not loaded");
            return nullptr;
        }

        // Check cache first
        auto it = m_ClassCache.find(className);
        if (it != m_ClassCache.end()) {
            return it->second;
        }

        // Parse namespace and class name
        size_t lastDot = className.find_last_of('.');
        std::string namespaceName = lastDot != std::string::npos ? className.substr(0, lastDot) : "";
        std::string classNameOnly = lastDot != std::string::npos ? className.substr(lastDot + 1) : className;

        // Get class from image
        MonoClass* klass = mono_class_from_name(
            m_AppImage,
            namespaceName.c_str(),
            classNameOnly.c_str()
        );

        if (!klass) {
            LOG_ERROR("Failed to find class: ", className);
            return nullptr;
        }

        // Cache for future use
        m_ClassCache[className] = klass;
        return klass;
    }


    MonoObject* MonoScriptEngine::CreateScriptInstance(const std::string& className) {
        if (!m_AppImage) {
            // Assembly not loaded - silently skip
            return nullptr;
        }

        MonoClass* klass = GetScriptClass(className);
        if (!klass) {
            return nullptr;
        }

        // Allocate object
        MonoObject* instance = mono_object_new(m_AppDomain, klass);
        if (!instance) {
            LOG_ERROR("Failed to create instance of: ", className);
            return nullptr;
        }

        // Call constructor
        mono_runtime_object_init(instance);

        return instance;
    }


    void MonoScriptEngine::DestroyScriptInstance(MonoObject* instance) {
        // Mono uses garbage collection, so we just need to clear references
        // The GC will handle cleanup
        if (instance) {
            // Optionally call OnDestroy if the class has it
            MonoClass* klass = mono_object_get_class(instance);
            MonoMethod* destroyMethod = mono_class_get_method_from_name(klass, "OnDestroy", 0);
            if (destroyMethod) {
                mono_runtime_invoke(destroyMethod, instance, nullptr, nullptr);
            }
        }
    }

    MonoMethod* MonoScriptEngine::GetMethod(MonoClass* klass, const std::string& methodName, int paramCount) {
        if (!klass) {
            return nullptr;
        }

        MonoMethod* method = mono_class_get_method_from_name(klass, methodName.c_str(), paramCount);
        return method;
    }

    void MonoScriptEngine::CallMethod(MonoObject* instance, const std::string& methodName) {
        CallMethod(instance, methodName, nullptr, 0);
    }

    void MonoScriptEngine::CallMethod(MonoObject* instance, const std::string& methodName, void** params, int paramCount) {
        if (!instance) {
            return;
        }

        MonoClass* klass = mono_object_get_class(instance);
        MonoMethod* method = GetMethod(klass, methodName, paramCount);

        if (!method) {
            // Method doesn't exist, which is fine (not all scripts need all methods)
            return;
        }

        // Invoke method
        MonoObject* exception = nullptr;
        mono_runtime_invoke(method, instance, params, &exception);

        if (exception) {
            // Log exception details
            MonoClass* exceptionClass = mono_object_get_class(exception);
            const char* exceptionName = mono_class_get_name(exceptionClass);
            LOG_ERROR("Exception in C# method '", methodName, "': ", exceptionName);
        }
    }

    void MonoScriptEngine::SetFieldValue(MonoObject* instance, const std::string& fieldName, void* value) {
        if (!instance) {
            return;
        }

        MonoClass* klass = mono_object_get_class(instance);
        MonoClassField* field = mono_class_get_field_from_name(klass, fieldName.c_str());

        if (!field) {
            LOG_ERROR("Field not found: ", fieldName);
            return;
        }

        mono_field_set_value(instance, field, value);
    }

    void* MonoScriptEngine::GetFieldValue(MonoObject* instance, const std::string& fieldName) {
        if (!instance) {
            return nullptr;
        }

        MonoClass* klass = mono_object_get_class(instance);
        MonoClassField* field = mono_class_get_field_from_name(klass, fieldName.c_str());

        if (!field) {
            LOG_ERROR("Field not found: ", fieldName);
            return nullptr;
        }

        void* value = nullptr;
        mono_field_get_value(instance, field, &value);
        return value;
    }

    // ============================================
    // Internal Calls - C++ functions callable from C#
    // ============================================

    // Forward declarations for internal call functions
    namespace InternalCalls {
        static void Log(MonoString* message);
        static void LogError(MonoString* message);
        static void LogWarning(MonoString* message);
        
        static uint64_t Entity_GetEntityID(MonoObject* entityObj);
        static bool Entity_HasComponent(uint64_t entityID, MonoReflectionType* componentType);
        
        static void Transform_GetPosition(uint64_t entityID, glm::vec3* outPosition);
        static void Transform_SetPosition(uint64_t entityID, glm::vec3* position);
        static void Transform_GetRotation(uint64_t entityID, glm::vec3* outRotation);
        static void Transform_SetRotation(uint64_t entityID, glm::vec3* rotation);
        static void Transform_GetScale(uint64_t entityID, glm::vec3* outScale);
        static void Transform_SetScale(uint64_t entityID, glm::vec3* scale);
        
        static bool Input_IsKeyPressed(int keyCode);
        static bool Input_IsMouseButtonPressed(int button);
        static void Input_GetMousePosition(glm::vec2* outPosition);
    }

    void MonoScriptEngine::RegisterInternalCalls() {
        LOG_INFO("Registering internal calls...");

        // Logging
        mono_add_internal_call("Engine.InternalCalls::Log", (void*)InternalCalls::Log);
        mono_add_internal_call("Engine.InternalCalls::LogError", (void*)InternalCalls::LogError);
        mono_add_internal_call("Engine.InternalCalls::LogWarning", (void*)InternalCalls::LogWarning);

        // Entity
        mono_add_internal_call("Engine.Entity::GetEntityID_Native", (void*)InternalCalls::Entity_GetEntityID);
        mono_add_internal_call("Engine.Entity::HasComponent_Native", (void*)InternalCalls::Entity_HasComponent);

        // Transform
        mono_add_internal_call("Engine.Transform::GetPosition_Native", (void*)InternalCalls::Transform_GetPosition);
        mono_add_internal_call("Engine.Transform::SetPosition_Native", (void*)InternalCalls::Transform_SetPosition);
        mono_add_internal_call("Engine.Transform::GetRotation_Native", (void*)InternalCalls::Transform_GetRotation);
        mono_add_internal_call("Engine.Transform::SetRotation_Native", (void*)InternalCalls::Transform_SetRotation);
        mono_add_internal_call("Engine.Transform::GetScale_Native", (void*)InternalCalls::Transform_GetScale);
        mono_add_internal_call("Engine.Transform::SetScale_Native", (void*)InternalCalls::Transform_SetScale);

        // Input
        mono_add_internal_call("Engine.Input::IsKeyPressed_Native", (void*)InternalCalls::Input_IsKeyPressed);
        mono_add_internal_call("Engine.Input::IsMouseButtonPressed_Native", (void*)InternalCalls::Input_IsMouseButtonPressed);
        mono_add_internal_call("Engine.Input::GetMousePosition_Native", (void*)InternalCalls::Input_GetMousePosition);

        LOG_INFO("Internal calls registered");
    }

    // ============================================
    // Internal Call Implementations
    // ============================================

    namespace InternalCalls {
        // Global scene pointer (set by ScriptSystem)
        static Scene* s_CurrentScene = nullptr;
        //auto& input = GetInput();
        static Input* s_InputSystem = nullptr;


        void SetCurrentScene(Scene* scene) {
            s_CurrentScene = scene;
        }

        void SetInputSystem(Input* input) {
            s_InputSystem = input;
        }

        void Log(MonoString* message) {
            char* cStr = mono_string_to_utf8(message);
            LOG_INFO("[C#] ", cStr);
            mono_free(cStr);
        }

        void LogError(MonoString* message) {
            char* cStr = mono_string_to_utf8(message);
            LOG_ERROR("[C#] ", cStr);
            mono_free(cStr);
        }

        void LogWarning(MonoString* message) {
            char* cStr = mono_string_to_utf8(message);
            LOG_WARNING("[C#] ", cStr);  // Fixed: was LOG_ERROR
            mono_free(cStr);
        }

        uint64_t Entity_GetEntityID(MonoObject* entityObj) {
            // Entity ID is stored in the C# Entity class
            return 0; // Placeholder - implement based on your C# Entity class structure
        }

        bool Entity_HasComponent(uint64_t entityID, MonoReflectionType* componentType) {
            if (!s_CurrentScene) return false;

            auto& registry = s_CurrentScene->GetRegistry();
            auto handle = static_cast<entt::entity>(entityID);

            // Check if entity exists in registry
            if (!registry.valid(handle)) return false;

            // Check component type (you'll need to map C# component types to C++ component types)
            // This is a simplified version
            return false; // Implement based on your component system
        }

        void Transform_GetPosition(uint64_t entityID, glm::vec3* outPosition) {
            if (!s_CurrentScene || !outPosition) return;

            auto& registry = s_CurrentScene->GetRegistry();
            auto handle = static_cast<entt::entity>(entityID);

            if (!registry.valid(handle)) return;
            if (!registry.all_of<TransformComponent>(handle)) return;

            auto& transform = registry.get<TransformComponent>(handle);
            *outPosition = transform.Position;
        }

        void Transform_SetPosition(uint64_t entityID, glm::vec3* position) {
            if (!s_CurrentScene || !position) return;

            auto& registry = s_CurrentScene->GetRegistry();
            auto handle = static_cast<entt::entity>(entityID);

            if (!registry.valid(handle)) return;
            if (!registry.all_of<TransformComponent>(handle)) return;

            auto& transform = registry.get<TransformComponent>(handle);
            transform.Position = *position;
        }

        void Transform_GetRotation(uint64_t entityID, glm::vec3* outRotation) {
            if (!s_CurrentScene || !outRotation) return;

            auto& registry = s_CurrentScene->GetRegistry();
            auto handle = static_cast<entt::entity>(entityID);

            if (!registry.valid(handle)) return;
            if (!registry.all_of<TransformComponent>(handle)) return;

            auto& transform = registry.get<TransformComponent>(handle);
            *outRotation = glm::degrees(glm::eulerAngles(transform.Rotation));  // Convert quat to euler
        }

        void Transform_SetRotation(uint64_t entityID, glm::vec3* rotation) {
            if (!s_CurrentScene || !rotation) return;

            auto& registry = s_CurrentScene->GetRegistry();
            auto handle = static_cast<entt::entity>(entityID);

            if (!registry.valid(handle)) return;
            if (!registry.all_of<TransformComponent>(handle)) return;

            auto& transform = registry.get<TransformComponent>(handle);
            transform.Rotation = glm::quat(glm::radians(*rotation));  // Convert euler to quat
        }

        void Transform_GetScale(uint64_t entityID, glm::vec3* outScale) {
            if (!s_CurrentScene || !outScale) return;

            auto& registry = s_CurrentScene->GetRegistry();
            auto handle = static_cast<entt::entity>(entityID);

            if (!registry.valid(handle)) return;
            if (!registry.all_of<TransformComponent>(handle)) return;

            auto& transform = registry.get<TransformComponent>(handle);
            *outScale = transform.Scale;
        }

        void Transform_SetScale(uint64_t entityID, glm::vec3* scale) {
            if (!s_CurrentScene || !scale) return;

            auto& registry = s_CurrentScene->GetRegistry();
            auto handle = static_cast<entt::entity>(entityID);

            if (!registry.valid(handle)) return;
            if (!registry.all_of<TransformComponent>(handle)) return;

            auto& transform = registry.get<TransformComponent>(handle);
            transform.Scale = *scale;
        }

        bool Input_IsKeyPressed(int keyCode) {
            return s_InputSystem->IsKeyPressed(keyCode);
        }


        bool Input_IsMouseButtonPressed(int button) {

            if (!s_InputSystem) {
                LOG_WARNING("[InternalCall] Input system not initialized");
                return false;
            }
            return s_InputSystem->IsMouseButtonPressed(button);
        }


        void Input_GetMousePosition(glm::vec2* outPosition) {
            if (!outPosition) return;
            // Access your input system
            // Example: *outPosition = Input::GetMousePosition();
        }
    }

    void SetScriptingInputSystem(Input* input) {
        InternalCalls::SetInputSystem(input);
    }
    // Expose SetCurrentScene for ScriptSystem to call
    void SetScriptingCurrentScene(Scene* scene) {
        InternalCalls::SetCurrentScene(scene);
    }

} // namespace Engine
