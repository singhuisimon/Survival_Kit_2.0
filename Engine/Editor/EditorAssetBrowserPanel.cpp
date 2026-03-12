#include <windows.h>
#include <algorithm>
#include "EditorAssetBrowserPanel.h"
#include "../Utility/Logger.h"
#include "../Engine/Editor/Editor.h"
#include "../Asset/AssetManager.h"
#include "../Asset/ResourceManager.h"
#include "../Asset/ResourceHelpers.h"
#include "../Prefab/PrefabRegistry.h"
#include "../Serialization/PrefabSerializer.h"

/*#include "../Animation/AnimationStorage.h"
#include "../BehaviourTree/BehaviourTreeEditor.h"
#include "../Graphics/Camera.h"
#include "../Graphics/Texture.h"
#include "../Scripting/ScriptSerializer.h"
#include "../Scripting/MonoScriptEngine.h"
#include  "../Serialization/MaterialSerializer.h"*/

namespace Engine
{
	void EditorAssetBrowserPanel::AssetBrowserPanel()
	{
		DisplayAssetsBrowser();
		DisplayDescriptorEditorPanel();
	}
#if 0
	void EditorAssetBrowserPanel::DisplayAssetsBrowser()
	{
		ImGui::SetNextWindowSize(ImVec2(600, 400));

		// Begin properties dockable window
		if (ImGui::Begin("Assets Browser", &assetsWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			//float leftColumnWidth = 200.0f * m_Editor->GetFontScale();
			ImGui::Columns(2, nullptr, true);
			//ImGui::SetColumnWidth(0, leftColumnWidth);
			//static std::string selectedFolder = "";
			//static ResourceType selectedType = ResourceType::UNKNOWN;

			// ================= Left column panel display all the resources folder ========================
			ImGui::BeginChild("Project List", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Text("Projects:");

			// For resources handled by Asset Browser
			if (ImGui::CollapsingHeader("Raw Resources", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& db = AM.db();
				auto allAssets = db.AllMutable();

				std::set<ResourceType> availableTypes;
				for (const auto* record : allAssets)
				{
					if (record && record->valid && record->type != ResourceType::UNKNOWN)
					{
						availableTypes.insert(record->type);
					}
				}

				for (const auto& type : availableTypes)
				{
					std::string typeName = resourceTypeToString(type);
					bool isSelected = (selectedType == type);

					if (ImGui::Selectable(typeName.c_str(), isSelected))
					{

						raw_asset = true;
						selectedType = type;
						selectedFolder = typeName;
						selectedResourcesIndex = -1;
					}
				}
				
			}

			// For resources handled by filepath (Prefabs and Scenes)
			if (ImGui::CollapsingHeader("Composed Resources", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto folders = m_Editor->getAssetsInFolder(getAssetFilePath("Sources/"));

				for (auto& folder : folders)
				{
					if (folder.name != "Audio" && folder.name != "Meshes" && folder.name != "Shaders" && folder.name != "Textures" && folder.name != "Material")
					{
						bool isSelected = (selectedFolder == folder.fullPath);
						if (ImGui::Selectable(folder.name.c_str(), isSelected))
						{
							selectedType = ResourceType::UNKNOWN;
							raw_asset = false;
							selectedFolder = folder.fullPath;
							selectedResourcesIndex = -1; // reset asset selection
						}
					}
				}
			}

			ImGui::EndChild();

			// ================= Right column panel - display assets of selected type ========================

			auto& db = AM.db();
			auto allAssets = db.AllMutable();

			std::vector<const AssetRecord*> filteredAssets;
			filteredAssets.reserve(allAssets.size());

			for (const auto* record : allAssets)
			{
				if (!record || !record->valid) continue;
				if (record->type == selectedType)
				{
					filteredAssets.push_back(record);
				}
			}

			// To get the files in the selected folder
			auto assetsList = m_Editor->getAssetsInFolder(selectedFolder);
			std::string currentOpenPath = "";

			
			if (!currentOpenPath.empty() && selectedType != ResourceType::UNKNOWN)
			{
				for (size_t i = 0; i < filteredAssets.size(); ++i)
				{
					if (filteredAssets[i]->sourcePath == currentOpenPath)
					{
						selectedResourcesIndex = static_cast<int>(i);
						break;
					}
				}
			}
			ImGui::NextColumn();
			ImGui::BeginChild("Asset List", ImVec2(0, 0), true);


			if (raw_asset && selectedResourcesIndex != -1)
			{
				ImGui::Text("Asset Selected: %s", filteredAssets[selectedResourcesIndex]->sourcePath.c_str());
			}
			else if (!raw_asset && selectedResourcesIndex != -1)
			{
				ImGui::Text("Asset Selected: %s", assetsList[selectedResourcesIndex].fullPath.c_str());
			}

			// For resources handled by Asset Browser
			if (!selectedFolder.empty() && raw_asset)
			{

				// Display filtered assets
				ImGui::Text(("Resources > " + resourceTypeToString(selectedType)).c_str());
				ImGui::Separator();

			
				const float baseThumbnailSize = 150.0f;
				const float thumbnailSize = baseThumbnailSize * m_Editor->GetFontScale(); // Scale the box
				const float padding = 10.0f * m_Editor->GetFontScale();
				const float cellSize = thumbnailSize + padding;
			
				float panelWidth = ImGui::GetContentRegionAvail().x;
				int itemsPerRow = std::max(1, static_cast<int>(panelWidth / cellSize));

				if (ImGui::BeginTable("AssetGrid", itemsPerRow))
				{
					
					for (size_t i = 0; i < filteredAssets.size(); ++i)
					{

						const auto* record = filteredAssets[i];

						std::filesystem::path assetPath(record->sourcePath);
						std::string filename = assetPath.filename().string();
						std::string extension = record->ext;
						std::string hash = record->contentHash;
						std::time_t writeTime = record->lastWriteTime;

						ImGui::TableNextColumn();
						std::string currentOpenPath = "";
						if (m_Editor->HasScenePath())
						{
							currentOpenPath = m_Editor->GetScenePath();
						}
						else if (m_Editor->HasPrefabPath())
						{
							currentOpenPath = m_Editor->GetPrefabPath();
						}

						bool isSelected = (selectedResourcesIndex == static_cast<int>(i)) ||
							(record->sourcePath == currentOpenPath);

						// Optional background color for selected
						if (isSelected)
						{
							ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.65f, 0.20f, 1.0f)); // selected color
							ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.75f, 0.30f, 1.0f));
							ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.85f, 0.55f, 0.15f, 1.0f));
						}

						// Unique ID per button
						ImGui::PushID(static_cast<int>(i));

						if (ImGui::Button(filename.c_str(), ImVec2(thumbnailSize, thumbnailSize)))
						{
							selectedResourcesIndex = static_cast<int>(i);
							ImGui::OpenPopup("AssetContextMenu");
						}

						// ======================= DRAG-DROP SOURCE  =======================
						// Enables dragging assets from the browser to component fields
						if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
						{
							// Package the GUID as the payload
							xresource::instance_guid draggedGuid = record->guid;
							ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &draggedGuid, sizeof(xresource::instance_guid));

							// Visual feedback while dragging
							ImGui::Text("Dragging: %s", filename.c_str());
							ImGui::Text("Type: %s", resourceTypeToString(record->type).c_str());

							ImGui::EndDragDropSource();
						}

						if (ImGui::BeginPopupContextItem("AssetContextMenu"))
						{
							ImGui::Text("%s", filename.c_str());

							// Only Texture and Meshes for now
							if (record->type == ResourceType::TEXTURE || record->type == ResourceType::MESH)
							{
								ImGui::Separator();

								if (ImGui::MenuItem("Edit"))
								{
									// open asset editor or show rename dialog
									LOG_INFO("Edit asset: ", filename);

									showDescriptorEditorPanel = true;
									currentEditingGuid = record->guid;
									editedAsset = filename;
								}

							}
							ImGui::EndPopup();
						}

						if (isSelected)
						{
							ImGui::PopStyleColor(3);
						}

						// ==================== Display info detail ==========================
						if (ImGui::IsItemHovered())
						{
							ImGui::BeginTooltip();
							ImGui::Text("Name: %s", filename.c_str());
							ImGui::Text("Type: %s", extension.c_str());
							ImGui::Text("Content Hash: %s", hash.c_str());

							char timeBuf[64];
							std::tm* tm_local = std::localtime(&writeTime);
							std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_local);
							ImGui::Text("Last Write Time: %s", timeBuf);

							ImGui::EndTooltip();
						}

						// ==================== To center text under thumbnail ================
						ImVec2 textSize = ImGui::CalcTextSize(filename.c_str());
						float textX = (thumbnailSize - textSize.x) * 0.5f;
						if (textX < 0) textX = 0;
						ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textX);
						ImGui::TextWrapped("%s", filename.c_str());

