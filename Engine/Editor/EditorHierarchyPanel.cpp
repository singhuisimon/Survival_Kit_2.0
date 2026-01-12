// Include Header Files
#include "EditorHierarchyPanel.h"
#include "../Engine/Editor/Editor.h"

#include "../Component/TagComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/PrefabComponent.h"
#include "../Serialization/PrefabInstantiator.h"
#include "../Utility/Logger.h"
#include "../Utility/AssetPath.h"

#include "../Prefab/Prefab.h"
#include "../Prefab/PrefabRegistry.h"
#include "../Transform/TransformSystem.h"

namespace Engine
{
	void EditorHierarchyPanel::HierarchyPanel()
	{
		//bool& hierarchyWindow = m_Editor->GetHierarchyWindowRef();
		if (!m_Editor->GetHierarchyWindowRef()) return;
		
		Scene* m_Scene = m_Editor->GetActiveScene();
		bool isPrefabScene = false;
		if (ImGui::Begin("Hierarchy", &m_Editor->GetHierarchyWindowRef()))
		{
			if (m_Scene) {
				std::string sceneName = m_Scene->GetName();
				// Check if scene name indicates it's a prefab
				isPrefabScene = (sceneName.find("Prefab:") == 0) ||
					(sceneName.find("prefab") != std::string::npos);
			}
			ImGui::BeginDisabled(isPrefabScene);
			if (ImGui::Button("Create Entity"))
			{
				ImGui::OpenPopup("CreateEntityPopup");
			}
			ImGui::EndDisabled();
			if (isPrefabScene && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
				ImGui::SetTooltip("Cannot create loose entities in prefab editing mode");
			}
		}
		if (ImGui::BeginPopup("CreateEntityPopup"))
		{
			
			if (ImGui::MenuItem("Create Entity"))
			{
				
				if (m_Scene)
				{
					auto newEntity = m_Scene->CreateEntity("New Entity");
					newEntity.AddComponent<TagComponent>("New Entity");
					newEntity.AddComponent<TransformComponent>();

					m_Editor->SetCurrSelectedEntity(newEntity);
					m_Editor->RetrievePickedID(static_cast<uint32_t>(newEntity.GetHandle()));
				}
				
			}
			
			
			auto prefabFiles = m_Editor->getAssetsInFolder(getAssetFilePath("Sources/Prefabs/"));
			ImGui::BeginDisabled(prefabFiles.empty());
			if (ImGui::MenuItem("Create Entity From Prefab"))
			{
				parentOfPrefabEntity = Entity{};
				ImGui::CloseCurrentPopup();
				openPrefabList = true;
			}
			ImGui::EndDisabled();
			ImGui::EndPopup(); // end pop up of CreateEntityPopup
		}

		ImGui::Separator();
		if (m_Scene)
		{
			// List all entities
			EntitiesList();
		}
		ImGui::End(); // end of hierarchy panel 
		CreateEntityFromPrefabPanel();
		OpenReplacePefabPanel();
	}

