/**
* @file Editor.cpp
* @brief Implementation of the functions of IMGUI_Manager class for running the IMGUI level editor.
* @author Liliana Hanawardani (45%), Saw Hui Shan (45%), Rio Shannon Yvon Leonardo (10%)
* @date September 8, 2025
* Copyright (C) 2025 DigiPen Institute of Technology.
* Reproduction or disclosure of this file or its contents without the
* prior written consent of DigiPen Institute of Technology is prohibited.
*/

// Include Header Files
#include "Editor.h"
#include "../Component/TagComponent.h"
#include "../Component/TransformComponent.h"
#include "../Serialization/SceneSerializer.h"

// Include other necessary headers
#include <GLFW/glfw3.h>

// Required for quaternion to Euler conversion
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

namespace Engine
{
	void Editor::SetScene(Engine::Scene* scene)
	{
		m_Scene = scene;
	}

	void Editor::OnInit()
	{
		if (m_Initialized)
		{
			LOG_INFO("Editor: Editor Already initialized.");
			return;
		}
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		io = &ImGui::GetIO(); (void)io;

		// Set config flag
		io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		m_Initialized = true;
	}

	void Editor::OnUpdate(Timestep ts)
	{
		//Start the ImGui frame
		StartImguiFrame();

		RenderEditor();

		//Complete Imgui rendering for the frame
		CompleteFrame();

		static float elapsedTime = 0.0f;
		elapsedTime += ts;
	}

