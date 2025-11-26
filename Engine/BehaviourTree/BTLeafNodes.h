/**
 * @file BTLeafNodes.h
 * @brief Leaf behaviour tree nodes (Actions and Conditions)
 * @author Amanda Leow Boon Suan (90%), Rio Shannon Yvon Leonardo (10%)
 * @date 3/11/2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */
#pragma once

#include "BTNode.h"
#include <functional>
#include "Utility/Types.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

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
        //float accumulatedtime;
        //float time = 60.0f;
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

    //ERRORS IN THIS NEED TO DOUBLE CHECK WILL BLANK IT FOR NOW
    /*class BTApplyLinearDampening : public BTNode {

    public:

        BTApplyLinearDampening(float dampening = 0.0f);
		const char* GetTypeName() const override;

		BTStatus Execute(BTContext& context) override;
		void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
		void SetProperty(const std::string& name, const std::string& value) override;

        float m_Dampening;
    };*/

	//Change this to create enemy of specific tag/kind e.g. loveletter, botnets, trojan, etc
    //Add vec3 spawn pos in the future
    class BTCreateShootableEnemy : public BTNode {
    public:
        BTCreateShootableEnemy(const std::string& tag = "");
        const char* GetTypeName() const override;

        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_Tag;
    };

    class BTCheckCollision : public BTNode {
    public:
        BTCheckCollision(const std::string& againstTag = "", bool destroyOnHit = false);

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_OtherTag;
        bool m_DestroyOnHit;
		std::string m_CollidedIDKey;
    };

    class BTDeleteCollidedEntity : public BTNode {
    public:
        BTDeleteCollidedEntity(const std::string& collidedKey = "");
        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_CollidedIDKey;

    };
    

    /**
     * @brief Spawns a configurable number of enemies at the spawner's position
     * @details Simple spawn node that creates N enemies with specified tag and prefab
     */
    class BTSpawnEnemies : public BTNode {
    public:
        BTSpawnEnemies(int count = 1, const std::string& enemyTag = "Enemy");

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        int m_SpawnCount;           // How many enemies to spawn
        std::string m_EnemyTag;     // Tag for spawned enemies
    };

    /**
     * @brief Spawns enemies in a grid pattern around the spawner
     * @details Creates enemies with spacing for visual clarity
     */
    class BTSpawnEnemiesGrid : public BTNode {
    public:
        BTSpawnEnemiesGrid(
            int count = 1,
            float spacing = 2.0f,
            const std::string& enemyTag = "Enemy"
        );

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        int m_SpawnCount;
        float m_Spacing;            // Distance between spawned enemies
        std::string m_EnemyTag;
    };

    /**
     * @brief Spawns an enemy at a specific world position
     * @details For enemies like "loveletter" that spawn at fixed locations
     */
    class BTSpawnEnemyAt : public BTNode {
    public:
        BTSpawnEnemyAt(
            const std::string& enemyTag = "Enemy",
            const glm::vec3& position = glm::vec3(0.0f)
        );

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_EnemyTag;
        glm::vec3 m_SpawnPosition;  // World position to spawn at
    };

    /**
     * @brief Spawns multiple enemy types with individual counts
     * @details Configure different quantities for each enemy type:
     *          - loveletter (spawns at specific point)
     *          - trojan (sometimes)
     *          - adware (sometimes)
     *          - worms (sometimes)
     *          - botnet (always, variable count)
     */
    class BTSpawnMultipleTypes : public BTNode {
    public:
        BTSpawnMultipleTypes();

        const char* GetTypeName() const override;
		void OnEnter(BTContext& context) override;
        BTStatus Execute(BTContext& context) override;
        void Reset() override;

        void UpdateCooldowns(float deltaTime);  // Update spawn point cooldowns

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        enum class EnemyType{
			LOVELETTER,
			TROJAN,
			ADWARE,
			WORMS,
			BOTNET
        };

        // Individual counts for each enemy type (0 = don't spawn)
        int m_LoveletterCount;
        int m_TrojanCount;
        int m_AdwareCount;
        int m_WormsCount;
        int m_BotnetCount;

        //float m_Spacing;            // Grid spacing for spawned enemies
        //glm::vec3 m_LoveletterPos;  // Specific spawn position for loveletter

        bool m_WallA = false;
		bool m_WallB = false;
		bool m_WallC = false;
		bool m_WallD = false;
		bool m_WallE = false;
        bool m_Boss = false;

        int m_SpawnPointCountA;
		int m_SpawnPointCountB;
		int m_SpawnPointCountC;
		int m_SpawnPointCountD;
		int m_SpawnPointCountE;

    private:
        struct SpawnPoint {
            std::string Tag;
            glm::vec3 Position;
            float CooldownTimer = 0.0f;       // counts down from 10 seconds
            bool IsAvailable = true;
        };

        struct WallInfo {
            std::string name;
            glm::vec3 position;
            glm::quat rotation;
			glm::vec3 scale;
			glm::vec3 direction;
			int spawnPointCount;
            std::vector<SpawnPoint> spawnPoints;  // actual spawn point entities (A1, A2, etc.)
		};

        std::unordered_map<std::string, WallInfo> m_Walls;

		std::vector<WallInfo> m_EnabledWalls;

		int m_totalSpawned = 0;

        //glm::quat LookRotation(const glm::vec3& forward, const glm::vec3& up = glm::vec3(0, 1, 0));

        glm::quat LookRotation(const glm::vec3& forward, const glm::vec3& up = glm::vec3(0, 1, 0)) {
            glm::vec3 f = glm::normalize(forward);
            glm::vec3 r = glm::normalize(glm::cross(up, f));
            glm::vec3 u = glm::cross(f, r);

            glm::mat3 rotMat(r, u, f);
            return glm::quat_cast(rotMat);
        }


    };



    /*
*   @brief Finds the nearest enemy entity within range
*   @details Searches for entities with a specified tag and stores position + entity in blackboard
*
*/

    class BTFindNearestEnemy : public BTNode {
    public:
        BTFindNearestEnemy(const std::string& enemyTag = "Enemy",
            const std::string& targetPosKey = "TargetPosition",
            const std::string& targetEntityKey = "TargetEnemy",
            float maxRange = 1000.0f);

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_EnemyTag;             // Tag to search for
        std::string m_TargetPosKey;         // Blackboard key to store target position (vec3)
        std::string m_TargetEntityKey;      // Blackboard key to store target entity
        float m_MaxRange;                   // Maximum search range
    };

    /*
    *   @brief Shoots a bullet at target with cooldown timer
    *   @details creates bullet entity and tracks firing cooldown
    *
    */

    class BTShootBullet : public BTNode {
    public:
        BTShootBullet(const std::string& bulletTag = "Bullet",
            float fireRate = 0.1f,
            float bulletSpeed = 3000.0f);

        const char* GetTypeName() const override;
        void OnEnter(BTContext& context) override;
        BTStatus Execute(BTContext& context) override;
        void Reset() override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_BulletTag;            // Tag for bullet entity
        float m_FireRate;                   // Time between shots (seconds)
        float m_BulletSpeed;                // Bullet speed
        float m_Cooldown;                   // Current cooldown timer
    };

    /*
    *   @brief Checks if entity has a valid target in blackboard
    *   @details Returns success if target exists and is valid, Failure otherwise
    *
    */

    class BTHasValidTarget : public BTNode {
    public:
        BTHasValidTarget(const std::string& targetEntityKey = "TargetEnemy");

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;

        std::string m_TargetEntityKey;      // Blackboard key to check
    };












    class BTCreateEntityByPrefab : public BTNode {
    public:
        BTCreateEntityByPrefab(const std::string& prefabName = "");

        const char* GetTypeName() const override;
        BTStatus Execute(BTContext& context) override;

        void GetProperties(std::vector<std::pair<std::string, std::string>>& properties) const override;
        void SetProperty(const std::string& name, const std::string& value) override;
		
        std::string m_PrefabName;
    };

} // namespace Engine
