#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#include "../Transform/TransformSystem.h"
#include "../ECS/Scene.h"
#include "../Component/TransformComponent.h"

#include <stack>

#include "glm/gtx/matrix_decompose.hpp"

namespace Engine {

	void TransformSystem::OnUpdate(Scene* scene, Timestep ts) {

		auto view = scene->GetRegistry().view<TransformComponent>();

		std::vector<entt::entity> roots;
		roots.reserve(view.size());

		for (auto entity : view) {

			auto& transform = view.get<TransformComponent>(entity);

			// Check if this transformation is a root. If transformation is a root, it will have no parents.
			if (transform.Parent == u32_max) {
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

	// Set parent of an entity
	void TransformSystem::SetParent(Scene* scene, entt::entity child, entt::entity new_parent) {

		auto view = scene->GetRegistry().view<TransformComponent>();
		auto& child_transform = view.get<TransformComponent>(child);

		// Store current world transform before changing parent
		glm::mat4 child_world = child_transform.WorldTransform;

		// If the child is currently parented to another entity, remove it from that parent's children list
		if (child_transform.Parent != u32_max) {
			// Get the children list of the old parent
			auto& old_parent_transform = view.get<TransformComponent>(static_cast<entt::entity>(child_transform.Parent));
			auto& old_children = old_parent_transform.Children;
			std::erase(old_children, static_cast<u32>(child));
		}

		child_transform.Parent = static_cast<u32>(new_parent);

		if (child_transform.Parent != u32_max) {
			// Add child to new parent's children list
			auto& parent_transform = view.get<TransformComponent>(new_parent);
			parent_transform.Children.push_back(static_cast<u32>(child));

			// Convert world transform to local space relative to new parent
			glm::mat4 parent_world_inverse = glm::inverse(parent_transform.WorldTransform);
			glm::mat4 new_local_transform  = parent_world_inverse * child_world;

			// Decompose the local transform back to Position/Rotation/Scale
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(new_local_transform, child_transform.Scale, child_transform.Rotation,
				child_transform.Position, skew, perspective);

			parent_transform.IsDirty = true;
			child_transform.IsDirty  = true;
		} else {
			// Unparenting: convert local to world
			glm::vec3 skew;
			glm::vec4 perspective;
			glm::decompose(child_world, child_transform.Scale, child_transform.Rotation,
				child_transform.Position, skew, perspective);

			child_transform.IsDirty = true;
		}
	}

	void TransformSystem::UnParent(Scene* scene, entt::entity child) {
		SetParent(scene, child, entt::null);
	}

	void TransformSystem::propagate(Scene* scene, entt::entity entity) {

		auto& registry = scene->GetRegistry();

		// Use stack to avoid recursion
		std::stack<entt::entity> process_stack;
		process_stack.push(entity);

		while (!process_stack.empty()) {
			entt::entity current_entity = process_stack.top();
			process_stack.pop();

			// verify if current entity exists and has transform component
			if (!registry.any_of<TransformComponent>(current_entity))
			{
				continue;
			}

			auto& current_transform = registry.get<TransformComponent>(current_entity);

			// for each of its children, apply propagate transformation
			for (auto ce : current_transform.Children) {

				entt::entity childEnt = static_cast<entt::entity>(ce);
				if (!registry.any_of<TransformComponent>(childEnt))
				{
					continue;
				}
				auto& child = registry.get<TransformComponent>(static_cast<entt::entity>(ce));
				glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), child.Position);
				glm::mat4 rotation_matrix = glm::toMat4(child.Rotation);
				glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), child.Scale);

				glm::mat4 transformation_matrix = translation_matrix * rotation_matrix * scale_matrix;

				// Computes the world transformation using the transformation matrix of the parent
				child.WorldTransform = current_transform.WorldTransform * transformation_matrix;
				child.LocalTransform = transformation_matrix;

				// Add child for propagation
				process_stack.push(static_cast<entt::entity>(ce));
			}
		}
	}

	int TransformSystem::GetPriority() const { return 51; }

	const char* TransformSystem::GetName() const { return "TransformSystem"; }
}
