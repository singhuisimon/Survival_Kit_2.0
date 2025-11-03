#pragma once
#include "../ECS/System.h"
#include "../ECS/Components.h"
#include <entt/entt.hpp>

namespace Engine {

	class TransformSystem : public System {
	public:
		void OnUpdate(Scene* scene, Timestep ts) override;
		int GetPriority() const override;
		const char* GetName() const override;

		static void SetParent(Scene* scene, entt::entity child, entt::entity new_parent);
		static void UnParent(Scene* scene, entt::entity child);

	private:

		/**
		 * @brief Propagate transformations from parent to children.
		 * @param root is the root transformation to propagate from.
		 */
		void propagate(Scene* scene, entt::entity root);
	};

}