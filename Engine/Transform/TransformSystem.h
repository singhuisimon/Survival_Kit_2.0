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
		
		static void SetWorldPosition(Scene* scene, entt::entity const entity, glm::vec3 const& position);
		static void SetWorldRotation(Scene* scene, entt::entity const entity, glm::quat const& rotation);
		static void SetWorldScale(Scene* scene, entt::entity const entity, glm::vec3 const& scale);

	private:

		/**
		 * @brief Propagate transformations from parent to children.
		 * @param root is the root transformation to propagate from.
		 */
		void propagate(Scene* scene, entt::entity root);

		static void FlushEntity(Scene* scene, entt::entity entity);
	};

}