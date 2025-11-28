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
#include "../BehaviourTree/BehaviourTreeEditor.h"

namespace Engine
{
	class PropertyPanelHelper
	{
	public:

		//-------------------- FOR BEHAVIOUR TREE -------------------------
		// Functions for BT Component Editor to replace child node when it changes
		static void ReplaceChildNode(std::shared_ptr<BTNode> parent,
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

		static void DrawBTNodeEditor(std::shared_ptr<BTNode> node, std::shared_ptr<BTNode> parent = nullptr)
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

		// ----------------- FOR PHYSICS -----------------------
		static const char* ColliderTypeToString(ColliderType& colliderType) {
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
	};
}