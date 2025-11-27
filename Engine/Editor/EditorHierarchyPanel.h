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

		/**************************************************************************
		* @brief
		* 	Recursively draws an entity and all its children in an ImGui tree view.
		**************************************************************************/

		static void DrawEntityParentAndChildren(Entity& entity, Scene* scene,Entity& selectedEntity, uint32_t& pickedID, 
										 Prefab* currentPrefab, std::unordered_set<std::string>& temporaryPrefabPaths, 
										 std::string& currPrefabPath, bool& replacePrefabPending, std::string& selectedPrefabPath) {

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

#if 0 // 25/11 4:38pm test prefab parent child entity
				// Temp Fix: Only handle prefabs for main entities WITHOUT children
				if (!hasParent &&!hasChildren) {
					// ===================== Prefab Section ==========================
					if (ImGui::BeginMenu("Prefabs"))
					{
						if (ImGui::MenuItem("Create Prefab"))
						{
							if (selectedEntity)
							{
								std::string entityName = selectedEntity.GetComponent<TagComponent>().Tag;
								auto prefabPath = getAssetFilePath("Sources/Prefabs/") + entityName + ".prefab";

	
								if (selectedEntity.HasComponent<PrefabComponent>())
								{
									auto& prefabComp = selectedEntity.GetComponent<PrefabComponent>();
									
									ImGui::EndMenu();
									return;
								}


								//  Check if prefab file already exists
								std::shared_ptr<Prefab> prefab;
								xresource::instance_guid existingGUID{};

								auto existingPrefab = PrefabSerializer::LoadPrefabFromFile(prefabPath);
								if (existingPrefab)
								{
									
									existingGUID = existingPrefab->GetGUID();

									// update existing prefab instead of creating new one
									prefab = PrefabSerializer::CreateEntityPrefab(selectedEntity, entityName);
									if (prefab)
									{
										
										prefab->SetGUID(existingGUID);
									
									}
								}
								else
								{
									prefab = PrefabSerializer::CreateEntityPrefab(selectedEntity, entityName);
									
								}

								if (!prefab)
								{
									
									ImGui::EndMenu();
									return;
								}


								// Save prefab to disk
								if (PrefabSerializer::SavePrefabToFile(*prefab, prefabPath))
								{
									// Register/Update prefab in registry
									auto existingInRegistry = PrefabRegistry::Get().GetPrefab(prefab->GetGUID());
									if (!existingInRegistry)
									{
										PrefabRegistry::Get().RegisterPrefab(prefab);
										
									}
									else
									{
										PrefabRegistry::Get().RegisterPrefab(prefab); 
									
									}

									currentPrefab = prefab.get();
									currPrefabPath = prefabPath;
									temporaryPrefabPaths.insert(prefabPath);

									
								}
								
							}
						}

						if (ImGui::MenuItem("Replace Prefab"))
						{
							replacePrefabPending = true;
							selectedPrefabPath = "";
						}

						ImGui::EndMenu(); // end prefab menu
					}

				}
#endif

				ImGui::Separator();

				if (!hasParent) { // For main entities
#if 1 // 25/11 4:42 Test prefab hierarchy panel
					// ===================== Prefab Section ==========================
					if (ImGui::BeginMenu("Prefabs"))
					{
						if (ImGui::MenuItem("Create Prefab"))
						{
							if (selectedEntity)
							{
								std::string entityName = selectedEntity.GetComponent<TagComponent>().Tag;
								auto prefabPath = getAssetFilePath("Sources/Prefabs/") + entityName + ".prefab";


								//if (selectedEntity.HasComponent<PrefabComponent>())
								//{
								//	//auto& prefabComp = selectedEntity.GetComponent<PrefabComponent>();

								//	ImGui::EndMenu();
								//	return;
								//}


								//  Check if prefab file already exists
								std::shared_ptr<Prefab> prefab;
								xresource::instance_guid existingGUID{};

								auto existingPrefab = PrefabSerializer::LoadPrefabFromFile(prefabPath);
								if (existingPrefab)
								{

									existingGUID = existingPrefab->GetGUID();

									// update existing prefab instead of creating new one
									prefab = PrefabSerializer::CreateEntityPrefab(selectedEntity, entityName);
									if (prefab)
									{

										prefab->SetGUID(existingGUID);

									}
								}
								else
								{
									prefab = PrefabSerializer::CreateEntityPrefab(selectedEntity, entityName);

								}

								if (!prefab)
								{

									ImGui::EndMenu();
									return;
								}


								// Save prefab to disk
								if (PrefabSerializer::SavePrefabToFile(*prefab, prefabPath))
								{
									// Register/Update prefab in registry
									auto existingInRegistry = PrefabRegistry::Get().GetPrefab(prefab->GetGUID());
									if (!existingInRegistry)
									{
										PrefabRegistry::Get().RegisterPrefab(prefab);

									}
									else
									{
										PrefabRegistry::Get().RegisterPrefab(prefab);

									}

									currentPrefab = prefab.get();
									currPrefabPath = prefabPath;
									temporaryPrefabPaths.insert(prefabPath);


								}

							}
						}

						if (ImGui::MenuItem("Replace Prefab"))
						{
							replacePrefabPending = true;
							selectedPrefabPath = "";
						}

						ImGui::EndMenu(); // end prefab menu
					}

#endif
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
					}
				}
				
				ImGui::EndPopup(); // end of the pop up context item

			}

			if (opened && hasChildren) {
#if 1 // original code bfr modified 

				auto& transform = entity.GetComponent<TransformComponent>();
				for (uint32_t childID : transform.Children)  //Directly iterate handles
				{
					entt::entity childEntt = static_cast<entt::entity>(childID);
					if ( !registry.all_of<TagComponent>(childEntt))
					{
						continue;
					}
					Entity childEntity(static_cast<entt::entity>(childID), &scene->GetRegistry());
					DrawEntityParentAndChildren(childEntity, scene, selectedEntity, pickedID, currentPrefab,
						temporaryPrefabPaths, currPrefabPath, replacePrefabPending, selectedPrefabPath);
				}
#endif

#if 0
				auto& transform = entity.GetComponent<TransformComponent>();
				for (uint32_t childID : transform.Children) {
					entt::entity childEntityID = static_cast<entt::entity>(childID);

					// EXTRA DEFENSIVE: Check multiple conditions
					if (scene->GetRegistry().valid(childEntityID) &&
						scene->GetRegistry().all_of<TagComponent>(childEntityID) &&
						scene->GetRegistry().all_of<TransformComponent>(childEntityID)) {

						try {
							Entity childEntity(childEntityID, &scene->GetRegistry());
							DrawEntityParentAndChildren(childEntity, scene, selectedEntity, pickedID, currentPrefab,
								temporaryPrefabPaths, currPrefabPath, replacePrefabPending, selectedPrefabPath);
						}
						catch (...) {
							LOG_ERROR("Exception when drawing child entity: ", childID);
						}
					}
					else {
						LOG_WARNING("Skipping invalid child entity: ", childID);
					}
				}

#endif 

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


	};
}