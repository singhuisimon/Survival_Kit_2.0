#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "../Transform/TransformSystem.h"
#include "../ECS/Scene.h"
#include "../Component/TransformComponent.h"

#include <stack>

namespace Engine {

	void TransformSystem::OnUpdate(Scene* scene, Timestep ts) {

		auto view = scene->GetRegistry().view<TransformComponent>();

		std::vector<entt::entity> roots;
		roots.reserve(view.size());

		for (auto entity : view) {

			auto& transform = view.get<TransformComponent>(entity);

			// Check if this transformation is a root. If transformation is a root, it will have no parents.
			if (transform.Parent == entt::null) {
				roots.push_back(entity);
			}
		}

		// Iterate through all the roots
		for (auto root : roots) {
			auto& transform = view.get<TransformComponent>(root);

			if (transform.IsDirty) {

				// Compute transformation for roots -> since roots have no parents, local transform == world transform
				glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), transform.Position);
				glm::mat4 rotation_matrix = glm::toMat4(transform.Rotation);
				glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), transform.Scale);

				glm::mat4 transformation_matrix = translation_matrix * rotation_matrix * scale_matrix;

				transform.WorldTransform = transform.LocalTransform = transformation_matrix;
				transform.IsDirty = false;
			}

			propagate(scene, root);
		}

		(void)ts;
	}

	void TransformSystem::propagate(Scene* scene, entt::entity entity) {

		auto& registry = scene->GetRegistry();

		std::stack<entt::entity> process_stack;
		process_stack.push(entity);

		while (!process_stack.empty()) {
			entt::entity current_entity = process_stack.top();
			process_stack.pop();

			auto& current_transform = registry.get<TransformComponent>(current_entity);

			// for each of its children, apply propagate transformation
			for (auto ce : current_transform.Children) {

				auto& child = registry.get<TransformComponent>(ce);

				glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), child.Position);
				glm::mat4 rotation_matrix = glm::toMat4(child.Rotation);
				glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), child.Scale);

				glm::mat4 transformation_matrix = translation_matrix * rotation_matrix * scale_matrix;

				// Computes the world transformation using the transformation matrix of the parent
				child.WorldTransform = current_transform.WorldTransform * transformation_matrix;
				child.LocalTransform = transformation_matrix;

				// Add child for propagation
				process_stack.push(ce);
			}
		}
	}

	int TransformSystem::GetPriority() const { return 1; }

	const char* TransformSystem::GetName() const { return "TransformSystem"; }
}