						ImGui::PopID();
						ImGui::NextColumn();
					}
					ImGui::EndTable();
				}
			}

			// For resources handled by filepath
			if (!selectedFolder.empty() && !raw_asset)
			{
				// display the selected folder name
				std::filesystem::path folderPath(selectedFolder);
				std::string folderName = folderPath.filename().string();
				ImGui::Text(("Resources > " + folderName).c_str());

				ImGui::Separator();

				const float baseThumbnailSize = 150.0f;
				const float thumbnailSize = baseThumbnailSize * m_Editor->GetFontScale(); // Scale the box
				const float padding = 10.0f * m_Editor->GetFontScale();
				const float cellSize = thumbnailSize + padding;
				float panelWidth = ImGui::GetContentRegionAvail().x;
				int itemsPerRow = std::max(1, (int)(panelWidth / cellSize));

				// int textureCount = -1;
				ImGui::Columns(itemsPerRow, nullptr, false);

				std::string currentOpenPath = "";
				if (m_Editor->HasScenePath())
				{
					currentOpenPath = m_Editor->GetScenePath();
				}
				else if (m_Editor->HasPrefabPath())
				{
					currentOpenPath = m_Editor->GetPrefabPath();
				}


				// loop through files in selected folder
				for (size_t i = 0; i < assetsList.size(); i++)
				{
					const auto& asset = assetsList[i];
					std::string fileName = asset.name;
					std::string filePath = asset.fullPath;

					ImGui::PushID(fileName.c_str());

					bool isSelected = (selectedResourcesIndex == static_cast<int>(i));

					if (isSelected)
					{
						// Change the button background color
						ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.65f, 0.20f, 1.0f)); // selected color
						ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.75f, 0.30f, 1.0f));
						ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.85f, 0.55f, 0.15f, 1.0f));
					}

					if (ImGui::Button(fileName.c_str(), ImVec2(thumbnailSize, thumbnailSize)))
					{
						selectedResourcesIndex = static_cast<int>(i);

						std::string extension = asset.name.substr(asset.name.find_last_of('.'));
						if (extension == ".json" && folderName != "BT") // For scene, not BT
						{
							LOG_DEBUG(" ==== Start Loading Scene ==== : ", fileName);
							m_Editor->SetScenePath(filePath); // update curr file path
							//currFileName = fileName; // store file name
							m_Editor->ClearPrefabPath();

							LOG_DEBUG("m_Scene->SetName(fileName)", fileName);
							if (m_Editor->GetActiveScene())
							{
								m_Editor->SetCurrSelectedEntity(Entity{});
								
								m_Editor->GetActiveScene()->GetRegistry().clear();
								m_Editor->GetActiveScene()->LoadFromFile(filePath);

							
								m_Editor->RetrievePickedID(0xFFFFFFFFu);
								m_Editor->SetOperation(static_cast<ImGuizmo::OPERATION>(-1));

								// Update settings 
								m_Editor->GetRenderer()->getBloomToggle() = m_Editor->GetActiveScene()->GetSceneSetting().s_BloomToggle;
								m_Editor->GetRenderer()->getBloomStrength() = m_Editor->GetActiveScene()->GetSceneSetting().s_BloomStrength;
								m_Editor->GetRenderer()->getBloomFilterRadius() = m_Editor->GetActiveScene()->GetSceneSetting().s_BloomFilterRadius;
								m_Editor->GetRenderer()->getExposure() = m_Editor->GetActiveScene()->GetSceneSetting().s_Exposure;

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

							LOG_DEBUG(" ==== End Loading Scene ==== : ", fileName);
						}
						else if (extension == ".prefab" && folderName != "BT") // FOr Prefab, not BT (To be fixed in M3)
						{
							LOG_DEBUG("=====Start Load Prefab File=========");
							m_Editor->SetPrefabPath(filePath);
							m_Editor->ClearScenePath();
							LOG_DEBUG("filePathName: ", filePath);

							std::string prefabName = "Prefab: " + fileName;
							Scene* newScene = m_Editor->CreateNewScene(prefabName);
							if (!newScene)
							{
								LOG_ERROR("Failed to create new scene");
								continue;
							}
							Prefab loadedPrefab;
							if (PrefabRegistry::Get().LoadPrefabFromFile(filePath, loadedPrefab))
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
							
						}
					}
					// to change the color of the selected
					if (isSelected)
					{
						ImGui::PopStyleColor(3);
					}

					// ==================== Display info detail ==========================
					if (ImGui::IsItemHovered())
					{
						ImGui::BeginTooltip();
						ImGui::Text("Name: %s", fileName.c_str());
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

			ImGui::EndChild();
			ImGui::Columns(1);

		}

		ImGui::End();
	}
