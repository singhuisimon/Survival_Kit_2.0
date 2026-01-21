#pragma once
#ifndef EDITOR_MENU_H
#define EDITOR_MENU_H

// include necessary file
#include <string>

// include header file
#include "../Utility/AssetPath.h"

namespace Engine
{
	// Forward Declaration
	class Editor;
	class Renderer;
	class EditorHierarchyPanel;

	class EditorMenu
	{
	public:

		EditorMenu(Editor* editor) : m_Editor(editor) {}
		~EditorMenu() = default;
		void EditorTopMenu();
		void OpenScenePanel();
		void SaveScenePanel();
		void OpenPrefabPanel();
		void SavePrefabPanel();
		void SaveCurrentPrefab();
		void DisplayHDRSettings();
		
	
	private:
		Editor* m_Editor = nullptr;
		bool m_NewScenePanel = false;
		bool m_OpenScenePanel = false;
		bool m_SaveScenePanel = false;
		bool m_CloseSaveAsPanel = false;
		bool m_OpenPrefabPanel = false;
		bool m_ShowHDRSettings = false;
		bool m_SavePrefabPanel = false;
		bool m_CloseSavePrefabPanel = false;
		bool m_SceneModified = false;
		bool m_SceneIsNew = false;

		std::string m_CurrScenePath = "";
		std::string m_CurrPrefabPath = "";
		char saveAsDefaultSceneName[128] = {}; // default new scene path (in SaveAsScenePanel)
		char saveAsDefaultPrefabName[256] = "";
	};
}

#endif // EDITOR_MENU_H