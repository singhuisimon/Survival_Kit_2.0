#pragma once
#include <entt/entt.hpp>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <any>
#include <vector>

namespace Engine {

	//forward declarations
	class BehaviourTree;
	class BehaviourNode;
	class Blackboard;

	/**
	* @brief Types of AI enemies in the game
	*/

	enum class AIType {
		NONE,
		LOVELETTER,
		TROJAN,
		ADWARE,
		//adding more enemy types as needed
	};

	/**
	* @brief Main AI component that drives behaviour
	*/

	struct AIComponent {

		//What type of AI is this?s
		AIType Type = AIType::NONE;

		BehaviourTree* Tree = nullptr;

		float TickInterval = 0.1f;

		float TimeSinceLastTick = 0.01f;

		//Is this AI Active?
		bool IsActive = true;

		//For debugging
		std::string CurrentStateName = "Idle";

	};

	/**
	* @brief Local memory storage for each AI entity
	* Acts like the AI's "brain" that remembers everything
	*/
	struct BlackboardComponent {

		entt::entity CurrentTarget = entt::null;

		glm::vec3 LastKnownTargetPosition = glm::vec3(0.0f);

		float TimeSinceTargetSeen = 0.0f;

		std::vector<entt::entity> ChildPayLoads;
		int ActivePayloadCount = 0;

		std::unordered_map<std::string, std::any> DataMap;

		template<typename T>
		void SetValue(const std::string& key, const T& value) {
			DataMap[key] = value;
		}

		//helper function to get data
		template<typename T>
		T GetValue(const std::string& key, const T& defaultValue = T{}) {
			auto it = DataMap.find(key);
			if (it != DataMap.end()) {
				try {
					return std::any_cast<T>(it->second);
				}
				catch (...) {
					return defaultValue;
				}
			}

			return defaultValue;
		}

		//check if there is a value
		bool HasValue(const std::string& key) const {
			return DataMap.find(key) != DataMap.end();
		}
	};

	/**
	* @brief For payload entities that belong to a parent (Like LoveLetter's payload
	*/
	struct PayloadComponent {
		entt::entity ParentEntity = entt::null;
		int PayLoadIndex = 0;
	};

} //end of namespace Engine