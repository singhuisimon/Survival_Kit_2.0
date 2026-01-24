#include <windows.h>
#include <algorithm>
#include "EditorPropertyPanel.h"
#include "../Utility/Logger.h"
#include "../Engine/Editor/Editor.h"
#include "../Animation/AnimationStorage.h"
#include "../Asset/AssetManager.h"
#include "../Asset/ResourceManager.h"
#include "../Asset/ResourceHelpers.h"
#include "../BehaviourTree/BehaviourTreeEditor.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Texture.h"
#include "../Scripting/ScriptSerializer.h"
#include "../Scripting/MonoScriptEngine.h"
#include "../Serialization/MaterialSerializer.h"
#include "../Serialization/ComponentSerializer.h"
#include "../Serialization/PrefabInstantiator.h"
#include "../Serialization/PrefabSerializer.h"

namespace Engine
{
	void EditorPropertyPanel::PropertyPanel()
	{
		/*if (m_SelectedEntity && m_SelectedEntity.HasComponent<PrefabComponent>()) {
			auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();
			ImGui::Text("[DEBUG] PrefabComponent overrides: %zu", prefabComp.componentOverrides.size());
		}*/
		if (!m_Editor->GetPropertyWindowRef()) return;
		//std::cout << "SelectedEntity: " << (uint32_t)m_SelectedEntity.GetHandle() << "\n";
		//Bug - Scene Switching
		m_Scene = m_Editor->GetActiveScene();
		m_SelectedEntity = m_Editor->GetSelectedEntity();
		m_ScenePath = m_Editor->GetScenePath();
		m_SceneName = m_Editor->GetSceneName();
		//std::cout << "m_ScenePath: " << m_ScenePath.c_str() << "\n";
		bool& open = m_Editor->GetPropertyWindowRef();
		if (ImGui::Begin("Properties", &open))
		{
			ImGui::Text("m_Scene: %s", m_Scene->GetName().c_str());
			ImGui::Text("m_ScenePath: %s", m_ScenePath.c_str());
			//ImGui::Text("m_SceneName: %s", m_SceneName.c_str());

			// Determine selected entity handle
			uint32_t entityHandle = static_cast<uint32_t>(m_SelectedEntity.GetHandle());
			ImGui::Text("SelectedEntity %u", entityHandle);
			//LOG_DEBUG("SelectedEntity handle: ", entityHandle);

			if (m_SelectedEntity) { // If entity is selected
				
				// Tag Component
				DisplayTagComponent();
				ImGui::Separator();

				// Calculate "..." button size
				ImVec2 dotTextSize = ImGui::CalcTextSize("...");
				ImVec2 dotButtonSize(dotTextSize.x + 8.0f, dotTextSize.y + 8.0f);

				// Prefab Component - TODO: Finish when prefabs are revamped
				DisplayPrefabComponent(dotButtonSize);

				// Transform Component
				DisplayTransformComponent(dotButtonSize);

				// RigidBody Component - TODO: Check when scene runs
				DisplayRigidBodyComponent(dotButtonSize);

				// MeshRenderer Component - TODO: Materials
				DisplayMeshRendererComponent(dotButtonSize);

				// Audio Component - TODO: Check when scene runs
				DisplayAudioComponent(dotButtonSize);

				// Reverb Component
				DisplayReverbZoneComponent(dotButtonSize);

				// Listener Component
				DisplayListenerComponent(dotButtonSize);

				// BT Component - TODO: Check when scene runs
				DisplayBTComponent(dotButtonSize);

				// Particle Component - TODO: Check when scene runs
				DisplayParticleComponent(dotButtonSize);

				// Script Component - TODO: Check when scripting is finished
				DisplayScriptComponent(dotButtonSize);

				// Light Component
				DisplayLightComponent(dotButtonSize);

				// Camera Component
				DisplayCameraComponent(dotButtonSize);

				// Animator Component
				DisplayAnimatorComponent(dotButtonSize);

				// Sprite Renderer Component
				DisplaySpriteRendererComponent(dotButtonSize);

				// Add Component Button
				AddComponent();

				// Animator Window - TODO: Move to Editor update
				AnimatorWindow();

			}
			else { // If no entity is selected

				ImGui::Text("No entity selected");
			}
		}

		ImGui::End();

	}

	// Tag Component
	void EditorPropertyPanel::DisplayTagComponent() {
		// Display entity name (TagComponent)
		if (m_SelectedEntity.HasComponent<TagComponent>())
		{
			auto& tag = m_SelectedEntity.GetComponent<TagComponent>();
			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::Tag);
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));
			if (isComponentOverridden)
				ImGui::PopStyleColor();

			char buffer[256];
			strncpy_s(buffer, sizeof(buffer), tag.Tag.c_str(), _TRUNCATE);
			if (ImGui::InputText("Name", buffer, sizeof(buffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string newTag = buffer;
				newTag.erase(newTag.find_last_not_of(" \t\n\r\f\v") + 1);
				newTag.erase(0, newTag.find_first_not_of(" \t\n\r\f\v"));

				if (!newTag.empty())
				{
					tag.Tag = newTag;
					MarkComponentOverridden(ComponentTypeID::Tag, "Tag");

				}

			}
		}
	}

	// Prefab Component - TODO
#if 1
	void EditorPropertyPanel::DisplayPrefabComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);
			bool openPrefabComp = ImGui::CollapsingHeader("Prefab Component", ImGuiTreeNodeFlags_DefaultOpen);

			bool removePrefabComp = false;
			ImGui::NextColumn();

			if (ImGui::Button("...###PrefabBtn", buttonSize)) {
				ImGui::OpenPopup("PrefabPopUp");
			}
			if (ImGui::BeginPopup("PrefabPopUp")) {
				if (ImGui::MenuItem("Remove Component")) {
					removePrefabComp = true;
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openPrefabComp) {
				auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

				ImGui::SeparatorText("Prefab Info");
				ImGui::Text("Component GUID: %llu", prefabComp.ComponentGUID.m_Value);
				ImGui::Text("Prefab Asset GUID: %llu", prefabComp.PrefabAssetGuid.m_Value);
				ImGui::Text("Prefab Name: %s", prefabComp.prefabName.c_str());
				ImGui::Text("Prefab Version: %u", prefabComp.prefabVersion);

				ImGui::Separator();
				ImGui::SeparatorText("Hierarchy");
				ImGui::Text("Is Prefab Root: %s", prefabComp.isPrefabRoot ? "Yes" : "No");
				ImGui::Text("Is Nested Prefab: %s", prefabComp.isNestedPrefab ? "Yes" : "No");

				if (prefabComp.isNestedPrefab) {
					ImGui::Text("Parent Prefab GUID: %llu", prefabComp.parentPrefabGuid.m_Value);
				}

				if (!prefabComp.childEntityIDs.empty()) {
					ImGui::SeparatorText("Child Entities");
					if (ImGui::BeginChild("ChildEntitiesRegion", ImVec2(0, 100), true)) {
						for (u32 childID : prefabComp.childEntityIDs) {
							Entity childEntity(static_cast<entt::entity>(childID), &m_Scene->GetRegistry());
							if (childEntity.HasComponent<TagComponent>()) {
								auto& tag = childEntity.GetComponent<TagComponent>();
								ImGui::Text("  %s (ID: %u)", tag.Tag.c_str(), childID);
							}
							else {
								ImGui::Text("  Entity ID: %u", childID);
							}
						}
					}
					ImGui::EndChild();
				}

				ImGui::Separator();
				ImGui::SeparatorText("Overrides");

				// DEBUG: Show override count
				ImGui::Text("Total Overrides Tracked: %zu", prefabComp.componentOverrides.size());

				if (prefabComp.HasOverrides()) {
					if (ImGui::TreeNode("Component Overrides")) {
						for (const auto& override : prefabComp.componentOverrides) {
							if (override.HasOverrides()) {
								std::string componentName = ComponentSerializer::GetComponentTypeName(override.componentType);
								std::string label = componentName + "##" + std::to_string(static_cast<u32>(override.componentType));

								if (ImGui::TreeNode(label.c_str())) {
									if (override.isAddedComponent) {
										ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Component Added");
									}
									if (override.isRemovedComponent) {
										ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Component Removed");
									}

									if (!override.modifiedPropertyNames.empty()) {
										ImGui::Text("Modified Properties:");
										for (const auto& propName : override.modifiedPropertyNames) {
											ImGui::BulletText("%s", propName.c_str());
										}
									}

									ImGui::TreePop();
								}
							}
						}
						ImGui::TreePop();
					}
					ImGui::Separator();
				}
				else {
					ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "No overrides");
				}
				
				if (prefabComp.isPrefabRoot)
				{
					ImGui::Separator();
					if (ImGui::Button("Revert All Overrides", ImVec2(200, 0))) {
						if (PrefabInstantiator::RevertToPrefab(m_SelectedEntity, m_Scene)) {
							LOG_INFO("Reverted all overrides successfully");
						}
						else {
							LOG_ERROR("Failed to revert overrides");
						}
					}
					ImGui::Spacing();
					if (ImGui::Button("Apply Overrides to Prefab", ImVec2(200, 0))) {
						if (PrefabInstantiator::ApplyOverridesToPrefab(m_SelectedEntity, m_Scene)) {
							LOG_INFO("Applied overrides to prefab successfully");
						}
						else {
							LOG_ERROR("Failed to apply overrides to prefab");
						}
					}
					// Warning text
					ImGui::Spacing();
					ImGui::TextColored(ImVec4(1.0f, 0.6f, 0.0f, 1.0f),
						"Warning: Apply will permanently modify the prefab asset!");
				}
				else {
					// Show message for child entities
					ImGui::Separator();
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
						"Prefab operations only available on root entity");
					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
						"Select the prefab root to modify overrides");
				}
			}
			if (removePrefabComp) {
				m_SelectedEntity.RemoveComponent<PrefabComponent>();
			}
		}
	}
#endif

#if 1
	void EditorPropertyPanel::DisplayTransformComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<TransformComponent>()) {

			// Check if component is overridden
			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::Transform);

			// Visual indicator
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));

			bool headerOpen = ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen);

			if (isComponentOverridden)
				ImGui::PopStyleColor();

			if (headerOpen) {
				auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();

				// Position
				glm::vec3 position = transform.Position;
				if (ImGui::DragFloat3("Position", &position.x, 0.1f)) {
					transform.SetPosition(position);
					MarkComponentOverridden(ComponentTypeID::Transform);  // REQUIRED!
				}

				// Rotation (in degrees)
				glm::vec3 rotation = glm::degrees(glm::eulerAngles(transform.Rotation));
				if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f)) {
					transform.SetRotation(rotation);
					MarkComponentOverridden(ComponentTypeID::Transform);  // REQUIRED!
				}

				// Scale
				glm::vec3 scale = transform.Scale;
				if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.001f)) {
					transform.SetScale(scale);
					MarkComponentOverridden(ComponentTypeID::Transform);  // REQUIRED!
				}

				u32 parent_id = transform.Parent;
				if (parent_id != u32_max) {
					if (ImGui::InputScalar("Parent", ImGuiDataType_U32, &parent_id)) {
						if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
							auto& oldPrefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

							if (!oldPrefabComp.isPrefabRoot) {
								// This entity is a prefab child - remove it from old parent's tracking
								Entity oldParent(static_cast<entt::entity>(transform.Parent),
									&m_Scene->GetRegistry());

								if (oldParent && oldParent.HasComponent<PrefabComponent>()) {
									auto& oldParentPrefabComp = oldParent.GetComponent<PrefabComponent>();
									u32 entityHandle = static_cast<u32>(m_SelectedEntity.GetHandle());

									auto it = std::find(oldParentPrefabComp.childEntityIDs.begin(),
										oldParentPrefabComp.childEntityIDs.end(),
										entityHandle);
									if (it != oldParentPrefabComp.childEntityIDs.end()) {
										oldParentPrefabComp.childEntityIDs.erase(it);
										LOG_INFO("Removed entity from parent's childEntityIDs");
									}
								}
							}
						}
						TransformSystem::SetParent(m_Scene, m_SelectedEntity, static_cast<entt::entity>(parent_id));
						MarkComponentOverridden(ComponentTypeID::Transform);  
					}
					ImGui::Text("Parent: %zu", parent_id);
				}
				else {
					ImGui::Text("Parent: None");
				}
			}
		}
	}
