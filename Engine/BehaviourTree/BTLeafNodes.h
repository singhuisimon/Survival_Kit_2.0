/**
 * @file BTLeafNodes.h
 * @brief Leaf behaviour tree nodes (Actions and Conditions)
 * @author AI System Team
 * @date 2025
 */

#pragma once

#include "BTNode.h"
#include <functional>
#include "Utility/Types.h"

namespace Engine {

    /**
     * @brief Action node - executes a custom action
     * @details Leaf node that performs actual game logic
     */
    class BTAction : public BTNode {
    public:
        using ActionFunc = std::function<BTStatus(BTContext&)>;

        BTAction(ActionFunc action = nullptr);

        const char* GetTypeName() const override;

        void SetAction(ActionFunc action);

        BTStatus Execute(BTContext& context) override;

    private:
        ActionFunc m_Action;
    };

    /**
     * @brief Condition node - checks a condition and returns success/failure
     */
    class BTCondition : public BTNode {
    public:
        using ConditionFunc = std::function<bool(BTContext&)>;

        BTCondition(ConditionFunc condition = nullptr);

        const char* GetTypeName() const override;

        void SetCondition(ConditionFunc condition);

        BTStatus Execute(BTContext& context) override;

    private:
        ConditionFunc m_Condition;
    };

    /**
     * @brief Wait node - waits for a specified duration
     */
    class BTWait : public BTNode {
    public:
        BTWait(float duration = 1.0f);

        const char* GetTypeName() const override;

        void OnEnter(BTContext& context) override;

        BTStatus Execute(BTContext& context) override;

        void Reset() override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        float m_Duration;
        float m_ElapsedTime;
    };

    /**
     * @brief Log node - outputs a debug message
     */
    class BTLog : public BTNode {
    public:
        BTLog(const std::string& message = "");

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        std::string m_Message;
        float accumulatedtime;
        float time = 60.0f;
    };

    /**
     * @brief SetBlackboard node - sets a blackboard value with type support
     */
    class BTSetBlackboard : public BTNode {
    public:
        BTSetBlackboard(const std::string& key = "", const std::string& value = "", const std::string& type = "string");

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        std::string m_Key;
        std::string m_Value;
        std::string m_Type;  // "bool", "int", "float", "string", "vec2", "vec3", "vec4", "entity"
    };

    /**
     * @brief CheckBlackboard node - checks if a blackboard value matches (string comparison)
     */
    class BTCheckBlackboard : public BTNode {
    public:
        BTCheckBlackboard(const std::string& key = "", const std::string& expectedValue = "");

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        std::string m_Key;
        std::string m_ExpectedValue;
    };

    /**
     * @brief SetBlackboardInt - convenience node for setting integer values
     */
    class BTSetBlackboardInt : public BTNode {
    public:
        BTSetBlackboardInt(const std::string& key = "", int value = 0);

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Key;
        int m_Value;
    };

    /**
     * @brief SetBlackboardFloat - convenience node for setting float values
     */
    class BTSetBlackboardFloat : public BTNode {
    public:
        BTSetBlackboardFloat(const std::string& key = "", float value = 0.0f);

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Key;
        float m_Value;
    };

    /**
     * @brief SetBlackboardBool - convenience node for setting boolean values
     */
    class BTSetBlackboardBool : public BTNode {
    public:
        BTSetBlackboardBool(const std::string& key = "", bool value = false);

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Key;
        bool m_Value;
    };

    /**
     * @brief SetBlackboardVec3 - convenience node for setting vec3 values
     */
    class BTSetBlackboardVec3 : public BTNode {
    public:
        BTSetBlackboardVec3(const std::string& key = "", const glm::vec3& value = glm::vec3(0.0f));

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Key;
        glm::vec3 m_Value;
    };

    /**
     * @brief Rotates the entity continuously at a specified speed
     * @details Rotates around the Y-axis (yaw rotation)
     */
    class BTRotateEntity : public BTNode {
    public:
        BTRotateEntity(float degreesPerSecond = 90.0f);

        const char* GetTypeName() const override { return "RotateEntity"; }
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        float m_RotationSpeed;  // Degrees per second
    };

    /**
     * @brief Moves entity towards a target position stored in blackboard
     * @details Reads "TargetPosition" (vec3) from blackboard
     */
    class BTMoveToTarget : public BTNode {
    public:
        BTMoveToTarget(float speed = 5.0f, float arrivalDistance = 0.5f);

