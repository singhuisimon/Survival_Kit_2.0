#include "EditorMenu.h"
#include "../Engine/Editor/Editor.h"
#include "../Asset/AssetManager.h"
#include "../Asset/ResourceManager.h"
#include "../Serialization/PrefabSerializer.h"
#include "../Prefab/PrefabRegistry.h"

#include <algorithm>
namespace Engine
{
	void EditorMenu::EditorTopMenu()
	{
		if (!m_Editor) return;

		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				// ---------------- New Scene -------------------
				if (ImGui::MenuItem("New Scene"))
				{
					LOG_INFO("Creating new scene.");
					if (m_Editor)
					{
						m_NewScenePanel = true;
						m_Editor->CreateNewScene("New Scene");
						m_CurrScenePath = "";
						m_Editor->SetScenePath(m_CurrScenePath);
						m_SceneIsNew = true;
						m_Editor->SetPrefabPath("");
						Renderer* renderer = m_Editor->GetRenderer();
						Camera3D& cam = renderer->getEditorCamera();
						cam.setEditorCamPosition(glm::vec3(0.f, 5.f, 5.f));
						cam.setEditorCamTarget(glm::vec3(0.f, 0.f, 0.f));
					}
				}
				
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Create new scene.");
				}
				// ----------------- Open Scene ---------------------
				if (ImGui::MenuItem("Open Scene"))
				{
					m_OpenScenePanel = true;
					m_CurrPrefabPath = "";
					m_Editor->SetPrefabPath(m_CurrPrefabPath);
					m_SceneIsNew = false;
					Renderer* renderer = m_Editor->GetRenderer();
					Camera3D& cam = renderer->getEditorCamera();
					cam.setEditorCamPosition(glm::vec3(0.f, 5.f, 5.f));
					cam.setEditorCamTarget(glm::vec3(0.f, 0.f, 0.f));
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Open scene from file.");
				}

				bool hasScenePath = !m_Editor->GetScenePath().empty();
				bool hasPrefabPath = !m_Editor->GetPrefabPath().empty();
				bool isInSceneMode = !hasPrefabPath && (hasScenePath || m_SceneIsNew);

				// ----------------- Save Scene ------------------------
				if (ImGui::MenuItem("Save Scene", nullptr, false, isInSceneMode))
				{
					m_CurrScenePath = m_Editor->GetScenePath();
					//Scene*m_Scene = m_Editor->GetActiveScene();
					//Renderer* m_Renderer = m_Editor->GetRenderer();

					LOG_DEBUG("====== Save Current Scene: ", m_CurrScenePath, " =======");
					if (!m_CurrScenePath.empty())
					{
						m_Editor->SaveActiveSceneToPath(m_CurrScenePath);
						m_SceneIsNew = false;
					}
					else
					{
						m_SaveScenePanel = true;
					}
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Save current scene.");
				}
				// --------------- Save Scene As -------------------
				if (ImGui::MenuItem("Save Scene As...", nullptr, false, isInSceneMode))
				{
					m_SaveScenePanel = true;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Save scene as a new file.");
				}
				// ----- Open Prefab temporary -----
				if (ImGui::MenuItem("Open Prefab"))
				{
					m_OpenPrefabPanel = true;
					m_CurrScenePath = "";
					m_Editor->SetScenePath(m_CurrScenePath);
					m_SceneIsNew = false;
					Renderer* renderer = m_Editor->GetRenderer();
					Camera3D& cam = renderer->getEditorCamera();
					cam.setEditorCamPosition(glm::vec3(0.f, 5.f, 5.f));
					cam.setEditorCamTarget(glm::vec3(0.f, 0.f, 0.f));
				}
				
				if (ImGui::MenuItem("Save Prefab", nullptr, false, hasPrefabPath))
				{
					if (m_Editor->HasPrefabPath() && !m_Editor->GetPrefabPath().empty()) {
						SaveCurrentPrefab();

					}
					else {
						// If no current prefab path, open "Save As" dialog
						m_SavePrefabPanel = true;
					}
				}
				bool isInPrefabMode = hasPrefabPath;
				if (ImGui::MenuItem("Save Prefab As...",  nullptr, false, isInPrefabMode))
				{
					m_SavePrefabPanel = true;
				}
				
