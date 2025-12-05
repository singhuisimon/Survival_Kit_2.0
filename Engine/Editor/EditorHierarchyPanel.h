#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Include Standard Headers
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <ImGuizmo.h>

// Include other necessary headers
#include "../Editor/Editor.h"
#include "../ECS/Components.h"
#include "../Utility/Logger.h"
#include "../Utility/AssetPath.h"
#include "../Prefab/Prefab.h"
#include "../Prefab/PrefabRegistry.h"
#include "../Serialization/PrefabInstantiator.h"
#include "../Transform/TransformSystem.h"
#include "../Serialization/PrefabSerializer.h"

namespace Engine
{
	class EditorHierarchyHelper
	{
	public:

		inline static bool openAttachEntityPopup = false;
		inline static Entity entityToAttach = Entity();

		inline static bool openSubEntityFromPrefabPopup = false;
		inline static Entity parentOfPrefabEntity = Entity();

		inline static std::vector<Entity> entitiesToDelete;
		inline static std::vector<Entity> parentlessChildren;

		/**************************************************************************
		* @brief
		* 	Recursively draws an entity and all its children in an ImGui tree view.
		**************************************************************************/

		static void DrawEntityParentAndChildren(Entity& entity, Scene* scene, Entity& selectedEntity, uint32_t& pickedID,
			Prefab* currentPrefab, std::unordered_set<std::string>& temporaryPrefabPaths,
			std::string& currPrefabPath, bool& replacePrefabPending, std::string& selectedPrefabPath, Scene* m_Scene) {

			// validate entity before accessing compon 
			auto& registry = scene->GetRegistry();
			entt::entity ent = (entt::entity)entity;
			if (!registry.all_of<TagComponent>(ent))
			{
				return;
			}

			auto& tag = entity.GetComponent<TagComponent>();
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;

			bool hasChildren = false; // Only true for main entity
			bool hasParent = false; // Only true for sub entities 
			if (entity.HasComponent<TransformComponent>()) {
				auto& transform = entity.GetComponent<TransformComponent>();
				hasChildren = transform.Children.empty() ? false : true;
				hasParent = transform.Parent == u32_max ? false : true;
			}

			if (!hasChildren) {
				flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			}

			if (selectedEntity == entity)
			{
				flags |= ImGuiTreeNodeFlags_Selected;
			}

			bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Tag.c_str());

			if (ImGui::IsItemClicked())
			{
				selectedEntity = entity;
				uint32_t newID = (uint32_t)entity;
				LOG_DEBUG("Clicked entity ID = ", newID, " | old m_PickedID = ", pickedID);
				pickedID = newID;
				LOG_DEBUG("Updated m_PickedID to ", pickedID);
			}

