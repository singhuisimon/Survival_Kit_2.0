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
	};
}
#endif // END OF EDITOR_PROPERTYPANEL_H