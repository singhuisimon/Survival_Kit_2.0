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

	void Editor::OnInit(GLuint texhandle)
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

		// Setup scaling
		ImGuiStyle& style = ImGui::GetStyle();

		// Set WindowRounding and ImGuiCol_WindowBg when viewport is enabled
		if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
		ImGui_ImplOpenGL3_Init("#version 410");

		// Save created FBO texture handle
		m_FBOTextureHandle = texhandle;

		m_Initialized = true;
	}

	void Editor::OnUpdate(Timestep ts)
	{
		if (!m_Initialized) return;

		//Start the ImGui frame
		StartImguiFrame();

		// Enable Docking Function
		ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

		displayTopMenu();

		renderViewport();

		// Panel Logic
		displayPropertiesPanel();

		displayHierarchyPanel();

		displayAssetsBrowserPanel();

		displayPerformanceProfilePanel(ts);

		//Complete Imgui rendering for the frame
		CompleteFrame();
	}

	void Editor::displayTopMenu()
	{
		if (ImGui::BeginMainMenuBar())
		{

			if (ImGui::BeginMenu("File"))
			{
				// --------------- New Scene -------------------
				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				{
					if (m_Scene)
					{
						m_Scene->GetRegistry().clear();
						currScenePath = "";
						isNewScene = true;
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Create new scene.");

				// --------------- Open Scene -------------------
				if (ImGui::MenuItem("Open Scene...", "Ctrl+O"))
				{
					openScenePanel = true;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Open scene from file.");

				// --------------- Save Scene -------------------
				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
				{
					if (!currScenePath.empty())
					{
						SceneSerializer serializer(m_Scene);
						if (serializer.Serialize(currScenePath))
							LOG_INFO("Scene saved successfully to: ", currScenePath);
						else
							LOG_ERROR("Failed to save scene to: ", currScenePath);
					}
					else
					{
						saveAsPanel = true;
					}
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Save current scene.");

				// --------------- Save Scene As -------------------
				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
				{
					saveAsPanel = true;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Save scene as a new file.");

				ImGui::Separator();

				// ====================== Script Section ==========================
				if (ImGui::MenuItem("Open Script"))
				{
					// open script logic
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Open script from file.");

				if (ImGui::MenuItem("New Script"))
				{
					createScript = true;
				}
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("Create a new script.");

				ImGui::Separator();

				// --------------- Exit -------------------
				if (ImGui::MenuItem("Exit", "Alt+F4"))
				{
					glfwSetWindowShouldClose(m_Window, GLFW_TRUE);
				}

				ImGui::EndMenu();
			}

			// ---------------- Display Current Scene Name ---------------------
			if (!currScenePath.empty())
			{
				std::filesystem::path filePath(currScenePath);
				std::string fileName = filePath.filename().string();

				ImGui::SameLine(ImGui::GetContentRegionAvail().x - 80.0f);
				ImGui::TextUnformatted(fileName.c_str());
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

			ImGui::EndMainMenuBar();
		}

		//  =========================== Open Scene pop up panel =====================================
		if (openScenePanel)
		{
			sceneOpenPanel();
		}

		// ========================== Save as Scene panel ============================
		if (saveAsPanel)
		{
			saveAsScenePanel();
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

		ImGui::End(); // End of the properties window
	}

	void Editor::displayAssetsBrowserPanel()
	{
		ImGui::SetNextWindowSize(ImVec2(600, 400));

		// Begin properties dockable window
		if (ImGui::Begin("Assets Browser", &assetsWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			
			ImGui::Columns(2, nullptr, true);
			static std::string selectedFolder = "";
			// ================= Left column panel display all the resources folder ========================
			ImGui::BeginChild("Project List", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Text("Projects:");
			if (ImGui::CollapsingHeader("Resources", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto folders = getAssetsInFolder(getAssetFilePath("Sources/"));

				for (auto& folder : folders)
				{
					bool isSelected = (selectedFolder == folder.fullPath);
					if (ImGui::Selectable(folder.name.c_str(), isSelected))
					{
						selectedFolder = folder.fullPath;
						selectedResourcesIndex = -1; // reset asset selection
					}
				}
			}
			ImGui::EndChild(); // end the left column 

			// ===================== Right column panel display the resources files =======================

			ImGui::NextColumn();
			ImGui::BeginChild("Assets Panel", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
			// if folder is selected, display the files
			if (!selectedFolder.empty())
			{
				// to get the files in the selected folder
				auto assetsList = getAssetsInFolder(selectedFolder); 
				// display the selected folder name
				std::filesystem::path folderPath(selectedFolder);
				std::string folderName = folderPath.filename().string();
				ImGui::Text(("Assets > " + folderName).c_str());

				ImGui::Separator();

				const float padding = 10.0f;
				const float thumbnailSize = 64.0f;
				const float cellSize = thumbnailSize + padding;
				float panelWidth = ImGui::GetContentRegionAvail().x;
				int itemsPerRow = std::max(1, (int)(panelWidth / cellSize));

				int textureCount = -1;
				ImGui::Columns(itemsPerRow, nullptr, false);

				// loop through files in selected folder
				for (size_t i = 0; i < assetsList.size(); i++)
				{
					const auto& asset = assetsList[i];
					std::string fileName = asset.name;
					std::string filePath = asset.fullPath;

					/*if (asset.name.ends_with(".png") || asset.name.ends_with(".jpeg"))
					{
						++textureCount;
					}*/

					ImGui::PushID(fileName.c_str());

					if (ImGui::Button(fileName.c_str(), ImVec2(thumbnailSize, thumbnailSize)))
					{
						selectedResourcesIndex = static_cast<int>(i);
						//currScenePath = filePath;

						std::string extension = asset.name.substr(asset.name.find_last_of('.'));
						if (extension == ".json") // if it is scene
						{
							if (m_Scene)
							{
								m_Scene->GetRegistry().clear();
								m_Scene->LoadFromFile(filePath);
								currScenePath = filePath; // update curr file path
							}
						}
					}

					// ==================== Display info detail ==========================
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text("Name: %s", fileName.c_str());
						//ImGui::Text("Type: %s", filePath.c_str();
						std::string extension = fileName.substr(fileName.find_last_of('.') + 1);
						ImGui::Text("Type: %s", extension.c_str());
						ImGui::EndTooltip();
					}

					// ==================== To center text under thumbnail ================
					ImVec2 textSize = ImGui::CalcTextSize(fileName.c_str());
					float textX = (thumbnailSize - textSize.x) * 0.5f;
					if (textX < 0) textX = 0;
					ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textX);
					ImGui::TextWrapped("%s", fileName.c_str());

					ImGui::PopID();
					ImGui::NextColumn();
					
				}
			}

			ImGui::EndChild(); // end of the right column
			ImGui::Columns(1);
		}
		ImGui::End(); // End of the assets browser window
	}

	void Editor::displayPerformanceProfilePanel(Timestep ts)
	{
		if (!performanceProfileWindow)
			return;
		
		ImGui::SetNextWindowSize(ImVec2(500, 300));
		if (ImGui::Begin("Performance Profile", &performanceProfileWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			ImGui::Text("Tracy Window:");
			if (ImGui::Button("Launch Tracy Window"))
			{
				// TO DO: Tracy Running 
			}

			ImGui::Separator();
			// ========================= ImGui Graph Section =============================
			float deltaTime = ts.GetSeconds();
			float currFPS = (deltaTime > 0.0f) ? 1.0f / deltaTime : 0.0f;
			float currFrameTime = ts.GetMilliseconds();

			// ======================= history statistics variables ============================
			static const int FPS_HISTORY_SIZE = 90;  //store up to 90 frames
			static float fpsHistory[FPS_HISTORY_SIZE] = {};
			static float frameTimeHistory[FPS_HISTORY_SIZE] = {}; //for frame time
			static int fpsHistoryOffset = 0;
			static int frameCount = 0;

			fpsHistory[fpsHistoryOffset] = currFPS;
			frameTimeHistory[fpsHistoryOffset] = currFrameTime;
			fpsHistoryOffset = (fpsHistoryOffset + 1) % FPS_HISTORY_SIZE;
			frameCount = std::min(frameCount + 1, FPS_HISTORY_SIZE);

			//========================== update min/max statistics =============================
			static float minFPS = FLT_MAX;
			static float maxFPS = 0.0f;
			static float minFrameTime = FLT_MAX;
			static float maxFrameTime = 0.0f;

			minFPS = std::min(minFPS, currFPS);
			maxFPS = std::max(maxFPS, currFPS);
			minFrameTime = std::min(minFrameTime, currFrameTime);
			maxFrameTime = std::max(maxFrameTime, currFrameTime);

			// ======================== Cal average ===================================
			float avgFPS = 0.0f;
			float avgFrameTime = 0.0f;

			for (int i = 0; i < FPS_HISTORY_SIZE; i++)
			{
				avgFPS += fpsHistory[i];
				avgFrameTime += frameTimeHistory[i];
			}

			avgFPS /= frameCount;
			avgFrameTime /= frameCount;

			// ======================= Showcase statistics ==============================
			ImGui::Text("Frame Statistics:");
			ImGui::Spacing();
			//create a table to display statistics better
			if (ImGui::BeginTable("StatsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
			{
				ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthFixed, 120.0f);
				ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
				ImGui::TableSetupColumn("Unit", ImGuiTableColumnFlags_WidthFixed, 40.0f);
				ImGui::TableHeadersRow();

				// Average FPS
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Average FPS:");
				ImGui::TableNextColumn();
				ImGui::Text("%.1f", avgFPS);
				ImGui::TableNextColumn();
				ImGui::Text("fps");

				// Average Frame Time
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Avg Frame Time:");
				ImGui::TableNextColumn();
				ImGui::Text("%.2f", avgFrameTime);
				ImGui::TableNextColumn();
				ImGui::Text("ms");

				// Min Frame Time
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Min Frame Time:");
				ImGui::TableNextColumn();
				ImGui::Text("%.2f", minFrameTime);
				ImGui::TableNextColumn();
				ImGui::Text("ms");

				// Max Frame Time
				ImGui::TableNextRow();
				ImGui::TableNextColumn();
				ImGui::Text("Max Frame Time:");
				ImGui::TableNextColumn();
				ImGui::Text("%.2f", maxFrameTime);
				ImGui::TableNextColumn();
				ImGui::Text("ms");

				ImGui::EndTable();
			}
			ImGui::Spacing();
			ImGui::Separator();
			ImGui::Spacing();

			// ============================= showcase performance graphs section ======================
			ImGui::Text("Performance Graphs:");
			ImGui::Spacing();
			float graphWidth = ImGui::GetContentRegionAvail().x;

			//----------------- FPS graph ---------------------
			char fpsOverlay[64];
			sprintf_s(fpsOverlay, sizeof(fpsOverlay), "FPS - avg %.1f", avgFPS);

			float fpsMinScale = (avgFPS - 30.0f > 0.0f) ? (avgFPS - 30.0f) : 0.0f;
			float fpsMaxScale = avgFPS + 30.0f;

			ImGui::PlotLines(
				"##FPS",
				fpsHistory,
				FPS_HISTORY_SIZE,
				fpsHistoryOffset,
				fpsOverlay,
				fpsMinScale,
				fpsMaxScale,
				ImVec2(graphWidth, 100.0f),
				sizeof(float)
			);
			// ----------- frame time graph -------------
			char frameTimeOverlay[64];
			sprintf_s(frameTimeOverlay, sizeof(frameTimeOverlay), "Frame Time (ms) - avg %.2f", avgFrameTime);

			// ---------- dynamic scaling ------------  
			float ftMinScale = std::max(avgFrameTime - 5.0f, 0.0f);
			float ftMaxScale = avgFrameTime + 5.0f;

			ImGui::PlotLines(
				"##FrameTime",
				frameTimeHistory,
				FPS_HISTORY_SIZE,
				fpsHistoryOffset,
				frameTimeOverlay,
				ftMinScale,
				ftMaxScale,
				ImVec2(graphWidth, 100.0f),
				sizeof(float)
			);

			ImGui::Spacing();
			ImGui::Separator();

			// ========= different coloring to indicate performance status ======= 
			ImGui::Spacing();
			if (currFPS >= 60.0f)
			{
				ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Performance: Excellent");
			}
			else if (currFPS >= 30.0f)
			{
				ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Performance: Good");
			}
			else
			{
				ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Performance: Poor");
			}

			ImGui::Spacing();

			if (ImGui::Begin("Performance Profile", &performanceProfileWindow))
			{
				ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
				ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
			}
			ImGui::End();
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
		// TODO: Get Texture from Graphics
		// auto texture = GFXM.getImguiTex();

		ImVec2 texture_pos = ImGui::GetCursorScreenPos();

		// Hard-coded values just in case (Will change values later)
		ImVec2 viewportSize =
		{
			 600,
			 600
		};

		if (m_Window) {

			int width = 0.f;
			int height = 0.f;
			glfwGetWindowSize(m_Window, &width, &height);

			viewportSize =
			{
				 static_cast<float>(static_cast<float>(width)) / 2.0f,
				 static_cast<float>(static_cast<float>(height)) / 2.0f
			};

		}

		ImGui::Begin("Viewport");

		// Uncomment this once the viewport texture has been obtained
		if (m_FBOTextureHandle) {
			ImVec2 imagePos = ImGui::GetCursorScreenPos();

			ImGui::Image((ImTextureID)(intptr_t)m_FBOTextureHandle,
				viewportSize,
				ImVec2(0, 1), ImVec2(1, 0));

			// TODO: Handle mouse in viewport
			//handleViewPortClick(imagePos, viewportSize);
		}

		ImGui::End();
		
		
		// Logic here 

	}

	// Helper function for top menu 
	void Editor::sceneOpenPanel()
	{
		// get all files inside scene
		auto sceneFiles = getAssetsInFolder(getAssetFilePath("Sources/Scenes"));
		if (openScenePanel)
		{
			ImGui::OpenPopup("Scene Level Selection");
		}

		// pop up panel to open scene file
		if (ImGui::BeginPopupModal("Scene Level Selection", nullptr, ImGuiWindowFlags_NoDocking))
		{
			ImGui::SetWindowSize(ImVec2(500, 200), ImGuiCond_Once);

			// list all scene files
			for (auto& scenesAsset : sceneFiles)
			{

				if (ImGui::Selectable(scenesAsset.name.c_str()))
				{
					if (!m_Scene)
					{
						LOG_ERROR("No active scene exists to load into!");
						continue;
					}
					//LOG_DEBUG("This is in", fullPath);
					// clear current scene
					auto& registry = m_Scene->GetRegistry();
					registry.clear();

					// load the selected scene file
					if (m_Scene->LoadFromFile(scenesAsset.fullPath))
					{
						//LOG_ERROR("Failed to load scene %s", sceneFiles);
						currScenePath = scenesAsset.fullPath;
						LOG_INFO("Scene loaded successfully: ", currScenePath);
						openScenePanel = false; //  reset after select scene
						ImGui::CloseCurrentPopup();
					}
				}
			}
			// --------------- Cancel Selection for Open Scene -----------------------
			if (ImGui::Button("Cancel"))
			{
				openScenePanel = false; //  reset after click cancel button
				ImGui::CloseCurrentPopup();
			}


			ImGui::EndPopup(); // end pop up panel for scene level selection
		}
		
		/*if (!openScenePanel)
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
		}*/
	}

	void Editor::saveAsScenePanel()
	{
		if (saveAsPanel)
		{
			ImGui::OpenPopup("Save As Panel");
		}
		if (ImGui::BeginPopupModal("Save As Panel", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{

			ImGui::InputText("File name", saveAsDefaultSceneName, IM_ARRAYSIZE(saveAsDefaultSceneName));

			// ------------------ Select save button to save new scene --------------------
			if (ImGui::Button("Save", ImVec2(120, 0)))
			{
				if (strlen(saveAsDefaultSceneName) == 0) // validate that file name is not empty
				{
					ImGui::OpenPopup("Empty Filename");
				}
				else
				{
					// default new scene path 
					std::string defaultNewScenePath = getAssetFilePath("Sources/Scenes/") + saveAsDefaultSceneName;

					if (!std::filesystem::path(defaultNewScenePath).has_extension()) {

						defaultNewScenePath += ".json"; // ensure .json extension
					}

					if (std::filesystem::exists(defaultNewScenePath))
					{
						ImGui::OpenPopup("Confirm Overwrite"); // if save as name repeat, open confirmation panel for overwrite it
					}
					else
					{

						m_Scene->SaveToFile(defaultNewScenePath); // save scene file
						//LOG_DEBUG("Scene save as: ", defaultNewScenePath);
						currScenePath = defaultNewScenePath; // update current scene path
						saveAsPanel = false; // to close pop up
						isNewScene = false;
						ImGui::CloseCurrentPopup();

					}
				}
			}

			// ------------------- Cancel save as button ---------------------
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				saveAsPanel = false; // reset to close pop up
				ImGui::CloseCurrentPopup();
			}

			// ----------------------- Overwrite Existing Save as Scene File -------------------
			if (ImGui::BeginPopupModal("Confirm Overwrite", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("File %s already exists.\nDo you want to replace it?", saveAsDefaultSceneName);

				ImGui::Separator();

				if (ImGui::Button("Yes", ImVec2(120, 0)))
				{
					// default new scene path 
					std::string defaultNewScenePath = getAssetFilePath("Sources/Scenes/") + saveAsDefaultSceneName;
					if (!std::filesystem::path(defaultNewScenePath).has_extension()) {
						defaultNewScenePath += ".json"; // ensure .json extension
					}
					std::cout << defaultNewScenePath << "json file test\n";
					m_Scene->SaveToFile(defaultNewScenePath);
					currScenePath = defaultNewScenePath;

					saveAsPanel = false;
					isNewScene = false;
					ImGui::CloseCurrentPopup(); // close save as panel
				}

				ImGui::SameLine();
				if (ImGui::Button("No", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup(); // end pop up confirm overwrite panel
			}

			// ---------------- If is Emty Filename Warning -------------------
			if (ImGui::BeginPopupModal("Empty Filename", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Please enter a file name.");

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			ImGui::EndPopup(); // end pop up for save as scene panel
		}

		/*if (!saveAsPanel)
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
		}*/
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

	std::vector<Editor::AssetEntry> Editor::getAssetsInFolder(const std::string& folderPath)
	{
		std::vector<AssetEntry> entries;

		if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath))
			return entries;

		for (const auto& entry : std::filesystem::directory_iterator(folderPath))
		{
			entries.push_back({
				entry.path().filename().string(),
				entry.path().generic_string(),
				});
		}

		return entries;
	}

	std::vector<std::pair<std::string, std::string>> Editor::getFilesInFolder(const std::string& folderName)
	{
		std::vector<std::pair<std::string, std::string>> files;
		// Implementation placeholder
		return files;
	}


} // end of namespace Engine
