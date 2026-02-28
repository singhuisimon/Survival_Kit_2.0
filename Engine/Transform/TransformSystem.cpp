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

				transform.WorldPosition = transform.Position;
				transform.WorldRotation = transform.Rotation;
				transform.WorldScale	= transform.Scale;

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

	void TransformSystem::SetWorldPosition(Scene* scene, entt::entity const entity, glm::vec3 const& position) {
		auto view = scene->GetRegistry().view<TransformComponent>();
		TransformComponent& transform = view.get<TransformComponent>(entity);

		if (transform.HasParent()) {
			TransformComponent& parent_transform = view.get<TransformComponent>(static_cast<entt::entity>(transform.GetParentEntity()));
			glm::mat4 inverse_world = glm::inverse(parent_transform.WorldTransform);
			glm::vec4 local_position = inverse_world * glm::vec4(position, 1.f);
			transform.Position = glm::vec3(local_position);
		}
		else {
			// Since this is a root transformation, it's Position is considered the world transform
			transform.Position = position;
		}
	
		transform.IsDirty = true;
	}

	void TransformSystem::SetWorldRotation(Scene* scene, entt::entity const entity, glm::quat const& rotation) {
		auto view = scene->GetRegistry().view<TransformComponent>();
		TransformComponent& transform = view.get<TransformComponent>(entity);

		if (transform.HasParent())
		{
			entt::entity parentEnt = static_cast<entt::entity>(transform.GetParentEntity());

			// Ensure parent WorldTransform is up to date before inverting
			FlushEntity(scene, parentEnt);

			TransformComponent& parent_transform = view.get<TransformComponent>(parentEnt);
			glm::quat parent_world_rotation = parent_transform.WorldRotation;
			transform.Rotation = glm::inverse(parent_world_rotation) * rotation;
		}
		else
		{
			transform.Rotation = rotation;
		}

		transform.IsDirty = true;
	}

	void TransformSystem::SetWorldScale(Scene* scene, entt::entity const entity, glm::vec3 const& scale) {
		auto view = scene->GetRegistry().view<TransformComponent>();
		TransformComponent& transform = view.get<TransformComponent>(entity);

		if (transform.HasParent()) {
			TransformComponent& parent_transform = view.get<TransformComponent>(static_cast<entt::entity>(transform.GetParentEntity()));

			// Extract the parent's world scale from the column vectors of its world matrix
			glm::vec3 parent_world_scale;
			parent_world_scale.x = glm::length(glm::vec3(parent_transform.WorldTransform[0]));
			parent_world_scale.y = glm::length(glm::vec3(parent_transform.WorldTransform[1]));
			parent_world_scale.z = glm::length(glm::vec3(parent_transform.WorldTransform[2]));

			// Divide the desired world scale by the parent's world scale to get the local scale
			transform.Scale = scale / parent_world_scale;
		}
		else {
			// Since this is a root transformation, its Scale is considered the world scale
			transform.Scale = scale;
		}

		transform.IsDirty = true;
	}

	void TransformSystem::propagate(Scene* scene, entt::entity const entity) {

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

				child.WorldPosition = glm::vec3(child.WorldTransform[3]);
				child.WorldRotation = current_transform.WorldRotation * child.Rotation;
				child.WorldScale.x = glm::length(glm::vec3(child.WorldTransform[0]));
				child.WorldScale.y = glm::length(glm::vec3(child.WorldTransform[1]));
				child.WorldScale.z = glm::length(glm::vec3(child.WorldTransform[2]));

				// Add child for propagation
				process_stack.push(static_cast<entt::entity>(ce));
			}
		}
	}

	void TransformSystem::FlushEntity(Scene* scene, entt::entity entity)
	{
		auto view = scene->GetRegistry().view<TransformComponent>();
		TransformComponent& t = view.get<TransformComponent>(entity);

		if (!t.IsDirty) return;

		if (t.HasParent())
		{
			// Recursively flush parent first
			FlushEntity(scene, static_cast<entt::entity>(t.GetParentEntity()));

			TransformComponent& parent = view.get<TransformComponent>(static_cast<entt::entity>(t.GetParentEntity()));

			glm::mat4 local = glm::translate(glm::mat4(1.f), t.Position)
				* glm::toMat4(t.Rotation)
				* glm::scale(glm::mat4(1.f), t.Scale);

			t.WorldTransform = parent.WorldTransform * local;
			t.LocalTransform = local;
			t.WorldPosition = glm::vec3(t.WorldTransform[3]);
			t.WorldRotation = parent.WorldRotation * t.Rotation;
			t.WorldScale.x = glm::length(glm::vec3(t.WorldTransform[0]));
			t.WorldScale.y = glm::length(glm::vec3(t.WorldTransform[1]));
			t.WorldScale.z = glm::length(glm::vec3(t.WorldTransform[2]));
		}
		else
		{
			glm::mat4 local = glm::translate(glm::mat4(1.f), t.Position)
				* glm::toMat4(t.Rotation)
				* glm::scale(glm::mat4(1.f), t.Scale);

			t.WorldTransform = t.LocalTransform = local;
			t.WorldPosition = t.Position;
			t.WorldRotation = t.Rotation;
			t.WorldScale = t.Scale;
		}

		t.IsDirty = false;
	}

	int TransformSystem::GetPriority() const { return 51; }

	const char* TransformSystem::GetName() const { return "TransformSystem"; }
}