				ImGui::EndMenu();
			}
			
			if (ImGui::BeginMenu("Compile"))
			{
				if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
				{
					AM.CompileAllAsset(0);
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				ImGui::MenuItem("Hierarchy", nullptr, &m_Editor->GetHierarchyWindowRef());
				ImGui::MenuItem("Property", nullptr, &m_Editor->GetPropertyWindowRef());
				ImGui::MenuItem("Performance", nullptr, &m_Editor->GetPerformanceProfileWindowRef());
				ImGui::MenuItem("Logger", nullptr, &m_Editor->GetLoggerWindowRef());
				ImGui::MenuItem("AudioFileTracker", nullptr, &m_Editor->GetAudioTrackerWindowRef());

				ImGui::Separator();

				ImGui::MenuItem("Render Settings", nullptr, &m_ShowHDRSettings);

				ImGui::EndMenu();
			}


			// ---------------- Display Current Scene Name ---------------------
			
			std::string displayPath = m_Editor->HasScenePath() ? m_Editor->GetScenePath() :
				m_Editor->HasPrefabPath() ? m_Editor->GetPrefabPath() : "";
			if (!displayPath.empty())
			{
			/*	std::string fileName = std::filesystem::path(displayPath).filename().string();
				float textWidth = ImGui::CalcTextSize(fileName.c_str()).x;
				float menuBarWidth = ImGui::GetWindowSize().x;
				ImGui::SameLine(menuBarWidth - textWidth - 10.0f);
				ImGui::TextUnformatted(fileName.c_str());*/
				float textWidth = ImGui::CalcTextSize(displayPath.c_str()).x;
				float menuBarWidth = ImGui::GetWindowSize().x;
				ImGui::SameLine(menuBarWidth - textWidth - 10.0f);
				ImGui::TextUnformatted(displayPath.c_str());
			}
			ImGui::EndMainMenuBar();
		}
		OpenScenePanel();
		SaveScenePanel();
		OpenPrefabPanel();
		SavePrefabPanel();
		DisplayHDRSettings();

	}

	void EditorMenu::OpenScenePanel()
	{
		//std::cout << " ============================== Start of OpenScenePanel =========================\n\n";
		if (!m_Editor) return;
		// get all files inside scene
		auto sceneFiles = m_Editor->getAssetsInFolder(getAssetFilePath("Sources/Scenes"));
	
		if (m_OpenScenePanel)
		{
			ImGui::OpenPopup("Scene Level Selection");
			m_OpenScenePanel = false;
		}
		if (ImGui::BeginPopupModal("Scene Level Selection", nullptr, ImGuiWindowFlags_NoDocking))
		{
			ImGui::SetWindowSize(ImVec2(500, 200), ImGuiCond_Once);
			for (auto& scenesAsset : sceneFiles)
			{
				
				if (ImGui::Selectable(scenesAsset.name.c_str()))
				{
					m_Editor->SetScenePath(scenesAsset.fullPath);
					m_CurrScenePath = m_Editor->GetScenePath();
					Scene* newScene = m_Editor->CreateNewScene(scenesAsset.name);
					if (!newScene)
					{
						LOG_ERROR("Failed to create new scene");
						continue;
					}
					if (newScene->LoadFromFile(scenesAsset.fullPath))
					{
						LOG_DEBUG("====== LoadFromFile=====");
						LOG_DEBUG("FilePath[m_CurrScenePath]: ", m_CurrScenePath);
						m_Editor->SetCurrSelectedEntity(Entity{});
						m_Editor->RetrievePickedID(0xFFFFFFFFu);
						m_Editor->SetActiveScene(newScene);
						m_Editor->SetSceneName(newScene->GetName());

						if (Renderer* renderer = m_Editor->GetRenderer())
						{
							const auto& settings = newScene->GetSceneSetting();
							renderer->getBloomToggle() = settings.s_BloomToggle;
							renderer->getBloomStrength() = settings.s_BloomStrength;
							renderer->getBloomFilterRadius() = settings.s_BloomFilterRadius;
							renderer->getExposure() = settings.s_Exposure;
							renderer->getGamma() = settings.s_Gamma;
						}

						if(AudioManager* audioManager = m_Editor->GetAudioManager())
						{
							const auto& settings = newScene->GetSceneSetting();
							audioManager->SetEditorCap(AudioType::SFX, settings.s_SFXVolume);
							audioManager->SetEditorCap(AudioType::BGM, settings.s_BGMVolume);
							audioManager->SetEditorCap(AudioType::UI, settings.s_UIVolume);
							audioManager->SetEditorCap(AudioType::VO, settings.s_VOVolume);
							audioManager->SetEditorCap(AudioType::GAMESFX, settings.s_GameSFXVolume);
						}

						// to register entity that contain prefab
						auto prefabFiles = m_Editor->getAssetsInFolder(getAssetFilePath("Sources/Prefabs"));
						for (auto& prefabAsset : prefabFiles)
						{
							// Filter for .prefab files only
							if (prefabAsset.name.find(".prefab") == std::string::npos) {
								continue;
							}

							Prefab prefab;
							if (PrefabSerializer::DeserializePrefab(prefabAsset.fullPath, prefab)) {
								if (prefab.guid.m_Value != 0 && !PrefabRegistry::Get().IsPrefabRegistered(prefab.guid)) {
									PrefabRegistry::Get().RegisterPrefab(prefab.guid, prefabAsset.fullPath, prefab.name);
									LOG_INFO("Registered prefab: ", prefab.name.c_str(),
										" (GUID: ", prefab.guid.m_Value, ")");
								}
							}
						}
					
						LOG_INFO("Scene loaded successfully: ", m_CurrScenePath);
						ImGui::CloseCurrentPopup();
					}
					
				}
			}
		
			// --------------- Cancel Selection for Open Scene -----------------------
			if (ImGui::Button("Cancel"))
			{
				m_OpenScenePanel = false; //  reset after click cancel button
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
		
	}

	void EditorMenu::SaveScenePanel()
	{
		if (!m_Editor) return;

		if (m_SaveScenePanel)
		{
			ImGui::OpenPopup("Save As Panel");
			memset(saveAsDefaultSceneName, 0, sizeof(saveAsDefaultSceneName));
			m_SaveScenePanel = false;
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
					std::string defaultNewScenePath = getAssetFilePath("Sources/Scenes/") + saveAsDefaultSceneName;
					if (!std::filesystem::path(defaultNewScenePath).has_extension())
					{
						defaultNewScenePath += ".json"; // ensure .json extension
					}

					if (std::filesystem::exists(defaultNewScenePath))
					{
						ImGui::OpenPopup("Confirm Overwrite"); // if save as name repeat, open confirmation panel for overwrite it
					}
					else
					{
						//Scene* m_Scene = m_Editor->GetActiveScene();
						//Renderer* m_Renderer = m_Editor->GetRenderer();

						m_Editor->SaveActiveSceneToPath(defaultNewScenePath);
						LOG_DEBUG("Scene saved as: ", defaultNewScenePath);
						m_SaveScenePanel = false;
						ImGui::CloseCurrentPopup();

					}
				}
			}
			// ------------------- Cancel save as button ---------------------
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
				m_SaveScenePanel = false;
			}
			// ----------------------- Overwrite Existing Save as Scene File -------------------
			if (ImGui::BeginPopupModal("Confirm Overwrite", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("File %s already exists.\nDo you want to replace it?", saveAsDefaultSceneName);
				ImGui::Separator();

				if (ImGui::Button("Yes", ImVec2(120, 0)))
				{
					std::string defaultNewScenePath = getAssetFilePath("Sources/Scenes/") + saveAsDefaultSceneName;
					m_Editor->SaveActiveSceneToPath(defaultNewScenePath);
				
					m_SaveScenePanel = false;
					m_CloseSaveAsPanel = true;
					ImGui::CloseCurrentPopup();

				}
				ImGui::SameLine();
				if (ImGui::Button("No", ImVec2(120, 0)))
				{
					m_SaveScenePanel = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			// ---------------- If is Emty Filename Warning -------------------
			if (ImGui::BeginPopupModal("Empty Filename", NULL, ImGuiWindowFlags_AlwaysAutoResize))
			{
				ImGui::Text("Please enter a file name.");

				if (ImGui::Button("OK", ImVec2(120, 0)))
				{
					m_SaveScenePanel = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			if (m_CloseSaveAsPanel)
			{
				ImGui::CloseCurrentPopup(); 
				m_SaveScenePanel = false;
				m_CloseSaveAsPanel = false;
			}
			ImGui::EndPopup();
		}

	}
	void EditorMenu::OpenPrefabPanel()
	{
		if (!m_Editor) return;
		// get all files inside scene
		auto prefabFiles = m_Editor->getAssetsInFolder(getAssetFilePath("Sources/Prefabs"));

		if (m_OpenPrefabPanel)
		{
			ImGui::OpenPopup("Prefabs Selection");
			m_OpenPrefabPanel = false;
		}
		if (ImGui::BeginPopupModal("Prefabs Selection", nullptr, ImGuiWindowFlags_NoDocking))
		{
			ImGui::SetWindowSize(ImVec2(500, 200), ImGuiCond_Once);
			for (auto& prefabsAsset : prefabFiles)
			{

				if (ImGui::Selectable(prefabsAsset.name.c_str()))
				{
					m_Editor->SetPrefabPath(prefabsAsset.fullPath);
					m_CurrPrefabPath = m_Editor->GetPrefabPath();
					std::string prefabName = "Prefab: " + prefabsAsset.name;
					Scene* newScene = m_Editor->CreateNewScene(prefabName);
					if (!newScene)
					{
						LOG_ERROR("Failed to create new scene");
						continue;
					}
					Prefab loadedPrefab;
					if (PrefabRegistry::Get().LoadPrefabFromFile(m_CurrPrefabPath, loadedPrefab))
					{
						LOG_INFO("Successfully loaded prefab: ", loadedPrefab.name);
						LOG_INFO("Prefab has ", loadedPrefab.entities.size(), " entities");

						// Instantiate the prefab into the scene
						Entity prefabRoot = PrefabInstantiator::InstantiatePrefab(
							newScene,
							loadedPrefab,
							Entity{}  // No parent
						);

						if (prefabRoot) {
							m_Editor->SetCurrSelectedEntity(prefabRoot);
							m_Editor->RetrievePickedID(static_cast<uint32_t>(prefabRoot.GetHandle()));
							LOG_INFO("Prefab instantiated successfully");
						}
						else {
							LOG_ERROR("Failed to instantiate prefab");
						}
					}
					else {
						LOG_ERROR("Failed to load prefab from file: ", m_CurrPrefabPath);
					}
					ImGui::CloseCurrentPopup();
				}
			}

			// --------------- Cancel Selection for Open Scene -----------------------
			if (ImGui::Button("Cancel"))
			{
				m_OpenScenePanel = false; //  reset after click cancel button
				ImGui::CloseCurrentPopup();
			}
			ImGui::EndPopup();
		}
	}
	void EditorMenu::DisplayHDRSettings()
	{
		if(!m_Editor || !m_Editor->GetRenderer())
			return;

		ImGui::Begin("Render Settings", &m_ShowHDRSettings);

		ImGui::SeparatorText("Quality");

		float& exposure = m_Editor->GetRenderer()->getExposure();

		// Exposure control
		if (ImGui::SliderFloat("Exposure", &exposure, 0.1f, 5.0f, "%.2f"))
		{
			// Exposure value changed
		}

		// Optional: Add a reset button
		if (ImGui::Button("Reset to Default"))
		{
			exposure = 1.0f;
		}

		// Optional: Add tooltip
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Reset exposure to default value (1.0)");
		}

		ImGui::SeparatorText("Lighting");

		/* 
		- Main Light
		- Additional Lights
		For both: 
		bool Cast Shadows
		float Shadows resolution
		int Per object limit
		*/

		ImGui::SeparatorText("Shadows");
		
		// ======================
		// Global bias
		// ======================
		// Slider
		auto& globalBias = m_Editor->GetRenderer()->getGlobalBias();
		if (ImGui::SliderFloat("Global Bias",
			&globalBias,
			0.0f, 2.0f, "%.3f"))
		{
			// Global bias changed
		}

		// Reset button for shadow settings
		if (ImGui::Button("Reset to Default##Shadow"))
		{
			globalBias = 0.005f;
		}

		// Tooltip for bloom settings
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Reset global bias (0.005) to default values");
		}

		ImGui::SeparatorText("Post-Processing");

		// ======================
		// Bloom toggle
		// ======================
		auto& bloomToggle = m_Editor->GetRenderer()->getBloomToggle();
		auto& bloomStrength = m_Editor->GetRenderer()->getBloomStrength();
		auto& bloomFilter = m_Editor->GetRenderer()->getBloomFilterRadius();
		ImGui::Checkbox("Enable Bloom", &bloomToggle);

		// ======================
		// Bloom strength
		// ======================
		// Slider
		if (ImGui::SliderFloat("Bloom Strength",
			&bloomStrength,
			0.0f, 1.0f, "%.3f"))
		{
			// Bloom strength changed
		}

		// Input box on the same line
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::InputFloat("##BloomStrengthInput",
			&bloomStrength,
			0.0f, 0.0f, "%.3f");

		// Clamp manually if you want to enforce range:
		bloomStrength = std::clamp(bloomStrength, 0.0f, 1.0f);

		// ======================
		// Bloom filter radius
		// ======================
		// Slider (typical useful range is small around 0.001 to 0.02)
		if (ImGui::SliderFloat("Filter Radius",
			&bloomFilter,
			0.001f, 0.02f, "%.4f"))
		{
			// Filter radius changed
		}

		// Input box on the same line
		ImGui::SameLine();
		ImGui::SetNextItemWidth(80.0f);
		ImGui::InputFloat("##FilterRadiusInput",
			&bloomFilter,
			0.0f, 0.0f, "%.4f");

		// Clamp to avoid nonsense values 
		bloomFilter = std::clamp(bloomFilter, 0.0001f, 0.05f);

		// Reset button for bloom settings
		if (ImGui::Button("Reset to Default##Bloom"))
		{
			bloomStrength = 0.01f;
			bloomFilter = 0.0025f;
		}

		// Tooltip for bloom settings
		if (ImGui::IsItemHovered())
		{
			ImGui::SetTooltip("Reset bloom strength (0.01) and filter radius (0.0025) to default values");
		}

		auto& gamma = m_Editor->GetRenderer()->getGamma();

		// Adjust gamma
		if (ImGui::SliderFloat("Gamma", &gamma, 1.2f, 3.2f, "%.1f"))
		{
			// Gamma changed
		}

		// Reset button for gamma
		if (ImGui::Button("Reset to Default##Gamma"))
		{
			gamma = 2.2f;
		}

		ImGui::SeparatorText("Debug Drawing");

		auto& dbg = m_Editor->GetRenderer()->GetPhysicsDebugSettings();

		// Master toggle
		ImGui::Checkbox("Enable Debug Draw", &dbg.enabled);

		// Disable the rest when off (nice UX)
		ImGui::BeginDisabled(!dbg.enabled);

		ImGui::Checkbox("Draw Shapes", &dbg.drawShape);
		ImGui::Checkbox("Wireframe", &dbg.wireframe);

		ImGui::Checkbox("Draw Bounding Box (AABB)", &dbg.drawBoundingBox);
		ImGui::Checkbox("Draw Center Of Mass", &dbg.drawCenterOfMass);
		ImGui::Checkbox("Draw World Transform", &dbg.drawWorldTransform);
		ImGui::Checkbox("Draw Velocity", &dbg.drawVelocity);

		ImGui::Separator();

		ImGui::Checkbox("Draw Constraints", &dbg.drawConstraints);
		ImGui::Checkbox("Draw Constraint Limits", &dbg.drawConstraintLimits);

		// Reset button
		if (ImGui::Button("Reset##PhysicsDebug"))
		{
			dbg.enabled = true;
			dbg.drawShape = true;
			dbg.wireframe = true;
			dbg.drawBoundingBox = false;
			dbg.drawCenterOfMass = false;
			dbg.drawWorldTransform = false;
			dbg.drawVelocity = false;
			dbg.drawConstraints = true;
			dbg.drawConstraintLimits = false;
		}

		ImGui::EndDisabled();

		ImGui::End();
	}

	void EditorMenu::SavePrefabPanel()
	{
		if (!m_Editor) return;

		Scene* currentScene = m_Editor->GetActiveScene();
		if (!currentScene) return;

		Entity selectedEntity = m_Editor->GetSelectedEntity();
		if (!selectedEntity) {
			// Show warning that no entity is selected
			if (m_SavePrefabPanel) {
				ImGui::OpenPopup("No Entity Selected");
				m_SavePrefabPanel = false;
			}

			if (ImGui::BeginPopupModal("No Entity Selected", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Please select an entity to save as prefab.");
				if (ImGui::Button("OK", ImVec2(120, 0))) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
			return;
		}

		if (m_SavePrefabPanel) {
			ImGui::OpenPopup("Save Prefab As");
			memset(saveAsDefaultPrefabName, 0, sizeof(saveAsDefaultPrefabName));

			// Pre-fill with selected entity's name if available
			if (selectedEntity.HasComponent<TagComponent>()) {
				std::string entityName = selectedEntity.GetComponent<TagComponent>().Name;
				size_t copySize = entityName.size();
				size_t maxSize = sizeof(saveAsDefaultPrefabName) - 1;
				if (copySize > maxSize) {
					copySize = maxSize;
				}
				strncpy(saveAsDefaultPrefabName, entityName.c_str(), copySize);
				saveAsDefaultPrefabName[copySize] = '\0';
			}

			m_SavePrefabPanel = false;
		}

		if (ImGui::BeginPopupModal("Save Prefab As", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
			ImGui::Text("Save Selected Entity as Prefab");
			ImGui::Separator();

			// Show selected entity name
			std::string selectedName = "None";
			if (selectedEntity.HasComponent<TagComponent>()) {
				selectedName = selectedEntity.GetComponent<TagComponent>().Name;
			}
			ImGui::Text("Selected Entity: %s", selectedName.c_str());

			ImGui::Spacing();
			ImGui::InputText("Prefab Name", saveAsDefaultPrefabName, IM_ARRAYSIZE(saveAsDefaultPrefabName));

			// Save button
			if (ImGui::Button("Save", ImVec2(120, 0))) {
				if (strlen(saveAsDefaultPrefabName) == 0) {
					ImGui::OpenPopup("Empty Prefab Name");
				}
				else {
					std::string defaultPrefabPath = getAssetFilePath("Sources/Prefabs/") + saveAsDefaultPrefabName;
					if (!std::filesystem::path(defaultPrefabPath).has_extension()) {
						defaultPrefabPath += ".prefab"; // Ensure .prefab extension
					}

					if (std::filesystem::exists(defaultPrefabPath)) {
						ImGui::OpenPopup("Confirm Prefab Overwrite");
					}
					else {
						// Actually save the prefab
						if (PrefabSerializer::SerializeEntityToPrefabFile(
							selectedEntity,
							saveAsDefaultPrefabName,
							defaultPrefabPath)) {

							LOG_INFO("Prefab saved successfully: ", defaultPrefabPath);

							// Update the current prefab path in editor
							m_CurrPrefabPath = defaultPrefabPath;
							m_Editor->SetPrefabPath(m_CurrPrefabPath);

							// Register the prefab
							Prefab savedPrefab;
							if (PrefabSerializer::DeserializePrefab(defaultPrefabPath, savedPrefab)) {
								if (!PrefabRegistry::Get().IsPrefabRegistered(savedPrefab.guid)) {
									PrefabRegistry::Get().RegisterPrefab(
										savedPrefab.guid,
										defaultPrefabPath,
										savedPrefab.name
									);
								}
							}

							m_SavePrefabPanel = false;
							ImGui::CloseCurrentPopup();
						}
						else {
							ImGui::OpenPopup("Save Failed");
						}
						
					}
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0))) {
				ImGui::CloseCurrentPopup();
				m_SavePrefabPanel = false;
			}

			// -------- Popup: Empty Prefab Name ----------
			if (ImGui::BeginPopupModal("Empty Prefab Name", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Please enter a prefab name.");
				if (ImGui::Button("OK", ImVec2(120, 0))) {
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			// -------- Popup: Confirm Overwrite ----------
			if (ImGui::BeginPopupModal("Confirm Prefab Overwrite", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
				ImGui::Text("Prefab '%s' already exists.\nDo you want to replace it?", saveAsDefaultPrefabName);
				ImGui::Separator();

				if (ImGui::Button("Yes", ImVec2(120, 0))) {
					std::string defaultPrefabPath = getAssetFilePath("Sources/Prefabs/") + saveAsDefaultPrefabName;
					if (!std::filesystem::path(defaultPrefabPath).has_extension()) {
						defaultPrefabPath += ".prefab";
					}
					
					if (PrefabSerializer::SerializeEntityToPrefabFile(
						selectedEntity,
						saveAsDefaultPrefabName,
						defaultPrefabPath)) {

						LOG_INFO("Prefab overwritten: ", defaultPrefabPath);
						std::string resourcesPath = convertAssetPathToRootResources(defaultPrefabPath);
						PrefabSerializer::SerializeEntityToPrefabFile(
							selectedEntity,
							saveAsDefaultPrefabName,
							resourcesPath
						);
						m_CurrPrefabPath = defaultPrefabPath;
						m_Editor->SetPrefabPath(m_CurrPrefabPath);
						
						// Update prefab registration
						Prefab savedPrefab;
						if (PrefabSerializer::DeserializePrefab(defaultPrefabPath, savedPrefab)) {
							if (PrefabRegistry::Get().IsPrefabRegistered(savedPrefab.guid)) {
								PrefabRegistry::Get().UnregisterPrefab(savedPrefab.guid);
							}
							PrefabRegistry::Get().RegisterPrefab(
								savedPrefab.guid,
								defaultPrefabPath,
								savedPrefab.name
							);
						}

						m_SavePrefabPanel = false;
						m_CloseSavePrefabPanel = true;
						ImGui::CloseCurrentPopup();
					}
					else {
						ImGui::OpenPopup("Save Failed");
						ImGui::CloseCurrentPopup();
					}
				}

				ImGui::SameLine();
				if (ImGui::Button("No", ImVec2(120, 0))) {
					m_SavePrefabPanel = false;
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}

			// Close the main popup if needed
			if (m_CloseSavePrefabPanel) {
				ImGui::CloseCurrentPopup();
				m_SavePrefabPanel = false;
				m_CloseSavePrefabPanel = false;
			}

			ImGui::EndPopup();
		}
	}

	void EditorMenu::SaveCurrentPrefab()
	{
		if (!m_Editor) return;

		std::string currentPrefabPath = m_Editor->GetPrefabPath();
		if (currentPrefabPath.empty()) {
			LOG_ERROR("No current prefab path to save to");
			return;
		}

		Scene* currentScene = m_Editor->GetActiveScene();
		if (!currentScene) return;

		Entity prefabRoot = Entity{};

		//  find an entity with PrefabComponent that's a root
		auto view = currentScene->GetRegistry().view<PrefabComponent>();
		for (auto entity : view) {
			Entity e(entity, &currentScene->GetRegistry());
			auto& prefabComp = e.GetComponent<PrefabComponent>();
			if (prefabComp.isPrefabRoot) {
				prefabRoot = e;
				break;
			}
		}

		//  Use the currently selected entity if no root found
		if (!prefabRoot) {
			prefabRoot = m_Editor->GetSelectedEntity();
			if (!prefabRoot) {
				LOG_ERROR("No entity selected to save as prefab");
				return;
			}
		}

		// Extract prefab name from the file path
		std::filesystem::path pathObj(currentPrefabPath);
		std::string prefabName = m_Editor->GetPrefabName();  
		LOG_INFO("Saving prefab to existing file: ", currentPrefabPath);

		if (PrefabSerializer::SerializeEntityToPrefabFile(
			prefabRoot,
			prefabName,
			currentPrefabPath)) {

			LOG_INFO("Prefab saved successfully: ", currentPrefabPath);
			std::string resourcesPath = convertAssetPathToRootResources(currentPrefabPath);
			if (PrefabSerializer::SerializeEntityToPrefabFile(
				prefabRoot,
				prefabName,
				resourcesPath)) {

				LOG_INFO("Prefab also saved to Resources: ", resourcesPath);
			}
			// Update prefab in registry
			Prefab savedPrefab;
			if (PrefabSerializer::DeserializePrefab(currentPrefabPath, savedPrefab)) {
				// Update registration
				if (PrefabRegistry::Get().IsPrefabRegistered(savedPrefab.guid)) {
					PrefabRegistry::Get().UnregisterPrefab(savedPrefab.guid);
				}
				PrefabRegistry::Get().RegisterPrefab(
					savedPrefab.guid,
					currentPrefabPath,
					savedPrefab.name
				);
			}
		}
		else {
			LOG_ERROR("Failed to save prefab to: ", currentPrefabPath);
		}
	}
}