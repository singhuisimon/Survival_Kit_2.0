#pragma once
#ifndef EDITOR_HIERARCHYPANEL_H
#define EDITOR_HIERARCHYPANEL_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Include Standard Headers
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <algorithm>
#include <ImGuizmo.h>

// Include other necessary headers
#include "../Editor/Editor.h"
#include "../ECS/Components.h"
#include "../Utility/Logger.h"

namespace Engine
{
	class Editor;
	class EditorHierarchyPanel
	{
	private:
		Editor* m_Editor = nullptr;

		bool openAttachEntityPopup = false;
		bool openSubEntityFromPrefabPopup = false;

		std::vector<Entity> entitiesToDelete;
		std::vector<Entity> parentlessChildren;

		Entity m_SelectedEntity {};
		Entity entityToAttach = Entity();
		Entity parentOfPrefabEntity = Entity();

		u32 m_PickedID = 0xFFFFFFFFu;

	public:
		EditorHierarchyPanel(Editor* editor) : m_Editor(editor) {};
		~EditorHierarchyPanel() = default;

		void HierarchyPanel();
		void EntitiesList();
		void DrawEntityTree(Entity& entity); // Original Code from DrawEntityParentAndChildren
		void DeleteEntityTree(Scene* scene);
		void CheckParentlessChildren(Scene* scene);
		void ClearParentlessChildren(Scene* scene);
	};
}

#endif // EDITOR_HIERARCHYPANEL_H