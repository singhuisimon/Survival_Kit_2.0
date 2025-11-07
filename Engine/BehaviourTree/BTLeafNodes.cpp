/**
 * @file BTLeafNodes.h
 * @brief Leaf behaviour tree nodes (Actions and Conditions)
 * @author AI System Team
 * @date 2025
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
#include "Transform/TransformSystem.h"

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
    }

    BTStatus BTModifyHealth::Execute(BTContext& context) {
        auto healthOpt = context.Blackboard.Get<float>(m_HealthKey);
        float currentHealth = healthOpt.value_or(100.0f);  // Default to 100 if not set

        currentHealth += m_Amount;
        context.Blackboard.Set(m_HealthKey, currentHealth);

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

        // CRITICAL: Return Running to keep executing every frame
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

        // Smooth rotate
        float t = glm::clamp(m_TurnSpeed * context.DeltaTime, 0.0f, 1.0f);
        tc.Rotation = glm::slerp(tc.Rotation, desired, t);
        tc.IsDirty = true;

        // Finish when close
        float angleDiff = 2.0f * std::acos(glm::clamp(glm::dot(glm::normalize(tc.Rotation),
            glm::normalize(desired)),
            -1.0f, 1.0f));

        if (angleDiff < glm::radians(1.0f))
            return BTStatus::Success;

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

} // namespace Engine
