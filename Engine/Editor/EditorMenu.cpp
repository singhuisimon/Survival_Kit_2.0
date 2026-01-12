#include "EditorMenu.h"
#include "../Engine/Editor/Editor.h"
#include "../Asset/AssetManager.h"
#include "../Asset/ResourceManager.h"
#include "../Serialization/PrefabSerializer.h"
#include "../Prefab/PrefabRegistry.h"
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
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Open scene from file.");
				}
				// ----------------- Save Scene ------------------------
				if (ImGui::MenuItem("Save Scene"))
				{
					m_CurrScenePath = m_Editor->GetScenePath();
					Scene*m_Scene = m_Editor->GetActiveScene();
					Renderer* m_Renderer = m_Editor->GetRenderer();

					LOG_DEBUG("====== Save Current Scene: ", m_CurrScenePath, " =======");
					if (!m_CurrScenePath.empty())
					{
						m_Editor->SaveActiveSceneToPath(m_CurrScenePath);
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
				if (ImGui::MenuItem("Save Scene As..."))
				{
					m_SaveScenePanel = true;
				}
				if (ImGui::IsItemHovered())
				{
					ImGui::SetTooltip("Save scene as a new file.");
				}
				// ----- Open Prefab temporary -----
				if (ImGui::MenuItem("Open Prefab File"))
				{
					m_OpenPrefabPanel = true;
					m_CurrScenePath = "";
					m_Editor->SetScenePath(m_CurrScenePath);
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

				ImGui::Separator();

				ImGui::MenuItem("HDR Settings", nullptr, &m_ShowHDRSettings);

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
						Scene* m_Scene = m_Editor->GetActiveScene();
						Renderer* m_Renderer = m_Editor->GetRenderer();

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

		ImGui::Begin("HDR Settings", &m_ShowHDRSettings);

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


		ImGui::Separator();

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

		ImGui::End();
	}

}