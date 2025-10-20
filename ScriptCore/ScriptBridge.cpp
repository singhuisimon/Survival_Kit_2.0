#include "pch.h"
#include "ScriptBridge.h"

namespace Core {

    // Static member definitions
    ScriptBridge::CreateEntityFunc ScriptBridge::CreateEntity = nullptr;
    ScriptBridge::DestroyEntityFunc ScriptBridge::DestroyEntity = nullptr;
    ScriptBridge::GetEntityNameFunc ScriptBridge::GetEntityName = nullptr;

    ScriptBridge::GetPositionFunc ScriptBridge::GetPosition = nullptr;
    ScriptBridge::SetPositionFunc ScriptBridge::SetPosition = nullptr;
    ScriptBridge::GetRotationFunc ScriptBridge::GetRotation = nullptr;
    ScriptBridge::SetRotationFunc ScriptBridge::SetRotation = nullptr;
    ScriptBridge::GetScaleFunc ScriptBridge::GetScale = nullptr;
    ScriptBridge::SetScaleFunc ScriptBridge::SetScale = nullptr;

    bool ScriptBridge::s_Initialized = false;

    void ScriptBridge::Initialize(
        CreateEntityFunc createEntity,
        DestroyEntityFunc destroyEntity,
        GetEntityNameFunc getEntityName,
        GetPositionFunc getPosition,
        SetPositionFunc setPosition,
        GetRotationFunc getRotation,
        SetRotationFunc setRotation,
        GetScaleFunc getScale,
        SetScaleFunc setScale)
    {
        CreateEntity = createEntity;
        DestroyEntity = destroyEntity;
        GetEntityName = getEntityName;

        GetPosition = getPosition;
        SetPosition = setPosition;
        GetRotation = getRotation;
        SetRotation = setRotation;
        GetScale = getScale;
        SetScale = setScale;

        s_Initialized = true;
        std::cout << "ScriptBridge initialized with ECS functions" << std::endl;
    }

    void ScriptBridge::Shutdown() {
        CreateEntity = nullptr;
        DestroyEntity = nullptr;
        GetEntityName = nullptr;
        GetPosition = nullptr;
        SetPosition = nullptr;
        GetRotation = nullptr;
        SetRotation = nullptr;
        GetScale = nullptr;
        SetScale = nullptr;

        s_Initialized = false;
    }

    bool ScriptBridge::IsInitialized() {
        return s_Initialized;
    }

} // namespace Core