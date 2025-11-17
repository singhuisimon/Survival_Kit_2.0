/**
 * @file BTNodeRegistry.cpp
 * @brief Definition of Behaviour Tree Node Registry, Where you register all the created nodes
 * @author Amanda Leow Boon Suan (90%), Rio Shannon Yvon Leonardo (10%)
 * @date 3/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#pragma once

#include "BTNodeRegistry.h"
#include "../Utility/Logger.h"

#include <unordered_map>
#include <memory>
#include <functional>
#include <string>

namespace Engine {


    //BTNodeRegistry

    /**
     * @brief Register a node type
     * @param setupProperties Optional callback to register node properties
     */
    template<typename T>
    void BTNodeRegistry::RegisterNodeType(
        const std::string& category,
        const std::string& description,
        std::function<void(BTNodeMetadata&)> setupProperties) {

        // Create a temporary instance to get the type name
        auto temp = std::make_shared<T>();
        std::string typeName = temp->GetTypeName();

        BTNodeTypeMetadata metadata(
            typeName,
            category,
            description,
            []() { return std::make_shared<T>(); },
            temp->CanHaveChildren()
        );

        // Setup properties if callback provided
        if (setupProperties && metadata.PropertyMetadata) {
            setupProperties(*metadata.PropertyMetadata);
        }

        m_NodeTypes[typeName] = metadata;
        LOG_INFO("BTNodeRegistry: Registered node type '", typeName, "' (Category: ", category, ")");
    }

    /**
     * @brief Create a node instance by type name
     */
    std::shared_ptr<BTNode> BTNodeRegistry::CreateNode(const std::string& typeName) const {
        auto it = m_NodeTypes.find(typeName);
        if (it == m_NodeTypes.end()) {
            LOG_ERROR("BTNodeRegistry: Unknown node type '", typeName, "'");
            return nullptr;
        }

        auto node = it->second.Factory();
        if (node) {
            LOG_TRACE("BTNodeRegistry: Created node of type '", typeName, "'");
        }
        return node;
    }

    /**
     * @brief Get metadata for a node type
     */
    const BTNodeTypeMetadata* BTNodeRegistry::GetMetadata(const std::string& typeName) const {
        auto it = m_NodeTypes.find(typeName);
        if (it == m_NodeTypes.end()) {
            return nullptr;
        }
        return &it->second;
    }

    /**
     * @brief Get all registered node types
     */
    const std::unordered_map<std::string, BTNodeTypeMetadata>& BTNodeRegistry::GetAllNodeTypes() const {
        return m_NodeTypes;
    }

    /**
     * @brief Get all node types in a specific category
     */
    std::vector<std::string> BTNodeRegistry::GetNodeTypesByCategory(const std::string& category) const {
        std::vector<std::string> types;
        for (const auto& [typeName, metadata] : m_NodeTypes) {
            if (metadata.Category == category) {
                types.push_back(typeName);
            }
        }
        return types;
    }

    /**
     * @brief Check if a node type is registered
     */
    bool BTNodeRegistry::IsNodeTypeRegistered(const std::string& typeName) const {
        return m_NodeTypes.find(typeName) != m_NodeTypes.end();
    }

    /**
     * @brief Clear all registered types
     */
    void BTNodeRegistry::Clear() {
        m_NodeTypes.clear();
        LOG_INFO("BTNodeRegistry: Cleared all node types");
    }

    /**
     * @brief Register all built-in node types
     */
    void BTNodeRegistry::RegisterBuiltInNodes() {
        auto& registry = Get();

        // Composite nodes (no parameters)
        registry.RegisterNodeType<BTSequence>("Composite", "Executes children in sequence until one fails");
        registry.RegisterNodeType<BTSelector>("Composite", "Executes children until one succeeds");

        // Parallel with properties
        registry.RegisterNodeType<BTParallel>("Composite", "Executes all children simultaneously",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_ENUM_PROPERTY(BTParallel, "SuccessPolicy", m_SuccessPolicy,
                    PolicyToString, StringToPolicy,
                    (std::vector<std::string>{"RequireAll", "RequireOne"}));
                BT_REGISTER_ENUM_PROPERTY(BTParallel, "FailurePolicy", m_FailurePolicy,
                    PolicyToString, StringToPolicy,
                    (std::vector<std::string>{"RequireAll", "RequireOne"}));
            });

        // Decorator nodes
        registry.RegisterNodeType<BTInverter>("Decorator", "Inverts the result of its child");
        registry.RegisterNodeType<BTSucceeder>("Decorator", "Always returns success");
        registry.RegisterNodeType<BTFailer>("Decorator", "Always returns failure");

        // Repeater with properties
        registry.RegisterNodeType<BTRepeater>("Decorator", "Repeats child N times or infinitely",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_INT_PROPERTY(BTRepeater, "RepeatCount", m_RepeatCount);
            });

        registry.RegisterNodeType<BTRepeatUntilFail>("Decorator", "Repeats child until it fails");

        // Cooldown with properties
        registry.RegisterNodeType<BTCooldown>("Decorator", "Prevents child from running during cooldown",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTCooldown, "CooldownTime", m_CooldownTime);
            });

        // Leaf nodes
		// for internal coding/testing purposes
        registry.RegisterNodeType<BTAction>("Action", "Executes custom action logic");
        registry.RegisterNodeType<BTCondition>("Condition", "Checks a condition");

        // Wait with properties
        registry.RegisterNodeType<BTWait>("Action", "Waits for a duration",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTWait, "Duration", m_Duration);
            });

        // Log with properties
        registry.RegisterNodeType<BTLog>("Action", "Outputs a debug message",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTLog, "Message", m_Message);
            });

        // SetBlackboard with properties
        registry.RegisterNodeType<BTSetBlackboard>("Action", "Sets a blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboard, "Key", m_Key);
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboard, "Type", m_Type);
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboard, "Value", m_Value);
            });


        // CheckBlackboard with properties
        registry.RegisterNodeType<BTCheckBlackboard>("Condition", "Checks a blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTCheckBlackboard, "Key", m_Key);
                BT_REGISTER_STRING_PROPERTY(BTCheckBlackboard, "ExpectedValue", m_ExpectedValue);
            });

        // Blackboard convenience nodes
        registry.RegisterNodeType<BTSetBlackboardInt>("Action", "Sets an integer blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboardInt, "Key", m_Key);
                BT_REGISTER_INT_PROPERTY(BTSetBlackboardInt, "Value", m_Value);
            });

        registry.RegisterNodeType<BTSetBlackboardFloat>("Action", "Sets a float blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboardFloat, "Key", m_Key);
                BT_REGISTER_FLOAT_PROPERTY(BTSetBlackboardFloat, "Value", m_Value);
            });

        registry.RegisterNodeType<BTSetBlackboardBool>("Action", "Sets a bool blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboardBool, "Key", m_Key);
                BT_REGISTER_BOOL_PROPERTY(BTSetBlackboardBool, "Value", m_Value);
            });

        registry.RegisterNodeType<BTSetBlackboardVec3>("Action", "Sets a vec3 blackboard value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSetBlackboardVec3, "Key", m_Key);
                BT_REGISTER_VEC3_PROPERTY(BTSetBlackboardVec3, "Value", m_Value);
            });

        LOG_INFO("BTNodeRegistry: Registered Blackboard Set node types (Int, Float, Bool, Vec3)");

        registry.RegisterNodeType<BTRotateEntity>("Action", "Rotates entity continously",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTRotateEntity, "RotationSpeed", m_RotationSpeed);
            });

        registry.RegisterNodeType<BTMoveToTarget>("Action", "Moves towards target position in blackboard",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTMoveToTarget, "MoveSpeed", m_MoveSpeed);
                BT_REGISTER_FLOAT_PROPERTY(BTMoveToTarget, "ArrivalDistance", m_ArrivalDistance);
                BT_REGISTER_STRING_PROPERTY(BTMoveToTarget, "TargetPositionKey", m_TargetPositionKey);
            });

        registry.RegisterNodeType<BTSetTargetPosition>("Action", "Sets target position in blackboard",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSetTargetPosition, "BlackboardKey", m_BlackboardKey);
                BT_REGISTER_VEC3_PROPERTY(BTSetTargetPosition, "TargetPosition", m_TargetPosition);
            });

        registry.RegisterNodeType<BTDestroySelf>("Action", "Destroys the current entity");

        registry.RegisterNodeType<BTDestroyEntityByTag>("Action", "Destroys entity by tag",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTDestroyEntityByTag, "Tag", m_Tag);
            });

        registry.RegisterNodeType<BTCheckHealth>("Condition", "Checks health against threshold",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTCheckHealth, "Threshold", m_Threshold);
                BT_REGISTER_STRING_PROPERTY(BTCheckHealth, "HealthKey", m_HealthKey);
                BT_REGISTER_ENUM_PROPERTY(BTCheckHealth, "Comparison", m_Comparison,
                    ComparisonToString, StringToComparison,
                    (std::vector<std::string>{"Greater", "Less", "Equal", "GreaterOrEqual", "LessOrEqual"}));
            });

        registry.RegisterNodeType<BTSetHealth>("Action", "Sets health value",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTSetHealth, "Health", m_Health);
                BT_REGISTER_STRING_PROPERTY(BTSetHealth, "HealthKey", m_HealthKey);
            });

        registry.RegisterNodeType<BTModifyHealth>("Action", "Modifies health (add/subtract)",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTModifyHealth, "Amount", m_Amount);
                BT_REGISTER_STRING_PROPERTY(BTModifyHealth, "HealthKey", m_HealthKey);
            });

        registry.RegisterNodeType<BTFaceMovementDirection>("Action", "Faces movement direction",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTFaceMovementDirection, "RotationSpeed", m_RotationSpeed);
                BT_REGISTER_STRING_PROPERTY(BTFaceMovementDirection, "TargetPositionKey", m_TargetPositionKey);
            });

        registry.RegisterNodeType<BTFaceTarget>("Action", "Faces target entity",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTFaceTarget, "RotationSpeed", m_RotationSpeed);
                BT_REGISTER_STRING_PROPERTY(BTFaceTarget, "TargetEntityKey", m_TargetEntityKey);
            });

        registry.RegisterNodeType<BTCheckEntityCount>("Condition", "Checks entity count by tag",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTCheckEntityCount, "Tag", m_Tag);
                BT_REGISTER_INT_PROPERTY(BTCheckEntityCount, "TargetCount", m_TargetCount);
            });

        registry.RegisterNodeType<BTStoreEntityCount>("Action", "Stores entity count in blackboard",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTStoreEntityCount, "Tag", m_Tag);
                BT_REGISTER_STRING_PROPERTY(BTStoreEntityCount, "CountKey", m_CountKey);
            });

        registry.RegisterNodeType<BTChangeColor>("Action", "Changes object color over time",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_INT_PROPERTY(BTChangeColor, "MaterialID", m_MaterialID);
                BT_REGISTER_FLOAT_PROPERTY(BTChangeColor, "ChangeInterval", m_ChangeInterval);
            });

        registry.RegisterNodeType<BTOrbitAroundPoint>("Action", "Moves in a circular direction around a point",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTOrbitAroundPoint, "OrbitRadius", m_OrbitRadius);
                BT_REGISTER_FLOAT_PROPERTY(BTOrbitAroundPoint, "OrbitSpeed", m_OrbitSpeed);
                BT_REGISTER_VEC3_PROPERTY(BTOrbitAroundPoint, "CenterPoint", m_CenterPoint);
                BT_REGISTER_STRING_PROPERTY(BTOrbitAroundPoint, "CenterPointKey", m_CenterPointKey);
            });

        registry.RegisterNodeType<BTRotateAxis>("Action", "Rotate entity around a chosen axis",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_VEC3_PROPERTY(BTRotateAxis, "Axis", m_Axis);
                BT_REGISTER_FLOAT_PROPERTY(BTRotateAxis, "DegreesPerSecond", m_DegPerSec);
            }
        );

        registry.RegisterNodeType<BTLookAtSmooth>("Action", "Rotate entity to face a world position (from Blackboard)",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTLookAtSmooth, "TargetKey", m_Key);
                BT_REGISTER_FLOAT_PROPERTY(BTLookAtSmooth, "TurnSpeedDeg", m_TurnSpeed);
            }
        );

        registry.RegisterNodeType<BTCountLoadWaypoints>("Action", "count and load waypoint into board",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTCountLoadWaypoints, "WaypointKey", m_WaypointKey);
                BT_REGISTER_STRING_PROPERTY(BTCountLoadWaypoints, "CountKey", m_CountKey);
            }
        );

        registry.RegisterNodeType<BTGetNextWaypoint>("Action", "Get and Set the next waypoint as target",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTGetNextWaypoint, "WaypointKey", m_WaypointKey);
                BT_REGISTER_STRING_PROPERTY(BTGetNextWaypoint, "CountKey", m_CountKey);
                BT_REGISTER_STRING_PROPERTY(BTGetNextWaypoint, "TargetKey", m_TargetKey);
            }
        );

        registry.RegisterNodeType<BTIncrementWaypointIndex>("Action", "Increment internal index to + 1");

        registry.RegisterNodeType<BTCheckWaypointReached>("Action", "Check if waypoint has been reached",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTCheckWaypointReached, "TargetKey", m_TargetKey);
                BT_REGISTER_FLOAT_PROPERTY(BTCheckWaypointReached, "ArrivalDistance", m_ArrivalDistance);
            }
        );

        registry.RegisterNodeType<BTCheckVisitAllWaypoints>("Action", "Check if all waypoint has been visited",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTCheckVisitAllWaypoints, "CountKey", m_CountKey);
            }
        );

        registry.RegisterNodeType<BTApplyLinearVelocity>("Action", "Check if all waypoint has been visited",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_VEC3_PROPERTY(BTApplyLinearVelocity, "LinearVelocity", m_linearVelocity);
            }
        );

        /*registry.RegisterNodeType<BTApplyLinearDampening>("Action", "Check if all waypoint has been visited",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_FLOAT_PROPERTY(BTApplyLinearDampening, "Dampening", m_Dampening);
            }
        );*/

        registry.RegisterNodeType<BTCreateShootableEnemy>("Action", "Create shootable enemy",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTCreateShootableEnemy, "Tag", m_Tag);
            }
        );

        registry.RegisterNodeType<BTCheckCollision>("Action", "Check for collision other tag entities",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTCheckCollision, "OtherTag", m_OtherTag);
                BT_REGISTER_STRING_PROPERTY(BTCheckCollision, "CollidedIDKey", m_CollidedIDKey);
                BT_REGISTER_BOOL_PROPERTY(BTCheckCollision, "DestroyOnHit", m_DestroyOnHit);
            }
        );
        
        registry.RegisterNodeType<BTDeleteCollidedEntity>("Action", "Delete collided entity",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTDeleteCollidedEntity, "CollidedIDKey", m_CollidedIDKey);
            }
        );

        // Multi-type spawner (for your 5 enemy types)
        registry.RegisterNodeType<BTSpawnMultipleTypes>("Action", "Spawns all 5 enemy types with individual counts",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_INT_PROPERTY(BTSpawnMultipleTypes, "LoveletterCount", m_LoveletterCount);
                BT_REGISTER_INT_PROPERTY(BTSpawnMultipleTypes, "TrojanCount", m_TrojanCount);
                BT_REGISTER_INT_PROPERTY(BTSpawnMultipleTypes, "AdwareCount", m_AdwareCount);
                BT_REGISTER_INT_PROPERTY(BTSpawnMultipleTypes, "WormsCount", m_WormsCount);
                BT_REGISTER_INT_PROPERTY(BTSpawnMultipleTypes, "BotnetCount", m_BotnetCount);
                BT_REGISTER_FLOAT_PROPERTY(BTSpawnMultipleTypes, "Spacing", m_Spacing);
                BT_REGISTER_VEC3_PROPERTY(BTSpawnMultipleTypes, "LoveletterPos", m_LoveletterPos);
				BT_REGISTER_BOOL_PROPERTY(BTSpawnMultipleTypes, "WallA", m_WallA);
				BT_REGISTER_BOOL_PROPERTY(BTSpawnMultipleTypes, "WallB", m_WallB);
				BT_REGISTER_BOOL_PROPERTY(BTSpawnMultipleTypes, "WallC", m_WallC);
				BT_REGISTER_BOOL_PROPERTY(BTSpawnMultipleTypes, "WallD", m_WallD);
                BT_REGISTER_BOOL_PROPERTY(BTSpawnMultipleTypes, "WallE", m_WallE);
				BT_REGISTER_BOOL_PROPERTY(BTSpawnMultipleTypes, "Boss", m_Boss);
            });

        // Spawn single enemy at specific position
        registry.RegisterNodeType<BTSpawnEnemyAt>("Action", "Spawns enemy at specific world position",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_STRING_PROPERTY(BTSpawnEnemyAt, "EnemyTag", m_EnemyTag);
                BT_REGISTER_VEC3_PROPERTY(BTSpawnEnemyAt, "SpawnPosition", m_SpawnPosition);
            });

        // Simple spawners
        registry.RegisterNodeType<BTSpawnEnemies>("Action", "Spawns N enemies at spawner position",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_INT_PROPERTY(BTSpawnEnemies, "SpawnCount", m_SpawnCount);
                BT_REGISTER_STRING_PROPERTY(BTSpawnEnemies, "EnemyTag", m_EnemyTag);
            });

        registry.RegisterNodeType<BTSpawnEnemiesGrid>("Action", "Spawns N enemies in a grid pattern",
            [](BTNodeMetadata& metadata) {
                BT_REGISTER_INT_PROPERTY(BTSpawnEnemiesGrid, "SpawnCount", m_SpawnCount);
                BT_REGISTER_FLOAT_PROPERTY(BTSpawnEnemiesGrid, "Spacing", m_Spacing);
                BT_REGISTER_STRING_PROPERTY(BTSpawnEnemiesGrid, "EnemyTag", m_EnemyTag);
            });

        LOG_INFO("BTNodeRegistry: Registered ", registry.m_NodeTypes.size(), " built-in node types");
    }


} // namespace Engine