	void EditorHierarchyPanel::EntitiesList()
	{
		Scene* m_Scene = m_Editor->GetActiveScene();

		//entitiesToDelete.clear();

		if (!m_Scene) return;
		
		auto viewEntities = m_Scene->GetRegistry().view<TagComponent>();

		for (auto handleEtt : viewEntities)
		{
			Entity entity(handleEtt, &m_Scene->GetRegistry());
			if (entity.HasComponent<TransformComponent>())
			{
				auto& transform = entity.GetComponent<TransformComponent>();
				if (transform.Parent == u32_max)
				{
					DrawEntityTree(entity);
				}
			}
		}

		//CheckParentlessChildren(m_Scene);
		//ClearParentlessChildren(m_Scene);

		// ==================== Main Entity Selection Part ==========================
		if (openAttachEntityPopup)
		{
			ImGui::OpenPopup("Main Entity Selection");
			openAttachEntityPopup = false;
		}

		if (ImGui::BeginPopupModal("Main Entity Selection", nullptr, ImGuiWindowFlags_NoDocking))
		{
#if 0
			ImGui::SetWindowSize(ImVec2(500, 400), ImGuiCond_Once);
			if (entityToAttach && entityToAttach.HasComponent<TagComponent>())
			{
				std::string attachName = entityToAttach.GetComponent<TagComponent>().Tag;
				ImGui::Text("Select a main entity to attach '%s' to:", attachName.c_str());
			}
			ImGui::Separator();

			// Store selected entity
			static Entity selectedMainEntity; // Or make it a member variable

			ImGui::BeginChild("##entity_list", ImVec2(0, 300), true);

			for (auto entityHandle : viewEntities)
			{
				Entity entity(entityHandle, &m_Scene->GetRegistry());
				if (!entity.HasComponent<TransformComponent>()) continue;

				auto& transform = entity.GetComponent<TransformComponent>();

				// Only show main entities (no parent) that aren't the entity itself
				if (transform.Parent == u32_max && entity != entityToAttach)
				{
					std::string entityName = entity.GetComponent<TagComponent>().Tag;

					// Make them selectable
					if (ImGui::Selectable(entityName.c_str(), selectedMainEntity == entity))
					{
						selectedMainEntity = entity;
					}
				}
			}
			ImGui::EndChild();

			ImGui::Separator();

			// Attach button (only enabled when something is selected)
			ImGui::BeginDisabled(!selectedMainEntity);
			if (ImGui::Button("Attach"))
			{
				if (selectedMainEntity && entityToAttach)
				{
					auto& parentTransform = selectedMainEntity.GetComponent<TransformComponent>();
					auto& childTransform = entityToAttach.GetComponent<TransformComponent>();

					parentTransform.Children.push_back((uint32_t)entityToAttach.GetHandle());
					childTransform.SetParent(selectedMainEntity);

					LOG_INFO("Attached '%s' as child of '%s'",
						entityToAttach.GetComponent<TagComponent>().Tag.c_str(),
						selectedMainEntity.GetComponent<TagComponent>().Tag.c_str());

					// Clear selection
					selectedMainEntity = Entity{};
					entityToAttach = Entity{};
					ImGui::CloseCurrentPopup();
				}
			}
			ImGui::EndDisabled();

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				// Clear everything on cancel
				selectedMainEntity = Entity{};
				entityToAttach = Entity{};
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
#endif
			ImGui::SetWindowSize(ImVec2(500, 400), ImGuiCond_Once);
			if (entityToAttach && entityToAttach.HasComponent<TagComponent>())
			{
				ImGui::Text("Select a main entity to attach :", entityToAttach.GetComponent<TagComponent>().Tag.c_str());
			}
			ImGui::Separator();
			bool selected = false;
			for (auto entityHandle : viewEntities)
			{
				Entity entity(entityHandle, &m_Scene->GetRegistry());
				auto& transform = entity.GetComponent<TransformComponent>();

				// Only show main entities (no parent) that aren't the entity itself
				if (transform.Parent == u32_max && entity != entityToAttach)
				{
					// Attach entityToAttach as child of selected entity
					auto& parentTransform = entity.GetComponent<TransformComponent>();
					parentTransform.Children.push_back((uint32_t)entityToAttach);
					auto& childTransform = entityToAttach.GetComponent<TransformComponent>();
					childTransform.SetParent(entity);
					selected = true;
					break;
				}
			}
			if (selected)
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::Separator();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}

		// ========================== Selected Prefab For Sub Entity Part ======================
		CheckParentlessChildren(m_Scene);
		ClearParentlessChildren(m_Scene);
		
	}

