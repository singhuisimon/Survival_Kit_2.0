// Include Header Files
#include "EditorHierarchyPanel.h"
#include "../Engine/Editor/Editor.h"

#include "../Component/TagComponent.h"
#include "../Component/TransformComponent.h"
#include "../Component/PrefabComponent.h"

#include "../Utility/Logger.h"
#include "../Utility/AssetPath.h"

#include "../Transform/TransformSystem.h"

namespace Engine
{
	void EditorHierarchyPanel::HierarchyPanel()
	{
		//bool& hierarchyWindow = m_Editor->GetHierarchyWindowRef();
		if (!m_Editor->GetHierarchyWindowRef()) return;
		
		Scene* m_Scene = m_Editor->GetActiveScene();

		if (ImGui::Begin("Hierarchy", &m_Editor->GetHierarchyWindowRef()))
		{
			if (ImGui::Button("Create Entity"))
			{
				ImGui::OpenPopup("CreateEntityPopup");
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
			ImGui::EndPopup(); // end pop up of CreateEntityPopup
		}

		ImGui::Separator();
		if (m_Scene)
		{
			// List all entities
			EntitiesList();
		}
		ImGui::End(); // end of hierarchy panel 
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
					if (entity.HasComponent<PrefabComponent>())
					{
						entity.RemoveComponent<PrefabComponent>();
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
}