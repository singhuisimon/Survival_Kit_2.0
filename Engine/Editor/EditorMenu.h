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
	
	private:
		Editor* m_Editor = nullptr;
		bool m_NewScenePanel = false;
		bool m_OpenScenePanel = false;
		bool m_SaveScenePanel = false;
		bool m_CloseSaveAsPanel = false;

		std::string m_CurrScenePath = "";
		char saveAsDefaultSceneName[128] = {}; // default new scene path (in SaveAsScenePanel)
	};
}

#endif // EDITOR_MENU_H