	void EditorHierarchyPanel::DrawEntityTree(Entity& entity)
	{
		Entity currentSelectedEntity = m_Editor->GetSelectedEntity();
		u32 currentPickedID = m_Editor->GetPickedID();

		Scene* m_Scene = m_Editor->GetActiveScene();
		if (!m_Scene) return;

		// validate entity before accessing compon 
		auto& registry = m_Scene->GetRegistry();
		entt::entity ent = entity.GetHandle();

		if (!registry.all_of<TagComponent>(ent)) return;

		auto& tag = entity.GetComponent<TagComponent>();
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_OpenOnDoubleClick |
			ImGuiTreeNodeFlags_SpanAvailWidth;

		bool hasChildren = false;
		bool hasParent = false;

		if (entity.HasComponent<TransformComponent>()) {
			auto& transform = entity.GetComponent<TransformComponent>();
			hasChildren = transform.Children.empty() ? false : true; // if chidren.empty(), hasChilren = false, otherwise true;
			hasParent = transform.Parent == u32_max ? false : true;
		}

		if (!hasChildren) {
			flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
		}

		if (currentSelectedEntity == entity)
		{
			flags |= ImGuiTreeNodeFlags_Selected;
		}

		bool openedTree = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Tag.c_str());

		if (ImGui::IsItemClicked())
		{

			//entt::entity entityHandle = entity.GetHandle();

			m_Editor->SetCurrSelectedEntity(entity);
			m_Editor->RetrievePickedID(static_cast<u32>(entity.GetHandle()));

			LOG_DEBUG("Selected entity: ", tag.Tag.c_str());
			LOG_DEBUG("new currSelectedEntity: ", static_cast<uint32_t>(m_Editor->GetSelectedEntity().GetHandle()));
			//LOG_DEBUG("NEW m_PickedID: ", m_PickedID);
			LOG_DEBUG("Editor's picked ID: ", m_Editor->GetPickedID());
		}