        const char* GetTypeName() const override { return "MoveToTarget"; }
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        float m_MoveSpeed;
        float m_ArrivalDistance;
        std::string m_TargetPositionKey;  // Blackboard key for target position
    };

    /**
     * @brief Sets a target position in the blackboard
     * @details Helper node to set a waypoint/target position
     */
    class BTSetTargetPosition : public BTNode {
    public:
        BTSetTargetPosition(const glm::vec3& position = glm::vec3(0.0f));

        const char* GetTypeName() const override { return "SetTargetPosition"; }
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        glm::vec3 m_TargetPosition;
        std::string m_BlackboardKey;
    };

    /**
     * @brief Destroys the current entity
     * @details Use with caution - this will destroy the entity running the BT
     */
    class BTDestroySelf : public BTNode {
    public:
        BTDestroySelf() = default;

        const char* GetTypeName() const override { return "DestroySelf"; }
        BTStatus Execute(BTContext& context) override;
    };

    /**
     * @brief Destroys an entity by tag
     */
    class BTDestroyEntityByTag : public BTNode {
    public:
        BTDestroyEntityByTag(const std::string& tag = "");

        const char* GetTypeName() const override { return "DestroyEntityByTag"; }
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Tag;
    };

    /**
     * @brief Checks if health meets a condition
     * @details Reads health from blackboard "Health" key
     */
    class BTCheckHealth : public BTNode {
    public:
        enum class Comparison {
            Greater,
            Less,
            Equal,
            GreaterOrEqual,
            LessOrEqual
        };

        BTCheckHealth(float threshold = 0.0f, Comparison comp = Comparison::Greater);

        const char* GetTypeName() const override { return "CheckHealth"; }
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        float m_Threshold;
        Comparison m_Comparison;
        std::string m_HealthKey;
    };

    /**
     * @brief Sets health value in blackboard
     */
    class BTSetHealth : public BTNode {
    public:
        BTSetHealth(float health = 100.0f);

        const char* GetTypeName() const override { return "SetHealth"; }
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        float m_Health;
        std::string m_HealthKey;
    };

    /**
     * @brief Modifies health value (add or subtract)
     */
    class BTModifyHealth : public BTNode {
    public:
        BTModifyHealth(float amount = 0.0f);

        const char* GetTypeName() const override { return "ModifyHealth"; }
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        float m_Amount;  // Positive to heal, negative to damage
        std::string m_HealthKey;
    };

    /**
     * @brief Rotates entity to face movement direction
     */
    class BTFaceMovementDirection : public BTNode {
    public:
        BTFaceMovementDirection(float rotationSpeed = 180.0f);

        const char* GetTypeName() const override { return "FaceMovementDirection"; }
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        float m_RotationSpeed;
        std::string m_TargetPositionKey;
    };

    /**
     * @brief Rotates entity to face a target entity
     * @details Reads entity pointer from blackboard
     */
    class BTFaceTarget : public BTNode {
    public:
        BTFaceTarget(float rotationSpeed = 180.0f);

        const char* GetTypeName() const override { return "FaceTarget"; }
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        float m_RotationSpeed;
        std::string m_TargetEntityKey;
    };

    /**
     * @brief Counts entities with specific tag
     */
    class BTCheckEntityCount : public BTNode {
    public:
        enum class Comparison {
            Greater,
            Less,
            Equal,
            GreaterOrEqual,
            LessOrEqual
        };

        BTCheckEntityCount(const std::string& tag = "", int targetCount = 0,
            Comparison comp = Comparison::Equal);

        const char* GetTypeName() const override { return "CheckEntityCount"; }
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Tag;
        int m_TargetCount;
        Comparison m_Comparison;
    };

    /**
     * @brief Stores entity count in blackboard
     */
    class BTStoreEntityCount : public BTNode {
    public:
        BTStoreEntityCount(const std::string& tag = "", const std::string& countKey = "EntityCount");

        const char* GetTypeName() const override { return "StoreEntityCount"; }
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Tag;
        std::string m_CountKey;
    };

    /**
     * @brief Changes the color/material of an object over time
     * @details Cycles between materials at specified intervals
     * Supports Changing material ID (for existing materials 0=blue, 1=orange)
     */
    class BTChangeColor : public BTNode {
    public:
        /**
         * @brief Constructor
         * @param materialID Material ID to set (0 = blue, 1 = orange)
         */
        BTChangeColor(u32 materialID = 0);

        const char* GetTypeName() const override;

