#pragma once
#ifndef EDITOR_ASSETBROWSERPANEL_H
#define EDITOR_ASSETBROWSERPANEL_H
// include necessary file
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// include header file
#include "../Utility/AssetPath.h"
#include "../Editor/Editor.h"
#include "../Asset/DescriptorEditor.h"

namespace Engine
{
	struct FilteredAssetInfo
	{
		std::string fileName;
		std::string folderPath;
		std::string fullPath;
		ResourceType type;  
	};

	struct DisplayableAsset
	{
		std::string fileName;        // Display name
		std::string fullPath;        // Full file path
		std::string displayFolder;   // Where it came from (for tooltip)
		AssetRecord* record;         // Can be nullptr for non-raw assets
		xresource::instance_guid guid;
		bool isRawAsset;             
		DisplayableAsset() : record(nullptr), guid{}, isRawAsset(true) {}
	};

	enum class AssetViewMode
	{
		Grid,
		List
	};

	class Editor;
	class EditorAssetBrowserPanel
	{

	private:
		Editor* m_Editor = nullptr;
		bool assetsWindow = true;

		ResourceType selectedType = ResourceType::UNKNOWN;
		bool raw_asset = false;
		std::string selectedFolder = ""; // for the selected folder in asset browser
		int selectedResourcesIndex = -1; // for the selected index in the assets browser

		DescriptorEditor descriptorEditor;
		bool showDescriptorEditorPanel = false;
		xresource::instance_guid currentEditingGuid;
		std::string editedAsset{};
		bool m_ShouldApplyOverrides = false;

		// for search function
		char m_SearchBuffer[256] = "";
		std::string m_SearchQuery = "";
		std::vector<FilteredAssetInfo> m_FilteredAssets;
		ResourceType m_FilterType = ResourceType::UNKNOWN;

		// Delete file
		bool m_ShowDeleteConfirmPopUp = false;
		DisplayableAsset m_AssetToDelete;

		AssetViewMode m_ViewMode = AssetViewMode::Grid;


		/*Scene* m_Scene = nullptr;
		Entity m_SelectedEntity{};
		std::string m_ScenePath{};
		std::string m_SceneName{};*/

	public:

		EditorAssetBrowserPanel(Editor* editor) : m_Editor(editor) {};
		~EditorAssetBrowserPanel() = default;

		void AssetBrowserPanel();

		void DisplayAssetsBrowser();
		void DisplayDescriptorEditorPanel();
		void RenderAssetGrid(const std::vector<DisplayableAsset>& displayAssets);
		void HandleAssetSelection(const DisplayableAsset& asset);
		std::vector<DisplayableAsset>BuildDisplayAssets(const std::vector<const AssetRecord*>& records, const std::string& displayFolder,bool isRawAssets);
		void FilterAssetsBySearchQuery();
		void SearchFolderRecursive(const std::string& folderPath, const std::string& displayFolder, const std::string& lowerQuery);
		void RenderAssetList(const std::vector<DisplayableAsset>& displayAssets);
	};
}
#endif // END OF EDITOR_PROPERTYPANEL_H