			// Right-click context menu
			if (ImGui::BeginPopupContextItem())
			{
				// ==================== Selected Entity Section =======================
				if (ImGui::MenuItem("Delete Entity"))
				{
					if (selectedEntity == entity)
					{
						selectedEntity = Entity();
						pickedID = 0xFFFFFFFFu;
					}

					entitiesToDelete.push_back(entity);

					// If this entity has a parent, unparent it first
					if (entity.HasComponent<TransformComponent>()) {

						if (hasParent) {
							TransformSystem::UnParent(scene, entity);
						}

						if (hasChildren) {
							auto& transform = entity.GetComponent<TransformComponent>();
							for (uint32_t childID : transform.Children)
							{
								Entity childEntity(static_cast<entt::entity>(childID), &scene->GetRegistry());
								if (selectedEntity == childEntity)
								{
									selectedEntity = Entity();
									pickedID = 0xFFFFFFFFu;
								}
								entitiesToDelete.push_back(childEntity);
							}
						}
					}

					ImGui::EndPopup();

					if (opened && hasChildren) {
						ImGui::TreePop();
					}
					return;	// Return early to prevent drawing (child) entities that are deleted
				}


				ImGui::Separator();

				if (!hasParent) { // For main entities

					// ===================== Prefab Section ==========================
					if (ImGui::BeginMenu("Prefabs"))
					{
						if (ImGui::MenuItem("Create Prefab"))
						{
							if (selectedEntity)
							{
								LOG_DEBUG(" ========== Start Create Prefab =========");
								std::string entityName = selectedEntity.GetComponent<TagComponent>().Tag;
								LOG_DEBUG(" entityName: ", entityName);
								auto prefabPath = getAssetFilePath("Sources/Prefabs/") + entityName + ".prefab";
								std::shared_ptr<Prefab> prefab;

								if (!hasChildren) // Entity Prefab
								{
									LOG_DEBUG("Dont have Children.");
									//prefab->SetName(entityName);
									//LOG_DEBUG(" SetName: ", prefab->GetName());
									prefab = PrefabSerializer::CreateEntityPrefab(selectedEntity, entityName);
									//PrefabSerializer::SavePrefabToFile(*prefab, prefabPath));
								}
								else
								{

									LOG_DEBUG("Have Children.");

									
									// Get all entities in the hierarchy (selected entity + children)
									std::vector<Entity> entitiesInHierarchy;
									entitiesInHierarchy.push_back(selectedEntity); // Add root
									//Scene* m_Scene;

									if (selectedEntity.HasComponent<TransformComponent>())
									{
										auto& transform = selectedEntity.GetComponent<TransformComponent>();
										for (uint32_t childID : transform.Children)
										{
											Entity childEntity((entt::entity)childID, &m_Scene->GetRegistry());
											if (childEntity)
											{
												entitiesInHierarchy.push_back(childEntity);
											}
										}
									}
									prefab = PrefabSerializer::CreateScenePrefab(m_Scene, entitiesInHierarchy, entityName);

								}

								if (prefab)
								{
									if (PrefabSerializer::SavePrefabToFile(*prefab, prefabPath))
									{
										PrefabRegistry::Get().RegisterPrefab(prefab);
										currentPrefab = prefab.get();
										currPrefabPath = prefabPath;
										temporaryPrefabPaths.insert(prefabPath);
										LOG_DEBUG("Prefab created successfully: ", prefabPath);
									}
								}

								PrefabSerializer::SavePrefabToFile(*prefab, convertAssetPathToRootResources(prefabPath));
								LOG_DEBUG(" ========== End Create Prefab =========");

							}
						}

						if (ImGui::MenuItem("Replace Prefab"))
						{
							replacePrefabPending = true;
							selectedPrefabPath = "";
						}

						ImGui::EndMenu(); // end prefab menu
					}
					if (ImGui::BeginMenu("Add New Sub-Entity"))
					{
						if (ImGui::MenuItem("Create New Sub-Entity"))
						{
							auto newEntity = scene->CreateEntity("New Entity");
							newEntity.AddComponent<TagComponent>("New Entity");
							newEntity.AddComponent<TransformComponent>();

							if (entity.HasComponent<TransformComponent>()) {
								auto& parentTransform = entity.GetComponent<TransformComponent>();
								parentTransform.Children.push_back((uint32_t)newEntity);
								auto& childTransform = newEntity.GetComponent<TransformComponent>();
								childTransform.SetParent(entity);
							}

							if (entity.HasComponent<PrefabComponent>())
							{
								auto& parentPrefab = entity.GetComponent<PrefabComponent>();
								auto& childPrefab = newEntity.AddComponent<PrefabComponent>();
								childPrefab.PrefabGUID = parentPrefab.PrefabGUID;
							}
						}
						ImGui::Separator();
						if (ImGui::MenuItem("Create Sub-Entity With Prefab"))
						{
							openSubEntityFromPrefabPopup = true;
							parentOfPrefabEntity = entity;
						}

						ImGui::EndMenu();
					}

					if (!hasChildren) {
						if (ImGui::MenuItem("Attach as Sub-Entity"))
						{
							openAttachEntityPopup = true;
							entityToAttach = entity;
						}
					}
				}
				else { // For sub-entities

					if (ImGui::MenuItem("Detach Sub-Entity"))
					{
						if (entity.HasComponent<TransformComponent>()) {
							TransformSystem::UnParent(scene, entity);
						}

						if (entity.HasComponent<PrefabComponent>())
						{
							entity.RemoveComponent<PrefabComponent>();
						}
					}
				}

				ImGui::EndPopup(); // end of the pop up context item

			}

