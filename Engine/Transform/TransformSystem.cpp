
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "../Transform/TransformSystem.h"
#include "../ECS/Scene.h"

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
				glm::mat4 rotation_matrix    = glm::toMat4(transform.Rotation);
				glm::mat4 scale_matrix		 = glm::scale(glm::mat4(1.0f), transform.Scale);

				glm::mat4 transformation_matrix = translation_matrix * rotation_matrix * scale_matrix;

				transform.WorldTransform = transform.LocalTransform = transformation_matrix;
				transform.IsDirty = false;
			}

			propagate(scene, root);
		}

	}

	int TransformSystem::GetPriority() const { return 30; }

	const char* TransformSystem::GetName() const { return "TransformSystem"; }

	void TransformSystem::propagate(Scene* scene, entt::entity root) {

		std::stack<entt::entity> st;
		st.push(root);

		// Propagate through the children transformations
		while (!st.empty()) {

			entt::entity e = st.top(); st.pop();
			auto& parent = scene->GetRegistry().get<TransformComponent>(e);

			glm::mat4 parent_matrix = parent.WorldTransform;

			// ce stands for child entity
			for (auto ce : parent.Children) {

				auto& child = scene->GetRegistry().get<TransformComponent>(ce);

				if (child.IsDirty || parent.IsDirty) {

					// Re-Compute transformation matrix for child
					glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), child.Position);
					glm::mat4 rotation_matrix    = glm::toMat4(child.Rotation);
					glm::mat4 scale_matrix	     = glm::scale(glm::mat4(1.0f), child.Scale);

					glm::mat4 transformation_matrix = translation_matrix * rotation_matrix * scale_matrix;

					child.LocalTransform = transformation_matrix;
					child.WorldTransform = parent_matrix * child.LocalTransform;
					child.IsDirty = false;
				}

				st.push(ce);
			}

		}
	}
}