		if (ImGui::BeginPopupContextItem())
		{
			// ==================== Selected Entity Section =======================
			if (ImGui::MenuItem("Delete Entity"))
			{
				entitiesToDelete.push_back(entity);

				if (currentSelectedEntity == entity)
				{
					m_Editor->SetCurrSelectedEntity(Entity{});
					m_Editor->RetrievePickedID(0xFFFFFFFFu);

				}

				// If this entity has a parent, unparent it first
				if (entity.HasComponent<TransformComponent>()) 
				{
					if (hasParent) 
					{
						TransformSystem::UnParent(m_Scene, entity);
					}

					if (hasChildren) 
					{
						auto& transform = entity.GetComponent<TransformComponent>();
						for (uint32_t childID : transform.Children)
						{
							Entity childEntity(static_cast<entt::entity>(childID), &m_Scene->GetRegistry());
							if (currentSelectedEntity == childEntity)
							{
								m_Editor->SetCurrSelectedEntity(Entity{});
								m_Editor->RetrievePickedID(0xFFFFFFFFu);
							}
							entitiesToDelete.push_back(childEntity);
						}
					}
				}
				//ImGui::EndMenu();
				ImGui::EndPopup();
				if (openedTree && hasChildren) 
				{
					ImGui::TreePop();
				}
				return;
			}

			ImGui::Separator();
			if (!hasParent) // for main entities
			{
				if (ImGui::BeginMenu("Prefabs"))
				{
					if (ImGui::MenuItem("Create Prefab"))
					{
						if (currentSelectedEntity)
						{
							LOG_DEBUG(" ========== Start Create Prefab =========");
							std::string entityName = currentSelectedEntity.GetComponent<TagComponent>().Tag;
							//LOG_DEBUG(" entityName: ", entityName);
							auto prefabPath = getAssetFilePath("Sources/Prefabs/") + entityName + ".prefab";
							
							if (PrefabInstantiator::CreatePrefabFromEntity(currentSelectedEntity, entityName, prefabPath))
							{
									LOG_INFO(" -------- PrefabInstantiator::CreatePrefabFromEntity is called. ----------------------");
									const auto& allPrefabs = PrefabRegistry::Get().GetAllPrefabs();
									LOG_INFO("Registered Prefabs (", allPrefabs.size(), "):");
									for (const auto& [guid, pathName] : allPrefabs) {
										LOG_INFO("  - ", pathName.second);
									}
							}
							else
							{
								LOG_ERROR("Failed to create prefab from entity: ");

							}
						}
					}
					if (ImGui::MenuItem("Replace Prefab"))
					{
						entityToReplace = entity;
						openReplacePrefabPanel = true;

					}
					ImGui::EndMenu();
				}
				// ======================= Add New Sub Entity Section =======================
				if (ImGui::BeginMenu("Add New Sub-Entity"))
				{
					if (ImGui::MenuItem("Create New Sub-Entity"))
					{
						auto newEntity = m_Scene->CreateEntity("New Entity");
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
							childPrefab.PrefabAssetGuid = parentPrefab.PrefabAssetGuid;
							childPrefab.prefabName = parentPrefab.prefabName;
							childPrefab.prefabVersion = parentPrefab.prefabVersion;
							childPrefab.isPrefabRoot = false;
							parentPrefab.childEntityIDs.push_back(static_cast<u32>(newEntity.GetHandle()));
						}
						
					}
					ImGui::Separator();

					if (ImGui::MenuItem("Create Sub-Entity With Prefab"))
					{
						parentOfPrefabEntity = entity;
						openPrefabList = true;
					}

					ImGui::EndMenu();
				}
				if (!hasChildren)
				{
					if (ImGui::MenuItem("Attach as Sub-Entity"))
					{
						openAttachEntityPopup = true;
						entityToAttach = entity;
					}
				}
			}
			else // for sub-entities part
			{
				if (ImGui::MenuItem("Detach Sub-Entity"))
				{
					if (entity.HasComponent<TransformComponent>()) 
					{
						TransformSystem::UnParent(m_Scene, entity);

					}
					
				}
			}
			ImGui::EndPopup();// end of the pop up context item
		}

		if (openedTree && hasChildren)
		{
			auto& transform = entity.GetComponent<TransformComponent>();
			for (uint32_t childID : transform.Children)  //Directly iterate handles
			{
				entt::entity childEntt = static_cast<entt::entity>(childID);
				if (!registry.all_of<TagComponent>(childEntt))
				{
					continue;
				}
				Entity childEntity(static_cast<entt::entity>(childID), &m_Scene->GetRegistry());
				DrawEntityTree(childEntity);
			}

			ImGui::TreePop();
		}
	

	}

	void EditorHierarchyPanel::DeleteEntityTree(Scene* scene) 
	{
		for (auto& entity : entitiesToDelete) {
			scene->DestroyEntity(entity);
		}
		entitiesToDelete.clear();
	}

	void EditorHierarchyPanel::CheckParentlessChildren(Scene* scene)
	{
		std::vector<std::pair<Entity, u32>> childrenParentID;
		std::vector<u32> parentsID;

		if (scene)
		{
			auto viewEntities = scene->GetRegistry().view<TagComponent>();
			for (auto entityHandle : viewEntities)
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

			for (auto& childAndParent : childrenParentID)
			{
				auto it = std::find(parentsID.begin(), parentsID.end(), childAndParent.second);

				if (it == parentsID.end()) 
				{
					parentlessChildren.push_back(childAndParent.first);
				}
			}

		}
		
	}

	void EditorHierarchyPanel::ClearParentlessChildren(Scene* scene)
	{
		if (!parentlessChildren.empty()) 
		{
			for (auto& entity : parentlessChildren) 
			{
				scene->DestroyEntity(entity);
			}
			parentlessChildren.clear();
		}
	}