			if (opened && hasChildren) {

				auto& transform = entity.GetComponent<TransformComponent>();
				for (uint32_t childID : transform.Children)  //Directly iterate handles
				{
					entt::entity childEntt = static_cast<entt::entity>(childID);
					if (!registry.all_of<TagComponent>(childEntt))
					{
						continue;
					}
					Entity childEntity(static_cast<entt::entity>(childID), &scene->GetRegistry());
					DrawEntityParentAndChildren(childEntity, scene, selectedEntity, pickedID, currentPrefab,
						temporaryPrefabPaths, currPrefabPath, replacePrefabPending, selectedPrefabPath, m_Scene);
				}

				ImGui::TreePop();
			}
		}

		static void DeleteEntityParentAndChildren(Scene* scene) {
			for (auto& entity : entitiesToDelete) {
				scene->DestroyEntity(entity);
			}
			entitiesToDelete.clear();
		}

		static void FillEntitiesWithChildrenToDelete(Entity& entity, Scene* scene,
			Entity& selectedEntity, uint32_t& pickedID) {

			if (selectedEntity == entity)
			{
				selectedEntity = Entity();
				pickedID = 0xFFFFFFFFu;
			}

			entitiesToDelete.push_back(entity);

			if (entity.HasComponent<TransformComponent>()) {

				auto& transform = entity.GetComponent<TransformComponent>();
				if (!transform.Children.empty()) {
					for (uint32_t childID : transform.Children)
					{
						Entity childEntity(static_cast<entt::entity>(childID), &scene->GetRegistry());
						if (selectedEntity == childEntity)
						{
							selectedEntity = Entity();
							pickedID = 0xFFFFFFFFu;
						}
						entitiesToDelete.push_back(childEntity);
					}
				}
			}

			return;
		}

		/**************************************************************************
		* @brief
		* 	Scans the scene hierarchy and identifies all child entities whose 
		*	parent no longer exists, 
		* @param scene
		*	The scene whose hierarchy should be validated.
		**************************************************************************/

		static void CheckForParentlessChildren(Scene* scene) {

			std::vector<std::pair<Entity, u32>> childrenParentID;
			std::vector<u32> parentsID;

			if (scene)
			{
				auto view = scene->GetRegistry().view<TagComponent>();

				for (auto entityHandle : view)
				{
					Entity checkEntity(entityHandle, &scene->GetRegistry());
					auto& transform = checkEntity.GetComponent<TransformComponent>();
					if (transform.Parent != u32_max) { // Child entities
						childrenParentID.push_back(std::make_pair(checkEntity, transform.Parent));
					}
					else {
						parentsID.push_back(static_cast<u32>(entityHandle));
					}
				}

				for (auto& childAndParent : childrenParentID) {

					auto it = std::find(parentsID.begin(), parentsID.end(), childAndParent.second);

					if (it == parentsID.end()) {
						parentlessChildren.push_back(childAndParent.first);
					}
				}

			}
		}

		/**************************************************************************
		* @brief
		* 	Destroy all entities previously detected as parentless. 
		* @param scene
		*	The scene whose hierarchy should be validated.
		**************************************************************************/
		static void ClearParentlessChildren(Scene* scene) {
			if (!parentlessChildren.empty()) {
				for (auto& entity : parentlessChildren) {
					scene->DestroyEntity(entity);
				}
				parentlessChildren.clear();
			}
		}
	};
}