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
				ImGui::MenuItem("Hierarchy", NULL, &m_Editor->GetHierarchyWindowRef());
				ImGui::MenuItem("Property", NULL, &m_Editor->GetPropertyWindowRef());
				ImGui::MenuItem("Performance", NULL, &m_Editor->GetPerformanceProfileWindowRef());
				ImGui::EndMenu();
			}

			// ---------------- Display Current Scene Name ---------------------
			if (!m_CurrScenePath.empty())
			{
				std::filesystem::path filePath(m_CurrScenePath);
				std::string fileName = filePath.string();

				float textWidth = ImGui::CalcTextSize(fileName.c_str()).x;
				float menuBarWidth = ImGui::GetWindowSize().x;

				ImGui::SameLine(menuBarWidth - textWidth - 10.0f);
				ImGui::TextUnformatted(fileName.c_str());
			}
			if (!m_CurrPrefabPath.empty())
			{
				std::filesystem::path filePath(m_CurrPrefabPath);
				std::string fileName = filePath.string();

				float textWidth = ImGui::CalcTextSize(fileName.c_str()).x;
				float menuBarWidth = ImGui::GetWindowSize().x;

				ImGui::SameLine(menuBarWidth - textWidth - 10.0f);
				ImGui::TextUnformatted(fileName.c_str());
			}

			ImGui::EndMainMenuBar();
		}
		OpenScenePanel();
		SaveScenePanel();
		OpenPrefabPanel();
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

					Entity prefabRoot = PrefabInstantiator::InstantiatePrefabFromFile(
						newScene,
						m_CurrPrefabPath,
						Entity{}  // No parent
					);
					if (prefabRoot) {
						m_Editor->SetCurrSelectedEntity(prefabRoot);
						m_Editor->RetrievePickedID(static_cast<uint32_t>(prefabRoot.GetHandle()));

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
}