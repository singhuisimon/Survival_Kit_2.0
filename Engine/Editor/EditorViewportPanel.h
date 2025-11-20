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
#include "../Utility/Logger.h"

namespace Engine
{
	class ViewportPanelHelper
	{
	public:
		static void ViewportButtons(bool& playing, Scene* scene, Entity& selectedEntity, 
									std::string& currentScenePath, std::string& currentFileName, 
									uint32_t& pickedID) {
			if (playing) {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255)); // Green
				ImGui::Text("PLAYING");
				ImGui::PopStyleColor();
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255)); // Red
				ImGui::Text("PAUSED");
				ImGui::PopStyleColor();
			}

			// --- Right-aligned buttons ---
			// Calculate total width of all buttons + spacing
			float spacing = ImGui::GetStyle().ItemSpacing.x;
			float playWidth = ImGui::CalcTextSize("Play").x + ImGui::GetStyle().FramePadding.x * 2;
			float pauseWidth = ImGui::CalcTextSize("Pause").x + ImGui::GetStyle().FramePadding.x * 2;
			float stopWidth = ImGui::CalcTextSize("Stop").x + ImGui::GetStyle().FramePadding.x * 2;

			float totalButtonsWidth = playWidth + pauseWidth + stopWidth + spacing * 2;

			// Move cursor to the right
			float availWidth = ImGui::GetContentRegionAvail().x;
			ImGui::SameLine(availWidth - totalButtonsWidth);

			if (playing) {

				ImGui::BeginDisabled();
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 200, 200, 255));

				ImGui::Button("Play");

				ImGui::PopStyleColor();
				ImGui::EndDisabled();
			}
			else {

				ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(144, 238, 144, 255));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(120, 220, 120, 255));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(100, 200, 100, 255));

				if (ImGui::Button("Play")) {
					playing = true;
				}

				ImGui::PopStyleColor(3);
			}

			ImGui::SameLine();

			if (playing) {

				ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 210, 100, 255));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(240, 190, 80, 255));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(220, 170, 60, 255));

				if (ImGui::Button("Pause")) {
					playing = false;
				}

				ImGui::PopStyleColor(3);
			}
			else {

				ImGui::BeginDisabled();
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 200, 200, 255));

				ImGui::Button("Pause");

				ImGui::PopStyleColor();
				ImGui::EndDisabled();
			}

			ImGui::SameLine();

			if (playing && !currentScenePath.empty() && !currentFileName.empty()) {
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 255));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(200, 0, 0, 255));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(255, 0, 0, 255));

				if (ImGui::Button("Stop")) {
					scene->GetRegistry().clear();
					selectedEntity = Entity();
					pickedID = 0xFFFFFFFFu;
					playing = false;
					scene->LoadFromFile(currentScenePath);
				}

				ImGui::PopStyleColor(3);
			}
			else {

				ImGui::BeginDisabled();
				ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 200, 200, 255));

				ImGui::Button("Stop");

				ImGui::PopStyleColor();
				ImGui::EndDisabled();
			}

			ImGui::Separator();

		}
	};
}