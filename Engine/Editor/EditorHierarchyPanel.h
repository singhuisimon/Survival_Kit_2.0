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

namespace Engine
{
	class EditorHierarchyHelper
	{
	public:

		/**************************************************************************
		* @brief
		* 	Recursively draws an entity and all its children in an ImGui tree view.
		**************************************************************************/

		static void DrawEntityParentAndChildern(Entity& entity, Scene* scene,Entity& selectedEntity, uint32_t& pickedID, 
										 Prefab* currentPrefab, std::unordered_set<std::string>& temporaryPrefabPaths, 
										 std::string& currPrefabPath, bool& replacePrefabPending, std::string& selectedPrefabPath) {

			auto& tag = entity.GetComponent<TagComponent>();
			ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
			
			bool hasChildren = false;
			if (entity.HasComponent<TransformComponent>()) {
				auto& transform = entity.GetComponent<TransformComponent>();
				hasChildren = transform.Children.empty() ? false : true;
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
					// If this entity has a parent, unparent it first
					if (entity.HasComponent<TransformComponent>()) {
						TransformSystem::UnParent(scene, entity);
					}

					scene->DestroyEntity(entity);
					if (selectedEntity == entity)
					{
						selectedEntity = Entity();
					}
				}

				// ===================== Prefab Section ==========================
				if (ImGui::BeginMenu("Prefabs"))
				{
					if (ImGui::MenuItem("Create Prefab"))
					{
						if (selectedEntity)
						{
							std::string entityName = selectedEntity.GetComponent<TagComponent>().Tag;
							auto prefab = PrefabSerializer::CreateEntityPrefab(selectedEntity, entityName);

							if (!prefab)
							{
								ImGui::EndMenu();
								ImGui::EndPopup();
								if (opened && hasChildren) { 
									ImGui::TreePop(); 
								}
								return;
							}

							auto prefabFolder = getAssetFilePath("Sources/Prefabs/") + entityName + ".prefab";

							if (PrefabSerializer::SavePrefabToFile(*prefab, prefabFolder))
							{
								PrefabRegistry::Get().RegisterPrefab(prefab);
								currentPrefab = prefab.get();
								currPrefabPath = prefabFolder;
								temporaryPrefabPaths.insert(prefabFolder);

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
				
				ImGui::EndPopup(); // end of the pop up context item

			}

			if (opened && hasChildren) {
				auto& transform = entity.GetComponent<TransformComponent>();
				for (uint32_t childID : transform.Children)  //Directly iterate handles
				{
					Entity childEntity(static_cast<entt::entity>(childID), &scene->GetRegistry());
					DrawEntityParentAndChildern(childEntity, scene, selectedEntity, pickedID, currentPrefab, 
						temporaryPrefabPaths, currPrefabPath, replacePrefabPending, selectedPrefabPath);
				}
				ImGui::TreePop();
			}

		}
	};
}