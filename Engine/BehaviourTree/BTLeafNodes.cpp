/**
 * @file BTLeafNodes.cpp
 * @brief Definition of Behaviour Tree Leaf Nodes Classes 
 * @author Amanda Leow Boon Suan (90%), Rio Shannon Yvon Leonardo (10%)
 * @date 3/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "BTLeafNodes.h"
#include "ECS/Entity.h" 
#include "ECS/Components.h"
#include "Utility/Logger.h"
#include "ECS/Scene.h"
#include <functional>
#include "Core/Application.h"
#include "Graphics/Material.h"
#include "ECS/Components.h"
#include "Transform/TransformSystem.h"
#include "Physics/PhysicsAPI.h"
#include "Physics/PhysicsSystem.h"
#include "Asset/AssetManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine {

    //Action

    BTAction::BTAction(ActionFunc action): m_Action(action) {}

    const char* BTAction::GetTypeName() const { return "Action"; }

    void BTAction::SetAction(ActionFunc action) { m_Action = action; }

    BTStatus BTAction::Execute(BTContext& context) {
        if (!m_Action) {
            return BTStatus::Failure;
        }
        return m_Action(context);
    }


    //BTCondition
    BTCondition::BTCondition(ConditionFunc condition) : m_Condition(condition) {}
    
    const char* BTCondition::GetTypeName() const { return "Condition"; }
    
    void BTCondition::SetCondition(ConditionFunc condition) { m_Condition = condition; }
    
    BTStatus BTCondition::Execute(BTContext& context) {
        if (!m_Condition) {
            return BTStatus::Failure;
        }
        return m_Condition(context) ? BTStatus::Success : BTStatus::Failure;
    }


    //BTWait
    BTWait::BTWait(float duration) : m_Duration(duration), m_ElapsedTime(0.0f) {}
    
    const char* BTWait::GetTypeName() const { return "Wait"; }
    
    void BTWait::OnEnter(BTContext& context) {
        (void)context;
        m_ElapsedTime = 0.0f;
    }
    
    BTStatus BTWait::Execute(BTContext& context) {
        m_ElapsedTime += context.DeltaTime;
    
        if (m_ElapsedTime >= m_Duration) {
            return BTStatus::Success;
        }
    
        return BTStatus::Running;
    }
    
    void BTWait::Reset() {
        m_ElapsedTime = 0.0f;
    }
    
    void BTWait::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Duration", std::to_string(m_Duration) });
    }
    
    void BTWait::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Duration") {
            m_Duration = std::stof(value);
        }
    }
    

    //BTLog:
    BTLog::BTLog(const std::string& message) : m_Message(message) {}

    const char* BTLog::GetTypeName() const { return "Log"; }

    BTStatus BTLog::Execute(BTContext& context) {
        (void)context;
        // In real implementation, use your Logger system
        // LOG_INFO("BehaviourTree: ", m_Message);
        accumulatedtime += context.DeltaTime;
        if (time > accumulatedtime) {
            LOG_INFO("LogNode: ", m_Message);
            accumulatedtime = 0.0f;
            return BTStatus::Success;
        }
        
        return BTStatus::Running;
    }
    
    void BTLog::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Message", m_Message });
    }
    
    void BTLog::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Message") {
            m_Message = value;
        }
    }

    //BTSetBlackboard
    BTSetBlackboard::BTSetBlackboard(const std::string& key, const std::string& value, const std::string& type)
        : m_Key(key), m_Value(value), m_Type(type) {}

    const char* BTSetBlackboard::GetTypeName() const { return "SetBlackboard"; }
    
    BTStatus BTSetBlackboard::Execute(BTContext& context) {
        if (m_Key.empty()) {
            return BTStatus::Failure;
        }
    
        // Use the blackboard's FromString method to set the value with proper type
        if (context.Blackboard.FromString(m_Key, m_Type, m_Value)) {
            return BTStatus::Success;
        }
    
        return BTStatus::Failure;
    }
    
    void BTSetBlackboard::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "Type", m_Type });
        properties.push_back({ "Value", m_Value });
    }
    
    void BTSetBlackboard::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Key") {
            m_Key = value;
        }
        else if (name == "Type") {
            m_Type = value;
        }
        else if (name == "Value") {
            m_Value = value;
        }
    }
    

    //BTCheckBlackboard
    BTCheckBlackboard::BTCheckBlackboard(const std::string& key, const std::string& expectedValue)
        : m_Key(key), m_ExpectedValue(expectedValue) {
    }
    
    const char* BTCheckBlackboard::GetTypeName() const { return "CheckBlackboard"; }
    
    BTStatus BTCheckBlackboard::Execute(BTContext& context) {
        if (m_Key.empty()) {
            return BTStatus::Failure;
        }
    
        if (!context.Blackboard.Has(m_Key)) {
            return BTStatus::Failure;
        }
    
        // Convert to string and compare
        std::string actualValue = context.Blackboard.ToString(m_Key);
        return (actualValue == m_ExpectedValue) ? BTStatus::Success : BTStatus::Failure;
    }
    
    void BTCheckBlackboard::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "ExpectedValue", m_ExpectedValue });
    }
    
    void BTCheckBlackboard::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Key") {
            m_Key = value;
        }
        else if (name == "ExpectedValue") {
            m_ExpectedValue = value;
        }
    }


    //BTSetBlackboardInt
    BTSetBlackboardInt::BTSetBlackboardInt(const std::string& key, int value)
        : m_Key(key), m_Value(value) {
    }
    
    const char* BTSetBlackboardInt::GetTypeName() const { return "SetBlackboardInt"; }
    
    BTStatus BTSetBlackboardInt::Execute(BTContext& context) {
        if (m_Key.empty()) return BTStatus::Failure;
        context.Blackboard.Set(m_Key, m_Value);
        return BTStatus::Success;
    }
    
    void BTSetBlackboardInt::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "Value", std::to_string(m_Value) });
    }
    
    void BTSetBlackboardInt::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Key") m_Key = value;
        else if (name == "Value") m_Value = std::stoi(value);
    }

    //BTSetBlackboardFloat
    BTSetBlackboardFloat::BTSetBlackboardFloat(const std::string& key, float value)
        : m_Key(key), m_Value(value) {
    }
    
    const char* BTSetBlackboardFloat::GetTypeName() const { return "SetBlackboardFloat"; }
    
    BTStatus BTSetBlackboardFloat::Execute(BTContext& context) {
        if (m_Key.empty()) return BTStatus::Failure;
        context.Blackboard.Set(m_Key, m_Value);
        return BTStatus::Success;
    }
    
    void BTSetBlackboardFloat::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "Value", std::to_string(m_Value) });
    }
    
    void BTSetBlackboardFloat::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Key") m_Key = value;
        else if (name == "Value") m_Value = std::stof(value);
    }

    //BTSetBlackboardBool
    BTSetBlackboardBool::BTSetBlackboardBool(const std::string& key, bool value)
        : m_Key(key), m_Value(value) {
    }
    
    const char* BTSetBlackboardBool::GetTypeName() const { return "SetBlackboardBool"; }
    
    BTStatus BTSetBlackboardBool::Execute(BTContext& context) {
        if (m_Key.empty()) return BTStatus::Failure;
        context.Blackboard.Set(m_Key, m_Value);
        return BTStatus::Success;
    }
    
    void BTSetBlackboardBool::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "Value", m_Value ? "true" : "false" });
    }
    
    void BTSetBlackboardBool::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Key") m_Key = value;
        else if (name == "Value") m_Value = (value == "true" || value == "1");
    }
    

    //BTSetBlackboardVec3:
    BTSetBlackboardVec3::BTSetBlackboardVec3(const std::string& key, const glm::vec3& value)
        : m_Key(key), m_Value(value) {
    }
    
    const char* BTSetBlackboardVec3::GetTypeName() const { return "SetBlackboardVec3"; }
    
    BTStatus BTSetBlackboardVec3::Execute(BTContext& context) {
        if (m_Key.empty()) return BTStatus::Failure;
        context.Blackboard.Set(m_Key, m_Value);
        return BTStatus::Success;
    }
    
    void BTSetBlackboardVec3::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Key", m_Key });
        properties.push_back({ "X", std::to_string(m_Value.x) });
        properties.push_back({ "Y", std::to_string(m_Value.y) });
        properties.push_back({ "Z", std::to_string(m_Value.z) });
    }
    
    void BTSetBlackboardVec3::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Key") m_Key = value;
        else if (name == "X") m_Value.x = std::stof(value);
        else if (name == "Y") m_Value.y = std::stof(value);
        else if (name == "Z") m_Value.z = std::stof(value);
    }

    //BTRotateEntity
    BTRotateEntity::BTRotateEntity(float degreesPerSecond)
        : m_RotationSpeed(degreesPerSecond) {
    }

    BTStatus BTRotateEntity::Execute(BTContext& context) {
        if (!context.Entity || !context.Entity->HasComponent<TransformComponent>()) {
            return BTStatus::Failure;
        }

        auto& transform = context.Entity->GetComponent<TransformComponent>();

        float rotationRadians = glm::radians(m_RotationSpeed * context.DeltaTime);
        glm::quat rotationDelta = glm::angleAxis(rotationRadians, glm::vec3(0, 1, 0));
        transform.Rotation = rotationDelta * transform.Rotation;
        transform.IsDirty = true;

        LOG_TRACE("ROTATING ENTITY");

        return BTStatus::Success;  // Continuous rotation
    }

    void BTRotateEntity::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "RotationSpeed", std::to_string(m_RotationSpeed) });
    }

    void BTRotateEntity::SetProperty(const std::string& name, const std::string& value) {
        if (name == "RotationSpeed") {
            m_RotationSpeed = std::stof(value);
        }
    }

    //BTMoveToTarget
    BTMoveToTarget::BTMoveToTarget(float speed, float arrivalDistance)
        : m_MoveSpeed(speed)
        , m_ArrivalDistance(arrivalDistance)
        , m_TargetPositionKey("TargetPosition") {
    }

    BTStatus BTMoveToTarget::Execute(BTContext& context) {
        // Validate context
        if (!context.Entity) {
            LOG_WARNING("BTMoveToTarget: No entity in context");
            return BTStatus::Failure;
        }

        if (!context.Entity->HasComponent<TransformComponent>()) {
            LOG_WARNING("BTMoveToTarget: Entity missing TransformComponent");
            return BTStatus::Failure;
        }

        // Get target position from blackboard
        auto targetPosOpt = context.Blackboard.Get<glm::vec3>(m_TargetPositionKey);
        if (!targetPosOpt) {
            LOG_WARNING("BTMoveToTarget: Target position '", m_TargetPositionKey, "' not found in blackboard");
            return BTStatus::Failure;
        }

        glm::vec3 targetPos = *targetPosOpt;
        auto& transform = context.Entity->GetComponent<TransformComponent>();

        // Calculate direction and distance
        glm::vec3 direction = targetPos - transform.Position;
        float distance = glm::length(direction);

        // CRITICAL FIX: Check for NaN or infinite values
        if (std::isnan(distance) || std::isinf(distance)) {
            LOG_ERROR("BTMoveToTarget: Invalid distance calculation (NaN/Inf)");
            return BTStatus::Failure;
        }

        // CRITICAL FIX: Validate DeltaTime
        if (context.DeltaTime <= 0.0f || std::isnan(context.DeltaTime) || std::isinf(context.DeltaTime)) {
            LOG_WARNING("BTMoveToTarget: Invalid DeltaTime (", context.DeltaTime, ")");
            return BTStatus::Running;  // Keep running, wait for valid delta
        }

        // Check if arrived
        if (distance <= m_ArrivalDistance) {
            LOG_TRACE("BTMoveToTarget: Arrived at target: x", targetPos.x, " y: ", targetPos.y, " z: ", targetPos.z);
            return BTStatus::Success;
        }

        // CRITICAL FIX: Ensure direction is not zero length
        if (distance < 0.0001f) {
            LOG_WARNING("BTMoveToTarget: Target too close to current position");
            return BTStatus::Success;  // Consider arrived
        }

        // Normalize direction
        direction = glm::normalize(direction);

        // The closer you get, the slower you move (ease-out behavior)
        float slowDownFactor = glm::clamp(distance / (m_ArrivalDistance * 5.0f), 0.1f, 1.0f);
        float speed = m_MoveSpeed * slowDownFactor;

        // Calculate movement with speed limit
        float frameMovement = speed * context.DeltaTime;
        //float frameMovement = m_MoveSpeed * context.DeltaTime;

        // CRITICAL FIX: Clamp movement to prevent overshooting
        if (frameMovement > distance) {
            // Arrived this frame
            transform.Position = targetPos;
        }
        else {
            // Move toward target
            glm::vec3 movement = direction * frameMovement;
            transform.Position += movement;
        }

        transform.IsDirty = true;

        return BTStatus::Running;
    }

    void BTMoveToTarget::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "MoveSpeed", std::to_string(m_MoveSpeed) });
        properties.push_back({ "ArrivalDistance", std::to_string(m_ArrivalDistance) });
        properties.push_back({ "TargetPositionKey", m_TargetPositionKey });
    }

    void BTMoveToTarget::SetProperty(const std::string& name, const std::string& value) {
        if (name == "MoveSpeed") {
            m_MoveSpeed = std::stof(value);
        }
        else if (name == "ArrivalDistance") {
            m_ArrivalDistance = std::stof(value);
        }
        else if (name == "TargetPositionKey") {
            m_TargetPositionKey = value;
        }
    }

    //BTSetTargetPosition
    BTSetTargetPosition::BTSetTargetPosition(const glm::vec3& position)
        : m_TargetPosition(position)
        , m_BlackboardKey("TargetPosition") {
    }

    BTStatus BTSetTargetPosition::Execute(BTContext& context) {
        context.Blackboard.Set(m_BlackboardKey, m_TargetPosition);
        return BTStatus::Success;
    }

    void BTSetTargetPosition::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "TargetX", std::to_string(m_TargetPosition.x) });
        properties.push_back({ "TargetY", std::to_string(m_TargetPosition.y) });
        properties.push_back({ "TargetZ", std::to_string(m_TargetPosition.z) });
        properties.push_back({ "BlackboardKey", m_BlackboardKey });
    }

    void BTSetTargetPosition::SetProperty(const std::string& name, const std::string& value) {
        if (name == "TargetX") {
            m_TargetPosition.x = std::stof(value);
        }
        else if (name == "TargetY") {
            m_TargetPosition.y = std::stof(value);
        }
        else if (name == "TargetZ") {
            m_TargetPosition.z = std::stof(value);
        }
        else if (name == "BlackboardKey") {
            m_BlackboardKey = value;
        }
    }

    //BTDestroySelf

    BTStatus BTDestroySelf::Execute(BTContext& context) {
        if (!context.Entity || !context.Scene) {
            return BTStatus::Failure;
        }

        LOG_INFO("BTDestroySelf: Destroying entity");
        TransformSystem::UnParent(context.Scene, *context.Entity); // Unparent before destroying
        context.Scene->DestroyEntity(*context.Entity);
        return BTStatus::Success;
    }

    //BTDestroyEntityByTag

    BTDestroyEntityByTag::BTDestroyEntityByTag(const std::string& tag)
        : m_Tag(tag) {
    }

    BTStatus BTDestroyEntityByTag::Execute(BTContext& context) {
        if (!context.Scene) {
            return BTStatus::Failure;
        }

        try {
            auto& registry = context.Scene->GetRegistry();
            auto view = registry.view<TagComponent>();

            // Store entity to destroy (don't destroy during iteration)
            entt::entity entityToDestroy = entt::null;

            for (auto entityHandle : view) {
                Entity entity(entityHandle, &registry);

                if (entity.HasComponent<TagComponent>()) {
                    auto& tag = entity.GetComponent<TagComponent>();
                    if (tag.Tag == m_Tag) {
                        LOG_INFO("BTDestroyEntityByTag: Found entity with tag '", m_Tag, "'");
                        // Store for destruction after loop
                        entityToDestroy = entityHandle;
                        break;
                    }
                }
            }

            // Destroy entity outside of iteration
            if (entityToDestroy != entt::null) {
                Entity entity(entityToDestroy, &registry);
				TransformSystem::UnParent(context.Scene, entity); // Unparent before destroying
                context.Scene->DestroyEntity(entity);
                LOG_INFO("BTDestroyEntityByTag: Successfully destroyed entity");
                return BTStatus::Success;
            }
            else {
                LOG_WARNING("BTDestroyEntityByTag: No entity found with tag '", m_Tag, "'");
                return BTStatus::Failure;
            }
        }
        catch (const std::exception& e) {
            LOG_ERROR("BTDestroyEntityByTag: Exception - ", e.what());
            return BTStatus::Failure;
        }
    }

    void BTDestroyEntityByTag::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Tag", m_Tag });
    }

    void BTDestroyEntityByTag::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Tag") {
            m_Tag = value;
        }
    }

    //BTCheckHealth
    BTCheckHealth::BTCheckHealth(float threshold, Comparison comp)
        : m_Threshold(threshold)
        , m_Comparison(comp)
        , m_HealthKey("Health") {
    }

    BTStatus BTCheckHealth::Execute(BTContext& context) {

        //LOG_INFO("HIIIIIIIIII FROM BTCHECKHEALTH");

        auto healthOpt = context.Blackboard.Get<float>(m_HealthKey);
        if (!healthOpt) {
            LOG_WARNING("BTCheckHealth: Health not found in blackboard");
            return BTStatus::Failure;
        }

        float health = *healthOpt;
        bool result = false;

        switch (m_Comparison) {
        case Comparison::Greater:
            result = health > m_Threshold;
            break;
        case Comparison::Less:
            result = health < m_Threshold;
            break;
        case Comparison::Equal:
            result = std::abs(health - m_Threshold) < 0.001f;
            break;
        case Comparison::GreaterOrEqual:
            result = health >= m_Threshold;
            break;
        case Comparison::LessOrEqual:
            result = health <= m_Threshold;
			LOG_INFO("BTCheckHealth result is", result ? "YES" : "NO");
			LOG_INFO("BTCheckHealth: Health (", health, ") <= Threshold (", m_Threshold, ")");
            break;
        }

        return result ? BTStatus::Success : BTStatus::Failure;
    }

    void BTCheckHealth::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Threshold", std::to_string(m_Threshold) });
        properties.push_back({ "Comparison", std::to_string(static_cast<int>(m_Comparison)) });
        properties.push_back({ "HealthKey", m_HealthKey });
    }

    void BTCheckHealth::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Threshold") {
            m_Threshold = std::stof(value);
        }
        else if (name == "Comparison") {
            m_Comparison = static_cast<Comparison>(std::stoi(value));
        }
        else if (name == "HealthKey") {
            m_HealthKey = value;
        }
    }

    //BTSetHealth
    BTSetHealth::BTSetHealth(float health)
        : m_Health(health)
        , m_HealthKey("Health") {
    }

    BTStatus BTSetHealth::Execute(BTContext& context) {
        context.Blackboard.Set(m_HealthKey, m_Health);
		LOG_TRACE("BTSetHealth: Set health to ", m_Health);
        return BTStatus::Success;
    }

    void BTSetHealth::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Health", std::to_string(m_Health) });
        properties.push_back({ "HealthKey", m_HealthKey });
    }

    void BTSetHealth::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Health") {
            m_Health = std::stof(value);
        }
        else if (name == "HealthKey") {
            m_HealthKey = value;
        }
    }

    //BTModifyHealth
    BTModifyHealth::BTModifyHealth(float amount)
        : m_Amount(amount)
        , m_HealthKey("Health") {
        LOG_INFO("HEALTH TO MODIFY IN BTMODIFYHEALTH IS: ", amount);
    }
    //change health to int give up
    BTStatus BTModifyHealth::Execute(BTContext& context) {
        //auto healthOpt = context.Blackboard.Get<float>(m_HealthKey);
        //float currentHealth = healthOpt.value_or(100.0f);  // Default to 100 if not set

        float currentHealth = context.Blackboard.GetOrDefault(m_HealthKey, 100.0f);

        if (m_Amount <= 0.01) {
            m_Amount = 0.01;
        }

        currentHealth -= m_Amount;
        context.Blackboard.Set(m_HealthKey, currentHealth);


		LOG_INFO("BTModifyHealth: Modified health by ", m_Amount, ", new health: ", currentHealth);
        return BTStatus::Success;
    }

    void BTModifyHealth::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Amount", std::to_string(m_Amount) });
        properties.push_back({ "HealthKey", m_HealthKey });
    }

    void BTModifyHealth::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Amount") {
            m_Amount = std::stof(value);
        }
        else if (name == "HealthKey") {
            m_HealthKey = value;
        }
    }

    //BTFaceMovementDirection
    BTFaceMovementDirection::BTFaceMovementDirection(float rotationSpeed)
        : m_RotationSpeed(rotationSpeed)
        , m_TargetPositionKey("TargetPosition") {
    }

    BTStatus BTFaceMovementDirection::Execute(BTContext& context) {
        if (!context.Entity || !context.Entity->HasComponent<TransformComponent>()) {
            return BTStatus::Failure;
        }

        auto targetPosOpt = context.Blackboard.Get<glm::vec3>(m_TargetPositionKey);
        if (!targetPosOpt) {
            // Instead of returning Failure which can cause freeze in certain composite structures,
            // return Success to allow the tree to continue
            LOG_TRACE("BTFaceMovementDirection: Target position not found in blackboard, skipping");
            return BTStatus::Success;  // Changed from Failure to Success
        }

        // Rest of the existing implementation...
        glm::vec3 targetPos = *targetPosOpt;
        auto& transform = context.Entity->GetComponent<TransformComponent>();

        glm::vec3 direction = targetPos - transform.Position;
        float distance = glm::length(direction);

        // CRITICAL FIX: Validate distance
        if (distance < 0.001f || std::isnan(distance) || std::isinf(distance)) {
            return BTStatus::Success;  // Already at target or invalid
        }

        //direction = glm::normalize(direction);

        //if (!context.Entity || !context.Entity->HasComponent<TransformComponent>()) {
        //    return BTStatus::Failure;
        //}

        //auto targetPosOpt = context.Blackboard.Get<glm::vec3>(m_TargetPositionKey);
        //if (!targetPosOpt) {
        //    return BTStatus::Failure;
        //}

        //glm::vec3 targetPos = *targetPosOpt;
        //auto& transform = context.Entity->GetComponent<TransformComponent>();

        //glm::vec3 direction = targetPos - transform.Position;
        //float distance = glm::length(direction);

        //// FIXED: Return Success when at arrival distance (matches BTMoveToTarget)
        //// This allows Parallel nodes to complete properly when movement finishes
        //if (distance < 0.5f) {
        //    return BTStatus::Success;
        //}

        //// CRITICAL FIX: Validate distance
        //if (distance < 0.001f || std::isnan(distance) || std::isinf(distance)) {
        //    return BTStatus::Success;  // Already at target or invalid
        //}

        //direction = glm::normalize(direction);

        // CRITICAL FIX: Validate normalized direction
        if (std::isnan(direction.x) || std::isnan(direction.y) || std::isnan(direction.z)) {
            LOG_WARNING("BTFaceMovementDirection: Invalid direction (NaN)");
            return BTStatus::Failure;
        }

        try {
            // Calculate target rotation to face direction
            glm::quat targetRotation = glm::quatLookAt(direction, glm::vec3(0, 1, 0));

            // CRITICAL FIX: Validate quaternion
            if (std::isnan(targetRotation.w) || std::isnan(targetRotation.x) ||
                std::isnan(targetRotation.y) || std::isnan(targetRotation.z)) {
                LOG_WARNING("BTFaceMovementDirection: Invalid rotation (NaN)");
                return BTStatus::Failure;
            }

            // Smooth interpolation
            float t = std::min(1.0f, (m_RotationSpeed * context.DeltaTime) / 180.0f);

            // CRITICAL FIX: Clamp t
            t = std::clamp(t, 0.0f, 1.0f);

            transform.Rotation = glm::slerp(transform.Rotation, targetRotation, t);
            transform.IsDirty = true;

            return BTStatus::Running;
        }
        catch (const std::exception& e) {
            LOG_ERROR("BTFaceMovementDirection: Exception - ", e.what());
            return BTStatus::Failure;
        }
    }

    void BTFaceMovementDirection::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "RotationSpeed", std::to_string(m_RotationSpeed) });
        properties.push_back({ "TargetPositionKey", m_TargetPositionKey });
    }

    void BTFaceMovementDirection::SetProperty(const std::string& name, const std::string& value) {
        if (name == "RotationSpeed") {
            m_RotationSpeed = std::stof(value);
        }
        else if (name == "TargetPositionKey") {
            m_TargetPositionKey = value;
        }
    }

    //BTFaceTarget
    BTFaceTarget::BTFaceTarget(float rotationSpeed)
        : m_RotationSpeed(rotationSpeed)
        , m_TargetEntityKey("TargetEntity") {
    }

    BTStatus BTFaceTarget::Execute(BTContext& context) {
        if (!context.Entity || !context.Entity->HasComponent<TransformComponent>()) {
            return BTStatus::Failure;
        }

        auto targetEntityOpt = context.Blackboard.Get<Entity*>(m_TargetEntityKey);
        if (!targetEntityOpt || !(*targetEntityOpt)) {
            return BTStatus::Failure;
        }

        Entity* targetEntity = *targetEntityOpt;
        if (!targetEntity->HasComponent<TransformComponent>()) {
            return BTStatus::Failure;
        }

        auto& transform = context.Entity->GetComponent<TransformComponent>();
        auto& targetTransform = targetEntity->GetComponent<TransformComponent>();

        glm::vec3 direction = targetTransform.Position - transform.Position;
        float distance = glm::length(direction);

        // FIXED: Return Success when very close (allows completion)
        if (distance < 0.1f) {
            return BTStatus::Success;
        }

        // Validate distance
        if (std::isnan(distance) || std::isinf(distance)) {
            LOG_WARNING("BTFaceTarget: Invalid distance");
            return BTStatus::Failure;
        }

        try {
            direction = glm::normalize(direction);

            // Validate normalized direction
            if (std::isnan(direction.x) || std::isnan(direction.y) || std::isnan(direction.z)) {
                LOG_WARNING("BTFaceTarget: Invalid direction");
                return BTStatus::Failure;
            }
        }
        catch (...) {

        }

        glm::quat targetRotation = glm::quatLookAt(direction, glm::vec3(0, 1, 0));

        float t = std::min(1.0f, (m_RotationSpeed * context.DeltaTime) / 180.0f);
        transform.Rotation = glm::slerp(transform.Rotation, targetRotation, t);
        transform.IsDirty = true;

        return BTStatus::Running;
    }

    void BTFaceTarget::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "RotationSpeed", std::to_string(m_RotationSpeed) });
        properties.push_back({ "TargetEntityKey", m_TargetEntityKey });
    }

    void BTFaceTarget::SetProperty(const std::string& name, const std::string& value) {
        if (name == "RotationSpeed") {
            m_RotationSpeed = std::stof(value);
        }
        else if (name == "TargetEntityKey") {
            m_TargetEntityKey = value;
        }
    }

    //BTCheckEntityCount
    BTCheckEntityCount::BTCheckEntityCount(const std::string& tag, int targetCount, Comparison comp)
        : m_Tag(tag)
        , m_TargetCount(targetCount)
        , m_Comparison(comp) {
    }

    BTStatus BTCheckEntityCount::Execute(BTContext& context) {
        if (!context.Scene) {
            LOG_WARNING("BTCheckEntityCount: No scene context");
            return BTStatus::Failure;
        }

        try {
            auto& registry = context.Scene->GetRegistry();
            auto view = registry.view<TagComponent>();

            int count = 0;
            for (auto entityHandle : view) {
                Entity entity(entityHandle, &registry);

                if (entity.HasComponent<TagComponent>()) {
                    auto& tag = entity.GetComponent<TagComponent>();

                    // CRITICAL FIX: Use partial match (contains) instead of exact match
                    if (tag.Tag.find(m_Tag) != std::string::npos) {
                        count++;
                    }
                }
            }

            LOG_TRACE("BTCheckEntityCount: Found ", count, " entities with tag containing '", m_Tag, "'");

            bool result = false;
            switch (m_Comparison) {
            case Comparison::Greater:
                result = count > m_TargetCount;
                break;
            case Comparison::Less:
                result = count < m_TargetCount;
                break;
            case Comparison::Equal:
                result = count == m_TargetCount;
                break;
            case Comparison::GreaterOrEqual:
                result = count >= m_TargetCount;
                break;
            case Comparison::LessOrEqual:
                result = count <= m_TargetCount;
                break;
            }

            return result ? BTStatus::Success : BTStatus::Failure;
        }
        catch (const std::exception& e) {
            LOG_ERROR("BTCheckEntityCount: Exception - ", e.what());
            return BTStatus::Failure;
        }
    }

    void BTCheckEntityCount::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Tag", m_Tag });
        properties.push_back({ "TargetCount", std::to_string(m_TargetCount) });
        properties.push_back({ "Comparison", std::to_string(static_cast<int>(m_Comparison)) });
    }

    void BTCheckEntityCount::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Tag") {
            m_Tag = value;
        }
        else if (name == "TargetCount") {
            m_TargetCount = std::stoi(value);
        }
        else if (name == "Comparison") {
            m_Comparison = static_cast<Comparison>(std::stoi(value));
        }
    }

    //BTStoreEntityCount
    BTStoreEntityCount::BTStoreEntityCount(const std::string& tag, const std::string& countKey)
        : m_Tag(tag)
        , m_CountKey(countKey) {
    }

    BTStatus BTStoreEntityCount::Execute(BTContext& context) {
        if (!context.Scene) {
            LOG_WARNING("BTStoreEntityCount: No scene context");
            return BTStatus::Failure;
        }

        try {
            auto& registry = context.Scene->GetRegistry();
            auto view = registry.view<TagComponent>();

            int count = 0;
            for (auto entityHandle : view) {
                Entity entity(entityHandle, &registry);

                if (entity.HasComponent<TagComponent>()) {
                    auto& tag = entity.GetComponent<TagComponent>();

                    // CRITICAL FIX: Use partial match (contains) instead of exact match
                    if (tag.Tag.find(m_Tag) != std::string::npos) {
                        count++;
                    }
                }
            }

            context.Blackboard.Set(m_CountKey, count);
            LOG_TRACE("BTStoreEntityCount: Stored count ", count, " for tag containing '", m_Tag, "'");

            return BTStatus::Success;
        }
        catch (const std::exception& e) {
            LOG_ERROR("BTStoreEntityCount: Exception - ", e.what());
            return BTStatus::Failure;
        }
    }

    void BTStoreEntityCount::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Tag", m_Tag });
        properties.push_back({ "CountKey", m_CountKey });
    }

    void BTStoreEntityCount::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Tag") {
            m_Tag = value;
        }
        else if (name == "CountKey") {
            m_CountKey = value;
        }
    }

    //BTChangeColor
    BTChangeColor::BTChangeColor(u32 materialID)
        : m_MaterialID(materialID)
        , m_ElapsedTime(0.0f)
        , m_ChangeInterval(1.0f) {
    }

    const char* BTChangeColor::GetTypeName() const {
        return "ChangeColor";
    }

    void BTChangeColor::OnEnter(BTContext& context) {
        (void)context;
        m_ElapsedTime = 0.0f;
        LOG_INFO("BTChangeColor: OnEnter - Starting continuous color change");
    }

    BTStatus BTChangeColor::Execute(BTContext& context) {
        // Validate entity exists
        if (!context.Entity) {
            LOG_WARNING("BTChangeColor: No entity in context");
            return BTStatus::Failure;
        }

        // Check if entity has MeshRendererComponent
        if (!context.Entity->HasComponent<MeshRendererComponent>()) {
            LOG_WARNING("BTChangeColor: Entity does not have MeshRendererComponent");
            return BTStatus::Failure;
        }
        
        m_ElapsedTime += context.DeltaTime;

        // Only change color when interval has passed
        if (m_ElapsedTime < m_ChangeInterval) {
            return BTStatus::Running;
        }

        // Get the MeshRendererComponent
        auto& meshRenderer = context.Entity->GetComponent<MeshRendererComponent>();

        // Toggle between materials (0 = blue, 1 = orange)
        //u32 newMaterialID = (meshRenderer.Material == 0) ? 1 : 0;

        LOG_INFO("BTChangeColor: Current Material = ", meshRenderer.Material,
            ", Changing to = ", m_MaterialID);

        if (m_MaterialID < 0) {
            m_MaterialID = 0;
        }
        else if(m_MaterialID > 1) {
            m_MaterialID = 1;
        }

        // Change the material
        meshRenderer.Material = m_MaterialID;

        LOG_INFO("BTChangeColor: Material after change = ", meshRenderer.Material,
            " (", (meshRenderer.Material == 0 ? "Blue" : "Orange"), ")");

        // Reset timer
        m_ElapsedTime = 0.0f;

        return BTStatus::Success;
    }

    void BTChangeColor::Reset() {
        m_ElapsedTime = 0.0f;
        LOG_INFO("BTChangeColor: Reset called");
    }

    void BTChangeColor::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "MaterialID", std::to_string(m_MaterialID) });
    }

    void BTChangeColor::SetProperty(const std::string& name, const std::string& value) {
        if (name == "MaterialID") {
            try {
                m_MaterialID = static_cast<u32>(std::stoul(value));
                // Clamp to valid range
                if (m_MaterialID > 1) {
                    m_MaterialID = 0;
                    LOG_WARNING("BTChangeColor: MaterialID clamped to 0 (valid range is 0-1)");
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR("BTChangeColor: Failed to parse MaterialID - ", e.what());
                m_MaterialID = 0;
            }
        }
        else if (name == "ChangeInterval") {
            try {
                m_ChangeInterval = std::stof(value);
                if (m_ChangeInterval <= 0.0f) {
                    m_ChangeInterval = 1.0f;
                    LOG_WARNING("BTChangeColor: ChangeInterval must be > 0, set to 1.0");
                }
            }
            catch (const std::exception& e) {
                LOG_ERROR("BTChangeColor: Failed to parse ChangeInterval - ", e.what());
                m_ChangeInterval = 1.0f;
            }
        }
    }

    // BTOrbitAroundPoint
    BTOrbitAroundPoint::BTOrbitAroundPoint(float orbitRadius, 
                                           float orbitSpeed, 
                                           const glm::vec3& centerPoint)
        : m_OrbitRadius(orbitRadius)
        , m_OrbitSpeed(orbitSpeed)
        , m_CenterPoint(centerPoint)
        , m_CurrentAngle(0.0f)
        , m_CenterPointKey("OrbitCenter") {
    }

    void BTOrbitAroundPoint::OnEnter(BTContext& context) {
        // Initialize the starting angle based on entity's current position relative to center
        if (context.Entity && context.Entity->HasComponent<TransformComponent>()) {
            auto& transform = context.Entity->GetComponent<TransformComponent>();
        
            // Check if there's a dynamic center point in the blackboard
            auto centerOpt = context.Blackboard.Get<glm::vec3>(m_CenterPointKey);
            if (centerOpt) {
                m_CenterPoint = *centerOpt;
            }
        
            // Calculate initial angle from current position
            glm::vec3 offset = transform.Position - m_CenterPoint;
            m_CurrentAngle = glm::degrees(std::atan2(offset.z, offset.x));
        }
    }

    BTStatus BTOrbitAroundPoint::Execute(BTContext& context) {
        if (!context.Entity || !context.Entity->HasComponent<TransformComponent>()) {
            return BTStatus::Failure;
        }

        auto& transform = context.Entity->GetComponent<TransformComponent>();

        // Validate DeltaTime
        if (context.DeltaTime <= 0.0f || std::isnan(context.DeltaTime) || std::isinf(context.DeltaTime)) {
            return BTStatus::Running;
        }

        // Check for dynamic center point update from blackboard
        auto centerOpt = context.Blackboard.Get<glm::vec3>(m_CenterPointKey);
        if (centerOpt) {
            m_CenterPoint = *centerOpt;
        }

        // Store the current Y position (height) - we want to orbit on the same plane
        float currentHeight = transform.Position.y;

        // Update the orbit angle
        m_CurrentAngle += m_OrbitSpeed * context.DeltaTime;
    
        // Keep angle in 0-360 range
        if (m_CurrentAngle >= 360.0f) {
            m_CurrentAngle -= 360.0f;
        } else if (m_CurrentAngle < 0.0f) {
            m_CurrentAngle += 360.0f;
        }

        // Convert angle to radians for trig functions
        float angleRad = glm::radians(m_CurrentAngle);

        // Calculate new position on the circular path (XZ plane)
        float newX = m_CenterPoint.x + m_OrbitRadius * std::cos(angleRad);
        float newZ = m_CenterPoint.z + m_OrbitRadius * std::sin(angleRad);

        // Set new position (maintaining the same height)
        // NOTE: We do NOT modify transform.Rotation - entity keeps its current rotation
        transform.Position = glm::vec3(newX, currentHeight, newZ);
        transform.IsDirty = true;

        // Always running - this creates continuous movement
        return BTStatus::Running;
    }

    void BTOrbitAroundPoint::Reset() {
        m_CurrentAngle = 0.0f;
    }

    void BTOrbitAroundPoint::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "OrbitRadius", std::to_string(m_OrbitRadius) });
        properties.push_back({ "OrbitSpeed", std::to_string(m_OrbitSpeed) });
        properties.push_back({ "CenterX", std::to_string(m_CenterPoint.x) });
        properties.push_back({ "CenterY", std::to_string(m_CenterPoint.y) });
        properties.push_back({ "CenterZ", std::to_string(m_CenterPoint.z) });
        properties.push_back({ "CenterPointKey", m_CenterPointKey });
    }

    void BTOrbitAroundPoint::SetProperty(const std::string& name, const std::string& value) {
        if (name == "OrbitRadius") {
            m_OrbitRadius = std::stof(value);
        }
        else if (name == "OrbitSpeed") {
            m_OrbitSpeed = std::stof(value);
        }
        else if (name == "CenterX") {
            m_CenterPoint.x = std::stof(value);
        }
        else if (name == "CenterY") {
            m_CenterPoint.y = std::stof(value);
        }
        else if (name == "CenterZ") {
            m_CenterPoint.z = std::stof(value);
        }
        else if (name == "CenterPointKey") {
            m_CenterPointKey = value;
        }
    }

	//BTRotateAxis
    // axis should be a unit-like direction (ex: (0,1,0) or (0,0,1))
        // degPerSec = rotation speed in degrees per second
    BTRotateAxis::BTRotateAxis(glm::vec3 axis, float degPerSec)
        : m_Axis(glm::normalize(axis)), m_DegPerSec(degPerSec) {
    }

    const char* BTRotateAxis::GetTypeName() const { return "RotateAxis"; }

    BTStatus BTRotateAxis::Execute(BTContext& context) {
        if (!context.Entity) return BTStatus::Failure;

        if (!context.Entity->HasComponent<TransformComponent>())
            return BTStatus::Failure;

        auto& tc = context.Entity->GetComponent<TransformComponent>();

        // Compute rotation amount this frame
        float angleRad = glm::radians(m_DegPerSec * context.DeltaTime);

        // Convert axis + angle into a quaternion
        glm::quat dq = glm::angleAxis(angleRad, glm::normalize(m_Axis));

        // Apply rotation: newRotation = dq * oldRotation (rotates in local space)
        tc.Rotation = dq * tc.Rotation;
        tc.IsDirty = true;

        return BTStatus::Success; // continuous action
    }

    void BTRotateAxis::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "AxisX", std::to_string(m_Axis.x) });
        properties.push_back({ "AxisY", std::to_string(m_Axis.y) });
        properties.push_back({ "AxisZ", std::to_string(m_Axis.z) });
        properties.push_back({ "DegreesPerSecond", std::to_string(m_DegPerSec) });
    }
    void BTRotateAxis::SetProperty(const std::string& name, const std::string& value) {
        if (name == "AxisX") m_Axis.x = std::stof(value);
        else if (name == "AxisY") m_Axis.y = std::stof(value);
        else if (name == "AxisZ") m_Axis.z = std::stof(value);
        else if (name == "DegreesPerSecond") m_DegPerSec = std::stof(value);
        m_Axis = glm::normalize(m_Axis);
    }

	//BTLookAtSmooth

    BTLookAtSmooth::BTLookAtSmooth(std::string targetKey, float turnSpeedDeg)
        : m_Key(std::move(targetKey)), m_TurnSpeed(glm::radians(turnSpeedDeg)) {
    }

    const char* BTLookAtSmooth::GetTypeName() const { return "LookAtSmooth"; }

    BTStatus BTLookAtSmooth::Execute(BTContext& context) {
        if (!context.Entity) return BTStatus::Failure;
        if (!context.Entity->HasComponent<TransformComponent>()) return BTStatus::Failure;
		if (!context.Blackboard.Has(m_Key)) return BTStatus::Failure;

        auto& tc = context.Entity->GetComponent<TransformComponent>();

        auto tgtOpt = context.Blackboard.Get<glm::vec3>(m_Key);
        if (!tgtOpt) return BTStatus::Failure;
        glm::vec3 target = *tgtOpt;

        // Compute direction to face
        glm::vec3 forward = target - tc.Position;
        if (glm::dot(forward, forward) < 1e-8f)
            return BTStatus::Success;
        forward = glm::normalize(forward);

        // World up
        glm::vec3 up(0, 0, 1);

        // Compute right vector
        glm::vec3 right = glm::normalize(glm::cross(up, forward));

        // Recompute up to ensure orthogonality
        up = glm::normalize(glm::cross(forward, right));

        // Important: GLM mat3 is COLUMN-MAJOR
        // col0 = forward (+X local)
        // col1 = right   (+Y local)
        // col2 = up      (+Z local)
        glm::mat3 rotMat(forward, right, up);

        glm::quat desired = glm::quat_cast(rotMat);

        // Finish when close
        float angleDiff = 2.0f * std::acos(glm::clamp(glm::dot(glm::normalize(tc.Rotation),
            glm::normalize(desired)),
            -1.0f, 1.0f));

        if (angleDiff < glm::radians(1.0f)) {
            LOG_TRACE("BTLookAtSmooth: Completed looking at target");
            return BTStatus::Success;
        }

        // Smooth rotate
        float t = glm::clamp(m_TurnSpeed * context.DeltaTime, 0.0f, 1.0f);
        tc.Rotation = glm::slerp(tc.Rotation, desired, t);
        tc.IsDirty = true;

        

        /*if (angleDiff < glm::radians(1.0f)) {
			LOG_TRACE("BTLookAtSmooth: Completed looking at target");
            return BTStatus::Success;
        }*/

        return BTStatus::Running;

    }

    void BTLookAtSmooth::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
		properties.push_back({ "TargetKey", m_Key });
        properties.push_back({ "TurnSpeedDeg", std::to_string(glm::degrees(m_TurnSpeed)) });
    }

    void BTLookAtSmooth::SetProperty(const std::string& name, const std::string& value) {
        if (name == "TargetKey") {
            m_Key = value;
        }
        else if (name == "TurnSpeedDeg") {
            m_TurnSpeed = glm::radians(std::stof(value));
        }
    }

    //BTLoadWayPoints
    BTCountLoadWaypoints::BTCountLoadWaypoints(std::string waypointKey, std::string countKey)
        : m_WaypointKey(waypointKey)
        , m_CountKey(countKey) {
	}

    const char* BTCountLoadWaypoints::GetTypeName() const { return "LoadWaypoints"; }

    BTStatus BTCountLoadWaypoints::Execute(BTContext& context) {
        
        if (!context.Scene) {
            LOG_WARNING("BTLoadWaypoints: No scene context");
            return BTStatus::Failure;
        }

		auto& registry = context.Scene->GetRegistry();
		auto view = registry.view<TagComponent>();

		int count = 0;

        for (auto entityHandle : view) {
			Entity entity(entityHandle, &registry);

            if(entity.HasComponent<TagComponent>()) {
                auto& tag = entity.GetComponent<TagComponent>();
                // CRITICAL FIX: Use partial match (contains) instead of exact match
                if (tag.Tag.find(m_WaypointKey) != std::string::npos) {
                    // Get TransformComponent
                    if (entity.HasComponent<TransformComponent>()) {
                        auto& transform = entity.GetComponent<TransformComponent>();
                        context.Blackboard.Set(m_WaypointKey + std::to_string(count), transform.Position);
                        count++;
                    }
                }
			}
        }

        context.Blackboard.Set(m_CountKey, count);

        LOG_TRACE("BTLoadWaypoints: Found ", count, " entities containing '", m_WaypointKey, "'");

		return BTStatus::Success;
	}

    void BTCountLoadWaypoints::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "WaypointKey", m_WaypointKey });
        properties.push_back({ "CountKey", m_CountKey });
    }

    void BTCountLoadWaypoints::SetProperty(const std::string& name, const std::string& value) {
        if (name == "WaypointKey") m_WaypointKey = value;
        else if (name == "CountKey") m_CountKey = value;
    }

    //BTGetNextWaypoint
    BTGetNextWaypoint::BTGetNextWaypoint(const std::string& waypointKey, const std::string& countKey,
        const std::string& targetKey) : m_WaypointKey(waypointKey), m_CountKey(countKey), m_TargetKey(targetKey) {
    }

    const char* BTGetNextWaypoint::GetTypeName() const { return "GetNextWaypoint"; }

    BTStatus BTGetNextWaypoint::Execute(BTContext& context) {

        auto countOpt = context.Blackboard.Get<int>(m_CountKey);
        if (!countOpt) {
            LOG_WARNING("BTGetNextWaypoint: Count key '", m_CountKey, "' not found in blackboard");
            return BTStatus::Failure;
        }

        int count = *countOpt;
        if (count <= 0) {
            LOG_WARNING("BTGetNextWaypoint: No waypoints loaded under key '", m_WaypointKey, "'");
            return BTStatus::Failure;
        }

        // Get current index
        int index = context.Blackboard.GetOrDefault<int>("CurrentWaypointIndex", 0);
        //int index = indexOpt.value_or(0);

        if (index < 0) {
            LOG_WARNING("BTGetNextWaypoint: 'CurrentWaypointIndex' not found or invalid in blackboard");
			return BTStatus::Failure;
        }

        // Get the waypoint transform
        glm::vec3 transformOpt = context.Blackboard.GetOrDefault<glm::vec3>(m_WaypointKey + std::to_string(index), glm::vec3(0));

        if (transformOpt == context.Entity->GetComponent<TransformComponent>().Position) {
            LOG_WARNING("BTGetNextWaypoint: Waypoint '", m_WaypointKey + std::to_string(index), "is the same as entity current position");
            return BTStatus::Failure;
        }

        // Store the target position
        context.Blackboard.Set(m_TargetKey, transformOpt);

        // Update index for next call
        //index = (index + 1) % count;
        context.Blackboard.Set("CurrentWaypointIndex", index);

        return BTStatus::Success;
	}

    void BTGetNextWaypoint::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "WaypointKey", m_WaypointKey });
        properties.push_back({ "CountKey", m_CountKey });
        properties.push_back({ "TargetKey", m_TargetKey });
    }

    void BTGetNextWaypoint::SetProperty(const std::string& name, const std::string& value) {
        if(name == "WaypointKey") m_WaypointKey = value;
        else if(name == "CountKey") m_CountKey = value;
		else if (name == "TargetKey") m_TargetKey = value;
    }

    //BTIncrementWaypointIndex

    const char* BTIncrementWaypointIndex::GetTypeName() const { return "IncrementWaypointIndex"; }

    BTStatus BTIncrementWaypointIndex::Execute(BTContext& context) {

        auto currIndex = context.Blackboard.Get<int>("CurrentWaypointIndex");
        if (!currIndex) {
            LOG_WARNING("BTIncrementWaypointIndex: 'CurrentWaypointIndex' not found in blackboard");
            return BTStatus::Failure;
        }

        int index = *currIndex;

        if (index < 0) {
            LOG_WARNING("BTIncrementWaypointIndex: No waypoints loaded");
            return BTStatus::Failure;
        }

        // Increment index
        index++;

		// Set the index back to blackboard
        context.Blackboard.Set("CurrentWaypointIndex", index);
        return BTStatus::Success;
	}

    //BTCheckWaypointReached
    BTCheckWaypointReached::BTCheckWaypointReached(const std::string& targetKey, float arrivalDistance)
        : m_TargetKey(targetKey), m_ArrivalDistance(arrivalDistance) {
	}

    const char* BTCheckWaypointReached::GetTypeName() const { return "CheckWaypointReached"; }

    BTStatus BTCheckWaypointReached::Execute(BTContext& context) {
        if (!context.Entity || !context.Entity->HasComponent<TransformComponent>()) {
            return BTStatus::Failure;
        }

        auto targetPosOpt = context.Blackboard.Get<glm::vec3>(m_TargetKey);
        if (!targetPosOpt) {
            LOG_WARNING("BTCheckWaypointReached: Target position key '", m_TargetKey, "' not found");
            return BTStatus::Failure;
        }

        glm::vec3 targetPos = *targetPosOpt;
        auto& transform = context.Entity->GetComponent<TransformComponent>();

        float distance = glm::length(targetPos - transform.Position);
        if (distance <= m_ArrivalDistance) {
            return BTStatus::Success;
        }
        else {
            return BTStatus::Failure;
        }
	}

    void BTCheckWaypointReached::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "TargetKey", m_TargetKey });
        properties.push_back({ "ArrivalDistance", std::to_string(m_ArrivalDistance)});
    }

    void BTCheckWaypointReached::SetProperty(const std::string& name, const std::string& value) {
        if(name == "TargetKey") {
            m_TargetKey = value;
        }
        else if(name == "ArrivalDistance") {
            m_ArrivalDistance = std::stof(value);
		}
    }

	//BTCheckVisitAllWaypoints - use invert
    BTCheckVisitAllWaypoints::BTCheckVisitAllWaypoints(const std::string& countKey): m_CountKey(countKey) {
	}

    const char* BTCheckVisitAllWaypoints::GetTypeName() const { return "CheckVisitAllWaypoints"; }

    BTStatus BTCheckVisitAllWaypoints::Execute(BTContext& context) {
        auto totalcount = context.Blackboard.GetOrDefault<int>(m_CountKey, -1);
        if (totalcount == -1) {
            LOG_WARNING("BTCheckVisitAllWaypoints: Count key '", m_CountKey, "' not found in blackboard");
            return BTStatus::Failure;
        }

        int visitCount = context.Blackboard.GetOrDefault<int>("CurrentWaypointIndex", -1);
        if (visitCount < 0) {
            LOG_WARNING("BTCheckVisitAllWaypoints: No waypoints visited under key 'CurrentWaypointIndex'");
            return BTStatus::Failure;
        }

        if (visitCount < totalcount || visitCount > totalcount) {
			LOG_TRACE("BTCheckVisitAllWaypoints: Visited ", visitCount, " out of ", totalcount, " waypoints");
            return BTStatus::Failure;
        }
        else if (visitCount == totalcount) {
			LOG_TRACE("BTCheckVisitAllWaypoints: All ", totalcount, " waypoints visited");
			return BTStatus::Success;
        }

		return BTStatus::Failure;
    }

    void BTCheckVisitAllWaypoints::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "CountKey", m_CountKey });
    }

    void BTCheckVisitAllWaypoints::SetProperty(const std::string& name, const std::string& value) {
        if (name == "CountKey") {
            m_CountKey = value;
        }
	}

	//BTApplyLinearVelocity
    BTApplyLinearVelocity::BTApplyLinearVelocity(glm::vec3 linearVelocity) : m_linearVelocity(linearVelocity) {
	}

    const char* BTApplyLinearVelocity::GetTypeName() const {
		return "ApplyLinearVelocity";
    }

    BTStatus BTApplyLinearVelocity::Execute(BTContext& context) {
        
        if(!context.Entity->HasComponent<RigidbodyComponent>()) {
			context.Entity->AddComponent<RigidbodyComponent>();
		}
        
        if (!context.Entity->GetComponent<RigidbodyComponent>().IsKinematic) {
			LOG_WARNING("BTApplyLinearVelocity: Rigidbody is not kinematic");
			return BTStatus::Failure;
        }

		PhysicsAPI::AddLinearVelocity(*context.Entity, m_linearVelocity);
        LOG_INFO("BTApplyLinearVelocity: Applied linear velocity (",
			m_linearVelocity.x, ", ", m_linearVelocity.y, ", ", m_linearVelocity.z, ") to entity.");

		return BTStatus::Success;
    }

    void BTApplyLinearVelocity::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "LinearVelocity.x", std::to_string(m_linearVelocity.x) });
        properties.push_back({ "LinearVelocity.y", std::to_string(m_linearVelocity.y) });
        properties.push_back({ "LinearVelocity.z", std::to_string(m_linearVelocity.z) });
    }

    void BTApplyLinearVelocity::SetProperty(const std::string& name, const std::string& value) {
        if(name == "LinearVelocity.x") {
            m_linearVelocity.x = std::stof(value);
        }
        else if(name == "LinearVelocity.y") {
            m_linearVelocity.y = std::stof(value);
        }
        else if(name == "LinearVelocity.z") {
            m_linearVelocity.z = std::stof(value);
		}
    }

	//BTApplyDampening
    /*BTApplyLinearDampening::BTApplyLinearDampening(float dampening) :
        m_Dampening(dampening) {
    }

    const char* BTApplyLinearDampening::GetTypeName() const {
        return "ApplyLinearDampening";
    }

    BTStatus BTApplyLinearDampening::Execute(BTContext& context) {
        if(!context.Entity->HasComponent<RigidbodyComponent>()) {
			LOG_WARNING("BTApplyDampening: Entity does not have RigidbodyComponent");
			return BTStatus::Failure;
		}

        PhysicsAPI::SetLinearDamping(*context.Entity, m_Dampening);
		LOG_INFO("BTApplyDampening: Applied dampening of ", m_Dampening, " to entity.");
        return BTStatus::Success;
    }

    void BTApplyLinearDampening::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Dampening", std::to_string(m_Dampening)});
    }

    void BTApplyLinearDampening::SetProperty(const std::string& name, const std::string& value) {
        if(name == "Dampening") {
            m_Dampening = std::stof(value);
        }
    }*/

    //BTCreateShootableEnemy
    BTCreateShootableEnemy::BTCreateShootableEnemy(const std::string& tag)
        : m_Tag(tag){
	}

    const char* BTCreateShootableEnemy::GetTypeName() const { return "CreateShootableEnemy"; }

    BTStatus BTCreateShootableEnemy::Execute(BTContext& context) {
        if (!context.Entity) {
			LOG_WARNING("BTCreateShootableEnemy: No entity in context");
			return BTStatus::Failure;
        }
        
        //create enemy entity
        Entity enemy = context.Scene->CreateEntity(m_Tag);
        enemy.AddComponent<MeshRendererComponent>();
        enemy.AddComponent<RigidbodyComponent>();
        enemy.AddComponent<BehaviourTreeComponent>();

        if (!enemy.HasComponent<TransformComponent>() || !enemy.HasComponent<RigidbodyComponent>() ||
            !enemy.HasComponent<BehaviourTreeComponent>()) {
			LOG_WARNING("BTCreateShootableEnemy: Failed to add required components to enemy entity");
            return BTStatus::Failure;
        }

        auto& transform = enemy.GetComponent<TransformComponent>();
        transform.Position = context.Entity->GetComponent<TransformComponent>().Position;

        auto& rb = enemy.GetComponent<RigidbodyComponent>();
        rb.IsKinematic = true;

        auto& bt = enemy.GetComponent<BehaviourTreeComponent>();
        bt.Active = true;
        bt.TreeAssetPath = "ShootableKillableEnemy.json";
        bt.ResetOnComplete = false;

        LOG_INFO("BTCreateShootableEnemy: Created shootable enemy with tag '", m_Tag, "'");
		return BTStatus::Success;
    }


    void BTCreateShootableEnemy::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "Tag", m_Tag });
    }

    void BTCreateShootableEnemy::SetProperty(const std::string& name, const std::string& value) {
        if (name == "Tag") {
			m_Tag = value;
        }
    }

    BTCheckCollision::BTCheckCollision(const std::string& otherTag, bool destroyOnHit)
        : m_OtherTag(otherTag), m_DestroyOnHit(destroyOnHit) {
	}

	const char* BTCheckCollision::GetTypeName() const { return "CheckCollision"; }

    BTStatus BTCheckCollision::Execute(BTContext& context) {

		//LOG_INFO("BTCheckCollision: Executing collision check for tag '", m_OtherTag, "'");

        if (!context.Scene || !context.Entity) {
			LOG_WARNING("BTCheckCollision: No scene or entity context");
			return BTStatus::Failure;
        }

		Entity self = *context.Entity;

        if (!self.HasComponent<TagComponent>()) {
			LOG_WARNING("BTCheckCollision: Entity does not have TagComponent");
			return BTStatus::Failure;
        }

        // Get collisions this frame (temporary instance ok)
        PhysicsAPI api;
        const auto& collisions = api.GetCollisionEvents();

        if (collisions.empty()) {
			LOG_WARNING("BTCheckCollision: No collisions detected this frame");
            return BTStatus::Failure;
        }

		// Get this entity's tag
        entt::entity selfID = (entt::entity)self;

        bool collided = false;
		Entity other(entt::null, nullptr);

        for (const auto& c : collisions) {
            entt::entity otherID = entt::null;

            if (c.entA == selfID){
                LOG_TRACE("BTCheckCollision: Checking collision with entity A");
            otherID = c.entB;
            }
            else if (c.entB == selfID) {
                LOG_TRACE("BTCheckCollision: Checking collision with entity B");
                otherID = c.entA;
            }
            else {
                continue;
            }

            other = context.Scene->GetEntity(otherID);
            if (!other || !other.HasComponent<TagComponent>()) {
				LOG_TRACE("BTCheckCollision: Other entity invalid or missing TagComponent");
                continue;
            }

            auto& tagOther = other.GetComponent<TagComponent>();

            if (tagOther.Tag.find(m_OtherTag) != std::string::npos) {
                collided = true;

                LOG_INFO("BTCheckCollision: '", self.GetComponent<TagComponent>().Tag,
                    "' collided with '", tagOther.Tag, "'");

                // Store in blackboard using TYPE-SAFE API
                if (!m_CollidedIDKey.empty()) {
                    context.Blackboard.Set<int>(m_CollidedIDKey, (int)otherID);
                    LOG_INFO("BTCheckCollision: Stored collided entity ID (", (int)otherID,
                        ") in Blackboard key '", m_CollidedIDKey, "'");
                }

                if (m_DestroyOnHit) {
					LOG_INFO("BTCheckCollision: Destroying entity '", tagOther.Tag, "' on hit");
                    context.Scene->DestroyEntity(other);
                }

                break;
            }
        }

		return collided ? BTStatus::Success : BTStatus::Failure;
    }

    void BTCheckCollision::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "OtherTag", m_OtherTag });
        properties.push_back({ "DestroyOnHit", m_DestroyOnHit? "true" : "false"});
        properties.push_back({ "CollidedIDKey", m_CollidedIDKey});
	}

    void BTCheckCollision::SetProperty(const std::string& name, const std::string& value) {
        if (name == "OtherTag") {
            m_OtherTag = value;
        } else if (name == "DestroyOnHit") {
            m_DestroyOnHit = (value == "true" || value == "1");
		} else if (name == "CollidedIDKey") {
            m_CollidedIDKey = value;
		}
	}

    //BTDeleteCollidedEntity
    BTDeleteCollidedEntity::BTDeleteCollidedEntity(const std::string& collidedIDKey)
        : m_CollidedIDKey(collidedIDKey) {
    }

    const char* BTDeleteCollidedEntity::GetTypeName() const {
        return "DeleteCollidedEntity";
	}

    BTStatus BTDeleteCollidedEntity::Execute(BTContext& context) {

        if (!context.Scene) {
            LOG_WARNING("BTDeleteCollidedEntity: No scene context");
			return BTStatus::Failure;
        }

        // Blackboard must contain a key
        if (m_CollidedIDKey.empty()) {
            LOG_WARNING("BTDeleteCollidedEntity: CollidedIDKey not set");
            return BTStatus::Failure;
        }

        // Try to get the stored entity ID (int) from blackboard
        std::optional<int> idOpt = context.Blackboard.Get<int>(m_CollidedIDKey);
        if (!idOpt) {
            // No stored collision → no deletion → safe fail
            return BTStatus::Failure;
        }

        entt::entity targetID = (entt::entity)*idOpt;

        // Validate the entity still exists
        if (!context.Scene->IsEntityValid(targetID)) {
            LOG_WARNING("BTDeleteCollidedEntity: Stored entity is no longer valid");
            return BTStatus::Failure;
        }

        // Destroy it
        Entity target = context.Scene->GetEntity(targetID);
        LOG_INFO("BTDeleteCollidedEntity: Destroying collided entity (ID: ", (int)targetID, ")");
        context.Scene->DestroyEntity(target);

        return BTStatus::Success;
    }

    void BTDeleteCollidedEntity::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "CollidedIDKey", m_CollidedIDKey });
	}

    void BTDeleteCollidedEntity::SetProperty(const std::string& name, const std::string& value) {
        if (name == "CollidedIDKey") {
            m_CollidedIDKey = value;
        }
	}

    // ============================================================================
    // BTSpawnEnemies - Simple spawn at spawner position
    // ============================================================================

    BTSpawnEnemies::BTSpawnEnemies(int count, const std::string& enemyTag)
        : m_SpawnCount(count)
        , m_EnemyTag(enemyTag) {
    }

    const char* BTSpawnEnemies::GetTypeName() const {
        return "SpawnEnemies";
    }

    BTStatus BTSpawnEnemies::Execute(BTContext& context) {
        if (!context.Entity || !context.Scene) {
            LOG_WARNING("BTSpawnEnemies: No entity or scene in context");
            return BTStatus::Failure;
        }

        // Get spawner position
        if (!context.Entity->HasComponent<TransformComponent>()) {
            LOG_WARNING("BTSpawnEnemies: Spawner has no TransformComponent");
            return BTStatus::Failure;
        }

        auto& spawnerTransform = context.Entity->GetComponent<TransformComponent>();
        glm::vec3 spawnPos = spawnerTransform.Position;

        int successCount = 0;

        // Spawn enemies
        for (int i = 0; i < m_SpawnCount; ++i) {
            Entity enemy = context.Scene->CreateEntity(m_EnemyTag);

            // Add required components
            enemy.AddComponent<MeshRendererComponent>();
            enemy.AddComponent<RigidbodyComponent>();

            // Set position
            if (enemy.HasComponent<TransformComponent>()) {
                auto& enemyTransform = enemy.GetComponent<TransformComponent>();
                enemyTransform.Position = spawnPos;
                successCount++;
            }
        }

        LOG_INFO("BTSpawnEnemies: Spawned ", successCount, "/", m_SpawnCount, " enemies");

        return (successCount > 0) ? BTStatus::Success : BTStatus::Failure;
    }

    void BTSpawnEnemies::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "SpawnCount", std::to_string(m_SpawnCount) });
        properties.push_back({ "EnemyTag", m_EnemyTag });
    }

    void BTSpawnEnemies::SetProperty(const std::string& name, const std::string& value) {
        if (name == "SpawnCount") {
            m_SpawnCount = std::max(1, std::stoi(value));
        }
        else if (name == "EnemyTag") {
            m_EnemyTag = value;
        }
    }

    // ============================================================================
    // BTSpawnEnemiesGrid - Spawn in a grid pattern
    // ============================================================================

    BTSpawnEnemiesGrid::BTSpawnEnemiesGrid(
        int count,
        float spacing,
        const std::string& enemyTag)
        : m_SpawnCount(count)
        , m_Spacing(spacing)
        , m_EnemyTag(enemyTag) {
    }

    const char* BTSpawnEnemiesGrid::GetTypeName() const {
        return "SpawnEnemiesGrid";
    }

    BTStatus BTSpawnEnemiesGrid::Execute(BTContext& context) {
        if (!context.Entity || !context.Scene) {
            LOG_WARNING("BTSpawnEnemiesGrid: No entity or scene in context");
            return BTStatus::Failure;
        }

        // Get spawner position
        if (!context.Entity->HasComponent<TransformComponent>()) {
            LOG_WARNING("BTSpawnEnemiesGrid: Spawner has no TransformComponent");
            return BTStatus::Failure;
        }

        auto& spawnerTransform = context.Entity->GetComponent<TransformComponent>();
        glm::vec3 centerPos = spawnerTransform.Position;

        // Calculate grid dimensions (make it roughly square)
        int gridWidth = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(m_SpawnCount))));
        int gridHeight = static_cast<int>(std::ceil(static_cast<float>(m_SpawnCount) / gridWidth));

        int successCount = 0;
        int enemyIndex = 0;

        // Spawn enemies in grid pattern
        for (int row = 0; row < gridHeight && enemyIndex < m_SpawnCount; ++row) {
            for (int col = 0; col < gridWidth && enemyIndex < m_SpawnCount; ++col) {
                Entity enemy = context.Scene->CreateEntity(m_EnemyTag);

                // Add required components
                enemy.AddComponent<MeshRendererComponent>();
                enemy.AddComponent<RigidbodyComponent>();

                // Calculate grid position (centered around spawner)
                if (enemy.HasComponent<TransformComponent>()) {
                    auto& enemyTransform = enemy.GetComponent<TransformComponent>();

                    float offsetX = (col - gridWidth / 2.0f) * m_Spacing;
                    float offsetZ = (row - gridHeight / 2.0f) * m_Spacing;

                    enemyTransform.Position = centerPos + glm::vec3(offsetX, 0.0f, offsetZ);
                    successCount++;
                }

                enemyIndex++;
            }
        }

        LOG_INFO("BTSpawnEnemiesGrid: Spawned ", successCount, "/", m_SpawnCount,
            " enemies in ", gridWidth, "x", gridHeight, " grid");

        return (successCount > 0) ? BTStatus::Success : BTStatus::Failure;
    }

    void BTSpawnEnemiesGrid::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "SpawnCount", std::to_string(m_SpawnCount) });
        properties.push_back({ "Spacing", std::to_string(m_Spacing) });
        properties.push_back({ "EnemyTag", m_EnemyTag });
    }

    void BTSpawnEnemiesGrid::SetProperty(const std::string& name, const std::string& value) {
        if (name == "SpawnCount") {
            m_SpawnCount = std::max(1, std::stoi(value));
        }
        else if (name == "Spacing") {
            m_Spacing = std::max(0.1f, std::stof(value));
        }
        else if (name == "EnemyTag") {
            m_EnemyTag = value;
        }
    }

    // ============================================================================
    // BTSpawnEnemyAt - Spawn at specific position
    // ============================================================================

    BTSpawnEnemyAt::BTSpawnEnemyAt(
        const std::string& enemyTag,
        const glm::vec3& position)
        : m_EnemyTag(enemyTag)
        , m_SpawnPosition(position) {
    }

    const char* BTSpawnEnemyAt::GetTypeName() const {
        return "SpawnEnemyAt";
    }

    BTStatus BTSpawnEnemyAt::Execute(BTContext& context) {
        if (!context.Scene) {
            LOG_WARNING("BTSpawnEnemyAt: No scene in context");
            return BTStatus::Failure;
        }

        // Create enemy
        Entity enemy = context.Scene->CreateEntity(m_EnemyTag);

        // Add required components
        enemy.AddComponent<MeshRendererComponent>();
        enemy.AddComponent<RigidbodyComponent>();

        // Set position to the specified spawn position
        if (enemy.HasComponent<TransformComponent>()) {
            auto& enemyTransform = enemy.GetComponent<TransformComponent>();
            enemyTransform.Position = m_SpawnPosition;

            LOG_INFO("BTSpawnEnemyAt: Spawned '", m_EnemyTag,
                "' at (", m_SpawnPosition.x, ", ", m_SpawnPosition.y, ", ", m_SpawnPosition.z, ")");
            return BTStatus::Success;
        }

        return BTStatus::Failure;
    }

    void BTSpawnEnemyAt::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "EnemyTag", m_EnemyTag });
        properties.push_back({ "SpawnPosition",
            std::to_string(m_SpawnPosition.x) + "," +
            std::to_string(m_SpawnPosition.y) + "," +
            std::to_string(m_SpawnPosition.z) });
    }

    void BTSpawnEnemyAt::SetProperty(const std::string& name, const std::string& value) {
        if (name == "EnemyTag") {
            m_EnemyTag = value;
        }
        else if (name == "SpawnPosition") {
            // Parse "x,y,z" format
            size_t comma1 = value.find(',');
            size_t comma2 = value.find(',', comma1 + 1);

            if (comma1 != std::string::npos && comma2 != std::string::npos) {
                m_SpawnPosition.x = std::stof(value.substr(0, comma1));
                m_SpawnPosition.y = std::stof(value.substr(comma1 + 1, comma2 - comma1 - 1));
                m_SpawnPosition.z = std::stof(value.substr(comma2 + 1));
            }
        }
    }

    // ============================================================================
    // BTSpawnMultipleTypes - Spawn all 5 enemy types with individual counts
    // ============================================================================

    BTSpawnMultipleTypes::BTSpawnMultipleTypes()
        : m_LoveletterCount(0)
        , m_TrojanCount(0)
        , m_AdwareCount(0)
        , m_WormsCount(0)
        , m_BotnetCount(0)
        , m_WallA(false)
		, m_WallB(false)
		, m_WallC(false)
		, m_WallD(false)
		, m_WallE(false)
		, m_Boss(false)
        , m_SpawnPointCountA(0)
        , m_SpawnPointCountB(0)
        , m_SpawnPointCountC(0)
        , m_SpawnPointCountD(0)
        , m_SpawnPointCountE(0)
       {
    }

    const char* BTSpawnMultipleTypes::GetTypeName() const {
        return "SpawnMultipleTypes";
    }

    //TODO:: NEED FIX THIS APPARENTLY CHILDNODES FOR ONENTER DOESN'T GET CALLED FUN
    void BTSpawnMultipleTypes::OnEnter(BTContext& context) {
        m_EnabledWalls.clear();
        m_totalSpawned = 0;

        // Reset all spawn point cooldowns and availability
        for (auto& wall : m_EnabledWalls) {
            for (auto& spawnPoint : wall.spawnPoints) {
                spawnPoint.CooldownTimer = 0.0f;
                spawnPoint.IsAvailable = true;
            }
        }

        LOG_TRACE("BTSpawnMultipleTypes OnEnter: Cleared walls and reset cooldowns");

	}

    BTStatus BTSpawnMultipleTypes::Execute(BTContext& context) {
        if (!context.Entity || !context.Scene) {
            LOG_WARNING("BTSpawnMultipleTypes: No entity or scene in context");
            return BTStatus::Failure;
        }

        // Update spawn point cooldowns
        UpdateCooldowns(context.DeltaTime);

		//take note missing BOSS logic (if boss stage, skip wall check) TODO::M4 Add boss logic later

		int totalcount = m_LoveletterCount + m_TrojanCount + m_AdwareCount + m_WormsCount + m_BotnetCount;

        auto& registry = context.Scene->GetRegistry();
        auto view = registry.view<TagComponent>();
        for (auto entityHandle : view) {

            Entity entity(entityHandle, &registry);

            //TODO:: FIX THE ROTATION SPAWNING - I JUST DO IT THIS WAY AS I DON'T THINK THERE IS ANYTHING ELSE I SHOULD BE DOING FOR THIS. [SINCE IT'S INITIAL SPAWN]
            if (entity.HasComponent<TagComponent>() && entity.HasComponent<TransformComponent>()) {
                auto& tag = entity.GetComponent<TagComponent>();
                auto& transform = entity.GetComponent<TransformComponent>();
                glm::quat rotation;

                if (m_WallA && tag.Tag == "Wall_A") {
                    //face the positive Z 
                    rotation = glm::quat(glm::radians(glm::vec3(0.0f, -90.0f, 0.0f)));
                    m_EnabledWalls.push_back({ tag.Tag, transform.Position, rotation, transform.Scale, glm::vec3(0.0f, 0.0f, 1.0f), m_SpawnPointCountA });
                }
                else if (m_WallB && tag.Tag == "Wall_B") {
                    //face the positive X
                    rotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
                    m_EnabledWalls.push_back({ tag.Tag, transform.Position, rotation, transform.Scale, glm::vec3(1.0f,0.0f,0.0f), m_SpawnPointCountB });
                }
                else if (m_WallC && tag.Tag == "Wall_C") {
                    //face the positive Z
                    rotation = glm::quat(glm::radians(glm::vec3(0.0f, -90.0f, 0.0f)));
                    m_EnabledWalls.push_back({ tag.Tag, transform.Position, rotation, transform.Scale, glm::vec3(0.0f, 0.0f, 1.0f), m_SpawnPointCountC });
                }
                else if (m_WallD && tag.Tag == "Wall_D") {
                    //face the positive X
                    rotation = glm::quat(glm::radians(glm::vec3(0.0f, 0.0f, 0.0f)));
                    m_EnabledWalls.push_back({ tag.Tag, transform.Position, rotation, transform.Scale, glm::vec3(1.0f,0.0f,0.0f), m_SpawnPointCountD });
                }
                else if (m_WallE && tag.Tag == "Wall_E") {
                    //face the negative X
                    rotation = glm::quat(glm::radians(glm::vec3(0.0f, 180.0f, 0.0f)));
                    m_EnabledWalls.push_back({ tag.Tag, transform.Position, rotation, transform.Scale, glm::vec3(-1.0f, 0.0f, 0.0f), m_SpawnPointCountE });
                }

            }
        }
        LOG_INFO("BTSpawnMultipleTypes: Found ", m_EnabledWalls.size(), " enabled walls");

        // Second pass: Find spawn point entities based on expected count
        // For Wall_A with SpawnPointCountA=2, look for entities tagged "A1" and "A2"
        for (auto& wall : m_EnabledWalls) {
            // Extract wall letter from wall name (e.g., "Wall_A" -> 'A')
            char wallLetter = wall.name.back();  // Gets 'A' from "Wall_A"
            int expectedCount = wall.spawnPointCount;

            LOG_INFO("BTSpawnMultipleTypes: Looking for ", expectedCount, " spawn points for ", wall.name);

            // Look for spawn point entities: A_1, A_2, A_3, etc.
            for (int i = 1; i <= expectedCount; ++i) {
                std::string spawnPointTag = std::string(1, wallLetter) + std::to_string(i);

                // Search for entity with this tag
                bool found = false;
                for (auto entityHandle : view) {
                    Entity entity(entityHandle, &registry);

                    if (entity.HasComponent<TagComponent>() && entity.HasComponent<TransformComponent>()) {
                        auto& tag = entity.GetComponent<TagComponent>();

                        if (tag.Tag == spawnPointTag) {
                            auto& transform = entity.GetComponent<TransformComponent>();

                            SpawnPoint sp;
                            sp.Tag = spawnPointTag;
                            sp.Position = transform.Position;
                            sp.CooldownTimer = 0.0f;
                            sp.IsAvailable = true;
                            wall.spawnPoints.push_back(sp);

                            LOG_INFO("BTSpawnMultipleTypes: Found spawn point '", spawnPointTag, "' at position (",
                                sp.Position.x, ", ", sp.Position.y, ", ", sp.Position.z, ")");
                            found = true;
                            break;
                        }
                    }
                }

                if (!found) {
                    LOG_WARNING("BTSpawnMultipleTypes: Missing spawn point '", spawnPointTag, "' for ", wall.name);
                }
            }

            // Validate we found all expected spawn points
            if (wall.spawnPoints.size() != static_cast<size_t>(expectedCount)) {
                LOG_WARNING("BTSpawnMultipleTypes: ", wall.name, " expected ", expectedCount,
                    " spawn points but found ", wall.spawnPoints.size());
            }
        }

        //walls should be enable if boss is disabled 
        //TODO::ADD IN CHECK TO SEE IF ITS PREP STAGE IF NEEDED IN FUTURE
        if (m_EnabledWalls.empty() && !m_Boss) {
            LOG_WARNING("BTSpawnMultipleTypes: No walls enabled for spawning despite not boss stage");
            return BTStatus::Failure;
        }

        //LOG_INFO("BTSpawnMultipleTypes: Found ", m_EnabledWalls.size(), " enabled walls");

        //TODO:: TAKE NOTE ENTITY CAN BE SEEN SPAWNING AT 0,0,0 BEFORE TELEPORTING TO THEIR RESPECTIVE POSITIONS
		// NEED TO FIX THIS ISSUE (LIKELIHOOD EXPOSE THE FACT ONCE YOU CREATE THE ENTITY, THERE IS AN OPTION TO EXCLUDE TRANSFORM INITIALLY?)

        // ========================================================================
        // 2. Spawn Loveletter at specific position 
        // ========================================================================
        auto spawnLoveLetter = [&]( const glm::vec3& position, const glm::quat& rotation) {

            Entity enemy = context.Scene->CreateEntity("loveletter");

            //enemy.AddComponent<RigidbodyComponent>();
            if (enemy.HasComponent<TransformComponent>()) {
                auto& transform = enemy.GetComponent<TransformComponent>();
                transform.Position = position;
                transform.Rotation = rotation;
                //transform.Scale = glm::vec3(0.002f);

                transform.IsDirty = true;

                // CRITICAL: Manually calculate WorldTransform immediately!
                glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), transform.Position);
                glm::mat4 rotation_matrix = glm::mat4_cast(transform.Rotation);
                glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), transform.Scale);
                glm::mat4 transformation_matrix = translation_matrix * rotation_matrix * scale_matrix;

                transform.WorldTransform = transformation_matrix;
                transform.LocalTransform = transformation_matrix;

                m_totalSpawned++;
            }

            auto& parentMesh = enemy.AddComponent<MeshRendererComponent>();
            std::string meshName = "E005_loveletter_v001.fbx";
            xresource::instance_guid inst_guid = AM.getAssetIdByFilename(meshName);
            parentMesh.MeshGuid = inst_guid;
            parentMesh.SubmeshIndex = 0;

            const int submeshCount = 11;
            for (int sub = 0; sub < submeshCount; ++sub) {
                Entity child = context.Scene->CreateEntity("loveletter_" + std::to_string(sub));
                auto& childTransform = child.GetComponent<TransformComponent>();
                childTransform.Position = glm::vec3(0.0f);
                childTransform.Scale = glm::vec3(1.0f);
                childTransform.Parent = enemy;
                childTransform.IsDirty = true;

                // Calculate child's LocalTransform
                glm::mat4 child_translation = glm::translate(glm::mat4(1.0f), childTransform.Position);
                glm::mat4 child_rotation = glm::mat4_cast(childTransform.Rotation);
                glm::mat4 child_scale = glm::scale(glm::mat4(1.0f), childTransform.Scale);
                childTransform.LocalTransform = child_translation * child_rotation * child_scale;

                if (enemy.HasComponent<TransformComponent>()) {
                    auto& parentTransform = enemy.GetComponent<TransformComponent>();
                    childTransform.WorldTransform = parentTransform.WorldTransform * childTransform.LocalTransform;
                    parentTransform.Children.push_back(child);
                }

                auto& childMesh = child.AddComponent<MeshRendererComponent>();
                childMesh.MeshGuid = inst_guid;
                childMesh.SubmeshIndex = sub;
            }
        };

        // ========================================================================
        // 3. Helper: Spawn single enemy at a random wall with offset
        // TODO::Ensure that depending on the wall itself, the offset changes. 
        // TODO::Ensure it is based off the wall size as well.
        // ========================================================================
        auto spawnAtRandomWall = [&](EnemyType type, const std::string& tag, const std::string& meshName) {
            // Pick a random wall
            int wallIndex = rand() % m_EnabledWalls.size();
            const WallInfo& wall = m_EnabledWalls[wallIndex];

            // Create enemy
            Entity enemy = context.Scene->CreateEntity(tag);

            if (enemy.HasComponent<TransformComponent>()) {
                auto& transform = enemy.GetComponent<TransformComponent>();

                // Calculate wall dimensions (assuming walls are axis-aligned or 90° rotated)
                float wallWidth = wall.scale.x;   // 100 units
                float wallHeight = wall.scale.y;  // 60 units

                // Use 80% of wall size to keep enemies away from edges
                float spawnWidth = wallWidth * 0.8f;
                float spawnHeight = wallHeight * 0.8f;

                // Random position within wall bounds
                float offsetX = ((rand() % 10000) / 10000.0f - 0.5f) * spawnWidth;  // -40 to +40
                float offsetY = ((rand() % 10000) / 10000.0f - 0.5f) * spawnHeight; // -24 to +24

                glm::vec3 spawnPos;
                if (wall.name == "Wall_B" || wall.name == "Wall_D" || wall.name == "Wall_E") {
                    // Wall faces along Z-axis, spawn spread in Z and Y
					LOG_INFO("BTSpawnMultipleTypes: Spread is in Z and Y, Spawning on wall '", wall.name, "'");
                    //transform.Position = wall.position + glm::vec3(0.0f, offsetY, offsetX);
                    spawnPos = wall.position + glm::vec3(0.0f, offsetY, offsetX);
                }
                else {
                    // Wall faces along X-axis, spawn spread in X and Y
					LOG_INFO("BTSpawnMultipleTypes: Spread is in X and Y, Spawning on wall '", wall.name, "'");
                    //transform.Position = wall.position + glm::vec3(offsetX, offsetY, 0.0f);
                    spawnPos = wall.position + glm::vec3(offsetX, offsetY, 0.0f);
                }
                transform.SetPosition(spawnPos);
                transform.Rotation = wall.rotation;
                transform.IsDirty = true;

                // CRITICAL: Manually calculate WorldTransform immediately!
                glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), transform.Position);
                glm::mat4 rotation_matrix = glm::mat4_cast(transform.Rotation);
                glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), transform.Scale);
                glm::mat4 transformation_matrix = translation_matrix * rotation_matrix * scale_matrix;

                transform.WorldTransform = transformation_matrix;
                transform.LocalTransform = transformation_matrix;


                m_totalSpawned++;            
            }

            auto& mesh = enemy.AddComponent<MeshRendererComponent>();
            mesh.Material = 1;

            xresource::instance_guid inst_guid = AM.getAssetIdByFilename(meshName);
            mesh.MeshGuid = inst_guid;

            enemy.AddComponent<RigidbodyComponent>();
            
        };

        // ========================================================================
        // 4. Spawn each enemy type at random enabled walls
        // ========================================================================

        // Spawn Trojans
        for (int i = 0; i < m_TrojanCount; ++i) {
            spawnAtRandomWall(EnemyType::TROJAN, "Trojan", "E003_trojan_v001.fbx");
        }

        // Spawn Adware
        for (int i = 0; i < m_AdwareCount; ++i) {
            spawnAtRandomWall(EnemyType::ADWARE, "Adware", "E007_adware_v001.fbx");
        }

        // Spawn Worms
        for (int i = 0; i < m_WormsCount; ++i) {
            spawnAtRandomWall(EnemyType::WORMS, "Worm_host", "E001_worm_host_v001.fbx");
        }

        // Spawn Botnets
        for (int i = 0; i < m_BotnetCount; ++i) {
            spawnAtRandomWall(EnemyType::BOTNET, "Botnet", "E004_botnet_v001.fbx");
        }

        int lovelettersSpawned = 0;
        for (int i = 0; i < m_LoveletterCount; ++i) {
            bool spawned = false;

            // Try to find an available spawn point
            for (auto& wall : m_EnabledWalls) {
                for (auto& spawnPoint : wall.spawnPoints) {
                    if (spawnPoint.IsAvailable && spawnPoint.CooldownTimer <= 0.0f) {
                        // Spawn at this point
                        spawnLoveLetter(spawnPoint.Position, wall.rotation);

                        // Start cooldown (10 seconds)
                        spawnPoint.CooldownTimer = 10.0f;
                        spawnPoint.IsAvailable = false;

                        spawned = true;
                        lovelettersSpawned++;
                        LOG_INFO("BTSpawnMultipleTypes: Spawned loveletter at spawn point '", spawnPoint.Tag, "'");
                        break;
                    }
                }
                if (spawned) break;
            }

            // If no spawn point available, spawn at random wall as fallback
            if (!spawned) {
                LOG_WARNING("BTSpawnMultipleTypes: No available spawn points, using random wall spawn");
                spawnAtRandomWall(EnemyType::LOVELETTER, "loveletter", "E005_loveletter_v001.fbx");
                lovelettersSpawned++;
            }
        }

        LOG_INFO("BTSpawnMultipleTypes: Spawned ", m_totalSpawned, " enemies total");
        LOG_INFO("  Loveletter: ", m_LoveletterCount);
        LOG_INFO("  Trojan: ", m_TrojanCount);
        LOG_INFO("  Adware: ", m_AdwareCount);
        LOG_INFO("  Worms: ", m_WormsCount);
        LOG_INFO("  Botnet: ", m_BotnetCount);

        return (m_totalSpawned == totalcount) ? BTStatus::Success : BTStatus::Failure;
    }

    void BTSpawnMultipleTypes::Reset() {
        m_EnabledWalls.clear();
        m_totalSpawned = 0;

        // Reset all spawn point cooldowns and availability
        for (auto& wall : m_EnabledWalls) {
            for (auto& spawnPoint : wall.spawnPoints) {
                spawnPoint.CooldownTimer = 0.0f;
                spawnPoint.IsAvailable = true;
            }
        }
    }

    void BTSpawnMultipleTypes::UpdateCooldowns(float deltaTime) {
        for (auto& wall : m_EnabledWalls) {
            for (auto& spawnPoint : wall.spawnPoints) {
                if (spawnPoint.CooldownTimer > 0.0f) {
                    spawnPoint.CooldownTimer -= deltaTime;

                    // Cooldown finished, make available again
                    if (spawnPoint.CooldownTimer <= 0.0f) {
                        spawnPoint.CooldownTimer = 0.0f;
                        spawnPoint.IsAvailable = true;
                        LOG_INFO("BTSpawnMultipleTypes: Spawn point '", spawnPoint.Tag, "' is now available");
                    }
                }
            }
        }
    }

    void BTSpawnMultipleTypes::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "LoveletterCount", std::to_string(m_LoveletterCount) });
        properties.push_back({ "TrojanCount", std::to_string(m_TrojanCount) });
        properties.push_back({ "AdwareCount", std::to_string(m_AdwareCount) });
        properties.push_back({ "WormsCount", std::to_string(m_WormsCount) });
        properties.push_back({ "BotnetCount", std::to_string(m_BotnetCount) });
		properties.push_back({ "WallA", m_WallA ? "true" : "false" });
		properties.push_back({ "WallB", m_WallB ? "true" : "false" });
		properties.push_back({ "WallC", m_WallC ? "true" : "false" });
		properties.push_back({ "WallD", m_WallD ? "true" : "false" });
		properties.push_back({ "WallE", m_WallE ? "true" : "false" });
		properties.push_back({ "Boss", m_Boss ? "true" : "false" });
		properties.push_back({ "SpawnPointA", std::to_string(m_SpawnPointCountA) });
		properties.push_back({ "SpawnPointB", std::to_string(m_SpawnPointCountB) });
		properties.push_back({ "SpawnPointC", std::to_string(m_SpawnPointCountC) });
		properties.push_back({ "SpawnPointD", std::to_string(m_SpawnPointCountD) });
		properties.push_back({ "SpawnPointE", std::to_string(m_SpawnPointCountE) });
    }

    void BTSpawnMultipleTypes::SetProperty(const std::string& name, const std::string& value) {
        if (name == "LoveletterCount") {
            m_LoveletterCount = std::max(0, std::stoi(value));
        }
        else if (name == "TrojanCount") {
            m_TrojanCount = std::max(0, std::stoi(value));
        }
        else if (name == "AdwareCount") {
            m_AdwareCount = std::max(0, std::stoi(value));
        }
        else if (name == "WormsCount") {
            m_WormsCount = std::max(0, std::stoi(value));
        }
        else if (name == "BotnetCount") {
            m_BotnetCount = std::max(0, std::stoi(value));
        } 
        else if (name == "WallA") {
            m_WallA = (value == "true" || value == "1");
        } 
        else if (name == "WallB") {
            m_WallB = (value == "true" || value == "1");
        } 
        else if (name == "WallC") {
            m_WallC = (value == "true" || value == "1");
        } 
        else if (name == "WallD") {
            m_WallD = (value == "true" || value == "1");
        } 
        else if (name == "WallE") {
            m_WallE = (value == "true" || value == "1");
        } 
        else if (name == "Boss") {
            m_Boss = (value == "true" || value == "1");
		} 
        else if (name == "SpawnPointA") {
            m_SpawnPointCountA = std::max(0, std::stoi(value));
        } 
        else if (name == "SpawnPointB") {
            m_SpawnPointCountB = std::max(0, std::stoi(value));
        } 
        else if (name == "SpawnPointC") {
            m_SpawnPointCountC = std::max(0, std::stoi(value));
        } 
        else if (name == "SpawnPointD") {
            m_SpawnPointCountD = std::max(0, std::stoi(value));
        } 
        else if (name == "SpawnPointE") {
            m_SpawnPointCountE = std::max(0, std::stoi(value));
		}
    }



    BTFindNearestEnemy::BTFindNearestEnemy(const std::string& enemyTag,
        const std::string& targetPosKey,
        const std::string& targetEntityKey,
        float maxRange) :
        m_EnemyTag(enemyTag), m_TargetPosKey(targetPosKey),
        m_TargetEntityKey(targetEntityKey), m_MaxRange(maxRange) {
    }

    const char* BTFindNearestEnemy::GetTypeName() const {
        return "FindNearestEnemy";
    }

    BTStatus BTFindNearestEnemy::Execute(BTContext& context) {
        if (!context.Scene || !context.Entity) {
            LOG_WARNING("BTFindNearestEnemy: Missing scene or entity context");
            return BTStatus::Failure;
        }

        Entity& self = *context.Entity;     // use reference
        if (!self.HasComponent<TransformComponent>()) {
            return BTStatus::Failure;
        }

        glm::vec3 myPosition = self.GetComponent<TransformComponent>().Position;

        // Find all entities with the enemy tag
        //auto view = context.Scene->GetRegistry().view<TagComponent, TransformComponent>();
        //Entity* nearestEnemy = nullptr;
        //float nearestDistance = m_MaxRange;
        //glm::vec3 nearestPosition = glm::vec3(0.0f);

        // Get self's entity handle for comparison
        entt::entity selfHandle = static_cast<entt::entity>(self);

        // find all entities with the enemy tag
        auto& registry = context.Scene->GetRegistry();
        //auto view = registry.view<TagComponent, TransformComponent>();

        // check for TransformComponent inside the loop instead
        const auto& view = registry.view<TagComponent>();
        

        Entity* nearestEnemy = nullptr;
        float nearestDistance = m_MaxRange;
        glm::vec3 nearestPosition = glm::vec3(0.0f);

        for (auto entity : view) {
            //Entity potentialEnemy = context.Scene->GetEntity(entity);

            //check for TransformComponent
            if (!registry.all_of<TransformComponent>(entity)) {
                continue;
            }

            // Get components directly from registry
            auto& tag = registry.get<TagComponent>(entity);
            auto& transform = registry.get<TransformComponent>(entity);

            // Skip if not the right tag
            if (tag.Tag.find(m_EnemyTag) == std::string::npos) {
                continue;
            }

            // Skip self
            if (entity == selfHandle) {
                continue;
            }

            glm::vec3 enemyPosition = transform.Position;
            float distance = glm::distance(myPosition, enemyPosition);

            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearestPosition = enemyPosition;
                nearestEnemy = new Entity(entity, &registry);
            }
        }

        if (nearestEnemy) {
            // Store both position (for BTLookAtSmooth) and entity (for validation)
            context.Blackboard.Set<glm::vec3>(m_TargetPosKey, nearestPosition);
            context.Blackboard.Set<Entity*>(m_TargetEntityKey, nearestEnemy);
            return BTStatus::Success;
        }

        // No enemy found - clear targets
        context.Blackboard.Set<Entity*>(m_TargetEntityKey, nullptr);
        return BTStatus::Failure;
    }

    void BTFindNearestEnemy::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "EnemyTag", m_EnemyTag });
        properties.push_back({ "TargetPosKey", m_TargetPosKey });
        properties.push_back({ "TargetEntityKey", m_TargetEntityKey });
        properties.push_back({ "MaxRange", std::to_string(m_MaxRange) });
    }

    void BTFindNearestEnemy::SetProperty(const std::string& name, const std::string& value) {
        if (name == "EnemyTag") {
            m_EnemyTag = value;
        }
        else if (name == "TargetPosKey") {
            m_TargetPosKey = value;
        }
        else if (name == "TargetEntityKey") {
            m_TargetEntityKey = value;
        }
        else if (name == "MaxRange") {
            m_MaxRange = std::stof(value);
        }
    }

    BTHasValidTarget::BTHasValidTarget(const std::string& targetEntityKey)
        : m_TargetEntityKey(m_TargetEntityKey) {
    }

    const char* BTHasValidTarget::GetTypeName() const {
        return "HasValidTarget";
    }

    BTStatus BTHasValidTarget::Execute(BTContext& context) {
        if (!context.Scene) {
            return BTStatus::Failure;
        }

        // check if target entity exists in Blackboard
        auto targetOpt = context.Blackboard.Get<Entity*>(m_TargetEntityKey);
        if (!targetOpt) {
            return BTStatus::Failure;
        }

        Entity* target = *targetOpt;
        if (!target) {
            return BTStatus::Failure;
        }

        // validate entity still exists in the registry
        auto& registry = context.Scene->GetRegistry();
        entt::entity targetHandle = static_cast<entt::entity>(*target);

        if (!registry.valid(targetHandle)) {
            //target was destroyed - clear from blackboard
            context.Blackboard.Set<Entity*>(m_TargetEntityKey, nullptr);
            return BTStatus::Failure;
        }

        return BTStatus::Success;

    }

    void BTHasValidTarget::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "TargetEntityKey", m_TargetEntityKey });
    }

    void BTHasValidTarget::SetProperty(const std::string& name, const std::string& value) {
        if (name == "TargetEntityKey") {
            m_TargetEntityKey = value;
        }
    }

    BTShootBullet::BTShootBullet(const std::string& bulletTag, float fireRate, float bulletSpeed)
        : m_BulletTag(bulletTag)
        , m_FireRate(fireRate)
        , m_BulletSpeed(bulletSpeed)
        , m_Cooldown(0.0f) {
    }

    const char* BTShootBullet::GetTypeName() const {
        return "ShootBullet";
    }

    void BTShootBullet::OnEnter(BTContext& context) {
        (void)context;
        m_Cooldown = 0.0f;      // reset cooldown when entering
    }

    BTStatus BTShootBullet::Execute(BTContext& context) {
        if (!context.Scene || !context.Entity) {
            return BTStatus::Failure;
        }

        // code has been accommodated for lack of prefab system

        //update cooldown
        m_Cooldown -= context.DeltaTime;
        if (m_Cooldown > 0.0f) {
            return BTStatus::Running;       // still cooling down
        }

        Entity& self = *context.Entity;
        if (!self.HasComponent<TransformComponent>()) {
            return BTStatus::Failure;
        }

        // Get target position from blackboard
        auto targetPosOpt = context.Blackboard.Get<glm::vec3>("TargetPosition");
        if (!targetPosOpt) {
            LOG_WARNING("BTShootBullet: NO Target position in blackboard");
            return BTStatus::Failure;
        }

        glm::vec3 targetPos = *targetPosOpt;
        auto& selfTransform = self.GetComponent<TransformComponent>();
        glm::vec3 selfPos = selfTransform.Position;

        // calculate direction to target
        glm::vec3 direction = targetPos - selfPos;
        float distance = glm::length(direction);

        if (distance < 0.001f) {
            LOG_WARNING("BTShootBullet: Target too close to shooter");
            return BTStatus::Failure;
        }

        direction = glm::normalize(direction);

        // create bullet entity (currently, no prefab system
        Entity bullet = context.Scene->CreateEntity(m_BulletTag);

        // add the required components
        if (!bullet.HasComponent<TransformComponent>()) {
            bullet.AddComponent<TransformComponent>();
        }
        if (!bullet.HasComponent<MeshRendererComponent>()) {
            bullet.AddComponent<MeshRendererComponent>();
        }
        if (!bullet.HasComponent<RigidbodyComponent>()) {
            bullet.AddComponent<RigidbodyComponent>();
        }

        // set bullet position (slightly offset from shooter
        auto& bulletTransform = bullet.GetComponent<TransformComponent>();
        bulletTransform.Position = selfPos + direction * 2.0f;  // spawn 2 units in front
        bulletTransform.Scale = glm::vec3(0.5f);    // small bullet

        // set bullet velocity using rigidbody
        auto& bulletRB = bullet.GetComponent<RigidbodyComponent>();
        bulletRB.IsKinematic = true;

        // apply velocity
        PhysicsAPI::AddLinearVelocity(bullet, direction * m_BulletSpeed);

        // Reset cooldown
        m_Cooldown = m_FireRate;

        LOG_TRACE("BTShootBullet: Fired bullet towards (",
            targetPos.x, ", ", targetPos.y, ", ", targetPos.z, ")");

        return BTStatus::Success;
    }

    void BTShootBullet::Reset() {
        m_Cooldown = 0.0f;
    }

    void BTShootBullet::GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const {
        properties.push_back({ "BulletTag", m_BulletTag });
        properties.push_back({ "FireRate", std::to_string(m_FireRate) });
        properties.push_back({ "BulletSpeed", std::to_string(m_BulletSpeed) });
    }

    void BTShootBullet::SetProperty(const std::string& name, const std::string& value) {
        if (name == "BulletTag") {
            m_BulletTag = value;
        }
        else if (name == "FireRate") {
            m_FireRate = std::stof(value);
        }
        else if (name == "BulletSpeed") {
            m_BulletSpeed = std::stof(value);
        }
    }

} // namespace Engine