#endif
	// RigidBody Component
	void EditorPropertyPanel::DisplayRigidBodyComponent(ImVec2& buttonSize){
		if (m_SelectedEntity.HasComponent<RigidbodyComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::RigidBody);
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));
			// col 1: RigidBody component header
			bool openRigidBody = ImGui::CollapsingHeader("Rigid Body", ImGuiTreeNodeFlags_DefaultOpen);

			if (isComponentOverridden)
				ImGui::PopStyleColor();

			bool removeRigidBody = false; // for remove part
			// col2: ...
			ImGui::NextColumn();
			if (ImGui::Button("...###RigidbodyBtn", buttonSize))
			{
				ImGui::OpenPopup("RigidBodyPopUp");
			}
			if (ImGui::BeginPopup("RigidBodyPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeRigidBody = true;
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Store original state BEFORE removal
						std::string originalJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::RigidBody);

						// Mark as removed
						prefabComp.MarkComponentRemoved(ComponentTypeID::RigidBody, originalJSON);
						LOG_INFO("Marked RigidBody as REMOVED override");
					}
				}
			
				ImGui::EndPopup();
			}
			ImGui::Columns(1);
			if (openRigidBody)
			{
				auto& rigidBody = m_SelectedEntity.GetComponent<RigidbodyComponent>();

				// Mass
				float rigidMass = rigidBody.GetMass();
				if (ImGui::DragFloat("Mass", &rigidMass, 0.1f, 0.0f, 1000.0f))
				{
					rigidBody.SetMass(rigidMass);
					MarkComponentOverridden(ComponentTypeID::RigidBody);
				}
				ImGui::Separator();

				// Kinematic
				ImGui::Text("Boolean to check if body is moved by code (not Physics)");
				bool isKinematic = rigidBody.IsKinematic;
				if (ImGui::Checkbox("Is Kinematic", &isKinematic))
				{
					rigidBody.SetKinematic(isKinematic);
					MarkComponentOverridden(ComponentTypeID::RigidBody);
				}
				ImGui::Separator();

				// Trigger
				ImGui::Text("If true, acts as a sensor with no collision response");
				bool isTrigger = rigidBody.IsTrigger;
				if (ImGui::Checkbox("Is Trigger", &isTrigger))
				{
					rigidBody.IsTrigger = isTrigger;
					MarkComponentOverridden(ComponentTypeID::RigidBody);
				}
				ImGui::Separator();

				// Gravity
				ImGui::Text("Whether gravity affects this body");
				bool useGravity = rigidBody.UseGravity;
				if (ImGui::Checkbox("Use Gravity", &useGravity))
				{
					rigidBody.SetGravityEnabled(useGravity);
					MarkComponentOverridden(ComponentTypeID::RigidBody);
				}
				ImGui::Separator();

				// Velocity
				glm::vec3 vel = rigidBody.GetVelocity();
				if (ImGui::DragFloat3("Velocity", &vel.x, 0.1f))
				{
					rigidBody.SetVelocity(vel);
					MarkComponentOverridden(ComponentTypeID::RigidBody);
				}

				// Angular Velocity
				glm::vec3 angVel = rigidBody.AngularVelocity;
				if (ImGui::DragFloat3("Angular Velocity", &angVel.x, 0.1f))
				{
					rigidBody.AngularVelocity = angVel;
					MarkComponentOverridden(ComponentTypeID::RigidBody);
				}

				if (ImGui::Button("Stop"))
				{
					rigidBody.Stop();
					MarkComponentOverridden(ComponentTypeID::RigidBody);
				}
				ImGui::Separator();

				// Linear Damping
				float linearDamping = rigidBody.LinearDamping;
				if (ImGui::DragFloat("Linear Damping", &linearDamping, 0.01f, 0.0f, 1.0f))
				{
					rigidBody.LinearDamping = linearDamping;
					MarkComponentOverridden(ComponentTypeID::RigidBody);
				}

				// Angular Damping
				float angularDamping = rigidBody.AngularDamping;
				if (ImGui::DragFloat("Angular Damping", &angularDamping, 0.01f, 0.0f, 1.0f))
				{
					rigidBody.AngularDamping = angularDamping;
					MarkComponentOverridden(ComponentTypeID::RigidBody);
				}

				// Restitution
				float restitution = rigidBody.Restitution;
				if (ImGui::DragFloat("Restitution", &restitution, 0.01f, 0.0f, 1.0f))
				{
					rigidBody.Restitution = restitution;
					MarkComponentOverridden(ComponentTypeID::RigidBody);
				}
				ImGui::Separator();

				// Collider Shape
				ColliderType colliderShape = rigidBody.Shape;
				if (ImGui::BeginCombo("Collider Shape", ColliderTypeToString(colliderShape)))
				{
					for (int i = 0; i < 4; ++i)
					{
						ColliderType type = (ColliderType)i;
						bool selected = (colliderShape == type);
						if (ImGui::Selectable(ColliderTypeToString(type), selected))
						{
							rigidBody.Shape = type;
							MarkComponentOverridden(ComponentTypeID::RigidBody);
						}
						if (selected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				// Collider-specific properties
				switch (rigidBody.Shape)
				{
				case ColliderType::AABB:
					ImGui::Text("AABB is automatically generated from the mesh.");
					break;
				case ColliderType::BOX:
					ImGui::Text("Box Properties");
					if (ImGui::DragFloat3("Box Half Extents", &rigidBody.BoxHalfExtents.x, 0.1f, 0.01f, 100.0f))
					{
						MarkComponentOverridden(ComponentTypeID::RigidBody);
					}
					break;
				case ColliderType::SPHERE:
					ImGui::Text("Sphere Properties");
					ImGui::Text("Sphere radius is originally determined from the mesh.");
					if (ImGui::DragFloat("Sphere Radius", &rigidBody.SphereRadius, 0.1f, 0.01f, 100.0f))
					{
						MarkComponentOverridden(ComponentTypeID::RigidBody);
					}
					break;
				case ColliderType::MESH:
					ImGui::Text("Mesh collider is generated directly from the mesh.");
					break;
				default:
					break;
				}
				ImGui::Separator();

				// Runtime Display Values (Read-only)
				ImGui::Text("Display Runtime Values:");
				ImGui::BeginDisabled();
				float speed = rigidBody.GetSpeed();
				ImGui::InputFloat("Speed (m/s)", &speed, 0.0f, 0.0f, "%.2f", ImGuiInputTextFlags_ReadOnly);
				bool isMoving = rigidBody.IsMoving();
				ImGui::Checkbox("Is Moving", &isMoving);
				bool isStatic = rigidBody.IsStatic();
				ImGui::Checkbox("Is Static", &isStatic);
				ImGui::EndDisabled();
			}

			// Remove Rigid Body Component
			if (removeRigidBody)
			{
				m_SelectedEntity.RemoveComponent<RigidbodyComponent>();
			}
		}
	}

#if 1
	void EditorPropertyPanel::DisplayMeshRendererComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<MeshRendererComponent>()) {
			ImGui::Separator();

			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			// Check if this component is overridden
			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::MeshRenderer);
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));
			bool openMeshComponent = ImGui::CollapsingHeader("Mesh Component", ImGuiTreeNodeFlags_DefaultOpen);
			if (isComponentOverridden)
				ImGui::PopStyleColor();
			bool removeMesh = false;

			// col2: ...
			ImGui::NextColumn();

			if (ImGui::Button("... ###MeshBtn", buttonSize)) {
				ImGui::OpenPopup("MeshPopUp");
			}
			if (ImGui::BeginPopup("MeshPopUp")) {
				if (ImGui::MenuItem("Remove Component")) {
					removeMesh = true;
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Store original state BEFORE removal
						std::string originalJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::MeshRenderer);

						// Mark as removed
						prefabComp.MarkComponentRemoved(ComponentTypeID::MeshRenderer, originalJSON);
						LOG_INFO("Marked RigidBody as REMOVED override");
					}
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openMeshComponent) {
				auto& mesh = m_SelectedEntity.GetComponent<MeshRendererComponent>();

				// ======================= Asset Reference Section =======================
				ImGui::SeparatorText("Asset References");

				static bool showWrongType = false;

				// Display asset reference fields
				DisplayAssetField("Mesh", mesh.MeshGuid, ResourceType::MESH, showWrongType, ComponentTypeID::MeshRenderer);
				DisplayAssetField("Material", mesh.MaterialGuid, ResourceType::MATERIAL, showWrongType, ComponentTypeID::MeshRenderer);

				if (showWrongType) {
					ImGui::OpenPopup("Incompatible Asset Type");
					showWrongType = false;
				}

				if (ImGui::BeginPopup("Incompatible Asset Type")) {
					ImGui::Text("The dropped asset type does not match the expected type.");
					if (ImGui::Button("Close")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				bool visible = mesh.Visible;
				if (ImGui::Checkbox("Visible", &visible)) {
					mesh.Visible = visible;
					MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
				}

				ImGui::Spacing();

				ImGui::SeparatorText("Lighting Properties");

				bool globalIlluminate = mesh.GlobalIlluminate;
				if (ImGui::Checkbox("Global Illuminate", &globalIlluminate)) {
					mesh.GlobalIlluminate = globalIlluminate;
					MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
				}

				bool shadowReceive = mesh.ShadowReceive;
				if (ImGui::Checkbox("Shadow Receive", &shadowReceive)) {
					mesh.ShadowReceive = shadowReceive;
					MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
				}

				//ShadowCastType castType = mesh.CastType;
				// --- Shadow Cast Type Dropdown ---
				const char* shadowCastTypes[] = { "Off", "On", "Two-Sided", "Shadows Only"};
				int currentCastType = static_cast<int>(mesh.CastType);

				if (ImGui::Combo("Cast Type", &currentCastType, shadowCastTypes, IM_ARRAYSIZE(shadowCastTypes)))
				{
					mesh.CastType = static_cast<ShadowCastType>(currentCastType);
					MarkComponentOverridden(ComponentTypeID::Light);
				}

				// Material Editor Section
				ImGui::SeparatorText("Material Properties");

				// Save Material Section

// Overwrite Current Material Button
				if (ImGui::Button("Overwrite Current Material")) {
					MaterialResource* material = RM.loadResource<MaterialResource>(convertToMaterialGuid(mesh.MaterialGuid));
					if (material) {
						std::string currentMaterialName = AM.getNameFromGuid(mesh.MaterialGuid);
						if (!currentMaterialName.empty()) {
							ImGui::OpenPopup("Confirm Overwrite");
						}
						else {
							ImGui::OpenPopup("No Material Selected");
						}
					}
				}

				// Save As New Material Button (on new row)
				if (ImGui::Button("Save As New Material")) {
					ImGui::OpenPopup("Save As New Material");
				}

				// --- Popups ---

				// Confirm Overwrite Popup
				if (ImGui::BeginPopupModal("Confirm Overwrite", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("Are you sure you want to overwrite the current material?");
					ImGui::Separator();

					if (ImGui::Button("Yes", ImVec2(120, 0))) {
						MaterialResource* material = RM.loadResource<MaterialResource>(convertToMaterialGuid(mesh.MaterialGuid));
						if (material) {
							std::string currentMaterialName = AM.getNameFromGuid(mesh.MaterialGuid);
							serializeMaterial(material, currentMaterialName);
							AM.scanAndProcess();
							ImGui::CloseCurrentPopup();
							ImGui::OpenPopup("Material Saved");
						}
					}

					ImGui::SameLine();

					if (ImGui::Button("No", ImVec2(120, 0))) {
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

				// Save As New Material Popup
				static char saveAsName[256] = "";
				if (ImGui::BeginPopupModal("Save As New Material", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("Enter a name for the new material:");
					ImGui::InputText("##SaveAsName", saveAsName, sizeof(saveAsName));
					ImGui::Separator();

					if (ImGui::Button("Save", ImVec2(120, 0))) {
						if (strlen(saveAsName) > 0) {
							MaterialResource* material = RM.loadResource<MaterialResource>(convertToMaterialGuid(mesh.MaterialGuid));
							if (material) {
								std::string filename = std::string(saveAsName);
								serializeMaterial(material, filename);
								AM.scanAndProcess();
								memset(saveAsName, 0, sizeof(saveAsName));
								ImGui::CloseCurrentPopup();
								ImGui::OpenPopup("Material Saved");
							}
						}
						else {
							ImGui::OpenPopup("Invalid Name");
						}
					}

					ImGui::SameLine();

					if (ImGui::Button("Cancel", ImVec2(120, 0))) {
						memset(saveAsName, 0, sizeof(saveAsName));
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}

				// Material Saved Popup
				if (ImGui::BeginPopupModal("Material Saved", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("Material saved successfully!");
					if (ImGui::Button("OK")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				// Invalid Name Popup
				if (ImGui::BeginPopupModal("Invalid Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("Please enter a valid material name.");
					if (ImGui::Button("OK")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				// No Material Selected Popup
				if (ImGui::BeginPopupModal("No Material Selected", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
					ImGui::Text("No material currently selected to overwrite.");
					if (ImGui::Button("OK")) {
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				// Get material reference
				MaterialResource* material = RM.loadResource<MaterialResource>(convertToMaterialGuid(mesh.MaterialGuid));

				if (material) {
					ImGui::Text("Shader: %s", material->shaderName.c_str());

					if (ImGui::CollapsingHeader("Texture Maps")) {
						DisplayAssetField("Base Map (Albedo)", material->baseMap, ResourceType::TEXTURE, showWrongType, ComponentTypeID::MeshRenderer);
						DisplayAssetField("Normal Map", material->normalMap, ResourceType::TEXTURE, showWrongType, ComponentTypeID::MeshRenderer);
						DisplayAssetField("Metallic Map [NOT AVAILABLE]", material->metallicMap, ResourceType::TEXTURE, showWrongType, ComponentTypeID::MeshRenderer);
						DisplayAssetField("Roughness Map [NOT AVAILABLE]", material->roughnessMap, ResourceType::TEXTURE, showWrongType, ComponentTypeID::MeshRenderer);
						DisplayAssetField("Emission Map [NOT AVAILABLE]", material->emissionMap, ResourceType::TEXTURE, showWrongType, ComponentTypeID::MeshRenderer);
						DisplayAssetField("Occlusion Map [NOT AVAILABLE]", material->occlusionMap, ResourceType::TEXTURE, showWrongType, ComponentTypeID::MeshRenderer);
					}

					if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen)) {
						if (ImGui::ColorEdit3("Base Color", material->baseColor.data(),
							ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB)) {
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
						}

						if (ImGui::ColorEdit3("Emission Color", material->emissionColor.data(),
							ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB)) {
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
						}
					}

					if (ImGui::CollapsingHeader("Material Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
						if (ImGui::SliderFloat("Metallic", &material->metallic, 0.0f, 1.0f, "%.2f")) {
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
						}

						if (ImGui::SliderFloat("Roughness", &material->roughness, 0.0f, 1.0f, "%.2f")) {
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
						}

						if (ImGui::SliderFloat("Opacity", &material->opacity, 0.0f, 1.0f, "%.2f")) {
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
						}

						if (ImGui::SliderFloat("Emission Strength", &material->emissionStrength, 0.0f, 100.0f, "%.2f")) {
							material->emissionStrength = std::max(0.0f, material->emissionStrength);
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
						}

						if (ImGui::SliderFloat("Alpha Threshold", &material->alphaThreshold, 0.0f, 1.0f, "%.3f")) {
							material->alphaThreshold = std::max(0.0f, std::min(1.0f, material->alphaThreshold));
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
						}

						if (ImGui::SliderFloat("Ambient Occlusion", &material->ambientOcclusion, 0.0f, 1.0f, "%.3f")) {
							material->ambientOcclusion = std::max(0.0f, std::min(1.0f, material->ambientOcclusion));
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
						}
					}

					if (ImGui::CollapsingHeader("UV Transform")) {
						if (ImGui::DragFloat2("Tiling", material->tiling.data(), 0.1f, 0.1f, 10.0f, "%.2f")) {
							material->tiling[0] = std::max(0.1f, material->tiling[0]);
							material->tiling[1] = std::max(0.1f, material->tiling[1]);
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
						}

						if (ImGui::DragFloat2("Offset", material->offset.data(), 0.01f, -10.0f, 10.0f, "%.3f")) {
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
						}
					}

					if (ImGui::CollapsingHeader("Render Flags")) {
						if (ImGui::Checkbox("Enable Emission", &material->enableEmission)) {
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);
						}
						if (ImGui::Checkbox("Alpha Test", &material->alphaTest)) {
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);
						}
						if (ImGui::Checkbox("Double Sided", &material->doubleSided)) {
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);
						}
						if (ImGui::Checkbox("Receive Shadows", &material->receiveShadows)) {
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);
						}
						if (ImGui::Checkbox("Cast Shadows", &material->castShadows)) {
							MarkComponentOverridden(ComponentTypeID::MeshRenderer);
						}
					}
				}

				ImGui::SeparatorText("Values for Debugging:");
				ImGui::Text("Material: %u", mesh.Material);

				ImU32 meshType = mesh.MeshType;
				if (ImGui::InputScalar("Mesh Type", ImGuiDataType_U32, &meshType)) {
					if (meshType == 0 || meshType == 1 || meshType == 2) {
						mesh.MeshType = meshType;
						MarkComponentOverridden(ComponentTypeID::MeshRenderer);  // MARK AS OVERRIDDEN
					}
				}

				ImGui::Text("Submesh Index: %u", mesh.SubmeshIndex);
				ImGui::Text("Texture: %u", mesh.Texture);
			}

			if (removeMesh) {
				m_SelectedEntity.RemoveComponent<MeshRendererComponent>();
			}
		}
	}
#endif

	// Audio Component
	void EditorPropertyPanel::DisplayAudioComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<AudioComponent>())
		{
			ImGui::Separator();

			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);
			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::Audio);
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));
			bool openAudioComponent = ImGui::CollapsingHeader("Audio Component", ImGuiTreeNodeFlags_DefaultOpen);
			if (isComponentOverridden)
				ImGui::PopStyleColor();
			bool removeAudio = false;

			// col2: ...
			ImGui::NextColumn();

			if (ImGui::Button("... ###AudioBtn", buttonSize))
			{
				ImGui::OpenPopup("AudioPopUp");
			}
			if (ImGui::BeginPopup("AudioPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeAudio = true;
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Store original state BEFORE removal
						std::string originalJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::Audio);

						// Mark as removed
						prefabComp.MarkComponentRemoved(ComponentTypeID::Audio, originalJSON);
						LOG_INFO("Marked RigidBody as REMOVED override");
					}
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openAudioComponent)
			{
				auto& audio = m_SelectedEntity.GetComponent<AudioComponent>();

				ImGui::Separator();

				if (audio.AudioFilePath.empty())
				{
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255)); // Red
					ImGui::Text("No audio file loaded. Please select and audio file below.");
					ImGui::PopStyleColor();
				}
				else
				{
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255)); // Green
					ImGui::Text("Audio Filename: %s", audio.AudioFilePath.c_str());
					ImGui::PopStyleColor();
				}

				// Get from Asset Manager
				auto& db = AM.db();
				auto allAssets = db.AllMutable();

				std::vector<std::string> audioAssetNames;
				audioAssetNames.reserve(allAssets.size());

				for (const auto* record : allAssets)
				{
					if (!record || !record->valid)
					{
						continue;
					}

					if (record->type == ResourceType::AUDIO)
					{
						std::string filepath = record->sourcePath;
						size_t lastSlash = filepath.find_last_of("/\\");
						std::string filename = (lastSlash == std::string::npos) ? filepath : filepath.substr(lastSlash + 1);

						audioAssetNames.push_back(filename);
					}
				}

				// Const char* for dropdown
				std::vector<const char*> audioAssets;
				audioAssets.reserve(audioAssetNames.size());
				for (auto& name : audioAssetNames)
				{
					audioAssets.push_back(name.c_str());
				}

				int currentIndex = 0;
				for (size_t i = 0; i < audioAssetNames.size(); ++i)
				{
					if (audioAssetNames[i] == audio.AudioFilePath)
					{
						currentIndex = static_cast<int>(i);
						break;
					}
				}

				// Dropdown menu
				std::string label = "Filepath";
				if (ImGui::Combo(label.c_str(), &currentIndex, audioAssets.data(), static_cast<int>(audioAssets.size())))
				{
					audio.SetAudioFile(audioAssetNames[currentIndex]);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}

				if (audio.AudioFilePath.empty())
				{
					ImGui::SameLine();
					if (ImGui::Button("Load File"))
					{
						audio.SetAudioFile(audioAssetNames[currentIndex]);
						MarkComponentOverridden(ComponentTypeID::Audio);
					}
				}

				if (audio.AudioFilePath.empty())
				{
					ImGui::BeginDisabled();
				}

				ImGui::Text("Audio Type:");
				AudioType type = audio.Type;

				if (ImGui::RadioButton("SFX", type == AudioType::SFX))
				{
					audio.SetAudioType(AudioType::SFX);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}
				if (ImGui::RadioButton("BGM", type == AudioType::BGM))
				{
					audio.SetAudioType(AudioType::BGM);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}
				if (ImGui::RadioButton("UI", type == AudioType::UI))
				{
					audio.SetAudioType(AudioType::UI);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}

				ImGui::Separator();
				ImGui::Text("Play State:");
				PlayState playState = audio.State;
				if (ImGui::RadioButton("Play", playState == PlayState::PLAY))
				{
					audio.SetState(PlayState::PLAY);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}
				if (ImGui::RadioButton("Pause", playState == PlayState::PAUSE))
				{
					audio.SetState(PlayState::PAUSE);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}
				if (ImGui::RadioButton("Stop##Audio", playState == PlayState::STOP))
				{
					audio.SetState(PlayState::STOP);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}

				ImGui::Separator();

				float volume = audio.Volume;
				if (ImGui::SliderFloat("Volume", &volume, 0.f, 1.f))
				{
					audio.SetVolume(volume);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}

				float pitch = audio.Pitch;
				if (ImGui::SliderFloat("Pitch", &pitch, 0.f, 1.f))
				{
					audio.SetPitch(pitch);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}

				ImGui::Separator();
				bool looping = audio.Loop;
				if (ImGui::Checkbox("Looping", &looping))
				{
					audio.SetLoop(looping);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}
				bool mute = audio.Mute;
				if (ImGui::Checkbox("Mute", &mute))
				{
					audio.SetMute(mute);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}
				bool is_3d = audio.Is3D;
				if (ImGui::Checkbox("3D", &is_3d))
				{
					audio.Set3D(is_3d);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}

				ImGui::Separator();
				float reverb = audio.ReverbProperties;
				if (ImGui::SliderFloat("Reverb", &reverb, 0.0f, 1.0f))
				{
					audio.SetReverbProperties(reverb);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}

				ImGui::SeparatorText("Only for 3D");

				std::string advice = "Max Distance needs to be higher than Min Distance to have attenuation";
				ImGui::TextDisabled("(i)");
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted(advice.c_str());
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}

				// Disable only if not 3D
				ImGui::BeginDisabled(!is_3d);

				float min_distance = audio.MinDistance;
				if (ImGui::SliderFloat("MinDistance", &min_distance, 0.0f, audio.MaxDistance))
				{
					if (is_3d)
					{
						audio.SetMinDistance(min_distance);
						MarkComponentOverridden(ComponentTypeID::Audio);
					}
					else
					{
						audio.SetMinDistance(1.f);
						MarkComponentOverridden(ComponentTypeID::Audio);
					}
				}

				float max_distance = audio.MaxDistance;
				if (ImGui::SliderFloat("MaxDistance", &max_distance, audio.MinDistance, 1000.f))
				{
					if (is_3d)
					{
						audio.SetMaxDistance(max_distance);
						MarkComponentOverridden(ComponentTypeID::Audio);
					}
					else
					{
						audio.SetMaxDistance(10.f);
						MarkComponentOverridden(ComponentTypeID::Audio);
					}
				}

				ImGui::EndDisabled();

				ImGui::Separator();

				float dopplerLevel = audio.DopplerLevel;
				if (ImGui::SliderFloat("Doppler", &dopplerLevel, 0.f, 5.f))
				{
					audio.SetDopplerLevel(dopplerLevel);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}

				ImGui::Text("RollOff Mode:");
				AudioRolloffMode mode = audio.RolloffMode;

				if (ImGui::RadioButton("INVERSE", mode == AudioRolloffMode::INVERSE))
				{
					audio.SetRolloffMode(AudioRolloffMode::INVERSE);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}
				if (ImGui::RadioButton("LINEAR", mode == AudioRolloffMode::LINEAR))
				{
					audio.SetRolloffMode(AudioRolloffMode::LINEAR);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}
				if (ImGui::RadioButton("LINEARSQUARE", mode == AudioRolloffMode::LINEARSQUARE))
				{
					audio.SetRolloffMode(AudioRolloffMode::LINEARSQUARE);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}

				ImGui::SeparatorText("Only for 2D");

				std::string advice2D = "Only for 2D sounds (not 3D). Controls where 2D sound is being played (From the left, right or both speakers).";
				ImGui::TextDisabled("(i)");
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();
					ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
					ImGui::TextUnformatted(advice2D.c_str());
					ImGui::PopTextWrapPos();
					ImGui::EndTooltip();
				}

				ImGui::BeginDisabled(is_3d);

				float pan = audio.Pan2D;
				if (ImGui::SliderFloat("Pan", &pan, -1.f, 1.f))
				{
					audio.SetPan(pan);
					MarkComponentOverridden(ComponentTypeID::Audio);
				}

				ImGui::EndDisabled();

				if (audio.AudioFilePath.empty())
				{
					ImGui::EndDisabled();

				}

			}
			// ---------------------- Remove Audio Component by ... -------------------------
			if (removeAudio)
			{
				m_SelectedEntity.RemoveComponent<AudioComponent>();
			}

		}
	}

	// Reverb Component
	void EditorPropertyPanel::DisplayReverbZoneComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<ReverbZoneComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);
			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::ReverbZone);
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));
			bool openReverbComponent = ImGui::CollapsingHeader("Reverb Zone Component", ImGuiTreeNodeFlags_DefaultOpen);
			if (isComponentOverridden)
				ImGui::PopStyleColor();
			bool removeReverb = false;

			// col2: ...
			ImGui::NextColumn();

			if (ImGui::Button("... ###ReverbBtn", buttonSize))
			{
				ImGui::OpenPopup("ReverbPopUp");
			}
			if (ImGui::BeginPopup("ReverbPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeReverb = true;
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Store original state BEFORE removal
						std::string originalJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::ReverbZone);

						// Mark as removed
						prefabComp.MarkComponentRemoved(ComponentTypeID::ReverbZone, originalJSON);
						LOG_INFO("Marked RigidBody as REMOVED override");
					}
				}
				
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openReverbComponent)
			{
				auto& reverbZone = m_SelectedEntity.GetComponent<ReverbZoneComponent>();

				const char* presets[] = { "Custom", "Generic", "Bathroom", "Room", "Cave", "Arena" };
				int currentIndex = static_cast<int>(reverbZone.Preset);

				ImGui::Text("Select an option:");

				// Dropdown menu
				if (ImGui::BeginCombo("Reverb Preset", presets[currentIndex]))
				{
					for (int i = 0; i < IM_ARRAYSIZE(presets); i++)
					{
						bool isSelected = (i == currentIndex);
						if (ImGui::Selectable(presets[i], isSelected))
						{
							// Update the enum when user picks a new item
							reverbZone.Preset = static_cast<ReverbPreset>(i);
							MarkComponentOverridden(ComponentTypeID::ReverbZone);
						}

						if (isSelected)
						{
							ImGui::SetItemDefaultFocus();
						}
					}
					ImGui::EndCombo();
				}

				if (reverbZone.Preset == ReverbPreset::Custom)
				{

					float& decayTime = reverbZone.DecayTime;
					if (ImGui::SliderFloat("Decay Time", &decayTime, 100.f, 20000.f))
					{
						reverbZone.SetDecayTime(decayTime);
						MarkComponentOverridden(ComponentTypeID::ReverbZone);
					}

					float& hfDecayRatio = reverbZone.HfDecayRatio;
					if (ImGui::SliderFloat("High-Frequency Decay Ratio", &hfDecayRatio, 0.f, 100.f))
					{
						reverbZone.SetHfDecayRatio(hfDecayRatio);
						MarkComponentOverridden(ComponentTypeID::ReverbZone);
					}

					float& diffusion = reverbZone.Diffusion;
					if (ImGui::SliderFloat("Diffusion", &diffusion, 0.f, 100.f))
					{
						reverbZone.SetDiffusion(diffusion);
						MarkComponentOverridden(ComponentTypeID::ReverbZone);
					}

					float& density = reverbZone.Density;
					if (ImGui::SliderFloat("Density", &density, 0.f, 100.f))
					{
						reverbZone.SetDensity(density);
						MarkComponentOverridden(ComponentTypeID::ReverbZone);
					}

					float& wetLevel = reverbZone.WetLevel;
					if (ImGui::SliderFloat("Wet Level", &wetLevel, -80.f, 20.f))
					{
						reverbZone.SetWetLevel(wetLevel);
						MarkComponentOverridden(ComponentTypeID::ReverbZone);
					}
				}

				float& minDistanceReverb = reverbZone.MinDistance;
				if (ImGui::InputFloat("MinDistance###minreverb", &minDistanceReverb))
				{
					reverbZone.SetMinDistance(minDistanceReverb);
					MarkComponentOverridden(ComponentTypeID::ReverbZone);
				}

				float& maxDistanceReverb = reverbZone.MaxDistance;
				if (ImGui::InputFloat("MaxDistance###maxreverb", &maxDistanceReverb))
				{
					reverbZone.SetMaxDistance(maxDistanceReverb);
					MarkComponentOverridden(ComponentTypeID::ReverbZone);
				}
			}
			//---------------------- Remove ReverbZone Component by ... -------------------------
			if (removeReverb)
			{
				m_SelectedEntity.RemoveComponent<ReverbZoneComponent>();

			}
		}
	}

	// Listener Component
	void EditorPropertyPanel::DisplayListenerComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<ListenerComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);
			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::Listerner);
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));
			bool openListenerComponent = ImGui::CollapsingHeader("Listener Component", ImGuiTreeNodeFlags_DefaultOpen);
			if (isComponentOverridden)
				ImGui::PopStyleColor();
			bool removeListener = false;

			// col2: ...
			ImGui::NextColumn();

			if (ImGui::Button("... ###ListenBtn", buttonSize))
			{
				ImGui::OpenPopup("ListenPopUp");
			}
			if (ImGui::BeginPopup("ListenPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeListener = true;
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Store original state BEFORE removal
						std::string originalJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::Listerner);

						// Mark as removed
						prefabComp.MarkComponentRemoved(ComponentTypeID::Listerner, originalJSON);
						LOG_INFO("Marked RigidBody as REMOVED override");
					}
				}
				
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openListenerComponent)
			{
				auto& listener = m_SelectedEntity.GetComponent<ListenerComponent>();
				bool& active = listener.Active;

				if (ImGui::Checkbox("Active###activeListener", &active))
				{
					listener.Active = active;
					MarkComponentOverridden(ComponentTypeID::Listerner);
				}
			}
			// -------------------------- Remove ListernerComponent -------------------------
			if (removeListener)
			{
				m_SelectedEntity.RemoveComponent<ListenerComponent>();

			}
		}
	}

	// BT Component
	void EditorPropertyPanel::DisplayBTComponent(ImVec2& buttonSize){
		if (m_SelectedEntity.HasComponent<BehaviourTreeComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool openBTComponent = ImGui::CollapsingHeader("Behaviour Tree Component", ImGuiTreeNodeFlags_DefaultOpen);

			bool removeBTComponent = false;

			ImGui::NextColumn();

			if (ImGui::Button("...###BTBtn", buttonSize))
			{
				ImGui::OpenPopup("BTPopUp");
			}
			if (ImGui::BeginPopup("BTPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeBTComponent = true;
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openBTComponent)
			{
				auto& ai_bt = m_SelectedEntity.GetComponent<BehaviourTreeComponent>();

				// Getting BT itself
				BehaviourTree& treeInstance = *(ai_bt.TreeInstance);
				if (ai_bt.TreeInstance)
				{
					size_t stackDepth = treeInstance.GetStackDepth();
					auto root = treeInstance.GetRootNode();

					char treeBuffer[256];
					strncpy_s(treeBuffer, sizeof(treeBuffer), treeInstance.GetName().c_str(), _TRUNCATE);
					if (ImGui::InputText("Tree", treeBuffer, sizeof(treeBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
					{
						std::string newName = treeBuffer;
						newName.erase(newName.find_last_not_of(" \t\n\r\f\v") + 1);
						newName.erase(0, newName.find_first_not_of(" \t\n\r\f\v"));

						if (!newName.empty())
						{
							treeInstance.SetName(newName);
							ai_bt.TreeAssetPath = newName + ".json";
						}

					}

					ImGui::Text("Current Asset Path: %s", ai_bt.TreeAssetPath.c_str());
					ImGui::Text("Stack Depth: %zu", stackDepth);

					if (root)
					{
						ImGui::Text("Current Root: %s [%s]", root->GetName().c_str(), root->GetTypeName());
						DrawBTNodeEditor(root);
					}
					else
					{
						ImGui::TextDisabled("No root node.");
						if (ImGui::Button("Create Root Node"))
						{
							auto rootNode = BehaviourTreeEditor::CreateNode("Selector");
							treeInstance.SetRootNode(rootNode);
						}
					}

					// --- Set Root Node ---
					ImGui::Separator();
					ImGui::Text("Root Node:");

					static int rootNodeTypeIndex = 0;
					auto allTypes = BehaviourTreeEditor::GetNodeTypesByCategory("Composite");
					ImGui::SetNextItemWidth(200.0f);
					if (ImGui::Combo("Node Type##Root", &rootNodeTypeIndex,
						[](void* data, int idx, const char** outText) -> bool
						{
							auto& types = *static_cast<std::vector<std::string>*>(data);
							*outText = types[idx].c_str();
							return true;
						},
						static_cast<void*>(&allTypes), (int)allTypes.size()))
					{
						;
					}

					// Button to create/set the root node
					if (ImGui::Button("Set Root Node"))
					{
						auto newRoot = BehaviourTreeEditor::CreateNode(allTypes[rootNodeTypeIndex]);
						treeInstance.SetRootNode(newRoot);
					}

					ImGui::Separator();

					// Reset the tree to initial state
					if (ImGui::Button("Reset"))
					{
						ai_bt.Reset();
					}

					ImGui::BeginDisabled();

					// Last execution status
					BTStatus& lastStatus = ai_bt.LastStatus;
					std::string lastStatusString{};
					if (lastStatus == BTStatus::Success)
					{
						lastStatusString = "Success";
					}
					else if (lastStatus == BTStatus::Failure)
					{
						lastStatusString = "Failure";
					}
					else
					{
						lastStatusString = "Running";
					}
					ImGui::Text("Execution Status: %s", lastStatusString.c_str());

					ImGui::EndDisabled();

					// Whether tree executes every frame
					bool& active = ai_bt.Active;
					if (ImGui::Checkbox("Active###activeBT", &active))
					{
						ai_bt.Active = active;
					}

					// Reset the tree when it completes
					bool& resetComplete = ai_bt.ResetOnComplete;
					if (ImGui::Checkbox("Reset On Complete", &resetComplete))
					{
						ai_bt.ResetOnComplete = resetComplete;
					}

					// Reference to current asset path
					std::string& treeAssetPath = ai_bt.TreeAssetPath;

					// Find BT folder

					std::filesystem::path repoRoot = getRepository();
					std::filesystem::path btPath = repoRoot / "Resources" / "Sources";

					auto folders = m_Editor->getAssetsInFolder(btPath.string());
					std::string btFolderPath;
					for (auto& folder : folders)
					{
						if (folder.name == "BT")
						{
							btFolderPath = folder.fullPath;
							break;
						}
					}

					// Collect all JSON assets in BT folder
					std::vector<Editor::AssetEntry> btAssets;
					if (!btFolderPath.empty())
					{
						auto files = m_Editor->getAssetsInFolder(btFolderPath);
						for (auto& f : files)
						{
							if (f.name.size() >= 5 && f.name.substr(f.name.size() - 5) == ".json")
								btAssets.push_back(f);
						}
					}

					// Determine current selection index
					int currentIndex = 0;
					for (size_t i = 0; i < btAssets.size(); ++i)
					{
						if (btAssets[i].name == treeAssetPath)
						{
							currentIndex = (int)i;
							break;
						}
					}

					// Draw the combo box
					ImGui::Text("Choose Tree Asset Path:");
					ImGui::SetNextItemWidth(400.0f);
					if (ImGui::Combo("##TreeAssetPath", &currentIndex,
						[](void* data, int idx, const char** outText) -> bool
						{
							auto& assets = *static_cast<std::vector<Editor::AssetEntry>*>(data);
							*outText = assets[idx].name.c_str();

							return true;
						},
						static_cast<void*>(&btAssets), (int)btAssets.size()))
					{
						if (currentIndex >= 0 && currentIndex < (int)btAssets.size())
						{
							treeAssetPath = btAssets[currentIndex].name;
						}
					}

					if (ImGui::Button("Load Tree"))
					{
						if (currentIndex >= 0 && currentIndex < (int)btAssets.size())
						{
							std::string chosenPath = btAssets[currentIndex].name;
							ai_bt.TreeInstance = BehaviourTreeEditor::LoadTree(chosenPath);
						}
					}

					if (ImGui::Button("Save Tree"))
					{
						BehaviourTreeEditor::SaveTree(treeInstance, ai_bt.TreeAssetPath);
					}

					static char changeNewNameBuffer[256] = "";
					static char saveNewFileName[256] = "";  // Changed from saveNewTreePath - clearer naming
					static char saveNewTreeName[256] = "";

					if (ImGui::Button("Rename Tree File"))
					{
						strncpy_s(changeNewNameBuffer, sizeof(changeNewNameBuffer), ai_bt.TreeAssetPath.c_str(), _TRUNCATE);
						ImGui::OpenPopup("TreeRename Panel");
					}

					if (ImGui::Button("Save Tree File As"))
					{
						strncpy_s(saveNewFileName, sizeof(saveNewFileName), ai_bt.TreeAssetPath.c_str(), _TRUNCATE);
						strncpy_s(saveNewTreeName, sizeof(saveNewTreeName), treeInstance.GetName().c_str(), _TRUNCATE);
						ImGui::OpenPopup("SaveTreeRename Panel");
					}

					// Rename Tree Panel
					if (ImGui::BeginPopupModal("TreeRename Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						ImGui::Text("Current file: %s", ai_bt.TreeAssetPath.c_str());
						ImGui::Separator();

						ImGui::Text("Enter file name ('.json' will be added automatically):");
						ImGui::InputText("New Tree Filename", changeNewNameBuffer, sizeof(changeNewNameBuffer));

						if (ImGui::Button("Rename File", ImVec2(120, 0)))
						{
							std::string newFileName = changeNewNameBuffer;
							if (!newFileName.empty())
							{
								std::string saveTreeName = newFileName + ".json";
								BehaviourTreeEditor::RenameFile(ai_bt.TreeAssetPath, saveTreeName, m_Scene);
								ImGui::CloseCurrentPopup();
							}
						}

						ImGui::SameLine();

						if (ImGui::Button("Cancel###RenameCancel", ImVec2(120, 0)))
						{  // Fixed ID
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

					// Save As Panel
					if (ImGui::BeginPopupModal("SaveTreeRename Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
					{
						ImGui::Text("Save tree as new file");
						ImGui::Separator();

						ImGui::Text("Current Asset Path: %s", ai_bt.TreeAssetPath.c_str());
						ImGui::Text("Enter file name ('.json' will be added automatically):");
						ImGui::InputText("New Filename", saveNewFileName, sizeof(saveNewFileName));
						ImGui::InputText("New Tree Name", saveNewTreeName, sizeof(saveNewTreeName));

						if (ImGui::Button("Save As Tree File", ImVec2(120, 0)))
						{
							std::string newSaveFileName = saveNewFileName;
							std::string newSaveTreeName = saveNewTreeName;
							if (!newSaveFileName.empty() && !newSaveTreeName.empty())
							{
								std::string saveFileName = newSaveFileName + ".json";
								BehaviourTreeEditor::SaveAs(ai_bt.TreeAssetPath, saveFileName, newSaveTreeName, true);
								ImGui::CloseCurrentPopup();
							}
						}

						ImGui::SameLine();

						if (ImGui::Button("Cancel###SaveAsCancel", ImVec2(120, 0)))
						{  // Fixed ID
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

				}
				else
				{
					ai_bt.TreeInstance = BehaviourTreeEditor::CreateNewTree("PlaceholderTreeName");
				}

			}
			// ----------------------------------- Remove BT Component -----------------------
			if (removeBTComponent)
			{
				m_SelectedEntity.RemoveComponent<BehaviourTreeComponent>();
			}
		}
	}

	// Particle Component
	void EditorPropertyPanel::DisplayParticleComponent(ImVec2& buttonSize){
		if (m_SelectedEntity.HasComponent<ParticleComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);
			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::ParticleSystem);
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));
			// Visual indicator
			
			bool openParticleComp = ImGui::CollapsingHeader("Particle System", ImGuiTreeNodeFlags_DefaultOpen);
			if (isComponentOverridden)
				ImGui::PopStyleColor();
			bool removeParticleComp = false;

			auto& particleComp = m_SelectedEntity.GetComponent<ParticleComponent>();

			ImGui::NextColumn();

			if (ImGui::Button("...###ParticleBtn", buttonSize))
			{
				ImGui::OpenPopup("ParticlePopUp");
			}
			if (ImGui::BeginPopup("ParticlePopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeParticleComp = true;
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Store original state BEFORE removal
						std::string originalJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::ParticleSystem);

						// Mark as removed
						prefabComp.MarkComponentRemoved(ComponentTypeID::ParticleSystem, originalJSON);
						LOG_INFO("Marked Particle Component as REMOVED override");
					}
					//return;
				}
				
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openParticleComp)
			{
				auto clearpc = [](ParticleComponent& pc) { pc.Particles.clear(), pc.EmissionAccumulator = 0.f; pc.DelayAccumualator = 0.f; };

				// Playback Controls
				ImGui::Text("Playback");
				if (ImGui::Button("Play###PlayParticle")) {
					if (particleComp.BurstMode)
						clearpc(particleComp);

					particleComp.Active = true;

					MarkComponentOverridden(ComponentTypeID::ParticleSystem);
				}
				ImGui::SameLine();
				if (ImGui::Button("Pause###PauseParticle")) {
					particleComp.Active = false;
					MarkComponentOverridden(ComponentTypeID::ParticleSystem);
				}
				ImGui::SameLine();
				if (ImGui::Button("Restart Animation##RestartPlaybackParticle")) {
					clearpc(particleComp);
					MarkComponentOverridden(ComponentTypeID::ParticleSystem);
				}
				ImGui::Spacing();

				if(ImGui::DragFloat("Play Delay", &particleComp.PlayDelay, 0.1f, 0.f, 60.f)) {
					MarkComponentOverridden(ComponentTypeID::ParticleSystem);
				}

				ImGui::Spacing();

				// Burst Mode Checkbox
				if (ImGui::Checkbox("Burst Mode", &particleComp.BurstMode))
				{
					MarkComponentOverridden(ComponentTypeID::ParticleSystem);
				}

				ImGui::SameLine();
				if (ImGui::Checkbox("Loop", &particleComp.Loop)) {
					MarkComponentOverridden(ComponentTypeID::ParticleSystem);
				}

				ImGui::Spacing();
				ImGui::Separator();

				// Emission Settings
				if (ImGui::TreeNodeEx("Emission", ImGuiTreeNodeFlags_DefaultOpen))
				{
					// Particle Type Dropdown
					const char* emissionShape[] = { "Point", "Box", "Sphere" };
					const char* currentShape = emissionShape[static_cast<uint32_t>(particleComp.Shape)];

					if (ImGui::BeginCombo("Emission Shape", currentShape))
					{
						for (int i = 0; i < 3; i++)
						{
							bool isSelected = (static_cast<uint32_t>(particleComp.Shape) == static_cast<uint32_t>(i));
							if (ImGui::Selectable(emissionShape[i], isSelected))
							{
								particleComp.Shape = static_cast<EmitterShape>(i);
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);
							}
							if (isSelected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::EndCombo();
					}

					// Conditional widgets based on selected shape
					if (particleComp.Shape == EmitterShape::BOX)
					{
						if (ImGui::DragFloat3("Box Size", &particleComp.EmissionBoxSize.x, 0.1f, 0.0f, 100000.0f))
						{
							MarkComponentOverridden(ComponentTypeID::ParticleSystem);
						}
					}
					else if (particleComp.Shape == EmitterShape::SPHERE)
					{
						if (ImGui::DragFloat("Sphere Radius", &particleComp.EmissionSphereRadius, 0.1f, 0.0f, 100000.0f))
						{
							MarkComponentOverridden(ComponentTypeID::ParticleSystem);
						}
					}

					if (ImGui::DragInt("Max Particles", (int*)&particleComp.MaxParticles, 1.0f, 1, 10000))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}
					if (ImGui::DragFloat("Emission Rate", &particleComp.EmissionRate, 0.1f, 0.0f, 1000.0f, "%.1f particles/sec"))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}
					if (ImGui::DragFloat("Particle Lifetime", &particleComp.ParticleLifetime, 0.1f, 0.1f, 100.0f, "%.1f seconds"))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}

					ImGui::TreePop();
				}

				// Particle Appearance
				if (ImGui::TreeNodeEx("Appearance", ImGuiTreeNodeFlags_DefaultOpen))
				{
					// Particle Type Dropdown
					const char* particleTypes[] = { "Cube", "Plane", "Sphere" };
					const char* currentType = particleTypes[particleComp.ParticleType];

					if (ImGui::BeginCombo("Particle Type", currentType))
					{
						for (int i = 0; i < 3; i++)
						{
							bool isSelected = (particleComp.ParticleType == static_cast<unsigned int>(i));
							if (ImGui::Selectable(particleTypes[i], isSelected))
							{
								particleComp.ParticleType = i;
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);
							}
							if (isSelected)
							{
								ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::EndCombo();
					}

					ImGui::Spacing();
					ImGui::Text("Color Range");
					if (ImGui::ColorEdit4("Color Min", &particleComp.ColorMin.x))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}
					if (ImGui::ColorEdit4("Color Max", &particleComp.ColorMax.x))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}

					ImGui::TreePop();
				}

				// Particle Behavior
				if (ImGui::TreeNodeEx("Behavior", ImGuiTreeNodeFlags_DefaultOpen))
				{
					if(ImGui::Checkbox("World Space##ParticleWorldSpace", &particleComp.WorldSpace)) 
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);
					}

					ImGui::Text("Size");

					if (ImGui::DragFloat3("Initial Size", &particleComp.StartSize.x, 0.1f))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}

					if (ImGui::DragFloat3("Default Size", &particleComp.DefaultSize.x, 0.1f))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}

					if (ImGui::DragFloat3("End Size", &particleComp.EndSize.x, 0.1f))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}

					if (ImGui::DragFloat("Growth Phase End", &particleComp.GrowPhaseEnd, 0.1f, 0.f, 1.f))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}

					if (ImGui::DragFloat("Shrink Phase Start", &particleComp.ShrinkPhaseStart, 0.1f, 0.f, 1.f))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}

					ImGui::Spacing();
					ImGui::Text("Velocity");
					if (ImGui::DragFloat3("Initial Velocity", &particleComp.InitialVelocity.x, 0.1f))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}
					if (ImGui::DragFloat("Min Speed", &particleComp.MinSpeed, 0.01f, 0.0f, 10.0f, "%.2f"))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}
					if (ImGui::DragFloat("Max Speed", &particleComp.MaxSpeed, 0.01f, 0.0f, 10.0f, "%.2f"))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
					}
					if (ImGui::DragFloat("Spread Angle", &particleComp.SpreadAngle, 0.5f, 0.0f, 180.0f, "%.1f degrees"))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);
					}

					ImGui::Spacing();
					ImGui::Text("Rotation");

					if (ImGui::Checkbox("Randomize Rotation", &particleComp.RandomizeRotation))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);
					}
					if (ImGui::DragFloat("Rotation Speed", &particleComp.RotationSpeed, 1.0f, -360.0f, 360.0f, "%.1f deg/sec"))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);
					}

					ImGui::TreePop();
				}

				// Randomization
				if (ImGui::TreeNodeEx("Randomization"))
				{
					if (ImGui::DragFloat("Velocity Randomness", &particleComp.VelocityRandomness, 0.01f, 0.0f, 1.0f, "%.2f"))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);
					}
					if (ImGui::DragFloat("Lifetime Randomness", &particleComp.LifetimeRandomness, 0.01f, 0.0f, 1.0f, "%.2f"))
					{
						MarkComponentOverridden(ComponentTypeID::ParticleSystem);
					}

					// Optional: Add tooltips for clarity
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("0 = no variation, 1 = maximum variation");
					}

					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Advanced"))
				{
					static bool showWrongTypeParticle = false;

					// Display asset reference fields
					DisplayAssetField("Mesh", particleComp.ParticleTypeAdvanced, ResourceType::MESH, showWrongTypeParticle, ComponentTypeID::ParticleSystem);
					DisplayAssetField("Material", particleComp.MaterialType, ResourceType::MATERIAL, showWrongTypeParticle, ComponentTypeID::ParticleSystem);

					if (showWrongTypeParticle) {
						ImGui::OpenPopup("Incompatible Asset Type");
						showWrongTypeParticle = false;
					}

					if (ImGui::BeginPopup("Incompatible Asset Type")) {
						ImGui::Text("The dropped asset type does not match the expected type.");
						if (ImGui::Button("Close")) {
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}

					// Material Editor Section
					ImGui::SeparatorText("Material Properties");

					// Save Material Section

					// Overwrite Current Material Button
					if (ImGui::Button("Overwrite Current Material")) {
						MaterialResource* material = RM.loadResource<MaterialResource>(convertToMaterialGuid(particleComp.MaterialType));
						if (material) {
							std::string currentMaterialName = AM.getNameFromGuid(particleComp.MaterialType);
							if (!currentMaterialName.empty()) {
								ImGui::OpenPopup("Confirm Overwrite");
							}
							else {
								ImGui::OpenPopup("No Material Selected");
							}
						}
					}

					// Save As New Material Button (on new row)
					if (ImGui::Button("Save As New Material")) {
						ImGui::OpenPopup("Save As New Material");
					}

					// --- Popups ---

					// Confirm Overwrite Popup
					if (ImGui::BeginPopupModal("Confirm Overwrite", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
						ImGui::Text("Are you sure you want to overwrite the current material?");
						ImGui::Separator();

						if (ImGui::Button("Yes", ImVec2(120, 0))) {
							MaterialResource* material = RM.loadResource<MaterialResource>(convertToMaterialGuid(particleComp.MaterialType));
							if (material) {
								std::string currentMaterialName = AM.getNameFromGuid(particleComp.MaterialType);
								serializeMaterial(material, currentMaterialName);
								AM.scanAndProcess();
								ImGui::CloseCurrentPopup();
								ImGui::OpenPopup("Material Saved");
							}
						}

						ImGui::SameLine();

						if (ImGui::Button("No", ImVec2(120, 0))) {
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

					// Save As New Material Popup
					static char saveAsName[256] = "";
					if (ImGui::BeginPopupModal("Save As New Material", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
						ImGui::Text("Enter a name for the new material:");
						ImGui::InputText("##SaveAsName", saveAsName, sizeof(saveAsName));
						ImGui::Separator();

						if (ImGui::Button("Save", ImVec2(120, 0))) {
							if (strlen(saveAsName) > 0) {
								MaterialResource* material = RM.loadResource<MaterialResource>(convertToMaterialGuid(particleComp.MaterialType));
								if (material) {
									std::string filename = std::string(saveAsName);
									serializeMaterial(material, filename);
									AM.scanAndProcess();
									memset(saveAsName, 0, sizeof(saveAsName));
									ImGui::CloseCurrentPopup();
									ImGui::OpenPopup("Material Saved");
								}
							}
							else {
								ImGui::OpenPopup("Invalid Name");
							}
						}

						ImGui::SameLine();

						if (ImGui::Button("Cancel", ImVec2(120, 0))) {
							memset(saveAsName, 0, sizeof(saveAsName));
							ImGui::CloseCurrentPopup();
						}

						ImGui::EndPopup();
					}

					// Material Saved Popup
					if (ImGui::BeginPopupModal("Material Saved", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
						ImGui::Text("Material saved successfully!");
						if (ImGui::Button("OK")) {
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}

					// Invalid Name Popup
					if (ImGui::BeginPopupModal("Invalid Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
						ImGui::Text("Please enter a valid material name.");
						if (ImGui::Button("OK")) {
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}

					// No Material Selected Popup
					if (ImGui::BeginPopupModal("No Material Selected", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
						ImGui::Text("No material currently selected to overwrite.");
						if (ImGui::Button("OK")) {
							ImGui::CloseCurrentPopup();
						}
						ImGui::EndPopup();
					}

					// Get material reference
					MaterialResource* material = RM.loadResource<MaterialResource>(convertToMaterialGuid(particleComp.MaterialType));

					if (material) {
						ImGui::Text("Shader: %s", material->shaderName.c_str());

						if (ImGui::CollapsingHeader("Texture Maps")) {
							DisplayAssetField("Base Map (Albedo)", material->baseMap, ResourceType::TEXTURE, showWrongTypeParticle, ComponentTypeID::ParticleSystem);
							DisplayAssetField("Normal Map", material->normalMap, ResourceType::TEXTURE, showWrongTypeParticle, ComponentTypeID::ParticleSystem);
							DisplayAssetField("Metallic Map [NOT AVAILABLE]", material->metallicMap, ResourceType::TEXTURE, showWrongTypeParticle, ComponentTypeID::ParticleSystem);
							DisplayAssetField("Roughness Map [NOT AVAILABLE]", material->roughnessMap, ResourceType::TEXTURE, showWrongTypeParticle, ComponentTypeID::ParticleSystem);
							DisplayAssetField("Emission Map [NOT AVAILABLE]", material->emissionMap, ResourceType::TEXTURE, showWrongTypeParticle, ComponentTypeID::ParticleSystem);
							DisplayAssetField("Occlusion Map [NOT AVAILABLE]", material->occlusionMap, ResourceType::TEXTURE, showWrongTypeParticle, ComponentTypeID::ParticleSystem);
						}

						if (ImGui::CollapsingHeader("Colors", ImGuiTreeNodeFlags_DefaultOpen)) {
							if (ImGui::ColorEdit3("Base Color", material->baseColor.data(),
								ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB)) {
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
							}

							if (ImGui::ColorEdit3("Emission Color", material->emissionColor.data(),
								ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB)) {
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
							}
						}

						if (ImGui::CollapsingHeader("Material Properties", ImGuiTreeNodeFlags_DefaultOpen)) {
							if (ImGui::SliderFloat("Metallic", &material->metallic, 0.0f, 1.0f, "%.2f")) {
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
							}

							if (ImGui::SliderFloat("Roughness", &material->roughness, 0.0f, 1.0f, "%.2f")) {
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
							}

							if (ImGui::SliderFloat("Opacity", &material->opacity, 0.0f, 1.0f, "%.2f")) {
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
							}

							if (ImGui::SliderFloat("Emission Strength", &material->emissionStrength, 0.0f, 100.0f, "%.2f")) {
								material->emissionStrength = std::max(0.0f, material->emissionStrength);
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
							}

							if (ImGui::SliderFloat("Alpha Threshold", &material->alphaThreshold, 0.0f, 1.0f, "%.3f")) {
								material->alphaThreshold = std::max(0.0f, std::min(1.0f, material->alphaThreshold));
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
							}

							if (ImGui::SliderFloat("Ambient Occlusion", &material->ambientOcclusion, 0.0f, 1.0f, "%.3f")) {
								material->ambientOcclusion = std::max(0.0f, std::min(1.0f, material->ambientOcclusion));
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
							}
						}

						if (ImGui::CollapsingHeader("UV Transform")) {
							if (ImGui::DragFloat2("Tiling", material->tiling.data(), 0.1f, 0.1f, 10.0f, "%.2f")) {
								material->tiling[0] = std::max(0.1f, material->tiling[0]);
								material->tiling[1] = std::max(0.1f, material->tiling[1]);
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
							}

							if (ImGui::DragFloat2("Offset", material->offset.data(), 0.01f, -10.0f, 10.0f, "%.3f")) {
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);  // MARK AS OVERRIDDEN
							}
						}

						if (ImGui::CollapsingHeader("Render Flags")) {
							if (ImGui::Checkbox("Enable Emission", &material->enableEmission)) {
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);
							}
							if (ImGui::Checkbox("Alpha Test", &material->alphaTest)) {
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);
							}
							if (ImGui::Checkbox("Double Sided", &material->doubleSided)) {
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);
							}
							if (ImGui::Checkbox("Receive Shadows", &material->receiveShadows)) {
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);
							}
							if (ImGui::Checkbox("Cast Shadows", &material->castShadows)) {
								MarkComponentOverridden(ComponentTypeID::ParticleSystem);
							}
						}
					}

					ImGui::SeparatorText("Values for Debugging:");
					ImGui::Text("Material: %u", particleComp.MaterialType);

					ImGui::TreePop();
				}

				// Statistics
				if (ImGui::TreeNode("Statistics"))
				{
					int aliveCount = 0;
					for (const auto& particle : particleComp.Particles)
					{
						if (particle.Alive) aliveCount++;
						
					}

					ImGui::Text("Alive: %d / %u", aliveCount, particleComp.MaxParticles);
					ImGui::Text("Pool Size: %zu", particleComp.Particles.size());
					ImGui::Text("Accumulator: %.2f", particleComp.EmissionAccumulator);
					ImGui::ProgressBar((float)aliveCount / (float)particleComp.MaxParticles);
					ImGui::TreePop();
				}

				ImGui::Spacing();
				ImGui::Separator();

				// Controls
				if (ImGui::Button("Clear Particles", ImVec2(150, 0)))
				{
					for (auto& particle : particleComp.Particles)
					{
						particle.Alive = false;
					}
				}

				ImGui::SameLine();

				if (ImGui::Button("Reset System", ImVec2(150, 0)))
				{
					particleComp.Particles.clear();
					particleComp.EmissionAccumulator = 0.0f;
					particleComp.DelayAccumualator = 0.f;
				}
			}
			// ----------------------------------------- Remove Particle Component -------------------------------
			if (removeParticleComp)
			{
				m_SelectedEntity.RemoveComponent<ParticleComponent>();
			}
		}
	}

	// Script Component
	void EditorPropertyPanel::DisplayScriptComponent(ImVec2& buttonSize){
		if (m_SelectedEntity.HasComponent<ScriptComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::Script);
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));

			bool openScriptComp = ImGui::CollapsingHeader("Script Component", ImGuiTreeNodeFlags_DefaultOpen);
			if (isComponentOverridden)
				ImGui::PopStyleColor();
			bool removeScriptComp = false;
			auto& scriptComp = m_SelectedEntity.GetComponent<ScriptComponent>();
			std::string scriptPath = getRepository() + "\\Scripts\\Game";
			auto scriptFiles = m_Editor->getAssetsInFolder(scriptPath);

			ImGui::NextColumn();
			if (ImGui::Button("...##ScriptBtn", buttonSize))
				ImGui::OpenPopup("ScriptPopUp");
			if (ImGui::BeginPopup("ScriptPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeScriptComp = true;
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Store original state BEFORE removal
						std::string originalJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::Script);

						// Mark as removed
						prefabComp.MarkComponentRemoved(ComponentTypeID::Script, originalJSON);
						LOG_INFO("Marked ScriptComponent as REMOVED override");
					}
				}
				ImGui::EndPopup();
			}
			ImGui::Columns(1);
			if (openScriptComp)
			{
				ImGui::Text("Instance: %s", scriptComp.ScriptInstance ? "Active" : "None");
				ImGui::Text("Started: %s", scriptComp.Started ? "Yes" : "No");

				if (!scriptFiles.empty())
				{
					if (ImGui::BeginCombo("Select Script", scriptComp.ScriptClassName.empty() ? "None" : scriptComp.ScriptClassName.substr(scriptComp.ScriptClassName.find_last_of('.') + 1).c_str()))
					{
						for (const auto& asset : scriptFiles)
						{
							std::string className = asset.name;
							if (className.ends_with(".cs"))
								className = className.substr(0, className.size() - 3); // remove extension
							std::string selectedClassName = "Game." + className;
							bool isSelected = scriptComp.ScriptClassName == selectedClassName;
							if (ImGui::Selectable(className.c_str(), isSelected))
							{
								if (scriptComp.ScriptClassName != selectedClassName) {
									MarkComponentOverridden(ComponentTypeID::Script);
								}
								// Destroy previous script instance if exists
								if (scriptComp.ScriptInstance)
								{
									MonoScriptEngine::GetInstance().DestroyScriptInstance((MonoObject*)scriptComp.ScriptInstance);
									scriptComp.ScriptInstance = nullptr;
									scriptComp.Started = false;
								}

								// Assign the new script class name
								scriptComp.ScriptClassName = selectedClassName;

								// DON'T create instance in editor - let ScriptSystem handle it!
								// Just setting the class name is enough
								// The instance will be created and EntityID will be bound when you play
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}
						ImGui::EndCombo();
					}

					// ===== NEW: DISPLAY SERIALIZED FIELDS =====
					ImGui::Separator();
					ImGui::TextColored(ImVec4(0.7f, 0.9f, 1.0f, 1.0f), "Serialized Fields:");
					ImGui::Separator();
					// THIS IS THE KEY LINE - renders all [SerializeField] fields
					if (scriptComp.ScriptInstance)
					{
						RenderSerializedFieldsInImGui((MonoObject*)scriptComp.ScriptInstance);
					}
					else
					{
						ImGui::TextDisabled("(No script instance - will be created when playing)");
					}
					if (ImGui::Button("Save Script Fields To JSON"))
					{
						if (scriptComp.ScriptInstance)
							SerializeScriptToDiskRapidJSON((MonoObject*)scriptComp.ScriptInstance, "SavedScriptFields.json");
					}
					// Similarly, add a load button to test deserialization:
					ImGui::SameLine();
					if (ImGui::Button("Load Script Fields From JSON"))
					{
						if (scriptComp.ScriptInstance)
							DeserializeScriptFromDiskRapidJSON((MonoObject*)scriptComp.ScriptInstance, "SavedScriptFields.json");
					}
					// ===== END NEW SERIALIZED FIELDS =====
				}
			}
			// Remove Script Component
			if (removeScriptComp)
				m_SelectedEntity.RemoveComponent<ScriptComponent>();
		}
	}

	// Light Component
	void EditorPropertyPanel::DisplayLightComponent(ImVec2& buttonSize){
	
		if (m_SelectedEntity.HasComponent<LightComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);
			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::Light);
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));
			bool openLightComp = ImGui::CollapsingHeader("Light Component", ImGuiTreeNodeFlags_DefaultOpen);
			if (isComponentOverridden)
				ImGui::PopStyleColor();
			bool removeLightComp = false;

			ImGui::NextColumn();

			if (ImGui::Button("...###LightBtn", buttonSize))
			{
				ImGui::OpenPopup("LightPopUp");
			}
			if (ImGui::BeginPopup("LightPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeLightComp = true;
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Store original state BEFORE removal
						std::string originalJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::Light);

						// Mark as removed
						prefabComp.MarkComponentRemoved(ComponentTypeID::Light, originalJSON);
						LOG_INFO("Marked RigidBody as REMOVED override");
					}
					//return;
				}
				/*if (isComponentOverridden) {
					ImGui::Separator();
					if (ImGui::MenuItem("Revert to Prefab")) {
						if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
							auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();
							std::string originalJSON = prefabComp.GetOriginalComponentJSON(ComponentTypeID::Light);

							if (!originalJSON.empty()) {
								if (ComponentSerializer::DeserializeComponent(m_SelectedEntity,
									ComponentTypeID::Light, originalJSON)) {
									prefabComp.ClearComponentOverride(ComponentTypeID::Light);
									LOG_INFO("Reverted Particle component to prefab state");
								}
							}
						}
					}
				}*/
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openLightComp)
			{
				ImGui::SeparatorText("General");

				auto& lightComp = m_SelectedEntity.GetComponent<LightComponent>();
				if (ImGui::Checkbox("Enabled###LightEnabled", &lightComp.Enabled))
				{
					MarkComponentOverridden(ComponentTypeID::Light);
				}

				// --- Light Type Dropdown ---
				const char* lightTypeNames[] = { "Directional", "Point", "Spot" };
				int currentLightType = static_cast<int>(lightComp.Type);

				if (ImGui::Combo("Light Type", &currentLightType, lightTypeNames, IM_ARRAYSIZE(lightTypeNames)))
				{
					lightComp.SetLightType(static_cast<LightType>(currentLightType));
					MarkComponentOverridden(ComponentTypeID::Light);
				}

				ImGui::SeparatorText("Emission");

				// --- Color ---
				glm::vec3 color = lightComp.Color;
				if (ImGui::ColorEdit3("Color", glm::value_ptr(color)))
				{
					lightComp.SetColorLinear(color); // uses setter
					MarkComponentOverridden(ComponentTypeID::Light);
				}


				// --- Intensity ---
				float intensity = lightComp.Intensity;
				if (ImGui::DragFloat("Intensity", &intensity, 0.05f, 0.0f, 100.0f, "%.2f"))
				{
					lightComp.SetIntensity(intensity); //  uses setter
					MarkComponentOverridden(ComponentTypeID::Light);
				}


				// --- Range  ---
				if (lightComp.Type != LightType::Directional)
				{
					float range = lightComp.Range;
					if (ImGui::DragFloat("Range", &range, 0.1f, 0.0f, 1000.0f, "%.2f"))
					{
						lightComp.SetRange(range); // uses setter
						MarkComponentOverridden(ComponentTypeID::Light);
					}
				}

				if (lightComp.Type == LightType::Spot)
				{
					float spotAngle = lightComp.SpotAngleDeg;
					if (ImGui::DragFloat("Spot Angle", &spotAngle, 0.1f, 1.0f, 179.0f, "%.2f"))
					{
						lightComp.SetSpotAngleDeg(spotAngle); // uses setter
						MarkComponentOverridden(ComponentTypeID::Light);
					}
				}
				// --- Indirect Multiplier ---
				float indirectMult = lightComp.IndirectMultiplier;
				if (ImGui::DragFloat("Indirect Multiplier", &indirectMult, 0.01f, 0.0f, 10.0f, "%.2f"))
				{
					lightComp.SetIndirectMultiplier(indirectMult); // uses setter
					MarkComponentOverridden(ComponentTypeID::Light);
				}

				ImGui::SeparatorText("Shadow");

				// --- Shadow Type Dropdown ---
				const char* shadowTypeNames[] = { "No", "Hard", "Soft" };
				int currentShadowType = static_cast<int>(lightComp.TypeShadow);

				if (ImGui::Combo("Shadow Type", &currentShadowType, shadowTypeNames, IM_ARRAYSIZE(shadowTypeNames)))
				{
					lightComp.SetShadowType(static_cast<ShadowType>(currentShadowType));
					MarkComponentOverridden(ComponentTypeID::Light);
				}

				// --- Resolution Dropdown ---
				const char* resolutionTypeNames[] = { "Low (256)", "Med (512)", "High (1024)" };

				int currentResolutionType = 0;
				switch (lightComp.Resolution)
				{
				case 256:  currentResolutionType = 0; break;
				case 512:  currentResolutionType = 1; break;
				case 1024: currentResolutionType = 2; break;
				default:   currentResolutionType = 2; break; // default to High
				}

				if (ImGui::Combo("Resolution", &currentResolutionType, resolutionTypeNames, IM_ARRAYSIZE(resolutionTypeNames)))
				{
					switch (currentResolutionType)
					{
					case 0: lightComp.Resolution = 256;  break; // Low
					case 1: lightComp.Resolution = 512;  break; // Med
					case 2: lightComp.Resolution = 1024; break; // High
					default: lightComp.Resolution = 1024; break;
					}

					MarkComponentOverridden(ComponentTypeID::Light);
				}

				// --- Strength Value ---
				float strength = lightComp.Strength;
				if (ImGui::DragFloat("Strength", &strength, 0.01f, 0.0f, 1.0f, "%.2f"))
				{
					lightComp.Strength = strength; 
					MarkComponentOverridden(ComponentTypeID::Light);
				}

				// --- Near Plane Value ---
				float rangeMin = std::min(0.1f, std::max(lightComp.Range, 0.0f) * 0.01f);
				rangeMin = std::max(rangeMin, 0.0f);	// Prevent neg min
				float nearPlaneLight = lightComp.NearPlane;
				if (ImGui::DragFloat("Near Plane", &nearPlaneLight, 0.01f, rangeMin, 10.0f, "%.2f"))
				{
					// Clamp to rule (in case user types a value)
					nearPlaneLight = std::clamp(nearPlaneLight, rangeMin, 10.0f);

					lightComp.NearPlane = nearPlaneLight;
					MarkComponentOverridden(ComponentTypeID::Light);
				}

			}
			// ---------------------------- Remove Light Comp --------------------------
			if (removeLightComp)
			{
				m_SelectedEntity.RemoveComponent<LightComponent>();
			}
		}
	}

	// Camera Component
	void EditorPropertyPanel::DisplayCameraComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<CameraComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);
			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::Camera);
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));
			bool openCameraComp = ImGui::CollapsingHeader("Camera Component", ImGuiTreeNodeFlags_DefaultOpen);
			if (isComponentOverridden)
				ImGui::PopStyleColor();
			bool removeCameraComp = false;
			ImGui::NextColumn();

			if (ImGui::Button("...###CameraBtn", buttonSize))
			{
				ImGui::OpenPopup("CameraPopUp");
			}
			if (ImGui::BeginPopup("CameraPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeCameraComp = true;
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Store original state BEFORE removal
						std::string originalJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::Camera);

						// Mark as removed
						prefabComp.MarkComponentRemoved(ComponentTypeID::Camera, originalJSON);
						LOG_INFO("Marked RigidBody as REMOVED override");
					}
					//return;
				}
				/*if (isComponentOverridden) {
					ImGui::Separator();
					if (ImGui::MenuItem("Revert to Prefab")) {
						if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
							auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();
							std::string originalJSON = prefabComp.GetOriginalComponentJSON(ComponentTypeID::Camera);

							if (!originalJSON.empty()) {
								if (ComponentSerializer::DeserializeComponent(m_SelectedEntity,
									ComponentTypeID::Camera, originalJSON)) {
									prefabComp.ClearComponentOverride(ComponentTypeID::Camera);
									LOG_INFO("Reverted Particle component to prefab state");
								}
							}
						}
					}
				}*/
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openCameraComp)
			{
				auto& camComp = m_SelectedEntity.GetComponent<CameraComponent>();

				// -------------------------------------------------
				// Enabled
				// -------------------------------------------------
				if (ImGui::Checkbox("Enabled###CamEnabled", &camComp.Enabled))
				{
					MarkComponentOverridden(ComponentTypeID::Camera);
				}

				// Only show the rest when the camera is enabled
				if (camComp.Enabled)
				{
					ImGui::Separator();

					// -------------------------------------------------
					// Auto aspect
					// -------------------------------------------------
					if (ImGui::Checkbox("Auto Aspect", &camComp.autoAspect))
					{
						MarkComponentOverridden(ComponentTypeID::Camera);
					}

					if (!camComp.autoAspect)
					{
						float aspect = camComp.Aspect;
						if (ImGui::DragFloat("Aspect", &aspect, 0.01f, 0.1f, 10.0f))
						{
							camComp.SetAspect(aspect);   // rebuilds projection
							MarkComponentOverridden(ComponentTypeID::Camera);
						}
					}

					// -------------------------------------------------
					// Projection type (Perspective / Orthographic)
					// -------------------------------------------------
					int projIndex = camComp.Projection ? 1 : 0;     // 0 = Persp, 1 = Ortho
					const char* projItems[] = { "Perspective", "Orthographic" };
					if (ImGui::Combo("Projection", &projIndex, projItems, IM_ARRAYSIZE(projItems)))
					{
						bool isOrtho = (projIndex == 1);
						camComp.SetProjection(isOrtho);             // rebuilds projection
						MarkComponentOverridden(ComponentTypeID::Camera);
					}

					// -------------------------------------------------
					// FOV or Ortho Height (Size.y)
					// -------------------------------------------------
					if (!camComp.Projection)
					{
						// Perspective show FOV
						float fov = camComp.FOV;
						if (ImGui::DragFloat("FOV", &fov, 0.1f, 10.0f, 120.0f))
						{
							camComp.SetFOV(fov);                    // rebuilds projection
							MarkComponentOverridden(ComponentTypeID::Camera);
						}
					}
					else
					{
						// Orthographic edit height only (Size.y)
						float orthoHeight = camComp.Size.y;
						if (ImGui::DragFloat("Ortho Height", &orthoHeight, 0.1f, 0.1f, 10000.0f))
						{
							camComp.SetSize({ camComp.Size.x, orthoHeight }); // x ignored, y used
							MarkComponentOverridden(ComponentTypeID::Camera);
						}
					}

					// -------------------------------------------------
					// Near / Far planes
					// -------------------------------------------------
					float nearPlane = camComp.NearPlane;
					if (ImGui::DragFloat("Near Plane", &nearPlane, 0.01f, 0.01f, camComp.FarPlane - 0.01f))
					{
						camComp.SetNearPlane(nearPlane);           // rebuilds projection
						MarkComponentOverridden(ComponentTypeID::Camera);
					}

					float farPlane = camComp.FarPlane;
					if (ImGui::DragFloat("Far Plane", &farPlane, 1.0f, camComp.NearPlane + 0.01f, 10000.0f))
					{
						camComp.SetFarPlane(farPlane);             // rebuilds projection
						MarkComponentOverridden(ComponentTypeID::Camera);
					}

					// -------------------------------------------------
					// Target
					// -------------------------------------------------
					glm::vec3 target = camComp.Target;
					if (ImGui::DragFloat3("Target", glm::value_ptr(target), 0.1f))
					{
						camComp.SetTarget(target);                 // only affects View
						MarkComponentOverridden(ComponentTypeID::Camera);
					}
				}
			}

			// ---------------------- Remove Camera Comp ---------------------------
			if (removeCameraComp)
			{
				m_SelectedEntity.RemoveComponent<CameraComponent>();
			}

		}
	}

	// Animation Component
	void EditorPropertyPanel::DisplayAnimatorComponent(ImVec2& buttonSize) {
		if (m_SelectedEntity.HasComponent<AnimatorComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);
			bool isComponentOverridden = IsComponentOverridden(ComponentTypeID::Animator);
			if (isComponentOverridden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));
			bool openAnimatorComponent = ImGui::CollapsingHeader("Animator Component", ImGuiTreeNodeFlags_DefaultOpen);
			if (isComponentOverridden)
				ImGui::PopStyleColor();
			bool removeAnimator = false;

			// Column 2: "..." button to remove component
			ImGui::NextColumn();

			if (ImGui::Button("... ###AnimatorBtn", buttonSize))
			{
				ImGui::OpenPopup("AnimatorPopUp");
			}
			if (ImGui::BeginPopup("AnimatorPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeAnimator = true;
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Store original state BEFORE removal
						std::string originalJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::Animator);

						// Mark as removed
						prefabComp.MarkComponentRemoved(ComponentTypeID::Animator, originalJSON);
						LOG_INFO("Marked RigidBody as REMOVED override");
					}
				}
				
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openAnimatorComponent)
			{
				auto& animator = m_SelectedEntity.GetComponent<AnimatorComponent>();

				// -----------------------------------------------------------------
				// Controller selection combo (from m_AnimatorControllerStorage)
				// -----------------------------------------------------------------
				std::vector<u32> controllerHandles;
				controllerHandles.reserve(m_AnimatorControllerStorage.size());

				std::vector<std::string> controllerLabels;
				controllerLabels.reserve(m_AnimatorControllerStorage.size());

				// Build a simple list of (handle, "Name (id)") pairs
				for (const auto& kv : m_AnimatorControllerStorage)
				{
					u32 handle = kv.first;
					const AnimatorController& ctrl = kv.second;

					controllerHandles.push_back(handle);

					std::string label;
					if (!ctrl.name.empty())
						label = ctrl.name + " (" + std::to_string(handle) + ")";
					else
						label = "Controller " + std::to_string(handle);

					controllerLabels.push_back(label);
				}

				// Find the currently assigned controller in the list
				int currentIndex = -1;
				for (int i = 0; i < static_cast<int>(controllerHandles.size()); ++i)
				{
					if (controllerHandles[i] == animator.controller)
					{
						currentIndex = i;
						break;
					}
				}

				const char* previewLabel = "(None)";
				if (currentIndex >= 0 && currentIndex < static_cast<int>(controllerLabels.size()))
					previewLabel = controllerLabels[currentIndex].c_str();

				ImGui::Text("Controller:");
				ImGui::SameLine();
				ImGui::SetNextItemWidth(200.0f);
				if (ImGui::BeginCombo("##AnimatorControllerCombo", previewLabel))
				{
					for (int i = 0; i < static_cast<int>(controllerHandles.size()); ++i)
					{
						bool isSelected = (i == currentIndex);
						const char* itemLabel = controllerLabels[i].c_str();

						if (ImGui::Selectable(itemLabel, isSelected))
						{
							// Assign new controller to this AnimatorComponent
							animator.controller = controllerHandles[i];
							animator.currentClipIndex = 0;
							animator.currentTime = 0.0f;
							MarkComponentOverridden(ComponentTypeID::Animator);
						}

						if (isSelected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndCombo();
				}

				// Debug/info line for handle
				ImGui::Text("Controller Handle: %u", animator.controller);

				ImGui::SeparatorText("Playback");

				// Basic animator state controls
				if (ImGui::Checkbox("Playing", &animator.playing))
				{
					MarkComponentOverridden(ComponentTypeID::Animator);
				}
				ImGui::SameLine();
				if (ImGui::Checkbox("Respect Clip Loop", &animator.respectClipLoop)) {
					MarkComponentOverridden(ComponentTypeID::Animator);
				}

				if (ImGui::DragFloat("Playback Speed", &animator.playbackSpeed, 0.01f, -5.0f, 5.0f))
				{
					MarkComponentOverridden(ComponentTypeID::Animator);
				}

				// We keep these for debugging / manual scrubbing
				if (ImGui::DragInt("Current Clip Index", (int*)(&animator.currentClipIndex), 1.0f, 0, 100))
				{
					MarkComponentOverridden(ComponentTypeID::Animator);
				}
				if (ImGui::DragFloat("Current Time", &animator.currentTime, 0.01f, 0.0f, 1000.0f))
				{
					MarkComponentOverridden(ComponentTypeID::Animator);
				}

				if (ImGui::Button("Restart Clip"))
				{
					animator.currentTime = 0.0f;
				}
				ImGui::SameLine();
				if (ImGui::Button("Stop##AnimatorComp"))
				{
					animator.currentTime = 0.0f;
					animator.playing = false;
				}

				ImGui::Separator();
				ImGui::TextWrapped("Use the Animator window (View -> Animator) to edit "
					"controllers, clips, and keyframes.");

				// Convenience: open & focus Animator window from here
				if (ImGui::Button("Open Animator Window"))
				{
					m_AnimatorWindow = true;
					m_FocusAnimatorNextFrame = true;
				}
			}

			if (removeAnimator)
			{
				m_SelectedEntity.RemoveComponent<AnimatorComponent>();
			}
		}
	}

	void EditorPropertyPanel::DisplaySpriteRendererComponent(ImVec2& buttonSize)
	{
		if (m_SelectedEntity.HasComponent<SpriteRendererComponent>())
		{
			ImGui::Separator();
			ImGui::Columns(2, nullptr, false);
			ImGui::SetColumnWidth(0, 200.0f);

			bool isComponentOverriden = IsComponentOverridden(ComponentTypeID::SpriteRenderer);
			if (isComponentOverriden)
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.6f, 0.4f, 0.1f, 0.5f));

			bool openSpriteRendererComponent = ImGui::CollapsingHeader("SpriteRenderer Component", ImGuiTreeNodeFlags_DefaultOpen);
			if (isComponentOverriden)
				ImGui::PopStyleColor();
			bool removeSpriteRenderer = false;

			ImGui::NextColumn();

			if (ImGui::Button("... ###SpriteRendereBtn", buttonSize))
			{
				ImGui::OpenPopup("SpriteRendererPopUp");
			}
			if (ImGui::BeginPopup("SpriteRendererPopUp"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					removeSpriteRenderer = true;

					if (m_SelectedEntity.HasComponent<PrefabComponent>())
					{
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Store original state BEFORE removal
						std::string originalJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::SpriteRenderer);

						// Mark as removed
						prefabComp.MarkComponentRemoved(ComponentTypeID::SpriteRenderer, originalJSON);
						LOG_INFO("Marked SpriteRenderer as REMOVED override");
					}
				}
				ImGui::EndPopup();
			}

			ImGui::Columns(1);

			if (openSpriteRendererComponent)
			{
				auto& spriteRenderer = m_SelectedEntity.GetComponent<SpriteRendererComponent>();

				// ======================= Asset Reference Section =======================
				ImGui::SeparatorText("Asset References");

				static bool showWrongType_ = false;

				DisplayAssetField("Texture", spriteRenderer.TextureGuid, ResourceType::TEXTURE, showWrongType_, ComponentTypeID::SpriteRenderer);

				if (showWrongType_)
				{
					ImGui::OpenPopup("Incompatible Asset Type");
					showWrongType_ = false;
				}

				// Popup for incompatible asset type
				if (ImGui::BeginPopup("Incompatible Asset Type"))
				{
					ImGui::Text("The dropped asset type does not match the expected type.");
					if (ImGui::Button("Close"))
					{
						ImGui::CloseCurrentPopup();
					}
					ImGui::EndPopup();
				}

				ImGui::Spacing();

				if (ImGui::ColorEdit4("Color", &spriteRenderer.Color.r, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_InputRGB))
					MarkComponentOverridden(ComponentTypeID::SpriteRenderer);

				if (ImGui::DragInt("Quad", (int*)(&spriteRenderer.Quad), 1, 0, 10))
					MarkComponentOverridden(ComponentTypeID::SpriteRenderer);

				if (ImGui::DragInt("Layer", (int*)(&spriteRenderer.SpriteLayer), 1, 0, 255))
					MarkComponentOverridden(ComponentTypeID::SpriteRenderer);
				
				if (ImGui::Checkbox("Set Active", &spriteRenderer.IsActive))
					MarkComponentOverridden(ComponentTypeID::SpriteRenderer);

				if (ImGui::Checkbox("Set Visible", &spriteRenderer.IsVisible))
					MarkComponentOverridden(ComponentTypeID::SpriteRenderer);
			}

			if (removeSpriteRenderer)
			{
				m_SelectedEntity.RemoveComponent<SpriteRendererComponent>();
			}
		}
	}

	// Animator Window
	void EditorPropertyPanel::AnimatorWindow(){
		
		if (!m_AnimatorWindow)
			return;

		// First-time size
		ImGui::SetNextWindowSize(ImVec2(1000.0f, 450.0f), ImGuiCond_FirstUseEver);

		if (!ImGui::Begin("Animator", &m_AnimatorWindow))
		{
			ImGui::End();
			return;
		}

		// If requested, focus this window on the next frame
		if (m_FocusAnimatorNextFrame)
		{
			ImGui::SetWindowFocus();
			m_FocusAnimatorNextFrame = false;
		}

		if (!m_Scene)
		{
			ImGui::TextUnformatted("No scene loaded.");
			ImGui::End();
			return;
		}

		if (!m_SelectedEntity)
		{
			ImGui::TextUnformatted("No entity selected.");
			ImGui::End();
			return;
		}

		// Selected entity must have an AnimatorComponent
		if (!m_SelectedEntity.HasComponent<AnimatorComponent>())
		{
			ImGui::TextUnformatted("Selected entity has no AnimatorComponent.");
			ImGui::End();
			return;
		}

		auto& animator = m_SelectedEntity.GetComponent<AnimatorComponent>();

		// ---------------------------------------------------------------------
		// Look up controller from storage (or allow user to create a new one)
		// ---------------------------------------------------------------------
		auto ctrlIt = m_AnimatorControllerStorage.find(animator.controller);
		if (ctrlIt == m_AnimatorControllerStorage.end())
		{
			ImGui::Text("Animator has no valid controller (handle %u).", animator.controller);
			ImGui::Spacing();

			if (ImGui::Button("Create New Controller"))
			{
				AnimatorController newCtrl{};
				newCtrl.name = "NewController";
				newCtrl.defaultClipIndex = 0;

				u32 newHandle = static_cast<u32>(m_AnimatorControllerStorage.size());
				newCtrl.id = newHandle;
				m_AnimatorControllerStorage[newHandle] = newCtrl;

				animator.controller = newHandle;
				animator.currentClipIndex = 0;
				animator.currentTime = 0.0f;
				m_DopesheetSelectedTrack = DopesheetTrackType::None;
				m_DopesheetSelectedKey = -1;
			}

			ImGui::End();
			return;
		}

		AnimatorController& controller = ctrlIt->second;

		// Static state for Controller "Save As" popup
		static bool s_OpenCtrlSaveAsPopup = false;
		static char s_CtrlFileNameBuf[128] = "NewController";

		// NEW: static state for Add/Remove clip popups
		static bool s_OpenAddClipPopup = false;
		static int  s_AddClipSelectedIndex = 0;
		static bool s_AddClipDuplicateWarning = false;

		static bool s_OpenRemoveClipPopup = false;
		static int  s_RemoveClipSelectedIndex = 0;

		// Helper: controller file toolbar (New / Save / Save As)
		bool newControllerCreated = false;
		auto DrawControllerFileToolbar = [&](AnimatorController& controllerRef, AnimatorComponent& animatorRef)
			{
				ImGui::SeparatorText("Controller");

				// Controller name edit
				{
					char ctrlNameBuf[128];
					strncpy_s(ctrlNameBuf, sizeof(ctrlNameBuf), controllerRef.name.c_str(), _TRUNCATE);

					if (ImGui::InputText("Controller Name", ctrlNameBuf, sizeof(ctrlNameBuf),
						ImGuiInputTextFlags_EnterReturnsTrue))
					{
						std::string newName = ctrlNameBuf;

						// Trim
						newName.erase(0, newName.find_first_not_of(" \t\n\r\f\v"));
						if (!newName.empty())
							newName.erase(newName.find_last_not_of(" \t\n\r\f\v") + 1);

						if (!newName.empty())
						{
							controllerRef.name = newName;
						}
					}
				}

				// --- New Controller ---
				if (ImGui::Button("New Controller"))
				{
					AnimatorController newCtrl{};
					newCtrl.name = "NewController";
					newCtrl.defaultClipIndex = 0;

					u32 newHandle = static_cast<u32>(m_AnimatorControllerStorage.size());
					newCtrl.id = newHandle;
					m_AnimatorControllerStorage[newHandle] = newCtrl;

					animatorRef.controller = newHandle;
					animatorRef.currentClipIndex = 0;
					animatorRef.currentTime = 0.0f;
					m_DopesheetSelectedTrack = DopesheetTrackType::None;
					m_DopesheetSelectedKey = -1;

					newControllerCreated = true;
				}

				ImGui::SameLine();

				// --- Save Controller ---
				if (ImGui::Button("Save Controller"))
				{
					Engine::SaveAnimatorControllerAsset(controller);
				}

				ImGui::SameLine();

				// --- Save Controller As ---
				if (ImGui::Button("Save Controller As"))
				{
					s_OpenCtrlSaveAsPopup = true;
					ImGui::OpenPopup("Save Controller As");
				}

				if (ImGui::BeginPopupModal("Save Controller As", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
				{
					if (s_OpenCtrlSaveAsPopup)
					{
						std::string initial = controllerRef.name.empty() ? "NewController" : controllerRef.name;
						std::snprintf(s_CtrlFileNameBuf, sizeof(s_CtrlFileNameBuf), "%s", initial.c_str());
						s_OpenCtrlSaveAsPopup = false;
					}

					ImGui::Text("File name (without extension):");
					ImGui::InputText("##CtrlFileName", s_CtrlFileNameBuf, IM_ARRAYSIZE(s_CtrlFileNameBuf));

					if (ImGui::Button("OK"))
					{
						std::string fileName = s_CtrlFileNameBuf;
						if (fileName.empty())
							fileName = "NewController";

						controllerRef.name = fileName;

						std::string path1 = "../../Resources/Sources/AnimationControllers/" + fileName + ".animcontroller";
						std::string path2 = "../bin/Debug/Resources/Sources/AnimationControllers/" + fileName + ".animcontroller";

						SerializeAnimationController(controllerRef, path1);
						SerializeAnimationController(controllerRef, path2);

						ImGui::CloseCurrentPopup();
					}

					ImGui::SameLine();

					if (ImGui::Button("Cancel"))
					{
						ImGui::CloseCurrentPopup();
					}

					ImGui::EndPopup();
				}
			};

		// =====================================================================
		// CASE 1: Controller exists but has NO clips yet.
		// Show a simpler UI that lets you save the controller and create first clip.
		// =====================================================================
		if (controller.clips.empty())
		{
			ImGui::Text("Entity: %s", m_SelectedEntity.GetComponent<TagComponent>().Tag.c_str());
			ImGui::Text("Controller: %s (handle %u)", controller.name.c_str(), controller.id);

			DrawControllerFileToolbar(controller, animator);
			if (newControllerCreated)
			{
				ImGui::End();
				return;
			}

			ImGui::SeparatorText("Clips");
			ImGui::TextUnformatted("This controller has no clips yet.");
			ImGui::Spacing();

			if (ImGui::Button("New Clip"))
			{
				AnimationClip newClip{};
				newClip.name = "NewClip";
				newClip.duration = 1.0f;
				newClip.loop = true;

				u32 newHandle = static_cast<u32>(m_AnimationClipStorage.size());
				newClip.id = newHandle;
				m_AnimationClipStorage[newHandle] = newClip;

				controller.clips.push_back(newHandle);
				controller.defaultClipIndex = 0;
				animator.currentClipIndex = 0;
				animator.currentTime = 0.0f;
				m_DopesheetSelectedTrack = DopesheetTrackType::None;
				m_DopesheetSelectedKey = -1;
			}

			ImGui::End();
			return;
		}

		// =====================================================================
		// CASE 2: Normal path ? controller has at least one clip
		// =====================================================================
		if (animator.currentClipIndex < 0 ||
			animator.currentClipIndex >= static_cast<int>(controller.clips.size()))
		{
			animator.currentClipIndex = 0;
		}

		u32 clipHandle = controller.clips[static_cast<size_t>(animator.currentClipIndex)];

		// Look up active clip
		AnimationClip* clipPtr = nullptr;
		{
			auto clipIt = m_AnimationClipStorage.find(clipHandle);
			if (clipIt != m_AnimationClipStorage.end())
				clipPtr = &clipIt->second;
		}

		if (!clipPtr)
		{
			ImGui::Text("Active clip handle %u is invalid.", clipHandle);
			ImGui::End();
			return;
		}

		AnimationClip& clip = *clipPtr;

		// Compute left/right sizes
		ImVec2 avail = ImGui::GetContentRegionAvail();
		float leftWidth = glm::clamp(avail.x * 0.32f, 260.0f, avail.x * 0.5f);

		// =====================================================================
		// Split into LEFT (meta + tracks) and RIGHT (dopesheet/curves)
		// =====================================================================
		ImGui::BeginChild(
			"Animator_LeftPanel",
			ImVec2(leftWidth, 0.0f),          // full height, fixed width
			true,
			ImGuiWindowFlags_HorizontalScrollbar
		);

		// =============================== LEFT PANE ============================
		// -----------------------------------------------------------------
		// Header: entity + controller + clip selection
		// -----------------------------------------------------------------
		ImGui::Text("Entity: %s", m_SelectedEntity.GetComponent<TagComponent>().Tag.c_str());
		ImGui::Text("Controller: %s", controller.name.c_str());

		// Controller toolbar (New / Save / Save As)
		DrawControllerFileToolbar(controller, animator);
		if (newControllerCreated)
		{
			ImGui::EndChild();
			ImGui::End();
			return;
		}

		// -----------------------------------------------------------------
		// Clip selection
		// -----------------------------------------------------------------
		ImGui::SeparatorText("Clip");

		// Clip dropdown
		{
			std::string previewName = clip.name.empty() ? "Unnamed Clip" : clip.name;

			if (ImGui::BeginCombo("Clip", previewName.c_str()))
			{
				for (int i = 0; i < static_cast<int>(controller.clips.size()); ++i)
				{
					u32 h = controller.clips[static_cast<size_t>(i)];
					auto it = m_AnimationClipStorage.find(h);

					const char* name = "(missing)";
					if (it != m_AnimationClipStorage.end() && !it->second.name.empty())
						name = it->second.name.c_str();

					bool selected = (i == static_cast<int>(animator.currentClipIndex));
					if (ImGui::Selectable(name, selected))
					{
						animator.currentClipIndex = i;
						animator.currentTime = 0.0f;
						m_DopesheetSelectedTrack = DopesheetTrackType::None;
						m_DopesheetSelectedKey = -1;
					}
					if (selected)
						ImGui::SetItemDefaultFocus();
				}
				ImGui::EndCombo();
			}
		}

		// -----------------------------------------------------------------
		// Add / Remove Clip popups
		// -----------------------------------------------------------------
		ImGui::Spacing();

		if (ImGui::Button("Add Clip"))
		{
			s_OpenAddClipPopup = true;
			s_AddClipSelectedIndex = 0;
			s_AddClipDuplicateWarning = false;
			ImGui::OpenPopup("Add Clip to Controller");
		}

		ImGui::SameLine();

		if (ImGui::Button("Remove Clip"))
		{
			s_OpenRemoveClipPopup = true;
			if (!controller.clips.empty())
			{
				s_RemoveClipSelectedIndex = glm::clamp((int)animator.currentClipIndex, 0, (int)controller.clips.size() - 1);
			}
			ImGui::OpenPopup("Remove Clip from Controller");
		}

		// --- Add Clip popup ---
		if (ImGui::BeginPopupModal("Add Clip to Controller", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			// Build list of all clips in storage
			std::vector<std::pair<u32, AnimationClip*>> allClips;
			allClips.reserve(m_AnimationClipStorage.size());
			for (auto& kv : m_AnimationClipStorage)
			{
				allClips.emplace_back(kv.first, &kv.second);
			}

			if (allClips.empty())
			{
				ImGui::TextUnformatted("No clips available in storage.");
			}
			else
			{
				if (s_AddClipSelectedIndex < 0 || s_AddClipSelectedIndex >= (int)allClips.size())
					s_AddClipSelectedIndex = 0;

				if (ImGui::BeginListBox("##AddClipList", ImVec2(350, 200)))
				{
					for (int i = 0; i < (int)allClips.size(); ++i)
					{
						u32 h = allClips[i].first;
						AnimationClip* c = allClips[i].second;

						std::string label = std::to_string(h) + " - " +
							(c->name.empty() ? "Unnamed Clip" : c->name);

						bool selected = (i == s_AddClipSelectedIndex);
						if (ImGui::Selectable(label.c_str(), selected))
							s_AddClipSelectedIndex = i;

						if (selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndListBox();
				}
			}

			if (s_AddClipDuplicateWarning)
			{
				ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
					"This clip is already in the controller.");
			}

			if (ImGui::Button("Add"))
			{
				if (!allClips.empty())
				{
					u32 handleToAdd = allClips[s_AddClipSelectedIndex].first;
					bool already = false;
					for (u32 existing : controller.clips)
					{
						if (existing == handleToAdd)
						{
							already = true;
							break;
						}
					}

					if (already)
					{
						s_AddClipDuplicateWarning = true;
					}
					else
					{
						controller.clips.push_back(handleToAdd);
						animator.currentClipIndex = (int)controller.clips.size() - 1;
						animator.currentTime = 0.0f;
						controller.defaultClipIndex = animator.currentClipIndex;
						m_DopesheetSelectedTrack = DopesheetTrackType::None;
						m_DopesheetSelectedKey = -1;

						s_AddClipDuplicateWarning = false;
						ImGui::CloseCurrentPopup();
					}
				}
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		// --- Remove Clip popup ---
		if (ImGui::BeginPopupModal("Remove Clip from Controller", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			// Build list of clips currently in controller
			std::vector<std::pair<u32, AnimationClip*>> controllerClipList;
			controllerClipList.reserve(controller.clips.size());
			for (u32 h : controller.clips)
			{
				auto it = m_AnimationClipStorage.find(h);
				AnimationClip* c = (it != m_AnimationClipStorage.end()) ? &it->second : nullptr;
				controllerClipList.emplace_back(h, c);
			}

			if (controllerClipList.empty())
			{
				ImGui::TextUnformatted("Controller has no clips.");
			}
			else
			{
				if (s_RemoveClipSelectedIndex < 0 ||
					s_RemoveClipSelectedIndex >= (int)controllerClipList.size())
				{
					s_RemoveClipSelectedIndex = 0;
				}

				if (ImGui::BeginListBox("##RemoveClipList", ImVec2(350, 200)))
				{
					for (int i = 0; i < (int)controllerClipList.size(); ++i)
					{
						u32 h = controllerClipList[i].first;
						AnimationClip* c = controllerClipList[i].second;

						std::string label = std::to_string(h) + " - ";
						if (c)
							label += (c->name.empty() ? "Unnamed Clip" : c->name);
						else
							label += "(missing)";

						bool selected = (i == s_RemoveClipSelectedIndex);
						if (ImGui::Selectable(label.c_str(), selected))
							s_RemoveClipSelectedIndex = i;
						if (selected)
							ImGui::SetItemDefaultFocus();
					}
					ImGui::EndListBox();
				}
			}

			if (ImGui::Button("Remove"))
			{
				if (!controllerClipList.empty())
				{
					int removeIndex = s_RemoveClipSelectedIndex;
					if (removeIndex >= 0 && removeIndex < (int)controller.clips.size())
					{
						controller.clips.erase(controller.clips.begin() + removeIndex);

						if (controller.clips.empty())
						{
							animator.currentClipIndex = 0;
							animator.currentTime = 0.0f;
							m_DopesheetSelectedTrack = DopesheetTrackType::None;
							m_DopesheetSelectedKey = -1;
						}
						else
						{
							if (animator.currentClipIndex >= (int)controller.clips.size())
								animator.currentClipIndex = (int)controller.clips.size() - 1;
							if (controller.defaultClipIndex >= (int)controller.clips.size())
								controller.defaultClipIndex = (int)controller.clips.size() - 1;
						}
					}
				}

				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();

			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

		// -----------------------------------------------------------------
		// Clip settings + time controls
		// -----------------------------------------------------------------
		ImGui::SeparatorText("Clip Settings");

		// Clip name edit
		{
			char nameBuffer[128];
			strncpy_s(nameBuffer, sizeof(nameBuffer), clip.name.c_str(), _TRUNCATE);

			if (ImGui::InputText("Clip Name", nameBuffer, sizeof(nameBuffer),
				ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string newName = nameBuffer;

				// Trimming
				newName.erase(0, newName.find_first_not_of(" \t\n\r\f\v"));
				newName.erase(newName.find_last_not_of(" \t\n\r\f\v") + 1);

				if (!newName.empty())
				{
					clip.name = newName;
				}
			}
		}

		ImGui::InputFloat("Duration (s)", &clip.duration);
		if (clip.duration <= 0.0f) clip.duration = 0.001f; // Ensure sane duration

		ImGui::Checkbox("Loop", &clip.loop);

		ImGui::SeparatorText("Time");

		// Clamp animator time
		animator.currentTime = glm::clamp(animator.currentTime, 0.0f, clip.duration);

		float scrubTime = animator.currentTime;
		if (ImGui::SliderFloat("Current Time", &scrubTime, 0.0f, clip.duration, "%.3f s"))
		{
			animator.currentTime = glm::clamp(scrubTime, 0.0f, clip.duration);
		}

		// -----------------------------------------------------------------
		// Track editors (tables) - now synced with timeline selection
		// -----------------------------------------------------------------
		ImGui::SeparatorText("Component");

		const char* componentItems[] = { "Transform", "UV Transform" };
		int componentIndex =
			(m_SelectedComponentTrack == AnimatorComponentTrack::Transform) ? 0 : 1;

		if (ImGui::Combo("##AnimatorComponent",
			&componentIndex,
			componentItems,
			IM_ARRAYSIZE(componentItems)))
		{
			m_SelectedComponentTrack = (componentIndex == 0)
				? AnimatorComponentTrack::Transform
				: AnimatorComponentTrack::UVTransform;
		}

		ImGui::Spacing();
		ImGui::SeparatorText("Tracks");

		if (m_SelectedComponentTrack == AnimatorComponentTrack::Transform)
		{
			ImGui::Text("Transform");

			// Helper: sort by time after edits
			auto sortByTime = [](auto& keys)
				{
					std::sort(keys.begin(), keys.end(),
						[](const auto& a, const auto& b) { return a.time < b.time; });
				};

			// Position track 
			if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::Button("Add Key##Pos"))
				{
					glm::vec3 defaultPos(0.0f);
					if (m_SelectedEntity.HasComponent<TransformComponent>())
						defaultPos = m_SelectedEntity.GetComponent<TransformComponent>().Position;

					PositionKeyframe k;
					k.time = animator.currentTime;
					k.position = defaultPos;
					clip.positionKeys.push_back(k);

					sortByTime(clip.positionKeys);
				}

				if (ImGui::BeginTable("PosKeysTable", 6,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
				{
					ImGui::TableSetupColumn("Index");
					ImGui::TableSetupColumn("Time");
					ImGui::TableSetupColumn("X##Pos");
					ImGui::TableSetupColumn("Y##Pos");
					ImGui::TableSetupColumn("Z##Pos");
					ImGui::TableSetupColumn("Remove");
					ImGui::TableHeadersRow();

					for (int i = 0; i < static_cast<int>(clip.positionKeys.size()); ++i)
					{
						auto& k = clip.positionKeys[static_cast<size_t>(i)];
						ImGui::PushID(i);

						ImGui::TableNextRow();

						bool isSelected = (m_DopesheetSelectedTrack == DopesheetTrackType::Position &&
							m_DopesheetSelectedKey == i);

						ImGui::TableSetColumnIndex(0);
						if (isSelected)
							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
						ImGui::Text("%d", i);
						if (isSelected)
							ImGui::PopStyleColor();

						// Click index cell to select and jump playhead
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Position;
							m_DopesheetSelectedKey = i;
							animator.currentTime = glm::clamp(k.time, 0.0f, clip.duration);
						}

						ImGui::TableSetColumnIndex(1);
						ImGui::DragFloat("##Time", &k.time, 0.01f, 0.0f, clip.duration);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Position;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(2);
						ImGui::DragFloat("##X", &k.position.x, 0.1f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Position;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(3);
						ImGui::DragFloat("##Y", &k.position.y, 0.1f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Position;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(4);
						ImGui::DragFloat("##Z", &k.position.z, 0.1f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Position;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(5);
						if (ImGui::SmallButton("X"))
						{
							clip.positionKeys.erase(clip.positionKeys.begin() + i);
							if (m_DopesheetSelectedTrack == DopesheetTrackType::Position &&
								m_DopesheetSelectedKey == i)
							{
								m_DopesheetSelectedTrack = DopesheetTrackType::None;
								m_DopesheetSelectedKey = -1;
							}
							ImGui::PopID();
							--i;
							continue;
						}

						ImGui::PopID();
					}

					sortByTime(clip.positionKeys);
					ImGui::EndTable();
				}
			}

			// Rotation track
			if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::Button("Add Key##Rot"))
				{
					glm::quat defaultRot = glm::quat(1, 0, 0, 0);
					if (m_SelectedEntity.HasComponent<TransformComponent>())
						defaultRot = m_SelectedEntity.GetComponent<TransformComponent>().Rotation;

					RotationKeyframe k;
					k.time = animator.currentTime;
					k.rotation = defaultRot;
					clip.rotationKeys.push_back(k);
					sortByTime(clip.rotationKeys);
				}

				if (ImGui::BeginTable("RotKeysTable", 6,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
				{
					ImGui::TableSetupColumn("Index");
					ImGui::TableSetupColumn("Time");
					ImGui::TableSetupColumn("Pitch");
					ImGui::TableSetupColumn("Yaw");
					ImGui::TableSetupColumn("Roll");
					ImGui::TableSetupColumn("Remove");
					ImGui::TableHeadersRow();

					for (int i = 0; i < static_cast<int>(clip.rotationKeys.size()); ++i)
					{
						auto& k = clip.rotationKeys[static_cast<size_t>(i)];
						ImGui::PushID(1000 + i);

						ImGui::TableNextRow();

						bool isSelected = (m_DopesheetSelectedTrack == DopesheetTrackType::Rotation &&
							m_DopesheetSelectedKey == i);

						ImGui::TableSetColumnIndex(0);
						if (isSelected)
							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
						ImGui::Text("%d", i);
						if (isSelected)
							ImGui::PopStyleColor();

						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Rotation;
							m_DopesheetSelectedKey = i;
							animator.currentTime = glm::clamp(k.time, 0.0f, clip.duration);
						}

						// Time
						ImGui::TableSetColumnIndex(1);
						ImGui::DragFloat("##Time", &k.time, 0.01f, 0.0f, clip.duration);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Rotation;
							m_DopesheetSelectedKey = i;
						}

						// --- quat -> Euler (deg) for UI ---
						glm::vec3 eulerDeg = glm::degrees(glm::eulerAngles(k.rotation));

						bool changed = false;

						ImGui::TableSetColumnIndex(2);
						changed |= ImGui::DragFloat("##Pitch", &eulerDeg.x, 1.0f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Rotation;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(3);
						changed |= ImGui::DragFloat("##Yaw", &eulerDeg.y, 1.0f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Rotation;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(4);
						changed |= ImGui::DragFloat("##Roll", &eulerDeg.z, 1.0f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Rotation;
							m_DopesheetSelectedKey = i;
						}

						// --- Only write back if something actually changed ---
						if (changed)
						{
							k.rotation = glm::quat(glm::radians(eulerDeg));
						}

						// Remove button
						ImGui::TableSetColumnIndex(5);
						if (ImGui::SmallButton("X"))
						{
							clip.rotationKeys.erase(clip.rotationKeys.begin() + i);
							if (m_DopesheetSelectedTrack == DopesheetTrackType::Rotation &&
								m_DopesheetSelectedKey == i)
							{
								m_DopesheetSelectedTrack = DopesheetTrackType::None;
								m_DopesheetSelectedKey = -1;
							}
							ImGui::PopID();
							--i;
							continue;
						}

						ImGui::PopID();
					}

					sortByTime(clip.rotationKeys);
					ImGui::EndTable();
				}
			}

			// Scale track
			if (ImGui::CollapsingHeader("Scale", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::Button("Add Key##Scale"))
				{
					glm::vec3 defaultScale(1.0f);
					if (m_SelectedEntity.HasComponent<TransformComponent>())
						defaultScale = m_SelectedEntity.GetComponent<TransformComponent>().Scale;

					ScaleKeyframe k;
					k.time = animator.currentTime;
					k.scale = defaultScale;
					clip.scaleKeys.push_back(k);

					sortByTime(clip.scaleKeys);
				}

				if (ImGui::BeginTable("ScaleKeysTable", 6,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
				{
					ImGui::TableSetupColumn("Index");
					ImGui::TableSetupColumn("Time");
					ImGui::TableSetupColumn("X##Scale");
					ImGui::TableSetupColumn("Y##Scale");
					ImGui::TableSetupColumn("Z##Scale");
					ImGui::TableSetupColumn("Remove");
					ImGui::TableHeadersRow();

					for (int i = 0; i < static_cast<int>(clip.scaleKeys.size()); ++i)
					{
						auto& k = clip.scaleKeys[static_cast<size_t>(i)];
						ImGui::PushID(2000 + i);

						ImGui::TableNextRow();

						bool isSelected = (m_DopesheetSelectedTrack == DopesheetTrackType::Scale &&
							m_DopesheetSelectedKey == i);

						ImGui::TableSetColumnIndex(0);
						if (isSelected)
							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
						ImGui::Text("%d", i);
						if (isSelected)
							ImGui::PopStyleColor();

						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Scale;
							m_DopesheetSelectedKey = i;
							animator.currentTime = glm::clamp(k.time, 0.0f, clip.duration);
						}

						ImGui::TableSetColumnIndex(1);
						ImGui::DragFloat("##Time", &k.time, 0.01f, 0.0f, clip.duration);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Scale;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(2);
						ImGui::DragFloat("##X", &k.scale.x, 0.1f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Scale;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(3);
						ImGui::DragFloat("##Y", &k.scale.y, 0.1f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Scale;
							m_DopesheetSelectedKey = i;
						}

						ImGui::TableSetColumnIndex(4);
						ImGui::DragFloat("##Z", &k.scale.z, 0.1f);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::Scale;
							m_DopesheetSelectedKey = i;
						}


						ImGui::TableSetColumnIndex(5);
						if (ImGui::SmallButton("X"))
						{
							clip.scaleKeys.erase(clip.scaleKeys.begin() + i);
							if (m_DopesheetSelectedTrack == DopesheetTrackType::Scale &&
								m_DopesheetSelectedKey == i)
							{
								m_DopesheetSelectedTrack = DopesheetTrackType::None;
								m_DopesheetSelectedKey = -1;
							}
							ImGui::PopID();
							--i;
							continue;
						}

						ImGui::PopID();
					}

					sortByTime(clip.scaleKeys);
					ImGui::EndTable();
				}
			}
		}
		else if (m_SelectedComponentTrack == AnimatorComponentTrack::UVTransform)
		{
			ImGui::Text("UV Transform");

			// Helper: sort by time after edits
			auto sortByTime = [](auto& keys)
				{
					std::sort(keys.begin(), keys.end(),
						[](const auto& a, const auto& b) { return a.time < b.time; });
				};

			// --------------------- Tiling track ---------------------
			if (ImGui::CollapsingHeader("Tiling", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::Button("Add Key##UVTiling"))
				{
					Engine::UVKeyframe k;
					k.time = animator.currentTime;

					// Default tiling 1,1 or copy last key
					if (!clip.uvTilingKeys.empty())
						k.value = clip.uvTilingKeys.back().value;
					else
						k.value = { 1.0f, 1.0f };

					clip.uvTilingKeys.push_back(k);
					sortByTime(clip.uvTilingKeys);
				}

				if (ImGui::BeginTable("UVTilingKeysTable", 5,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
				{
					ImGui::TableSetupColumn("Index");
					ImGui::TableSetupColumn("Time");
					ImGui::TableSetupColumn("U##UVTiling");
					ImGui::TableSetupColumn("V##UVTiling");
					ImGui::TableSetupColumn("Remove");
					ImGui::TableHeadersRow();

					for (int i = 0; i < static_cast<int>(clip.uvTilingKeys.size()); ++i)
					{
						auto& k = clip.uvTilingKeys[static_cast<size_t>(i)];
						ImGui::PushID(3000 + i);

						ImGui::TableNextRow();

						bool isSelected = (m_DopesheetSelectedTrack == DopesheetTrackType::UVTiling &&
							m_DopesheetSelectedKey == i);

						// Index
						ImGui::TableSetColumnIndex(0);
						if (isSelected)
							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
						ImGui::Text("%d", i);
						if (isSelected)
							ImGui::PopStyleColor();

						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVTiling;
							m_DopesheetSelectedKey = i;
							animator.currentTime = glm::clamp(k.time, 0.0f, clip.duration);
						}

						// Time
						ImGui::TableSetColumnIndex(1);
						ImGui::DragFloat("##Time", &k.time, 0.01f, 0.0f, clip.duration);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVTiling;
							m_DopesheetSelectedKey = i;
						}

						// U
						ImGui::TableSetColumnIndex(2);
						float u = k.value[0];
						if (ImGui::DragFloat("##U", &u, 0.01f))
							k.value[0] = u;
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVTiling;
							m_DopesheetSelectedKey = i;
						}

						// V
						ImGui::TableSetColumnIndex(3);
						float v = k.value[1];
						if (ImGui::DragFloat("##V", &v, 0.01f))
							k.value[1] = v;
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVTiling;
							m_DopesheetSelectedKey = i;
						}

						// Remove
						ImGui::TableSetColumnIndex(4);
						if (ImGui::SmallButton("X"))
						{
							clip.uvTilingKeys.erase(clip.uvTilingKeys.begin() + i);
							if (m_DopesheetSelectedTrack == DopesheetTrackType::UVTiling &&
								m_DopesheetSelectedKey == i)
							{
								m_DopesheetSelectedTrack = DopesheetTrackType::None;
								m_DopesheetSelectedKey = -1;
							}
							ImGui::PopID();
							--i;
							continue;
						}

						ImGui::PopID();
					}


					sortByTime(clip.uvTilingKeys);
					ImGui::EndTable();
				}
			}

			// --------------------- Offset track ---------------------
			if (ImGui::CollapsingHeader("Offset", ImGuiTreeNodeFlags_DefaultOpen))
			{
				if (ImGui::Button("Add Key##UVOffset"))
				{
					Engine::UVKeyframe k;
					k.time = animator.currentTime;

					// Default offset 0,0 or copy last key
					if (!clip.uvOffsetKeys.empty())
						k.value = clip.uvOffsetKeys.back().value;
					else
						k.value = { 0.0f, 0.0f };

					clip.uvOffsetKeys.push_back(k);
					sortByTime(clip.uvOffsetKeys);
				}

				if (ImGui::BeginTable("UVOffsetKeysTable", 5,
					ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
				{
					ImGui::TableSetupColumn("Index");
					ImGui::TableSetupColumn("Time");
					ImGui::TableSetupColumn("U##UVOffset");
					ImGui::TableSetupColumn("V##UVOffset");
					ImGui::TableSetupColumn("Remove");
					ImGui::TableHeadersRow();

					for (int i = 0; i < static_cast<int>(clip.uvOffsetKeys.size()); ++i)
					{
						auto& k = clip.uvOffsetKeys[static_cast<size_t>(i)];
						ImGui::PushID(4000 + i);

						ImGui::TableNextRow();

						bool isSelected = (m_DopesheetSelectedTrack == DopesheetTrackType::UVOffset &&
							m_DopesheetSelectedKey == i);

						// Index
						ImGui::TableSetColumnIndex(0);
						if (isSelected)
							ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
						ImGui::Text("%d", i);
						if (isSelected)
							ImGui::PopStyleColor();

						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVOffset;
							m_DopesheetSelectedKey = i;
							animator.currentTime = glm::clamp(k.time, 0.0f, clip.duration);
						}

						// Time
						ImGui::TableSetColumnIndex(1);
						ImGui::DragFloat("##Time", &k.time, 0.01f, 0.0f, clip.duration);
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVOffset;
							m_DopesheetSelectedKey = i;
						}

						// U
						ImGui::TableSetColumnIndex(2);
						float u = k.value[0];
						if (ImGui::DragFloat("##U", &u, 0.01f))
							k.value[0] = u;
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVOffset;
							m_DopesheetSelectedKey = i;
						}

						// V
						ImGui::TableSetColumnIndex(3);
						float v = k.value[1];
						if (ImGui::DragFloat("##V", &v, 0.01f))
							k.value[1] = v;
						if (ImGui::IsItemClicked())
						{
							m_DopesheetSelectedTrack = DopesheetTrackType::UVOffset;
							m_DopesheetSelectedKey = i;
						}

						// Remove
						ImGui::TableSetColumnIndex(4);
						if (ImGui::SmallButton("X"))
						{
							clip.uvOffsetKeys.erase(clip.uvOffsetKeys.begin() + i);
							if (m_DopesheetSelectedTrack == DopesheetTrackType::UVOffset &&
								m_DopesheetSelectedKey == i)
							{
								m_DopesheetSelectedTrack = DopesheetTrackType::None;
								m_DopesheetSelectedKey = -1;
							}
							ImGui::PopID();
							--i;
							continue;
						}

						ImGui::PopID();
					}


					sortByTime(clip.uvOffsetKeys);
					ImGui::EndTable();
				}
			}
		}

		// ---------------------------------------------------------------------
		// File toolbar: New / Save / Save As (for the current clip)
		// ---------------------------------------------------------------------
		if (clipPtr)
		{
			ImGui::SeparatorText("File");

			// ---- New Clip: create a brand-new asset and attach it ----
			if (ImGui::Button("New Clip"))
			{
				AnimationClip newClip{};
				newClip.name = "NewClip";
				newClip.duration = 1.0f;
				newClip.loop = true;

				// Allocate new handle = current storage size (simple allocator)
				u32 newHandle = static_cast<u32>(m_AnimationClipStorage.size());
				newClip.id = newHandle;

				m_AnimationClipStorage[newHandle] = newClip;

				// Attach to controller and select it
				controller.clips.push_back(newHandle);
				animator.currentClipIndex = static_cast<int>(controller.clips.size() - 1);

				// Reset selection
				m_DopesheetSelectedTrack = DopesheetTrackType::None;
				m_DopesheetSelectedKey = -1;
			}

			ImGui::SameLine();

			// ---- Save: overwrite current clip file (same id/handle) ----
			if (ImGui::Button("Save"))
			{
				Engine::SaveAnimationClipAsset(clip);
			}

			ImGui::SameLine();

			// ---- Save As: clone current clip into a new asset + new id ----
			static bool openSaveAsPopup = false;
			if (ImGui::Button("Save As"))
			{
				openSaveAsPopup = true;
				ImGui::OpenPopup("Save Clip As");
			}

			if (ImGui::BeginPopupModal("Save Clip As", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			{
				static char fileNameBuf[128] = "NewClip";

				// Pre-fill with current clip name when popup opens
				if (openSaveAsPopup)
				{
					std::string initial = clip.name.empty() ? "NewClip" : clip.name;
					std::snprintf(fileNameBuf, sizeof(fileNameBuf), "%s", initial.c_str());
					openSaveAsPopup = false;
				}

				ImGui::Text("File name (without extension):");
				ImGui::InputText("##ClipFileName", fileNameBuf, IM_ARRAYSIZE(fileNameBuf));

				if (ImGui::Button("OK"))
				{
					std::string fileName = fileNameBuf;
					if (fileName.empty())
						fileName = "NewClip";

					// Clone current clip into a new asset
					AnimationClip newClip = clip;
					newClip.name = fileName;

					u32 newHandle = static_cast<u32>(m_AnimationClipStorage.size());
					newClip.id = newHandle;

					m_AnimationClipStorage[newHandle] = newClip;
					controller.clips.push_back(newHandle);
					animator.currentClipIndex = static_cast<int>(controller.clips.size() - 1);

					std::string path1 = "../../Resources/Sources/AnimationClips/" + fileName + ".animclip";
					std::string path2 = "../bin/Debug/Resources/Sources/AnimationClips/" + fileName + ".animclip";

					SerializeAnimationClip(newClip, path1);
					SerializeAnimationClip(newClip, path2);

					ImGui::CloseCurrentPopup();
				}

				ImGui::SameLine();

				if (ImGui::Button("Cancel"))
				{
					ImGui::CloseCurrentPopup();
				}

				ImGui::EndPopup();
			}
		}

		ImGui::EndChild(); // End of left pane

		// =============================== RIGHT PANE ===========================
		ImGui::SameLine();

		ImGui::BeginChild("Animator_RightPanel", ImVec2(0.0f, 0.0f), true);

		// View mode buttons
		ImGui::SeparatorText("View");
		if (ImGui::RadioButton("Dopesheet", m_AnimatorViewMode == AnimatorViewMode::Dopesheet))
			m_AnimatorViewMode = AnimatorViewMode::Dopesheet;
		ImGui::SameLine();
		if (ImGui::RadioButton("Curves", m_AnimatorViewMode == AnimatorViewMode::Curves))
			m_AnimatorViewMode = AnimatorViewMode::Curves;

		ImGui::Separator();

		//// Colors per track
		//const ImU32 colPos = IM_COL32(80, 200, 120, 255);
		//const ImU32 colRot = IM_COL32(220, 100, 100, 255);
		//const ImU32 colScale = IM_COL32(100, 140, 230, 255);
		//const ImU32 colPosSel = IM_COL32(130, 255, 170, 255);
		//const ImU32 colRotSel = IM_COL32(255, 170, 170, 255);
		//const ImU32 colScaleSel = IM_COL32(160, 190, 255, 255);
		//const ImU32 colPlayhead = IM_COL32(255, 255, 50, 255);

		if (m_AnimatorViewMode == AnimatorViewMode::Dopesheet)
		{

			// Colors per track
			const ImU32 colPos = IM_COL32(80, 200, 120, 255);
			const ImU32 colRot = IM_COL32(220, 100, 100, 255);
			const ImU32 colScale = IM_COL32(100, 140, 230, 255);
			const ImU32 colPosSel = IM_COL32(130, 255, 170, 255);
			const ImU32 colRotSel = IM_COL32(255, 170, 170, 255);
			const ImU32 colScaleSel = IM_COL32(160, 190, 255, 255);

			const ImU32 colUVTiling = IM_COL32(220, 200, 80, 255);
			const ImU32 colUVOffset = IM_COL32(120, 180, 220, 255);
			const ImU32 colUVTilingSel = IM_COL32(255, 240, 120, 255);
			const ImU32 colUVOffsetSel = IM_COL32(170, 220, 255, 255);

			const ImU32 colPlayhead = IM_COL32(255, 255, 50, 255);

			// Legend
			ImGui::Text("Legend:");
			ImDrawList* legendList = ImGui::GetWindowDrawList();

			auto drawLegendItem = [&](ImU32 col, const char* label)
				{
					ImVec2 p = ImGui::GetCursorScreenPos();
					ImVec2 sz(12.0f, 12.0f);
					legendList->AddRectFilled(p, ImVec2(p.x + sz.x, p.y + sz.y), col);
					ImGui::Dummy(sz);
					ImGui::SameLine();
					ImGui::TextUnformatted(label);
				};

			if (m_SelectedComponentTrack == AnimatorComponentTrack::Transform)
			{
				drawLegendItem(colPos, "Position");
				drawLegendItem(colRot, "Rotation");
				drawLegendItem(colScale, "Scale");
			}
			else if (m_SelectedComponentTrack == AnimatorComponentTrack::UVTransform)
			{
				drawLegendItem(colUVTiling, "UV Tiling");
				drawLegendItem(colUVOffset, "UV Offset");
			}

			ImGui::NewLine();

			// Timeline canvas
			ImVec2 canvasPos = ImGui::GetCursorScreenPos();
			ImVec2 canvasSize = ImGui::GetContentRegionAvail();
			if (canvasSize.x < 50.0f) canvasSize.x = 50.0f;
			if (canvasSize.y < 60.0f) canvasSize.y = 60.0f;
			ImVec2 canvasEnd = ImVec2(canvasPos.x + canvasSize.x, canvasPos.y + canvasSize.y);

			ImGui::InvisibleButton("##AnimDopesheet", canvasSize);
			bool  timelineHovered = ImGui::IsItemHovered();
			ImDrawList* drawList = ImGui::GetWindowDrawList();

			// Background
			drawList->AddRectFilled(canvasPos, canvasEnd, IM_COL32(20, 20, 20, 255));
			drawList->AddRect(canvasPos, canvasEnd, IM_COL32(80, 80, 80, 255));

			float midY = 0.5f * (canvasPos.y + canvasEnd.y);
			float keyRadius = 4.0f;
			float keyRadiusSq = keyRadius * keyRadius;

			// Time -> X mapping
			auto timeToX = [&](float t)
				{
					float u = (clip.duration > 0.0f) ? (t / clip.duration) : 0.0f;
					if (u < 0.0f) u = 0.0f;
					if (u > 1.0f) u = 1.0f;
					return canvasPos.x + u * canvasSize.x;
				};

			// ---------------- CLICK HANDLING (select nearest key or scrub) ----------------
			if (timelineHovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
			{
				ImVec2 mousePos = ImGui::GetIO().MousePos;
				float bestDistSq = keyRadiusSq;
				DopesheetTrackType bestTrack = DopesheetTrackType::None;
				int bestIndex = -1;

				auto testKeys = [&](const auto& keys,
					DopesheetTrackType trackType,
					float yOffset)
					{
						for (int i = 0; i < static_cast<int>(keys.size()); ++i)
						{
							float x = timeToX(keys[static_cast<size_t>(i)].time);
							float y = midY + yOffset;

							float dx = mousePos.x - x;
							float dy = mousePos.y - y;
							float d2 = dx * dx + dy * dy;

							if (d2 < bestDistSq)
							{
								bestDistSq = d2;
								bestTrack = trackType;
								bestIndex = i;
							}
						}
					};

				if (m_SelectedComponentTrack == AnimatorComponentTrack::Transform)
				{
					testKeys(clip.positionKeys, DopesheetTrackType::Position, -12.0f);
					testKeys(clip.rotationKeys, DopesheetTrackType::Rotation, 0.0f);
					testKeys(clip.scaleKeys, DopesheetTrackType::Scale, +12.0f);
				}
				else if (m_SelectedComponentTrack == AnimatorComponentTrack::UVTransform)
				{
					testKeys(clip.uvTilingKeys, DopesheetTrackType::UVTiling, -6.0f);
					testKeys(clip.uvOffsetKeys, DopesheetTrackType::UVOffset, +6.0f);
				}

				if (bestTrack != DopesheetTrackType::None)
				{
					// Select nearest key and jump playhead
					m_DopesheetSelectedTrack = bestTrack;
					m_DopesheetSelectedKey = bestIndex;

					float keyTime = 0.0f;
					switch (bestTrack)
					{
					case DopesheetTrackType::Position:
						keyTime = clip.positionKeys[static_cast<size_t>(bestIndex)].time;
						break;
					case DopesheetTrackType::Rotation:
						keyTime = clip.rotationKeys[static_cast<size_t>(bestIndex)].time;
						break;
					case DopesheetTrackType::Scale:
						keyTime = clip.scaleKeys[static_cast<size_t>(bestIndex)].time;
						break;
					case DopesheetTrackType::UVTiling:
						keyTime = clip.uvTilingKeys[static_cast<size_t>(bestIndex)].time;
						break;
					case DopesheetTrackType::UVOffset:
						keyTime = clip.uvOffsetKeys[static_cast<size_t>(bestIndex)].time;
						break;
					default:
						break;
					}

					animator.currentTime = glm::clamp(keyTime, 0.0f, clip.duration);
				}
				else
				{
					// Clicked empty region: scrub time
					float tNorm = (mousePos.x - canvasPos.x) / canvasSize.x;
					tNorm = glm::clamp(tNorm, 0.0f, 1.0f);
					animator.currentTime = tNorm * clip.duration;
					m_DopesheetSelectedTrack = DopesheetTrackType::None;
					m_DopesheetSelectedKey = -1;
				}
			}

			// ---------------- DRAW KEYS (with selection highlight) ----------------
			auto drawTrackKeys = [&](auto& keys,
				DopesheetTrackType trackType,
				ImU32 col,
				ImU32 colSelected,
				float yOffset)
				{
					for (int i = 0; i < static_cast<int>(keys.size()); ++i)
					{
						float x = timeToX(keys[static_cast<size_t>(i)].time);
						float y = midY + yOffset;

						bool selected = (m_DopesheetSelectedTrack == trackType &&
							m_DopesheetSelectedKey == i);

						ImU32 useCol = selected ? colSelected : col;
						drawList->AddCircleFilled(ImVec2(x, y), keyRadius, useCol);
					}
				};

			if (m_SelectedComponentTrack == AnimatorComponentTrack::Transform)
			{
				drawTrackKeys(clip.positionKeys, DopesheetTrackType::Position,
					colPos, colPosSel, -12.0f);
				drawTrackKeys(clip.rotationKeys, DopesheetTrackType::Rotation,
					colRot, colRotSel, 0.0f);
				drawTrackKeys(clip.scaleKeys, DopesheetTrackType::Scale,
					colScale, colScaleSel, +12.0f);
			}
			else if (m_SelectedComponentTrack == AnimatorComponentTrack::UVTransform)
			{
				drawTrackKeys(clip.uvTilingKeys, DopesheetTrackType::UVTiling,
					colUVTiling, colUVTilingSel, -6.0f);
				drawTrackKeys(clip.uvOffsetKeys, DopesheetTrackType::UVOffset,
					colUVOffset, colUVOffsetSel, +6.0f);
			}

			// Playhead line
			{
				float x = timeToX(animator.currentTime);
				drawList->AddLine(ImVec2(x, canvasPos.y),
					ImVec2(x, canvasEnd.y),
					colPlayhead, 2.0f);
			}
		}
		else // ============================== CURVES VIEW ====================== 
		{
			// Choose which track to display curves for
			DopesheetTrackType track = m_DopesheetSelectedTrack;

			// If nothing explicitly selected, pick something sensible
			if (track == DopesheetTrackType::None)
			{
				if (m_SelectedComponentTrack == AnimatorComponentTrack::Transform)
				{
					if (!clip.positionKeys.empty())      track = DopesheetTrackType::Position;
					else if (!clip.rotationKeys.empty()) track = DopesheetTrackType::Rotation;
					else if (!clip.scaleKeys.empty())    track = DopesheetTrackType::Scale;
				}
				else if (m_SelectedComponentTrack == AnimatorComponentTrack::UVTransform)
				{
					if (!clip.uvTilingKeys.empty())      track = DopesheetTrackType::UVTiling;
					else if (!clip.uvOffsetKeys.empty()) track = DopesheetTrackType::UVOffset;
				}
			}

			if (track == DopesheetTrackType::None)
			{
				ImGui::TextUnformatted("No keys to display curves for.");
				ImGui::EndChild();
				ImGui::End();
				return;
			}

			const ImU32 COL_X = IM_COL32(230, 90, 90, 255);   // red
			const ImU32 COL_Y = IM_COL32(100, 220, 120, 255);   // green
			const ImU32 COL_Z = IM_COL32(110, 160, 255, 255);   // blue

			// Legend
			switch (track)
			{
			case DopesheetTrackType::Position:
				DrawCurveLegendRow("Position", "X", COL_X, "Y", COL_Y, "Z", COL_Z);
				break;
			case DopesheetTrackType::Rotation:
				DrawCurveLegendRow("Rotation", "Pitch", COL_X, "Yaw", COL_Y, "Roll", COL_Z);
				break;
			case DopesheetTrackType::Scale:
				DrawCurveLegendRow("Scale", "X", COL_X, "Y", COL_Y, "Z", COL_Z);
				break;
			case DopesheetTrackType::UVTiling:
				// We only really use X/Y; Z legend is dummy
				DrawCurveLegendRow("UV Tiling", "U", COL_X, "V", COL_Y, "-", IM_COL32(80, 80, 80, 255));
				break;
			case DopesheetTrackType::UVOffset:
				DrawCurveLegendRow("UV Offset", "U", COL_X, "V", COL_Y, "-", IM_COL32(80, 80, 80, 255));
				break;
			default:
				break;
			}

			ImGui::Separator();

			ImVec2 cPos = ImGui::GetCursorScreenPos();
			ImVec2 cSize = ImGui::GetContentRegionAvail();
			if (cSize.x < 50.0f) cSize.x = 50.0f;
			if (cSize.y < 80.0f) cSize.y = 80.0f;
			ImVec2 cEnd = ImVec2(cPos.x + cSize.x, cPos.y + cSize.y);

			ImGui::InvisibleButton("##AnimCurves", cSize);
			ImDrawList* cDraw = ImGui::GetWindowDrawList();

			cDraw->AddRectFilled(cPos, cEnd, IM_COL32(20, 20, 20, 255));
			cDraw->AddRect(cPos, cEnd, IM_COL32(80, 80, 80, 255));

			auto timeToXCurve = [&](float t)
				{
					float u = (clip.duration > 0.0f) ? (t / clip.duration) : 0.0f;
					if (u < 0.0f) u = 0.0f;
					if (u > 1.0f) u = 1.0f;
					return cPos.x + u * cSize.x;
				};

			// Generic vec3 curve drawer (Position / Rotation / Scale)
			auto drawVec3Curves = [&](const auto& keys, auto getVec)
				{
					if (keys.size() < 2)
						return;

					std::vector<glm::vec3> values;
					values.reserve(keys.size());

					float minVal = 0.0f, maxVal = 0.0f;
					bool  first = true;

					for (size_t i = 0; i < keys.size(); ++i)
					{
						glm::vec3 v = getVec(keys[i]);
						values.push_back(v);

						float localMin = std::min(v.x, std::min(v.y, v.z));
						float localMax = std::max(v.x, std::max(v.y, v.z));

						if (first)
						{
							minVal = localMin;
							maxVal = localMax;
							first = false;
						}
						else
						{
							if (localMin < minVal) minVal = localMin;
							if (localMax > maxVal) maxVal = localMax;
						}
					}

					if (maxVal - minVal < 1e-3f)
					{
						maxVal += 0.5f;
						minVal -= 0.5f;
					}

					auto valToY = [&](float v)
						{
							float u = (v - minVal) / (maxVal - minVal);
							if (u < 0.0f) u = 0.0f;
							if (u > 1.0f) u = 1.0f;
							return cEnd.y - u * cSize.y;
						};

					auto drawAxis = [&](int axis, ImU32 color)
						{
							ImVec2 prev;
							bool   hasPrev = false;
							for (size_t i = 0; i < keys.size(); ++i)
							{
								float t = keys[i].time;
								float x = timeToXCurve(t);
								float v = (axis == 0) ? values[i].x :
									(axis == 1) ? values[i].y : values[i].z;
								float y = valToY(v);

								ImVec2 cur(x, y);
								if (hasPrev)
									cDraw->AddLine(prev, cur, color, 2.0f);
								prev = cur;
								hasPrev = true;
							}
						};

					// X/Y/Z curves
					drawAxis(0, COL_X);
					drawAxis(1, COL_Y);
					drawAxis(2, COL_Z);
				};

			// Vec2 curve drawer (UV Tiling / Offset)
			auto drawVec2Curves = [&](const auto& keys, auto getVec)
				{
					if (keys.size() < 2)
						return;

					std::vector<glm::vec2> values;
					values.reserve(keys.size());

					float minVal = 0.0f, maxVal = 0.0f;
					bool  first = true;

					for (size_t i = 0; i < keys.size(); ++i)
					{
						glm::vec2 v = getVec(keys[i]);
						values.push_back(v);

						float localMin = std::min(v.x, v.y);
						float localMax = std::max(v.x, v.y);

						if (first)
						{
							minVal = localMin;
							maxVal = localMax;
							first = false;
						}
						else
						{
							if (localMin < minVal) minVal = localMin;
							if (localMax > maxVal) maxVal = localMax;
						}
					}

					if (maxVal - minVal < 1e-3f)
					{
						maxVal += 0.5f;
						minVal -= 0.5f;
					}

					auto valToY = [&](float v)
						{
							float u = (v - minVal) / (maxVal - minVal);
							if (u < 0.0f) u = 0.0f;
							if (u > 1.0f) u = 1.0f;
							return cEnd.y - u * cSize.y;
						};

					auto drawAxis = [&](int axis, ImU32 color)
						{
							ImVec2 prev;
							bool   hasPrev = false;
							for (size_t i = 0; i < keys.size(); ++i)
							{
								float t = keys[i].time;
								float x = timeToXCurve(t);
								float v = (axis == 0) ? values[i].x : values[i].y;
								float y = valToY(v);

								ImVec2 cur(x, y);
								if (hasPrev)
									cDraw->AddLine(prev, cur, color, 2.0f);
								prev = cur;
								hasPrev = true;
							}
						};

					// U/V curves
					drawAxis(0, COL_X);
					drawAxis(1, COL_Y);
				};

			// Dispatch to correct curve drawer
			switch (track)
			{
			case DopesheetTrackType::Position:
				drawVec3Curves(clip.positionKeys,
					[](const PositionKeyframe& k) { return k.position; });
				break;
			case DopesheetTrackType::Rotation:
				drawVec3Curves(clip.rotationKeys,
					[](const RotationKeyframe& k)
					{
						glm::vec3 euler = glm::degrees(glm::eulerAngles(k.rotation));
						return euler;
					});
				break;
			case DopesheetTrackType::Scale:
				drawVec3Curves(clip.scaleKeys,
					[](const ScaleKeyframe& k) { return k.scale; });
				break;
			case DopesheetTrackType::UVTiling:
				drawVec2Curves(clip.uvTilingKeys,
					[](const UVKeyframe& k)
					{
						return glm::vec2(k.value[0], k.value[1]);
					});
				break;
			case DopesheetTrackType::UVOffset:
				drawVec2Curves(clip.uvOffsetKeys,
					[](const UVKeyframe& k)
					{
						return glm::vec2(k.value[0], k.value[1]);
					});
				break;
			default:
				break;
			}

		}
		ImGui::EndChild(); // End of right pane

		ImGui::End();
	}

	// Add Component
	void EditorPropertyPanel::AddComponent(){
		ImGui::Separator();
		ImVec2 windowSize = ImGui::GetWindowSize(); // get Properties window sizes
		ImVec2  addComponetbtnSize(140, 40); // set button size

		// Calculate centered position for x axis
		ImGui::SetCursorPosX((windowSize.x - addComponetbtnSize.x) * 0.5f);

		if (ImGui::Button("Add Component", addComponetbtnSize))
		{
			ImGui::OpenPopup("AddComponentPopup");
		}
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Add new component.");
		}

		if (ImGui::BeginPopup("AddComponentPopup"))
		{

			// -------------------- Add Transform Component -------------------------
			bool hasTransform = m_SelectedEntity.HasComponent<TransformComponent>();

			ImGui::BeginDisabled(hasTransform);

			if (ImGui::MenuItem("Transform3D Component"))
			{
				if (!hasTransform)
				{
					m_SelectedEntity.AddComponent<TransformComponent>();
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasTransform)
				{
					ImGui::SetTooltip("Position, rotation, and scale of the object.");
				}
			}

			ImGui::EndDisabled();

			// ------------------------ Add RigidBody Component ----------------------------
			bool hasRigidBody = m_SelectedEntity.HasComponent<RigidbodyComponent>();
			ImGui::BeginDisabled(hasRigidBody);

			if (ImGui::MenuItem("RigidBody Component"))
			{
				if (!hasRigidBody)
				{
					m_SelectedEntity.AddComponent<RigidbodyComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();
						// Serialize the newly added component
						std::string componentJSON = ComponentSerializer::SerializeComponent(
							m_SelectedEntity, ComponentTypeID::RigidBody);
						prefabComp.MarkComponentAdded(ComponentTypeID::RigidBody, componentJSON);
					}
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::RigidBody) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::RigidBody);
							prefabComp.MarkComponentAdded(ComponentTypeID::RigidBody, componentJSON);
							LOG_INFO("Marked RigidBody as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::RigidBody);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::RigidBody);
							LOG_INFO("Marked RigidBody as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasRigidBody)
				{
					ImGui::SetTooltip("Simulates physical: movement, rotation, and collisions.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Mesh Component ----------------------------
			bool hasMeshRenderComponent = m_SelectedEntity.HasComponent<MeshRendererComponent>();
			ImGui::BeginDisabled(hasMeshRenderComponent);

			if (ImGui::MenuItem("MeshRenderer Component"))
			{
				if (!hasMeshRenderComponent)
				{
					m_SelectedEntity.AddComponent<MeshRendererComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::MeshRenderer) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::MeshRenderer);
							prefabComp.MarkComponentAdded(ComponentTypeID::MeshRenderer, componentJSON);
							LOG_INFO("Marked MeshRenderer as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::MeshRenderer);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::MeshRenderer);
							LOG_INFO("Marked MeshRenderer as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasMeshRenderComponent)
				{
					ImGui::SetTooltip("Defines the visual 3D model of the object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Audio Component ----------------------------
			bool hasAudioComponent = m_SelectedEntity.HasComponent<AudioComponent>();
			ImGui::BeginDisabled(hasAudioComponent);

			if (ImGui::MenuItem("Audio Component"))
			{
				if (!hasAudioComponent)
				{
					m_SelectedEntity.AddComponent<AudioComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::Audio) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::Audio);
							prefabComp.MarkComponentAdded(ComponentTypeID::Audio, componentJSON);
							LOG_INFO("Marked Audio as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::Audio);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::Audio);
							LOG_INFO("Marked Audio as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasAudioComponent)
				{
					ImGui::SetTooltip("Adds sound playback to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Reverb Component ----------------------------
			bool hasReverbComponent = m_SelectedEntity.HasComponent<ReverbZoneComponent>();
			ImGui::BeginDisabled(hasReverbComponent);

			if (ImGui::MenuItem("Reverb Zone Component"))
			{
				if (!hasReverbComponent)
				{
					m_SelectedEntity.AddComponent<ReverbZoneComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::ReverbZone) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::ReverbZone);
							prefabComp.MarkComponentAdded(ComponentTypeID::ReverbZone, componentJSON);
							LOG_INFO("Marked ReverbZone as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::ReverbZone);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::ReverbZone);
							LOG_INFO("Marked ReverbZone as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasReverbComponent)
				{
					ImGui::SetTooltip("Adds reverb zone to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Listener Component ----------------------------
			bool hasListenerComponent = m_SelectedEntity.HasComponent<ListenerComponent>();
			ImGui::BeginDisabled(hasListenerComponent);

			if (ImGui::MenuItem("Listener Component"))
			{
				if (!hasListenerComponent)
				{
					m_SelectedEntity.AddComponent<ListenerComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::Listerner) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::Listerner);
							prefabComp.MarkComponentAdded(ComponentTypeID::Listerner, componentJSON);
							LOG_INFO("Marked Listerner as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::Listerner);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::Listerner);
							LOG_INFO("Marked Listerner as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasListenerComponent)
				{
					ImGui::SetTooltip("Sets object as listener.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Behavior Tree Component ----------------------------
			bool hasBehaviorTree = m_SelectedEntity.HasComponent<BehaviourTreeComponent>();
			ImGui::BeginDisabled(hasBehaviorTree);

			if (ImGui::MenuItem("Behaviour Tree Component"))
			{
				if (!hasBehaviorTree)
				{
					m_SelectedEntity.AddComponent<BehaviourTreeComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::BehaviourTree) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::BehaviourTree);
							prefabComp.MarkComponentAdded(ComponentTypeID::BehaviourTree, componentJSON);
							LOG_INFO("Marked BehaviourTree as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::BehaviourTree);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::BehaviourTree);
							LOG_INFO("Marked BehaviourTree as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasBehaviorTree)
				{
					ImGui::SetTooltip("Adds behaviour tree to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Particle Component ----------------------------
			bool hasParticleSystem = m_SelectedEntity.HasComponent<ParticleComponent>();
			ImGui::BeginDisabled(hasParticleSystem);

			if (ImGui::MenuItem("Particle System Component"))
			{
				if (!hasParticleSystem)
				{
					m_SelectedEntity.AddComponent<ParticleComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::ParticleSystem) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::ParticleSystem);
							prefabComp.MarkComponentAdded(ComponentTypeID::ParticleSystem, componentJSON);
							LOG_INFO("Marked ParticleSystem as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::ParticleSystem);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::ParticleSystem);
							LOG_INFO("Marked ParticleSystem as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasParticleSystem)
				{
					ImGui::SetTooltip("Adds particle system to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Script Component ----------------------------
			bool hasScriptComponent = m_SelectedEntity.HasComponent<ScriptComponent>();
			ImGui::BeginDisabled(hasScriptComponent);

			if (ImGui::MenuItem("Script Component"))
			{
				if (!hasScriptComponent)
				{
					m_SelectedEntity.AddComponent<ScriptComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::Script) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::Script);
							prefabComp.MarkComponentAdded(ComponentTypeID::Script, componentJSON);
							LOG_INFO("Marked Script as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::Script);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::Script);
							LOG_INFO("Marked Script as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasScriptComponent)
				{
					ImGui::SetTooltip("Add script to this object.");
				}
			}
			ImGui::EndDisabled();
			// ------------------------ Add Light Component ----------------------------
			bool hasLightComponent = m_SelectedEntity.HasComponent<LightComponent>();
			ImGui::BeginDisabled(hasLightComponent);

			if (ImGui::MenuItem("Light Component"))
			{
				if (!hasLightComponent)
				{
					m_SelectedEntity.AddComponent<LightComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::Light) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::Light);
							prefabComp.MarkComponentAdded(ComponentTypeID::Light, componentJSON);
							LOG_INFO("Marked Light as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::Light);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::Light);
							LOG_INFO("Marked Light as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasLightComponent)
				{
					ImGui::SetTooltip("Add light to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Canera Component ----------------------------
			bool hasCameraComponent = m_SelectedEntity.HasComponent<CameraComponent>();
			ImGui::BeginDisabled(hasCameraComponent);

			if (ImGui::MenuItem("Camera Component"))
			{
				if (!hasCameraComponent)
				{
					m_SelectedEntity.AddComponent<CameraComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::Camera) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::Camera);
							prefabComp.MarkComponentAdded(ComponentTypeID::Camera, componentJSON);
							LOG_INFO("Marked Camera as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::Camera);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::Camera);
							LOG_INFO("Marked Camera as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasCameraComponent)
				{
					ImGui::SetTooltip("Add camera to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add Animator Component ----------------------------
			bool hasAnimatorComponent = m_SelectedEntity.HasComponent<AnimatorComponent>();
			ImGui::BeginDisabled(hasAnimatorComponent);

			if (ImGui::MenuItem("Animator Component"))
			{
				if (!hasAnimatorComponent)
				{
					m_SelectedEntity.AddComponent<AnimatorComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::Animator) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::Animator);
							prefabComp.MarkComponentAdded(ComponentTypeID::Animator, componentJSON);
							LOG_INFO("Marked Animator as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::Animator);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::Animator);
							LOG_INFO("Marked Animator as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasAnimatorComponent)
				{
					ImGui::SetTooltip("Adds animation playback data (controller, clip index, time) to this object.");
				}
			}
			ImGui::EndDisabled();

			// ------------------------ Add SpriteRenderer Component ----------------------------
			bool hasSpriteRenderer = m_SelectedEntity.HasComponent<SpriteRendererComponent>();
			ImGui::BeginDisabled(hasSpriteRenderer);

			if (ImGui::MenuItem("Sprite Renderer Component"))
			{
				if (!hasSpriteRenderer)
				{
					m_SelectedEntity.AddComponent<SpriteRendererComponent>();
					if (m_SelectedEntity.HasComponent<PrefabComponent>()) {
						auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();

						// Check if this component exists in the prefab blueprint
						bool existsInPrefab = false;
						if (prefabComp.isPrefabRoot) {
							// For root, check prefab registry
							Prefab prefab;
							if (PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
								if (const PrefabEntityData* entityData = prefab.GetRootEntity()) {
									for (const auto& comp : entityData->components) {
										if (comp.type == ComponentTypeID::SpriteRenderer) {
											existsInPrefab = true;
											break;
										}
									}
								}
							}
						}

						// Mark appropriately
						if (!existsInPrefab) {
							// New component not in prefab = added override
							std::string componentJSON = ComponentSerializer::SerializeComponent(
								m_SelectedEntity, ComponentTypeID::SpriteRenderer);
							prefabComp.MarkComponentAdded(ComponentTypeID::SpriteRenderer, componentJSON);
							LOG_INFO("Marked SpriteRenderer as ADDED component (not in prefab)");
						}
						else {
							// Component exists in prefab but was removed, now re-added
							// Clear the removal flag
							prefabComp.ClearComponentRemoval(ComponentTypeID::SpriteRenderer);
							prefabComp.ClearAllOverridesForComponent(ComponentTypeID::SpriteRenderer);
							LOG_INFO("Marked SpriteRenderer as RESTORED (was removed, now re-added)");
						}
					}
				}
			}
			if (ImGui::IsItemHovered())
			{
				if (!hasSpriteRenderer)
				{
					ImGui::SetTooltip("Adds 2D sprite data to this object.");
				}
			}
			ImGui::EndDisabled();
			ImGui::EndPopup(); // end pop up for Add Component  
		}
	}

	// Helper Functions
	const char* EditorPropertyPanel::ColliderTypeToString(ColliderType& colliderType) {
		if (colliderType == ColliderType::AABB) {
			return "AABB";
		}
		else if (colliderType == ColliderType::BOX) {
			return "Box";
		}
		else if (colliderType == ColliderType::MESH) {
			return "Mesh";
		}
		else if (colliderType == ColliderType::SPHERE) {
			return "Sphere";
		}
		return "Unknown";
	}

	void EditorPropertyPanel::ReplaceChildNode(std::shared_ptr<BTNode> parent,
		std::shared_ptr<BTNode> oldChild,
		std::shared_ptr<BTNode> newChild)
	{
		auto& children = parent->GetChildren();
		for (size_t i = 0; i < children.size(); ++i)
		{
			if (children[i] == oldChild)
			{
				// Replaces old child node with new child
				children[i] = newChild;
				break;
			}
		}
	}

	void EditorPropertyPanel::DrawBTNodeEditor(std::shared_ptr<BTNode> node, std::shared_ptr<BTNode> parent)
	{
		if (!node) return;

		// Unique label for ImGui to avoid ID collisions
		std::string labelID = node->GetName() + "##" + std::to_string(reinterpret_cast<uintptr_t>(node.get()));

		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (!node->GetChildren().empty()) {
			flags |= ImGuiTreeNodeFlags_DefaultOpen;
		}
		bool nodeOpen = ImGui::TreeNodeEx(labelID.c_str(), flags, "%s [%s]", node->GetName().c_str(), node->GetTypeName());

		// --- Right-click context menu ---
		if (ImGui::BeginPopupContextItem())
		{
			static int selectedType = 0;
			static bool addChild = false;

			// Add Child 
			if (node->CanHaveChildren()) {
				// Show Add Child submenu
				if (ImGui::BeginMenu("Add Child"))
				{
					auto allTypes = BehaviourTreeEditor::GetAllNodeTypes();

					ImGui::Text("Select Node Type:");
					ImGui::Separator();

					for (int i = 0; i < (int)allTypes.size(); ++i)
					{
						bool isSelected = (selectedType == i);
						if (ImGui::Selectable(allTypes[i].c_str(), isSelected))
						{
							selectedType = i;
							addChild = true;
						}
						if (isSelected) {
							ImGui::SetItemDefaultFocus();
						}
					}

					ImGui::EndMenu();
				}

				if (addChild)
				{
					addChild = false;
					auto allTypes = BehaviourTreeEditor::GetAllNodeTypes();
					auto newChild = BehaviourTreeEditor::CreateNode(allTypes[selectedType]);
					BehaviourTreeEditor::AddChildNode(node, newChild);
				}
			}

			// If not a root node
			if (parent)
			{
				if (ImGui::MenuItem("Remove Node"))
				{
					// Leaf node that's child of decorator cannot delete without deleting decorator
					BehaviourTreeEditor::RemoveChildNode(parent, node);
					ImGui::EndPopup();

					// If opened, pop tree node before returning
					if (nodeOpen) {
						ImGui::TreePop();
					}

					return;
				}
			}

			ImGui::EndPopup();
		}

		// --- Node Editing ---
		if (nodeOpen)
		{
			// Node name
			char nameBuffer[128];
			strncpy_s(nameBuffer, node->GetName().c_str(), _TRUNCATE);
			if (ImGui::InputText(("Name##" + labelID).c_str(), nameBuffer, IM_ARRAYSIZE(nameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
			{
				std::string newName = nameBuffer;
				newName.erase(newName.find_last_not_of(" \t\n\r\f\v") + 1);
				newName.erase(0, newName.find_first_not_of(" \t\n\r\f\v"));

				if (!newName.empty()) {
					node->SetName(nameBuffer);
				}

			}

			std::vector<std::string> allTypes = BehaviourTreeEditor::GetAllNodeTypes();
			//Get current index of node type
			int currentTypeIndex = 0;
			for (size_t i = 0; i < allTypes.size(); ++i)
			{
				if (allTypes[i] == node->GetTypeName())
				{
					currentTypeIndex = (int)i;
					break;
				}
			}

			// Node type selector
			if (ImGui::Combo(("Type##" + labelID).c_str(), &currentTypeIndex,
				[](void* data, int idx, const char** outText) {
					auto& types = *static_cast<std::vector<std::string>*>(data);
					*outText = types[idx].c_str();
					return true;
				},
				static_cast<void*>(&allTypes), (int)allTypes.size()))
			{
				// Only change the type if its not a root node
				if (parent)
				{
					// Make new node of chosen type
					auto newNode = BehaviourTreeEditor::CreateNode(allTypes[currentTypeIndex]);
					newNode->SetName(node->GetName());

					// Transfer children
					for (auto& child : node->GetChildren()) {
						newNode->AddChild(child);
					}

					// Replace node in parent's children list
					ReplaceChildNode(parent, node, newNode);
				}
			}

			std::vector<std::pair<std::string, std::string>> properties;
			node->GetProperties(properties);

			// Helper lambda function to determine if textbox only handles numeric
			auto isNumeric = [](const std::string& s) {
				if (s.empty()) {
					return false;
				}
				bool hasDot = false;
				bool hasDash = false;
				for (unsigned char c : s) {
					if (std::isdigit(c)) {
						continue;
					}
					else if (c == '.') {
						if (hasDot) {
							return false;
						}
						hasDot = true;
					}
					else if (c == '-') {
						if (hasDash) {
							return false;
						}
						hasDash = true;
					}
					else {
						return false;
					}
				}
				return true;
				};

			// Node properties
			for (auto& [name, value] : properties)
			{
				bool oldIsNumeric = isNumeric(value);

				char valueBuffer[256];
				snprintf(valueBuffer, sizeof(valueBuffer), "%s", value.c_str());

				float fullWidth = ImGui::GetContentRegionAvail().x;
				ImGui::PushItemWidth(fullWidth * 0.5f - ImGui::GetStyle().ItemSpacing.x * 0.5f);

				if (ImGui::InputText(name.c_str(), valueBuffer, sizeof(valueBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
				{
					std::string newValue = valueBuffer;
					newValue.erase(newValue.find_last_not_of(" \t\n\r\f\v") + 1);
					newValue.erase(0, newValue.find_first_not_of(" \t\n\r\f\v"));

					if (!newValue.empty()) {

						bool newIsNumeric = isNumeric(newValue);

						if (!(oldIsNumeric && !newIsNumeric))
						{
							node->SetProperty(name, newValue);
						}
					}
				}

				ImGui::PopItemWidth();
			}

			// --- Draw children recursively ---
			auto children = node->GetChildren();
			for (auto& child : children)
			{
				DrawBTNodeEditor(child, node);
			}
			ImGui::TreePop();
		}
	}

	void EditorPropertyPanel::DrawCurveLegendRow(const char* label,
		const char* c0Label, ImU32 c0,
		const char* c1Label, ImU32 c1,
		const char* c2Label, ImU32 c2)
	{
		ImGui::TextUnformatted(label);

		auto drawEntry = [](const char* lbl, ImU32 col)
			{
				ImGui::SameLine();
				ImGui::Dummy(ImVec2(10.0f, 0.0f)); // small gap
				ImGui::SameLine();

				ImVec2 p = ImGui::GetCursorScreenPos();
				ImVec2 size(12.0f, 12.0f);
				ImDrawList* dl = ImGui::GetWindowDrawList();
				dl->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), col, 2.0f);
				dl->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), IM_COL32(0, 0, 0, 255), 2.0f);

				ImGui::Dummy(size);
				ImGui::SameLine();
				ImGui::TextUnformatted(lbl);
			};

		drawEntry(c0Label, c0);
		drawEntry(c1Label, c1);
		drawEntry(c2Label, c2);
	}
	bool EditorPropertyPanel::IsComponentOverridden(ComponentTypeID componentType) {
		if (!m_SelectedEntity || !m_SelectedEntity.HasComponent<PrefabComponent>()) {
			return false;
		}

		auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();
		return prefabComp.IsComponentOverridden(componentType);
	}

	void EditorPropertyPanel::MarkComponentOverridden(ComponentTypeID componentType, const std::string& propertyName){
		if (!m_SelectedEntity || !m_SelectedEntity.HasComponent<PrefabComponent>()) {
			return;
		}

		auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();
		prefabComp.MarkComponentModified(componentType, propertyName);
	}

	/*bool EditorPropertyPanel::IsComponentRemoved(ComponentTypeID componentType) {
		if (!m_SelectedEntity || !m_SelectedEntity.HasComponent<PrefabComponent>()) {
			return false;
		}

		auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();
		return prefabComp.IsComponentRemoved(componentType);
	}*/

	/*void EditorPropertyPanel::MarkComponentRemoved(ComponentTypeID componentType) {
		if (!m_SelectedEntity || !m_SelectedEntity.HasComponent<PrefabComponent>()) {
			return;
		}

		auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();
		std::string originalJSON = ComponentSerializer::SerializeComponent(
			m_SelectedEntity, componentType);
		prefabComp.MarkComponentRemoved(componentType, originalJSON);
	}*/

	bool EditorPropertyPanel::IsComponentAddedToInstance(ComponentTypeID type) {
		if (!m_SelectedEntity || !m_SelectedEntity.HasComponent<PrefabComponent>()) {
			return false;
		}
		auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();
		return prefabComp.IsComponentAdded(type);
	}

	bool EditorPropertyPanel::WasComponentInPrefab(ComponentTypeID type) {
		if (!m_SelectedEntity || !m_SelectedEntity.HasComponent<PrefabComponent>()) {
			return false;
		}

		auto& prefabComp = m_SelectedEntity.GetComponent<PrefabComponent>();
		Prefab prefab;
		if (!PrefabRegistry::Get().LoadPrefab(prefabComp.PrefabAssetGuid, prefab)) {
			return false;
		}

		const PrefabEntityData* entityData = prefabComp.isPrefabRoot ?
			prefab.GetRootEntity() :
			prefab.FindEntityByLocalID(prefabComp.prefabLocalID);

		if (!entityData) return false;

		for (const auto& comp : entityData->components) {
			if (comp.type == type) return true;
		}
		return false;
	}

	void EditorPropertyPanel::DisplayAssetField(const char* label, xresource::instance_guid& guid, ResourceType expectedType, bool& errorFlag, ComponentTypeID type)
	{
		// Get the filename from the GUID
		std::string displayName = AM.getNameFromGuid(guid);
		if (displayName.empty()) {
			displayName = "<None>";
		}

		// Create a buffer for the input text (read-only display)
		char buffer[256];
		strncpy(buffer, displayName.c_str(), sizeof(buffer) - 1);
		buffer[sizeof(buffer) - 1] = '\0';

		ImGui::Text("%s", label);
		ImGui::SameLine();

		// Input text field (read-only)
		ImGui::PushID(label);
		ImGui::InputText("##AssetRef", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

		// Drag-drop target
		if (ImGui::BeginDragDropTarget()) {
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ASSET_BROWSER_ITEM")) {
				xresource::instance_guid droppedGuid = *(const xresource::instance_guid*)payload->Data;

				// Verify the asset type matches what's expected
				const AssetRecord* record = AM.getAssetRecord(droppedGuid);
				if (record && record->type == expectedType) {
					guid = droppedGuid;
					MarkComponentOverridden(type);  // MARK AS OVERRIDDEN
				}
				else {
					errorFlag = true;
				}
			}
			ImGui::EndDragDropTarget();
		}

		// Context menu to clear the reference
		if (ImGui::BeginPopupContextItem()) {
			if (ImGui::MenuItem("Clear Reference")) {
				guid = xresource::instance_guid();
				MarkComponentOverridden(type);  // MARK AS OVERRIDDEN
			}
			ImGui::EndPopup();
		}

		ImGui::PopID();
    }
}