#if 0
	void EditorHierarchyPanel::CreateEntityFromPrefabPanel()
	{
		if (openPrefabList)
		{
			ImGui::OpenPopup("createEttPrefab");
			openPrefabList = false;
		}
		if (ImGui::BeginPopupModal("createEttPrefab", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			auto prefabFiles = m_Editor->getAssetsInFolder(getAssetFilePath("Sources/Prefabs/"));

			ImGui::Text("Select a prefab to instantiate:");
			ImGui::Separator();

			if (prefabFiles.empty()) {
				ImGui::Text("No prefabs found in Sources/Prefabs/");
			}
			else 
			{
				for (auto& file : prefabFiles)
				{
					if (ImGui::Selectable(file.name.c_str()))
					{
						Scene* scene = m_Editor->GetActiveScene();
						if (!scene) 
						{
							LOG_ERROR("No active scene to instantiate prefab into");
						}
						else 
						{
							Entity prefabInstance = PrefabInstantiator::InstantiatePrefabFromFile(
								scene,
								file.fullPath,
								Entity{} 
							);

							if (prefabInstance) 
							{
								// Select the newly created entity
								m_Editor->SetCurrSelectedEntity(prefabInstance);
								m_Editor->RetrievePickedID(static_cast<uint32_t>(prefabInstance.GetHandle()));
							}
							else {
								LOG_ERROR("Failed to instantiate prefab: ", file.fullPath);
							}

						}
						ImGui::CloseCurrentPopup();
						
					}
				}
				if (ImGui::Button("Cancel"))
				{
					openPrefabList = false;
					ImGui::CloseCurrentPopup();
				}
			}
			
			ImGui::EndPopup();
		}
	}
#endif
	void EditorHierarchyPanel::CreateEntityFromPrefabPanel()
	{
		if (openPrefabList)
		{
			ImGui::OpenPopup("createEttPrefab");
			openPrefabList = false;
		}

		if (ImGui::BeginPopupModal("createEttPrefab", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			auto prefabFiles = m_Editor->getAssetsInFolder(getAssetFilePath("Sources/Prefabs/"));

			// Add parent info display
			if (parentOfPrefabEntity)
			{
				std::string parentName = parentOfPrefabEntity.GetComponent<TagComponent>().Tag;
				ImGui::Text("Will attach to: %s", parentName.c_str());
				ImGui::Separator();
			}

			ImGui::Text("Select a prefab to instantiate:");
			ImGui::Separator();

			if (prefabFiles.empty()) {
				ImGui::Text("No prefabs found in Sources/Prefabs/");
			}
			else
			{
				for (auto& file : prefabFiles)
				{
					if (ImGui::Selectable(file.name.c_str()))
					{
						Scene* scene = m_Editor->GetActiveScene();
						if (!scene)
						{
							LOG_ERROR("No active scene to instantiate prefab into");
						}
						else
						{
							Entity prefabInstance = PrefabInstantiator::InstantiatePrefabFromFile(
								scene,
								file.fullPath,
								parentOfPrefabEntity
							);

							if (prefabInstance)
							{
								// Select the newly created entity
								m_Editor->SetCurrSelectedEntity(prefabInstance);
								m_Editor->RetrievePickedID(static_cast<uint32_t>(prefabInstance.GetHandle()));
							}
							else {
								LOG_ERROR("Failed to instantiate prefab: ", file.fullPath);
							}
						}
						ImGui::CloseCurrentPopup();
					}
				}
			}

			ImGui::Separator();
			if (ImGui::Button("Cancel"))
			{
				// Clear the parent when cancelled
				parentOfPrefabEntity = Entity{};
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}


	void EditorHierarchyPanel::OpenReplacePefabPanel()
	{
		if (openReplacePrefabPanel)
		{
			ImGui::OpenPopup("Replace Prefab List");
			openReplacePrefabPanel = false;
		}

		// Handle the popup modal
		if (ImGui::BeginPopupModal("Replace Prefab List", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			Scene* m_Scene = m_Editor->GetActiveScene();

			// Check if entityToReplace is valid
			if (!m_Scene || !entityToReplace || !entityToReplace.HasComponent<TagComponent>())
			{
				ImGui::Text("Error: No entity selected to replace");

				if (ImGui::Button("Close##ReplaceError"))
				{
					entityToReplace = Entity{};
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
				return;
			}

			std::string entityName = entityToReplace.GetComponent<TagComponent>().Tag;
			ImGui::Text("Replace entity '%s' with prefab:", entityName.c_str());
			ImGui::Separator();

			// Get all prefab files
			auto prefabFiles = m_Editor->getAssetsInFolder(getAssetFilePath("Sources/Prefabs/"));

			if (prefabFiles.empty())
			{
				ImGui::Text("No prefabs found in Sources/Prefabs/");
			}
			else
			{
				// Display prefab selection list
				for (auto& file : prefabFiles)
				{
					if (ImGui::Selectable(file.name.c_str()))
					{
						// Store parent info and position ONLY
						Entity parentEntity;
						glm::vec3 position = glm::vec3(0.0f);
						std::string oldEntityName = entityName;

						// Save parent and position if entity has TransformComponent
						if (entityToReplace.HasComponent<TransformComponent>())
						{
							auto& oldTransform = entityToReplace.GetComponent<TransformComponent>();

							// Save parent
							if (oldTransform.Parent != u32_max)
							{
								parentEntity = Entity(static_cast<entt::entity>(oldTransform.Parent), &m_Scene->GetRegistry());
							}

							// Save position ONLY
							position = oldTransform.Position;

							// Unparent from old parent if exists
							if (parentEntity)
							{
								TransformSystem::UnParent(m_Scene, entityToReplace);
							}

							// Remove children from old entity
							std::vector<uint32_t> childrenCopy = oldTransform.Children;
							for (uint32_t childID : childrenCopy)
							{
								Entity child(static_cast<entt::entity>(childID), &m_Scene->GetRegistry());
								if (child)
								{
									TransformSystem::UnParent(m_Scene, child);
								}
							}
						}

						// Store selection state
						bool wasSelected = (m_Editor->GetSelectedEntity() == entityToReplace);

						// Delete the old entity
						m_Scene->DestroyEntity(entityToReplace);

						// Instantiate the new prefab with parent
						Entity prefabInstance = PrefabInstantiator::InstantiatePrefabFromFile(
							m_Scene,
							file.fullPath,
							parentEntity
						);

						if (prefabInstance)
						{
							// Change the name to match the old entity
							if (prefabInstance.HasComponent<TagComponent>())
							{
								auto& tag = prefabInstance.GetComponent<TagComponent>();
								tag.Tag = oldEntityName;
							}

							// Apply position ONLY - keep rotation and scale from prefab
							if (prefabInstance.HasComponent<TransformComponent>())
							{
								auto& newTransform = prefabInstance.GetComponent<TransformComponent>();
								newTransform.SetPosition(position);

								LOG_DEBUG("Applied position - Pos: ", position.x, ",", position.y, ",", position.z);
							}

							// Restore selection if it was selected
							if (wasSelected)
							{
								m_Editor->SetCurrSelectedEntity(prefabInstance);
								m_Editor->RetrievePickedID(static_cast<uint32_t>(prefabInstance.GetHandle()));
							}

							LOG_INFO("Replaced entity '", oldEntityName.c_str(), "' with prefab: ", file.name.c_str());
						}
						else
						{
							LOG_ERROR("Failed to instantiate prefab: ", file.fullPath.c_str());
						}

						// Clear and close
						entityToReplace = Entity{};
						ImGui::CloseCurrentPopup();
						break;
					}
				}
			}

			ImGui::Separator();

			if (ImGui::Button("Cancel##ReplacePrefab", ImVec2(100, 0)))
			{
				entityToReplace = Entity{};
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}
	}
}