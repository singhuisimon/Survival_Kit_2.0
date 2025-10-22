#pragma once
#include "ECS/System.h"
#include <memory>

namespace Engine
{
	//forward declarations
	class BehaviourTree;

	/**
		@brief System that updates all AI entities

		This system:
		- Updates AI behaviour trees
		- Manages AI tick rates (not all AI needs to think every frame)
		- Handles communication between parent-child entities (Like LoveLetter and payloads
	*/

	class AISystem : public System {

	public:
		AISystem();
		~AISystem() override;

		void OnInit(Scene* scene)  override;
		void OnUpdate(Scene* scene, Timestep ts) override;
		void OnShutdown(Scene* scene) override;

		//Priority
		int GetPriority() const override { return 50; }
		const char* GetName() const override { return "AISystem"; }

	private:
		//initialize behaviour trees for different enemy types
		void SetupBehaviourTrees();

		//create specific enemy trees
		std::shared_ptr<BehaviourTree> CreateLoveLetterTree();
		std::shared_ptr<BehaviourTree> CreateTrojanTree();
		std::shared_ptr<BehaviourTree> CreateAdwareTree();

		//handle payload-parent communication
		void ProcessPayloadCommunication(Scene* scene);

		//storage for behaviour trees (shared between entities of same type)
		std::shared_ptr<BehaviourTree> m_TestTree;
		std::shared_ptr<BehaviourTree> m_LoveLetterTree;
		std::shared_ptr<BehaviourTree> m_TrojjanTree;
		std::shared_ptr<BehaviourTree> m_AdwareTree;

		bool m_Initialized = false;

	};

}	//end of namespace Engine