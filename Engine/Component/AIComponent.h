#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <any>
#include <vector>

#include <variant>
#include <memory>

namespace Engine {

	//forward declarations
	class BehaviourTree;
	class BehaviourNode;

	/**
	* @brief Types that can be stored in the blackboard
	*/

	using BlackboardValue = std::variant<
		bool,
		int,
		float,
		glm::vec3,
		std::string,
		entt::entity //for storing target entities
	>;

	/**
	* @brief Blackboard - shared memory for AI to store and retrieve data
	* @details - Each AI entity has its own blackboard instance
	*/

	using Blackboard = std::unordered_map<std::string, BlackboardValue>;

	/**
	* @brief AI Component - Holds Behaviour Tree and execution state
	* @details - Attached to entities that need AI behaviour
	*/

	struct AIComponent {

		//FOR BEHAVIOUR TREE
		std::string TreeAssetPath;				//path to behaviour tree
		BehaviourTree* Tree = nullptr;			//pointer to the loaded tree
		BehaviourNode* CurrentNode = nullptr;	//currently executing node

		//BLACKBOARD
		Blackboard Data;						//shared memory for this AI

		//STATE TRACKING
		bool Active = true;
		float TickRate = 0.0f;
		float TimeSinceLastTick = 0.0f;

		//DEBUGGING
		bool DebugDraw = false;
		std::string CurrentState = "Idle";

		//DEFAULT CONSTRUCTOR
		AIComponent()
			: TreeAssetPath("")
			, Tree(nullptr)
			, CurrentNode(nullptr)
			, Active(true)
			, TickRate(0.0f)
			, TimeSinceLastTick(0.0f)
			, DebugDraw(false)
			, CurrentState("Idle") {
		}

		//CONSTRUCTOR WITH TREE PATH
		explicit AIComponent(const std::string& treePath)
			: TreeAssetPath(treePath)
			, Tree(nullptr)
			, CurrentNode(nullptr)
			, Active(true)
			, TickRate(0.0f)
			, TimeSinceLastTick(0.0f)
			, DebugDraw(false)
			, CurrentState("Idle") {
		}

		/**
		* @brief Helper to get blackboard value
		*/
		template<typename T>
		void SetBlackBoardValue(const std::string& key, const T& value) {
			Data[key] = value;
		}

		/**
		* @brief Helper to get a  blackboard value
		* @return Pointer to value if exists, else nullptr
		*/

		template<typename T>
		T* GetBlackboardValue(const std::string& key) {
			auto it = Data.find(key);
			if (it != Data.end()) {
				return std::_Get_difference_type<T>(&it->second);
			}
			return nullptr;
		}

		/**
		* @brief Check if blackboard has a key
		*/
		bool HasBlackboardKey(const std::string& key) const {
			return Data.find(key) != Data.end();
		}

	};

	/**
	* @brief Payload Component - For enemies that can explode
	* @details Used by enemies like "LoveLetter" that deal damage based on Payload
	*/

	struct PayloadComponent {
		float CurrentPayload = 0.0f;
		float MaxPayload = 100.0f;
		float DamageMultiplier = 1.0f;
		float ExplosionRadius = 5.0f;
		bool WillExplodeOnCoreCollision = true;

		PayloadComponent()
			: CurrentPayload(0.0f)
			, MaxPayload(100.0f)
			, DamageMultiplier(1.0f)
			, ExplosionRadius(5.0f)
			, WillExplodeOnCoreCollision(true) {
		}
	};

} //end of namespace Engine