#endif

#if 1
	void EditorAssetBrowserPanel::DisplayAssetsBrowser()
	{
		ImGui::SetNextWindowSize(ImVec2(600, 400));
		bool isOpen = ImGui::Begin("Assets Browser", &assetsWindow, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
		if (isOpen)
		{
			// ===== SEARCH BAR =====
			ImGui::Text("Search:");
			ImGui::SameLine();
			ImGui::PushItemWidth(500.0f);
			if (ImGui::InputText("##AssetSearch", m_SearchBuffer, sizeof(m_SearchBuffer)))
			{
				m_SearchQuery = std::string(m_SearchBuffer);
				FilterAssetsBySearchQuery();
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			ImGui::PushItemWidth(150.0f);
			if (ImGui::BeginCombo("##TypeFilter", resourceTypeToString(m_FilterType).c_str()))
			{
				if (ImGui::Selectable("All Types", m_FilterType == ResourceType::UNKNOWN))
				{
					m_FilterType = ResourceType::UNKNOWN;
					FilterAssetsBySearchQuery(); // Refresh search
				}

				for (int i = 0; i < (int)ResourceType::UNKNOWN; i++)
				{
					ResourceType t = static_cast<ResourceType>(i);
					if (ImGui::Selectable(resourceTypeToString(t).c_str(), m_FilterType == t))
					{
						m_FilterType = t;
						FilterAssetsBySearchQuery(); // Refresh search
					}
				}
				ImGui::EndCombo();
			}
			ImGui::PopItemWidth();
			ImGui::SameLine();
			if (ImGui::Button("Clear"))
			{
				m_SearchBuffer[0] = '\0';
				m_SearchQuery = "";
				m_FilterType = ResourceType::UNKNOWN;
				m_FilteredAssets.clear();
				selectedResourcesIndex = -1;
			}

			ImGui::SameLine();
			if (ImGui::Button(m_ViewMode == AssetViewMode::Grid ? "List View" : "Grid View"))
			{
				m_ViewMode = (m_ViewMode == AssetViewMode::Grid)
					? AssetViewMode::List
					: AssetViewMode::Grid;
			}

			ImGui::Separator();

			bool isSearching = !m_SearchQuery.empty() || m_FilterType != ResourceType::UNKNOWN;

			ImGui::Columns(2, nullptr, true);

			// ===== LEFT COLUMN: Folder Selection =====
			ImGui::BeginChild("Project List", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Text("Projects:");

			if (ImGui::CollapsingHeader("Raw Resources", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto& db = AM.db();
				auto allAssets = db.AllMutable();

				std::set<ResourceType> availableTypes;
				for (const auto* record : allAssets)
				{
					if (record && record->valid && record->type != ResourceType::UNKNOWN)
					{
						availableTypes.insert(record->type);
					}
				}

				for (const auto& type : availableTypes)
				{
					std::string typeName = resourceTypeToString(type);
					bool isSelected = (selectedType == type);

					if (ImGui::Selectable(typeName.c_str(), isSelected))
					{
						raw_asset = true;
						selectedType = type;
						selectedFolder = typeName;
						selectedResourcesIndex = -1;
					}
				}
			}

			if (ImGui::CollapsingHeader("Composed Resources", ImGuiTreeNodeFlags_DefaultOpen))
			{
				auto folders = m_Editor->getAssetsInFolder(getAssetFilePath("Sources/"));

				for (auto& folder : folders)
				{
					if (folder.name != "Audio" && folder.name != "Meshes" && folder.name != "Shaders" &&
						folder.name != "Textures" && folder.name != "Material")
					{
						bool isSelected = (selectedFolder == folder.fullPath);
						if (ImGui::Selectable(folder.name.c_str(), isSelected))
						{
							selectedType = ResourceType::UNKNOWN;
							raw_asset = false;
							selectedFolder = folder.fullPath;
							selectedResourcesIndex = -1;
						}
					}
				}
			}

			ImGui::EndChild();

			// ===== RIGHT COLUMN: Asset Display =====
			ImGui::NextColumn();
			ImGui::BeginChild("Asset List", ImVec2(0, 0), true);

			// Build the list of assets to display
			std::vector<DisplayableAsset> assetsToDisplay;

			if (isSearching)
			{
				// Use search results - convert FilteredAssetInfo to DisplayableAsset
				ImGui::Text("Search Results (%zu found)", m_FilteredAssets.size());
				ImGui::Separator();

				// Get database once for efficiency
				auto& db = AM.db();
				auto allAssets = db.AllMutable();

				for (const auto& filteredInfo : m_FilteredAssets)
				{
					DisplayableAsset displayAsset;
					displayAsset.fileName = filteredInfo.fileName;
					displayAsset.fullPath = filteredInfo.fullPath;
					displayAsset.displayFolder = filteredInfo.folderPath;
					displayAsset.isRawAsset = false;
					displayAsset.record = nullptr;
					displayAsset.guid = xresource::instance_guid{};

					// Try to find the record in asset manager
					for (const auto* record : allAssets)
					{
						if (!record || !record->valid) continue;

						// Try exact path match first
						if (record->sourcePath == filteredInfo.fullPath)
						{
							displayAsset.record = const_cast<AssetRecord*>(record);
							displayAsset.guid = record->guid;
							displayAsset.isRawAsset = true;
							break;
						}

						// Try filename match as fallback for filesystem-based assets
						std::filesystem::path recordFile(record->sourcePath);
						std::filesystem::path searchFile(filteredInfo.fullPath);
						if (recordFile.filename() == searchFile.filename() &&
							record->type != ResourceType::UNKNOWN)
						{
							displayAsset.record = const_cast<AssetRecord*>(record);
							displayAsset.guid = record->guid;
							displayAsset.isRawAsset = true;
							break;
						}
					}

					assetsToDisplay.push_back(displayAsset);
				}
			}
			else if (!selectedFolder.empty() && raw_asset)
			{
				// Display raw assets
				ImGui::Text(("Resources > " + resourceTypeToString(selectedType)).c_str());
				ImGui::Separator();

				auto& db = AM.db();
				auto allAssets = db.AllMutable();
				std::vector<const AssetRecord*> filteredAssets;

				for (const auto* record : allAssets)
				{
					if (!record || !record->valid) continue;
					if (record->type == selectedType)
					{
						filteredAssets.push_back(record);
					}
				}

				assetsToDisplay = BuildDisplayAssets(filteredAssets, resourceTypeToString(selectedType), true);
			}
			else if (!selectedFolder.empty() && !raw_asset)
			{
				// Display filesystem-based assets
				std::filesystem::path folderPath(selectedFolder);
				std::string folderName = folderPath.filename().string();
				ImGui::Text(("Resources > " + folderName).c_str());
				ImGui::Separator();

				auto assetsList = m_Editor->getAssetsInFolder(selectedFolder);

				for (const auto& asset : assetsList)
				{
					// Skip subdirectories, only show files
					if (asset.name.find('.') == std::string::npos) continue;

					DisplayableAsset displayAsset;
					displayAsset.fileName = asset.name;
					displayAsset.fullPath = asset.fullPath;
					std::filesystem::path p(asset.fullPath);
					ResourceType detectedType = detectResourceTypeFromPath(asset.fullPath);
					displayAsset.displayFolder = resourceTypeToString(detectedType);
					displayAsset.isRawAsset = false;
					displayAsset.record = nullptr;

					assetsToDisplay.push_back(displayAsset);
				}
			}

			// ===== UNIFIED RENDERING =====
			if (!assetsToDisplay.empty())
			{
				if (!assetsToDisplay.empty())
				{
					if (m_ViewMode == AssetViewMode::Grid)
						RenderAssetGrid(assetsToDisplay);
					else
						RenderAssetList(assetsToDisplay);
				}
			}
			else if (!isSearching)
			{
				ImGui::TextDisabled("No assets found");
			}

			ImGui::EndChild();
			ImGui::Columns(1);

		}
		ImGui::End();
	}

#endif


	void EditorAssetBrowserPanel::DisplayDescriptorEditorPanel()
	{

		if (!showDescriptorEditorPanel)
		{
			descriptorEditor.Clear();
			return;
		}

		if (ImGui::Begin("Descriptor Editor Panel", &showDescriptorEditorPanel, ImGuiWindowFlags_NoDocking))
		{
			LOG_DEBUG("displayDescriptorEditorPanel OPEN");

			if (!descriptorEditor.IsLoaded() || currentEditingGuid != descriptorEditor.GetGuid())
			{
				if (!descriptorEditor.Load(currentEditingGuid))
				{
					ImGui::Text("Failed to load descriptor for %s", editedAsset.c_str());
				}
			}
			else
			{
				ImGui::Columns(2, nullptr, true);

				// Drawing asset in descriptor editor if it is a texture
				if (descriptorEditor.GetType() == ResourceType::TEXTURE)
				{
					auto* texture = RM.loadResource<TextureResource>(Engine::convertToTextureGuid(currentEditingGuid));
					if (texture != nullptr)
					{
						float tex_w = static_cast<float>(texture->width);
						float tex_h = static_cast<float>(texture->height);


						ImVec2 window_size = ImGui::GetWindowSize();
						float win_w = window_size.x * 3 / 4;
						float win_h = window_size.y * 3 / 4;

						float aspect = tex_w / tex_h;

						ImVec2 viewportSize;
						if (win_w / win_h > aspect)
						{
							viewportSize.x = win_h * aspect;
							viewportSize.y = win_h;
						}
						else
						{
							viewportSize.x = win_w;
							viewportSize.y = win_w / aspect;
						}

						ImGui::Image(
							(ImTextureID)(intptr_t)((GLuint)texture->textureID),
							viewportSize,
							ImVec2(0, 0), ImVec2(1, 1)
						);
					}
				}

				ImGui::NextColumn();

				ImGui::SeparatorText("Asset Information");
				ImGui::Text("Asset Name: %s", descriptorEditor.GetDisplayName().c_str());
				ImGui::Text("Source: %s", descriptorEditor.GetSourcePath().c_str());

				std::string assetType{};
				switch (descriptorEditor.GetType())
				{
				case ResourceType::TEXTURE:
					assetType = "Texture";
					break;
				case ResourceType::MESH:
					assetType = "Mesh";
					break;
				default:
					break;
				}
				ImGui::Text("Asset Type: %s", assetType.c_str());

				ImGui::Spacing();
				ImGui::SeparatorText("Editable Properties");

				// Check type
				if (descriptorEditor.GetType() == ResourceType::TEXTURE)
				{

					TextureSettings* settings = descriptorEditor.GetTextureSettings();

					auto quality = settings->quality;
					if (ImGui::SliderFloat("Quality", &quality, 0.0f, 1.0f))
					{
						settings->quality = quality;
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Minimaps", &settings->generateMipmaps))
					{
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("sRGB", &settings->srgb))
					{
						descriptorEditor.MarkModified();
					}

					if (ImGui::BeginCombo("Compression", settings->compression.c_str()))
					{
						for (auto& option : descriptorEditor.GetCompressionOptions())
						{
							if (ImGui::Selectable(option.c_str()))
							{
								settings->compression = option;
								descriptorEditor.MarkModified();
							}
						}
						ImGui::EndCombo();
					}

					if (ImGui::BeginCombo("Usage", settings->usageType.c_str()))
					{
						for (auto& option : descriptorEditor.GetUsageTypeOptions())
						{
							if (ImGui::Selectable(option.c_str()))
							{
								settings->usageType = option;
								descriptorEditor.MarkModified();
							}
						}
						ImGui::EndCombo();
					}
				}
				else if (descriptorEditor.GetType() == ResourceType::MESH)
				{

					MeshSettings* settings = descriptorEditor.GetMeshSettings();

					// ========== TRANSFORM SECTION ==========
					ImGui::SeparatorText("Transform");

					// Scale
					float meshScale = settings->scale;
					if (ImGui::DragFloat("Scale", &meshScale, 0.001f, 0.0001f, 1000.0f, "%.4f"))
					{
						settings->scale = meshScale;
						descriptorEditor.MarkModified();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Uniform scale factor (e.g., 0.001 for mm to m)");
					}

					ImGui::Spacing();

					// Position
					ImGui::Text("Position Offset:");
					float position[3] = { settings->positionX, settings->positionY, settings->positionZ };
					if (ImGui::DragFloat3("Position", position, 0.1f))
					{
						settings->positionX = position[0];
						settings->positionY = position[1];
						settings->positionZ = position[2];
						descriptorEditor.MarkModified();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Position offset in mesh units (X, Y, Z)");
					}

					ImGui::Spacing();

					// Rotation
					ImGui::Text("Rotation (Degrees):");
					float rotation[3] = { settings->rotationX, settings->rotationY, settings->rotationZ };
					if (ImGui::DragFloat3("Rotation", rotation, 1.0f, -180.0f, 180.0f))
					{
						settings->rotationX = rotation[0];
						settings->rotationY = rotation[1];
						settings->rotationZ = rotation[2];
						descriptorEditor.MarkModified();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Rotation in degrees (X=Pitch, Y=Yaw, Z=Roll)");
					}



					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// ========== VERTEX DATA SECTION ==========
					ImGui::SeparatorText("Vertex Data");

					if (ImGui::Checkbox("Include Position", &settings->includePos))
					{
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Include Normals", &settings->includeNormals))
					{
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Include Colors", &settings->includeColors))
					{
						descriptorEditor.MarkModified();
					}

					if (ImGui::Checkbox("Include Texture Coordinates", &settings->includeTexCoords))
					{
						descriptorEditor.MarkModified();
					}

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// ========== OUTPUT SETTINGS SECTION ==========
					ImGui::SeparatorText("Output Settings");

					char formatBuffer[256];
					strncpy_s(formatBuffer, sizeof(formatBuffer), settings->outputFormat.c_str(), _TRUNCATE);
					if (ImGui::InputText("Output Format", formatBuffer, sizeof(formatBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
					{
						settings->outputFormat = std::string(formatBuffer);
						descriptorEditor.MarkModified();
					}

					if (ImGui::BeginCombo("Index Type", settings->indexType.c_str()))
					{
						for (auto& option : descriptorEditor.GetIndexTypeOptions())
						{
							if (ImGui::Selectable(option.c_str()))
							{
								settings->indexType = option;
								descriptorEditor.MarkModified();
							}
						}
						ImGui::EndCombo();
					}

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					// ========== OPTIMIZATION SECTION ==========
					ImGui::SeparatorText("Optimization");

					if (ImGui::Checkbox("Optimize Vertices", &settings->optimizeVertices))
					{
						descriptorEditor.MarkModified();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Remove duplicate vertices and optimize for cache");
					}

					if (ImGui::Checkbox("Generate Normals", &settings->generateNormals))
					{
						descriptorEditor.MarkModified();
					}
					if (ImGui::IsItemHovered())
					{
						ImGui::SetTooltip("Generate normals if missing");
					}

				}

				if (!descriptorEditor.GetTags().empty())
				{
					ImGui::SeparatorText("Tags");
					for (auto& tag : descriptorEditor.GetTags())
					{
						ImGui::Text("%s", tag.c_str());
					}
				}

				ImGui::SeparatorText("Last Imported");
				std::time_t writeTime = descriptorEditor.GetLastImported();
				char timeBuf[64];
				std::tm* tm_local = std::localtime(&writeTime);
				std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_local);
				ImGui::Text("Last Write Time: %s", timeBuf);

				static std::string notifMsg{};
				static ImVec4 notifColour(0.0f, 0.0f, 0.0f, 0.0f);

				if (ImGui::Button("Validate Descriptor"))
				{
					if (descriptorEditor.Validate())
					{
						notifMsg = "Descriptor is Valid";
						notifColour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
					}
					else
					{
						notifMsg = "Descriptor is NOT Valid";
						notifColour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
					}
				}

				if (!notifMsg.empty())
				{

					ImGui::TextColored(notifColour, "%s", notifMsg.c_str());

					static float notifTimer = 2.0f;
					notifTimer -= ImGui::GetIO().DeltaTime;

					if (notifTimer <= 0.0f)
					{
						notifTimer = 2.0f;
						notifMsg.clear();
					}
				}

				// Save button
				if (descriptorEditor.IsModified())
				{
					if (ImGui::Button("Save & Compile"))
					{
						if (descriptorEditor.Save())
						{
							notifMsg = "Descriptor is Saved";
							notifColour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);

							//compile
							if (AM.CompileSingleAsset(currentEditingGuid, true))
							{
								notifMsg = "Saved and Compiled successfully!";
								notifColour = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);//green 
							}
							else
							{
								notifMsg = "Saved but compilation FAILED";
								notifColour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);  // Red for error
							}
						}
						else
						{
							notifMsg = "Descriptor is NOT Saved";
							notifColour = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
						}
					}


				}
			}

			ImGui::End();
		}
	}
	void EditorAssetBrowserPanel::FilterAssetsBySearchQuery()
	{
		m_FilteredAssets.clear();

		if (m_SearchQuery.empty())
		{
			return;
		}

		// Convert search query to lowercase for case-insensitive search
		std::string lowerQuery = m_SearchQuery;
		std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

		// First, search the asset database for raw assets
		auto& db = AM.db();
		auto allAssets = db.AllMutable();

		for (const auto* record : allAssets)
		{
			if (!record || !record->valid) continue;

			// Extract filename from sourcePath
			std::filesystem::path assetPath(record->sourcePath);
			std::string fileName = assetPath.filename().string();

			// Convert asset name to lowercase
			std::string lowerFileName = fileName;
			std::transform(lowerFileName.begin(), lowerFileName.end(), lowerFileName.begin(), ::tolower);
			bool matchesText = lowerFileName.find(m_SearchQuery) != std::string::npos;
			bool matchesType = (m_FilterType == ResourceType::UNKNOWN) || (record->type == m_FilterType);
			if (matchesText && matchesType)
			{
				FilteredAssetInfo info;
				info.fileName = fileName;
				info.folderPath = resourceTypeToString(record->type);
				info.fullPath = record->sourcePath;

				// Check if we already added this (avoid duplicates)
				bool alreadyAdded = false;
				for (const auto& existing : m_FilteredAssets)
				{
					if (existing.fullPath == info.fullPath)
					{
						alreadyAdded = true;
						break;
					}
				}

				if (!alreadyAdded)
				{
					m_FilteredAssets.push_back(info);
				}
			}
		}

		//// Then search filesystem folders for other asset types
		//std::vector<std::string> foldersToSearch = {
		//"Sources/",
		//"Texture/",
		//"Scenes/",
		//"Shaders/",
		//"Material/",
		//"Mesh/",
		//"Prefabs/",
		//"AnimationClips/",
		//"Audio/",
		//"AudioSetting/",
		//"BT/",
		//"Fonts/"
		//};
		//// Search through filesystem folders
		//for (const auto& folder : foldersToSearch)
		//{
		//	SearchFolderRecursive(getAssetFilePath(folder), folder, lowerQuery);
		//}
		// Replace the entire foldersToSearch vector + loop with this:
		SearchFolderRecursive(getAssetFilePath("Sources/"), "Sources/", lowerQuery);
	}
	void EditorAssetBrowserPanel::SearchFolderRecursive(const std::string& folderPath, const std::string& displayFolder, const std::string& lowerQuery)
	{
		auto assets = m_Editor->getAssetsInFolder(folderPath);

		for (const auto& asset : assets)
		{

			bool isFile = asset.name.find('.') != std::string::npos;

			if (!isFile)
			{

				SearchFolderRecursive(folderPath + asset.name + "/", displayFolder, lowerQuery);
				continue;
			}
			std::string lowerFileName = asset.name;
			std::transform(lowerFileName.begin(), lowerFileName.end(), lowerFileName.begin(), ::tolower);
			//bool matchesType = (m_FilterType == ResourceType::UNKNOWN) || (detectedType == m_FilterType);
			// Convert asset name to lowercase
			bool matchesText = lowerQuery.empty() || lowerFileName.find(lowerQuery) != std::string::npos;

			ResourceType detectedType = detectResourceTypeFromPath(asset.fullPath);
			std::string actualDisplayFolder = resourceTypeToString(detectedType);

			bool matchesType = (m_FilterType == ResourceType::UNKNOWN) || (detectedType == m_FilterType);
			//if (m_FilterType != ResourceType::UNKNOWN) {
			//	std::string ext = std::filesystem::path(asset.name).extension().string();
			//	// Map your extensions to ResourceTypes
			//	if (m_FilterType == ResourceType::ENTITY_PREFAB && ext != ".prefab") matchesType = false;
			//	else if (m_FilterType == ResourceType::SCENE_PREFAB && ext != ".json") matchesType = false;
			//	else if (m_FilterType == ResourceType::MESH && ext != ".fbx") matchesType = false;
			//}

			//bool matchesText = lowerQuery.empty() || lowerFileName.find(lowerQuery) != std::string::npos;

			if (matchesText && matchesType)
			{
				// Skip if already added from asset DB
				bool alreadyAdded = false;
				for (const auto& existing : m_FilteredAssets)
				{
					if (existing.fullPath == asset.fullPath ||
						existing.fileName == asset.name) // fallback check
					{
						alreadyAdded = true;
						break;
					}
				}

				if (!alreadyAdded)
				{
					FilteredAssetInfo info;
					info.fileName = asset.name;
					std::filesystem::path p(asset.fullPath);
					info.folderPath = actualDisplayFolder;
					info.fullPath = asset.fullPath;
					m_FilteredAssets.push_back(info);
				}
			}
		}
	}

	void EditorAssetBrowserPanel::RenderAssetGrid(const std::vector<DisplayableAsset>& displayAssets)
	{
		const float baseThumbnailSize = 150.0f;
		const float thumbnailSize = baseThumbnailSize * m_Editor->GetFontScale();
		const float padding = 10.0f * m_Editor->GetFontScale();
		const float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int itemsPerRow = std::max(1, static_cast<int>(panelWidth / cellSize));

		if (ImGui::BeginTable("AssetGridTable", itemsPerRow))
		{
			for (size_t i = 0; i < displayAssets.size(); ++i)
			{
				const auto& asset = displayAssets[i];
				ImGui::TableNextColumn();

				bool isSelected = (!asset.fullPath.empty() && (
					(m_Editor->HasScenePath() && asset.fullPath == m_Editor->GetScenePath()) ||
					(m_Editor->HasPrefabPath() && asset.fullPath == m_Editor->GetPrefabPath())
					));

				// ===== STYLING =====
				int styleColorsPushed = 0;
				if (isSelected)
				{
					ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.95f, 0.65f, 0.20f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.75f, 0.30f, 1.0f));
					ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.85f, 0.55f, 0.15f, 1.0f));
					styleColorsPushed = 3;
				}

				ImGui::PushID(static_cast<int>(i));

				// ===== BUTTON =====
				if (ImGui::Button(asset.fileName.c_str(), ImVec2(thumbnailSize, thumbnailSize)))
				{
					selectedResourcesIndex = static_cast<int>(i);

					// Handle different asset types on click
					HandleAssetSelection(asset);
				}

				// Pop style colors IMMEDIATELY
				if (styleColorsPushed > 0)
				{
					ImGui::PopStyleColor(styleColorsPushed);
				}

				// ===== DRAG AND DROP =====
				if (asset.isRawAsset && asset.record)
				{
					if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
					{
						xresource::instance_guid draggedGuid = asset.guid;
						ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &draggedGuid, sizeof(xresource::instance_guid));

						ImGui::Text("Dragging: %s", asset.fileName.c_str());
						ImGui::Text("Type: %s", resourceTypeToString(asset.record->type).c_str());

						ImGui::EndDragDropSource();
					}
				}

				// ===== CONTEXT MENU =====
				if (ImGui::BeginPopupContextItem("AssetContextMenu"))
				{
					ImGui::Text("%s", asset.fileName.c_str());

					if (asset.isRawAsset && asset.record)
					{
						if (asset.record->type == ResourceType::TEXTURE || asset.record->type == ResourceType::MESH)
						{
							ImGui::Separator();

							if (ImGui::MenuItem("Edit"))
							{
								LOG_INFO("Edit asset: ", asset.fileName);
								showDescriptorEditorPanel = true;
								currentEditingGuid = asset.guid;
								editedAsset = asset.fileName;
							}
						}
					}

					// Delete file
					ImGui::Separator();
					if (ImGui::MenuItem("Delete"))
					{
						m_AssetToDelete = asset;
						m_ShowDeleteConfirmPopUp = true;
					}
					ImGui::EndPopup();
				}

				// ===== TOOLTIP =====
				if (ImGui::IsItemHovered())
				{
					ImGui::BeginTooltip();

					ImGui::Text("Name: %s", asset.fileName.c_str());
					//ImGui::Text("Path: %s", asset.displayFolder.c_str());

					if (asset.isRawAsset && asset.record)
					{
						ImGui::Text("Type: %s", resourceTypeToString(asset.record->type).c_str());
						ImGui::Text("Content Hash: %s", asset.record->contentHash.c_str());

						char timeBuf[64];
						std::tm* tm_local = std::localtime(&asset.record->lastWriteTime);
						std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_local);
						ImGui::Text("Last Write Time: %s", timeBuf);
					}
					ImGui::EndTooltip();
				}

				// ===== CENTER TEXT UNDER THUMBNAIL =====
				ImVec2 textSize = ImGui::CalcTextSize(asset.fileName.c_str());
				float textX = (thumbnailSize - textSize.x) * 0.5f;
				if (textX < 0) textX = 0;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + textX);
				ImGui::TextWrapped("%s", asset.fileName.c_str());

				// Show folder path below
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
				ImVec2 folderSize = ImGui::CalcTextSize(asset.displayFolder.c_str());
				float folderX = (thumbnailSize - folderSize.x) * 0.5f;
				if (folderX < 0) folderX = 0;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + folderX);
				ImGui::TextWrapped("%s", asset.displayFolder.c_str());
				ImGui::PopStyleColor();

				ImGui::PopID();
			}
			ImGui::EndTable();
		}
		//ImGui::EndChild();

		if (m_ShowDeleteConfirmPopUp)
		{
			ImGui::OpenPopup("Confirm Delete");
			m_ShowDeleteConfirmPopUp = false;
		}

		if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Are you sure you want to delete:");
			ImGui::Text("%s", m_AssetToDelete.fileName.c_str());
			ImGui::Separator();

			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				if (std::filesystem::exists(m_AssetToDelete.fullPath))
				{
					std::string resourcesPath = convertAssetPathToRootResources(m_AssetToDelete.fullPath);
					std::filesystem::remove(m_AssetToDelete.fullPath);
					std::filesystem::remove(resourcesPath);
					LOG_INFO("Deleted asset: ", m_AssetToDelete.fullPath);
					LOG_INFO("Deleted asset from root: ", resourcesPath);

					// Clear selection if the deleted asset was selected
					if (m_Editor->HasScenePath() && m_Editor->GetScenePath() == m_AssetToDelete.fullPath)
						m_Editor->SetScenePath("");
					else if (m_Editor->HasPrefabPath() && m_Editor->GetPrefabPath() == m_AssetToDelete.fullPath)
						m_Editor->ClearPrefabPath();

					selectedResourcesIndex = -1;
				}
				ImGui::CloseCurrentPopup();
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
			}

			ImGui::EndPopup();
		}

	}

	void EditorAssetBrowserPanel::HandleAssetSelection(const DisplayableAsset& asset)
	{
		std::string extension = asset.fullPath.substr(asset.fullPath.find_last_of('.'));

		// Handle scene files
		if (extension == ".json")
		{
			LOG_DEBUG("==== Start Loading Scene ====: ", asset.fileName);
			m_Editor->SetScenePath(asset.fullPath);
			m_Editor->ClearPrefabPath();

			if (m_Editor->GetActiveScene())
			{
				m_Editor->SetCurrSelectedEntity(Entity{});
				m_Editor->GetActiveScene()->GetRegistry().clear();
				m_Editor->GetActiveScene()->LoadFromFile(asset.fullPath);

				m_Editor->RetrievePickedID(0xFFFFFFFFu);
				m_Editor->SetOperation(static_cast<ImGuizmo::OPERATION>(-1));

				// Update settings
				m_Editor->GetRenderer()->getBloomToggle() = m_Editor->GetActiveScene()->GetSceneSetting().s_BloomToggle;
				m_Editor->GetRenderer()->getBloomStrength() = m_Editor->GetActiveScene()->GetSceneSetting().s_BloomStrength;
				m_Editor->GetRenderer()->getBloomFilterRadius() = m_Editor->GetActiveScene()->GetSceneSetting().s_BloomFilterRadius;
				m_Editor->GetRenderer()->getExposure() = m_Editor->GetActiveScene()->GetSceneSetting().s_Exposure;

				m_Editor->GetAudioManager()->SetEditorCap(AudioType::MASTER, m_Editor->GetActiveScene()->GetSceneSetting().s_MasterVolume);
				m_Editor->GetAudioManager()->SetEditorCap(AudioType::SFX, m_Editor->GetActiveScene()->GetSceneSetting().s_SFXVolume);
				m_Editor->GetAudioManager()->SetEditorCap(AudioType::BGM, m_Editor->GetActiveScene()->GetSceneSetting().s_BGMVolume);
				m_Editor->GetAudioManager()->SetEditorCap(AudioType::UI, m_Editor->GetActiveScene()->GetSceneSetting().s_UIVolume);
				m_Editor->GetAudioManager()->SetEditorCap(AudioType::VO, m_Editor->GetActiveScene()->GetSceneSetting().s_VOVolume);
				m_Editor->GetAudioManager()->SetEditorCap(AudioType::GAMESFX, m_Editor->GetActiveScene()->GetSceneSetting().s_GameSFXVolume);
			}

			// Register prefabs
			auto prefabFiles = m_Editor->getAssetsInFolder(getAssetFilePath("Sources/Prefabs"));
			for (auto& prefabAsset : prefabFiles)
			{
				if (prefabAsset.name.find(".prefab") == std::string::npos) continue;

				Prefab prefab;
				if (PrefabSerializer::DeserializePrefab(prefabAsset.fullPath, prefab))
				{
					if (prefab.guid.m_Value != 0 && !PrefabRegistry::Get().IsPrefabRegistered(prefab.guid))
					{
						PrefabRegistry::Get().RegisterPrefab(prefab.guid, prefabAsset.fullPath, prefab.name);
						LOG_INFO("Registered prefab: ", prefab.name.c_str());
					}
				}
			}

			LOG_DEBUG("==== End Loading Scene ====: ", asset.fileName);
		}
		// Handle prefab files
		else if (extension == ".prefab")
		{
			LOG_DEBUG("===== Start Load Prefab File =========");
			m_Editor->SetPrefabPath(asset.fullPath);
			m_Editor->ClearScenePath();

			std::string prefabName = "Prefab: " + asset.fileName;
			Scene* newScene = m_Editor->CreateNewScene(prefabName);
			if (!newScene)
			{
				LOG_ERROR("Failed to create new scene");
				return;
			}

			Prefab loadedPrefab;
			if (PrefabRegistry::Get().LoadPrefabFromFile(asset.fullPath, loadedPrefab))
			{
				LOG_INFO("Successfully loaded prefab: ", loadedPrefab.name);
				LOG_INFO("Prefab has ", loadedPrefab.entities.size(), " entities");

				Entity prefabRoot = PrefabInstantiator::InstantiatePrefab(
					newScene,
					loadedPrefab,
					Entity{}
				);

				if (prefabRoot)
				{
					m_Editor->SetCurrSelectedEntity(prefabRoot);
					m_Editor->RetrievePickedID(static_cast<uint32_t>(prefabRoot.GetHandle()));
					LOG_INFO("Prefab instantiated successfully");
				}
				else
				{
					LOG_ERROR("Failed to instantiate prefab");
				}
			}

			LOG_DEBUG("===== End Load Prefab File =========");
		}
	}

	std::vector<DisplayableAsset> EditorAssetBrowserPanel::BuildDisplayAssets(
		const std::vector<const AssetRecord*>& records,
		const std::string& displayFolder,
		bool isRawAssets)
	{
		std::vector<DisplayableAsset> result;

		for (size_t i = 0; i < records.size(); ++i)
		{
			if (isRawAssets && !records[i])
				continue;

			DisplayableAsset displayAsset;
			displayAsset.isRawAsset = isRawAssets;
			displayAsset.displayFolder = displayFolder;

			if (isRawAssets)
			{
				displayAsset.record = const_cast<AssetRecord*>(records[i]);
				displayAsset.guid = records[i]->guid;
				displayAsset.fullPath = records[i]->sourcePath;

				std::filesystem::path assetPath(records[i]->sourcePath);
				displayAsset.fileName = assetPath.filename().string();
			}

			result.push_back(displayAsset);
		}

		return result;
	}

	void EditorAssetBrowserPanel::RenderAssetList(const std::vector<DisplayableAsset>& displayAssets)
	{
		for (size_t i = 0; i < displayAssets.size(); ++i)
		{
			const auto& asset = displayAssets[i];

			bool isSelected = (!asset.fullPath.empty() && (
				(m_Editor->HasScenePath() && asset.fullPath == m_Editor->GetScenePath()) ||
				(m_Editor->HasPrefabPath() && asset.fullPath == m_Editor->GetPrefabPath())
				));

			ImGui::PushID(static_cast<int>(i));

			// Clickable row per asset
			if (ImGui::Selectable(asset.fileName.c_str(), isSelected, ImGuiSelectableFlags_AllowOverlap))
			{
				selectedResourcesIndex = static_cast<int>(i);
				HandleAssetSelection(asset);
			}

			// ===== TOOLTIP =====
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("Name: %s", asset.fileName.c_str());
				if (asset.isRawAsset && asset.record)
				{
					ImGui::Text("Type: %s", resourceTypeToString(asset.record->type).c_str());
					ImGui::Text("Hash: %s", asset.record->contentHash.c_str());
					char timeBuf[64];
					std::tm* tm_local = std::localtime(&asset.record->lastWriteTime);
					std::strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_local);
					ImGui::Text("Last Write: %s", timeBuf);
				}
				ImGui::EndTooltip();
			}

			// ===== CONTEXT MENU =====
			if (ImGui::BeginPopupContextItem("AssetListContextMenu"))
			{
				ImGui::Text("%s", asset.fileName.c_str());
				if (asset.isRawAsset && asset.record)
				{
					if (asset.record->type == ResourceType::TEXTURE ||
						asset.record->type == ResourceType::MESH)
					{
						ImGui::Separator();
						if (ImGui::MenuItem("Edit"))
						{
							showDescriptorEditorPanel = true;
							currentEditingGuid = asset.guid;
							editedAsset = asset.fileName;
						}
					}
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Delete"))
				{
					m_AssetToDelete = asset;
					m_ShowDeleteConfirmPopUp = true;
				}
				ImGui::EndPopup();
			}

			// ===== DRAG AND DROP =====
			if (asset.isRawAsset && asset.record)
			{
				if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
				{
					xresource::instance_guid draggedGuid = asset.guid;
					ImGui::SetDragDropPayload("ASSET_BROWSER_ITEM", &draggedGuid,
						sizeof(xresource::instance_guid));
					ImGui::Text("Dragging: %s", asset.fileName.c_str());
					ImGui::EndDragDropSource();
				}
			}

			// Optional: dim the folder/type info on the same line to the right
			ImGui::SameLine();
			ImGui::TextDisabled("  %s", asset.displayFolder.c_str());

			ImGui::PopID();
		}

		// ===== DELETE CONFIRM POPUP =====
		if (m_ShowDeleteConfirmPopUp)
		{
			ImGui::OpenPopup("Confirm Delete");
			m_ShowDeleteConfirmPopUp = false;
		}

		if (ImGui::BeginPopupModal("Confirm Delete", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Are you sure you want to delete:");
			ImGui::Text("%s", m_AssetToDelete.fileName.c_str());
			ImGui::Separator();

			if (ImGui::Button("Delete", ImVec2(120, 0)))
			{
				if (std::filesystem::exists(m_AssetToDelete.fullPath))
				{
					std::string resourcesPath = convertAssetPathToRootResources(m_AssetToDelete.fullPath);
					std::filesystem::remove(m_AssetToDelete.fullPath);
					std::filesystem::remove(resourcesPath);

					if (m_Editor->HasScenePath() && m_Editor->GetScenePath() == m_AssetToDelete.fullPath)
						m_Editor->SetScenePath("");
					else if (m_Editor->HasPrefabPath() && m_Editor->GetPrefabPath() == m_AssetToDelete.fullPath)
						m_Editor->ClearPrefabPath();

					selectedResourcesIndex = -1;
				}
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel", ImVec2(120, 0)))
				ImGui::CloseCurrentPopup();

			ImGui::EndPopup();
		}
	}
}