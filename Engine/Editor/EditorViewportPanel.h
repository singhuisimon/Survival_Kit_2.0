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
#include "../Graphics/Renderer.h"

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
					//scene->InitializeSystems();
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

			ImGui::SeparatorText("Viewport");

		}

		static void ViewPortClickAndTeleport(uint32_t& pickedID, uint32_t& lastClickedID,
			float& lastClickedTime, const float& doubleClickedTime,
			Entity& selectedEntity, Renderer* renderer) {

			float currentTime = (float)ImGui::GetTime();
			if (pickedID == lastClickedID && (currentTime - lastClickedTime) < doubleClickedTime) {

				//LOG_DEBUG("Double Clicked ID{", pickedID, "}");

				if (selectedEntity.HasComponent<TransformComponent>()) {
					TransformComponent& targetCamPos = selectedEntity.GetComponent<TransformComponent>();
					Camera3D& editorCam = renderer->getEditorCamera();

					glm::vec3 entityPos = targetCamPos.Position;
					float offsetDistance = 5.f;

					if (selectedEntity.HasComponent<MeshRendererComponent>()) {
						glm::vec3 scale = targetCamPos.Scale;
						float maxScale = glm::max(glm::max(scale.x, scale.y), scale.z);
						offsetDistance = maxScale * 1.5f;
					}

					glm::vec3 cameraPos = editorCam.getCamPos();
					glm::vec3 cameraTarget = editorCam.getEditorCamTarget();

					glm::vec3 viewDir = cameraTarget - cameraPos;
					if (glm::dot(viewDir, viewDir) < 1e-8f) {

						viewDir = glm::vec3(0.0f, 0.0f, -1.0f);
					}
					else {
						viewDir = glm::normalize(viewDir);
					}

					glm::vec3 newCamPos = entityPos - viewDir * offsetDistance;

					editorCam.setEditorCamPosition(newCamPos);
					editorCam.setEditorCamTarget(entityPos);

					LOG_DEBUG("Moved to Clicked ID {", newCamPos.x, ", ", newCamPos.y, ", ", newCamPos.z, "}");

				}

				lastClickedTime = 0.0;
				lastClickedID = 0xFFFFFFFFu;
			}
			else {
				lastClickedTime = currentTime;
				lastClickedID = pickedID;
			}
		}

		static void CameraControl(Renderer* renderer) {
			ImGui::SeparatorText("Camera Controls");

			Camera3D& editorCam = renderer->getEditorCamera();
			float& camSpeed = editorCam.getEditorCamSpeed();
			if (ImGui::DragFloat("Editor Camera Movement Speed", &camSpeed, 0.01f, 0.0f, 1e10f)) {
				editorCam.setEditorCamSpeed(camSpeed);
			}
			float& zoomSpeed = editorCam.getEditorZoomSpeed();
			if (ImGui::DragFloat("Editor Camera Zoom Speed", &zoomSpeed, 0.01f, 0.0f, 1e10f)) {
				editorCam.setEditorZoomSpeed(zoomSpeed);
			}
		}
	};
}