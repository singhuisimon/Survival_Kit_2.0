#pragma once

#include "ECS/Scene.h"
#include "ECS/System.h"
#include "ECS/Entity.h"
#include "AIManager.h"
#include "Component/AIComponent.h"

namespace Engine
{

	class AISystem : public System {

	public:

		/**
		* @brief constructor
		* @param aiManager Pointer to the global AI Manager
		*/
		explicit AISystem(AIManager* aiManager);
		~AISystem();

		//SYSTEM INTERFACE

		void onInit(Scene* scene);
		void onUpdate(Scene* scene, Timestep ts);
		void onShutdown(Scene* scene);

		int getPriority() const { return 50; }
		const char* getName() const { return "AISystem"; }

		//CONFIGURATION

		/**
		* @brief Set global AI tick rate
		* @param tickRate Seconds between ticks (0 = every frame)
		*/
		void SetGlobalTickRate(float tickRate) { m_GlobalTickRate = tickRate;  }

		/**
		* @brief Get global AI tick rate
		*/
		float GetGlobalTickRate() const { return m_GlobalTickRate; }

		/**
		* @brief Enable/disable debug drawing for all AI
		*/
		void SetDebugDrawAll(bool enable) { m_DebugDrawAll = enable; }

	private:

		AIManager* m_AIManager;
		bool m_Initialized = false;

		//performance settings
		float m_GlobalTickRate = 0.0f;  // 0 = tick every frame
		bool m_DebugDrawAll = false;

		/**
		* @brief Process all AI Entities
		*/
		void ProcessAIEntities(Scene* scene, float deltaTime);

		/**
		 * @brief Ensure AI component has a loaded behaviour tree
		 * @param entity The entity with AIComponent
		 * @param ai The AI component
		 * @return True if tree is ready to tick
		 */
		//bool EnsureBehaviourTreeLoaded(Entity entity, AIComponent& ai);

		/**
		  * @brief Tick an AI's behaviour tree
		  * @param entity The entity
		  * @param ai The AI component
		  * @param transform Optional transform component
		  */
		//void TickBehaviourTree(Entity entity, AIComponent& ai, TransformComponent* transform);

		/**
		  * @brief Check if AI should tick this frame
		  * @param ai The AI component
		  * @param deltaTime Time since last frame
		  * @return True if should tick
		  */
		bool ShouldTick(AIComponent& ai, float deltaTime);

		/**
		 * @brief Cleanup AI component when entity is destroyed
		 */
		void OnAIComponentRemoved(entt::registry& registry, entt::entity entity);

		/**
		 * @brief Initialize common blackboard values
		 * @param entity The entity
		 * @param ai The AI component
		 */
		void InitializeBlackboard(Entity entity, AIComponent& ai);
	};

}	//end of namespace Engine