	void Editor::displayTopMenu()
	{
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				{
					// Clear current scene
					m_Scene->GetRegistry().clear();
				}

				if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
				{
					openScenePanel = true;
				}

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
				{
					if (!currScenePath.empty())
					{
						// Save to current path
						SceneSerializer serializer(m_Scene);
						if (serializer.Serialize(currScenePath))
						{
							LOG_INFO("Scene saved successfully to: ", currScenePath);
						}
						else
						{
							LOG_ERROR("Failed to save scene to: ", currScenePath);
						}
					}
					else
					{
						// No current path, open save dialog
						saveAsPanel = true;
					}
				}

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
				{
					saveAsPanel = true;
				}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit", "Alt+F4"))
				{
					// Signal the application to close
					glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Undo", "Ctrl+Z", false, false)) {}  // Disabled for now
				if (ImGui::MenuItem("Redo", "Ctrl+Y", false, false)) {}  // Disabled for now
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "Ctrl+X", false, false)) {}
				if (ImGui::MenuItem("Copy", "Ctrl+C", false, false)) {}
				if (ImGui::MenuItem("Paste", "Ctrl+V", false, false)) {}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Hierarchy", NULL, &hierachyWindow);
				ImGui::MenuItem("Properties", NULL, &inspectorWindow);
				ImGui::MenuItem("Performance Profile", NULL, &performanceProfileWindow);
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
	}

	void Editor::displayPropertiesPanel()
	{
		if (!inspectorWindow)
			return;

		if (ImGui::Begin("Properties", &inspectorWindow))
		{
			if (m_SelectedEntity)
			{
				// Display entity name (TagComponent)
				if (m_SelectedEntity.HasComponent<TagComponent>())
				{
					auto& tag = m_SelectedEntity.GetComponent<TagComponent>();
					char buffer[256];
					strncpy_s(buffer, sizeof(buffer), tag.Tag.c_str(), _TRUNCATE);
					if (ImGui::InputText("Name", buffer, sizeof(buffer)))
					{
						tag.Tag = std::string(buffer);
					}
				}

				ImGui::Separator();

				// Display TransformComponent
				if (m_SelectedEntity.HasComponent<TransformComponent>())
				{
					if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
					{
						auto& transform = m_SelectedEntity.GetComponent<TransformComponent>();

						// Position
						glm::vec3 position = transform.Position;
						if (ImGui::DragFloat3("Position", &position.x, 0.1f))
						{
							transform.SetPosition(position);
						}

						// Rotation (in degrees)
						glm::vec3 rotation = glm::degrees(glm::eulerAngles(transform.Rotation));
						if (ImGui::DragFloat3("Rotation", &rotation.x, 1.0f))
						{
							// Convert back to quaternion
							transform.SetRotation(rotation);
						}

						// Scale
						glm::vec3 scale = transform.Scale;
						if (ImGui::DragFloat3("Scale", &scale.x, 0.1f, 0.001f))
						{
							transform.SetScale(scale);
						}
					}
				}

				// Display other components...
			}
			else
			{
				ImGui::Text("No entity selected");
			}
		}
		ImGui::End();
	}

	void Editor::displayHierarchyPanel()
	{
		if (!hierachyWindow)
			return;

		if (ImGui::Begin("Hierarchy", &hierachyWindow))
		{
			// Button to create new entity
			if (ImGui::Button("Create Entity"))
			{
				auto entity = m_Scene->CreateEntity("New Entity");
				entity.AddComponent<TagComponent>("New Entity");
				entity.AddComponent<TransformComponent>();
			}

			ImGui::Separator();

			// List all entities
			if (m_Scene)
			{
				auto view = m_Scene->GetRegistry().view<TagComponent>();

				for (auto entityHandle : view)
				{
					Entity entity(entityHandle, &m_Scene->GetRegistry());
					auto& tag = entity.GetComponent<TagComponent>();

					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
					if (m_SelectedEntity == entity)
					{
						flags |= ImGuiTreeNodeFlags_Selected;
					}

					ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.Tag.c_str());

					if (ImGui::IsItemClicked())
					{
						m_SelectedEntity = entity;
					}

					// Right-click context menu
					if (ImGui::BeginPopupContextItem())
					{
						if (ImGui::MenuItem("Delete Entity"))
						{
							m_Scene->DestroyEntity(entity);
							if (m_SelectedEntity == entity)
							{
								m_SelectedEntity = Entity();
							}
						}
						ImGui::EndPopup();
					}
				}
			}
		}
		ImGui::End();
	}

	void Editor::displayPerformanceProfilePanel()
	{
		if (!performanceProfileWindow)
			return;

		if (ImGui::Begin("Performance Profile", &performanceProfileWindow))
		{
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
			ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
		}
		ImGui::End();
	}

	void Editor::StartImguiFrame()
	{
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void Editor::renderViewport()
	{
		// Placeholder for viewport rendering
		// This will be implemented when the rendering system is complete
	}

	void Editor::RenderEditor()
	{
		// Create the dockspace
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);
		ImGui::SetNextWindowViewport(viewport->ID);

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse;
		window_flags |= ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace", nullptr, window_flags);
		ImGui::PopStyleVar();

		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

		// Display menu bar
		displayTopMenu();

		ImGui::End();

		// Display panels
		displayHierarchyPanel();
		displayPropertiesPanel();
		displayPerformanceProfilePanel();
		renderViewport();

		// Display dialogs
		sceneOpenPanel();
		saveAsScenePanel();
	}

	void Editor::sceneOpenPanel()
	{
		if (!openScenePanel)
			return;

		ImGui::OpenPopup("Open Scene");

		if (ImGui::BeginPopupModal("Open Scene", &openScenePanel))
		{
			static char scenePath[256] = "Resources/Scenes/";

			ImGui::Text("Enter scene file path:");
			ImGui::InputText("##scenepath", scenePath, sizeof(scenePath));

			if (ImGui::Button("Open"))
			{
				SceneSerializer serializer(m_Scene);
				if (serializer.Deserialize(scenePath))
				{
					currScenePath = scenePath;
					LOG_INFO("Scene loaded successfully from: " + std::string(scenePath));
				}
				else
				{
					LOG_ERROR("Failed to load scene from: " + std::string(scenePath));
				}
				openScenePanel = false;
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				openScenePanel = false;
			}

			ImGui::EndPopup();
		}
	}

	void Editor::saveAsScenePanel()
	{
		if (!saveAsPanel)
			return;

		ImGui::OpenPopup("Save Scene As");

		if (ImGui::BeginPopupModal("Save Scene As", &saveAsPanel))
		{
			static char scenePath[256] = "Resources/Scenes/";

			ImGui::Text("Enter scene file name:");
			ImGui::InputText("##scenepath", scenePath, sizeof(scenePath));

			// Add .json extension if not present
			std::string pathStr(scenePath);
			if (pathStr.find(".json") == std::string::npos)
			{
				pathStr += ".json";
			}

			if (ImGui::Button("Save"))
			{
				SceneSerializer serializer(m_Scene);
				if (serializer.Serialize(pathStr))
				{
					currScenePath = pathStr;
					LOG_INFO("Scene saved successfully to: " + pathStr);
				}
				else
				{
					LOG_ERROR("Failed to save scene to: " + pathStr);
				}
				saveAsPanel = false;
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				saveAsPanel = false;
			}

			ImGui::EndPopup();
		}
	}

	void Editor::CompleteFrame()
	{
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			GLFWwindow* backup_current_context = glfwGetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			glfwMakeContextCurrent(backup_current_context);
		}
	}

	std::vector<std::pair<std::string, std::string>> Editor::getFilesInFolder(const std::string& folderName)
	{
		std::vector<std::pair<std::string, std::string>> files;
		// Implementation placeholder
		return files;
	}

	void Editor::displayAssetsBrowserPanel()
	{
		// Implementation placeholder
	}
}