        void OnEnter(BTContext& context) override;
        BTStatus Execute(BTContext& context) override;
        void Reset() override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        // Allow direct property access for registration macros
        u32 m_MaterialID;
        float m_ChangeInterval = 1.0f;  // Time between color changes
        float m_ElapsedTime = 0.0f;     // Accumulated time
    };


    /**
     * @brief Orbits around a center point without changing rotation
     * @details Moves in a circular path around a center point on the XZ plane.
     *          Entity rotation is not modified - it maintains its current facing direction.
     */
    class BTOrbitAroundPoint : public BTNode {
    public:
        BTOrbitAroundPoint(float orbitRadius = 5.0f,
            float orbitSpeed = 90.0f,
            const glm::vec3& centerPoint = glm::vec3(0.0f));

        const char* GetTypeName() const override { return "OrbitAroundPoint"; }
        void OnEnter(BTContext& context) override;
        BTStatus Execute(BTContext& context) override;
        void Reset() override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        float m_OrbitRadius;          // Radius of the circular orbit
        float m_OrbitSpeed;           // Degrees per second around the circle
        glm::vec3 m_CenterPoint;      // Center point of the orbit
        float m_CurrentAngle;         // Current angle in the orbit (in degrees)
        std::string m_CenterPointKey; // Blackboard key for dynamic center point
    };

    class BTRotateAxis : public BTNode {
    public:
        // axis should be a unit-like direction (ex: (0,1,0) or (0,0,1))
        // degPerSec = rotation speed in degrees per second
        BTRotateAxis(glm::vec3 axis = glm::vec3(0, 0, 1), float degPerSec = 30.0f);

        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        // Editor / JSON support
        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;

        void SetProperty(const std::string& name, const std::string& value) override;

        glm::vec3 m_Axis;
        float m_DegPerSec;
    };

    class BTLookAtSmooth : public BTNode {
    public:
        BTLookAtSmooth(std::string targetKey = "TargetPoint", float turnSpeedDeg = 120.0f);

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& p) const override;

        void SetProperty(const std::string& n, const std::string& v) override;

        std::string m_Key;
        float m_TurnSpeed; // radians per second
    };

    /**
     * @brief Loads waypoints from script into blackboard
     * @details Finds entities with tag that contains the waypoint key
     * Sets up the waypoint array and initializes current index to 0
     */
    class BTCountLoadWaypoints : public BTNode {
    public:
        BTCountLoadWaypoints(std::string waypointKey = "", std::string countKey = "Count");

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_WaypointKey;
        std::string m_CountKey;
    };

    /**
     * @brief Gets the next waypoint from the array
     * @details Reads current index, fetches waypoint, outputs to target key
     * Returns Success if waypoint retrieved, Failure if no more waypoints
     */
    class BTGetNextWaypoint : public BTNode {
    public:
        BTGetNextWaypoint(const std::string& waypointKey = "", const std::string& countKey = "Count",
            const std::string& targetKey = "TargetPosition");

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_WaypointKey;
        std::string m_CountKey;
        std::string m_TargetKey;
    };

    /**
     * @brief Increments the waypoint index
     * @details Moves to next waypoint in the array
     * Returns Success if incremented, Failure if at end of array
     */
    class BTIncrementWaypointIndex : public BTNode {
    public:
        BTIncrementWaypointIndex() = default;

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;
    };

    /**
     * @brief Checks if waypoint has been reached
     * @details Compares entity position to target waypoint position
     * Returns Success if within arrival distance, Running otherwise
     */
    class BTCheckWaypointReached : public BTNode {
    public:
        BTCheckWaypointReached(const std::string& targetKey = "", float arrivalDistance = 1.0f);

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_TargetKey;
        float m_ArrivalDistance;
    };

    class BTCheckVisitAllWaypoints : public BTNode {
    public:
        BTCheckVisitAllWaypoints(const std::string& countKey = "Count");

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;
        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

		std::string m_CountKey;
    };

    //linear velocity - move in a singular directoin
    //dampening

    class BTApplyLinearVelocity : public BTNode {
    public:
        BTApplyLinearVelocity(glm::vec3 linearVelocity = glm::vec3(0));
        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;
        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        glm::vec3 m_linearVelocity;
    };

    class BTApplyLinearDampening : public BTNode {

    public:

        BTApplyLinearDampening(float dampening = 0.0f);
		const char* GetTypeName() const override;

		BTStatus Execute(BTContext& context) override;
		void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
		void SetProperty(const std::string& name, const std::string& value) override;

        float m_Dampening;
    };


} // namespace Engine
