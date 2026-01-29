#pragma once
#include "../ECS/System.h"
#include "../ECS/Components.h"

namespace Engine
{

	class ScriptSystem : public System
	{
	public:
		ScriptSystem() = default;

		void OnInit(Scene *scene) override;
		void OnUpdate(Scene *scene, Timestep ts) override;
		void OnShutdown(Scene *scene) override;

		int GetPriority() const override
		{
			return 50;
		}
		const char *GetName() const override
		{
			return "ScriptSystem";
		}

	private:
		Scene *m_Scene = nullptr;
		bool m_IsShuttingDown = false;
		float m_FixedAccumulator = 0.0f;
		float m_FixedDeltaSeconds = 1.0f / 60.0f;
		int   m_MaxFixedSubsteps = 8;
	};

} // namespace Engine
