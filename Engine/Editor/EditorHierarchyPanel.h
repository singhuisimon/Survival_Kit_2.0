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
#include "../Serialization/PrefabInstantiator.h"

namespace Engine
{
	class Editor;
	class EditorHierarchyPanel
	{
	private:
		Editor* m_Editor = nullptr;

		bool openAttachEntityPopup = false;
		bool openSubEntityFromPrefabPopup = false;
		bool openPrefabList = false;
		bool openPrefabListAtRoot = false;
		bool openPrefabListAsChild = false;
		bool openReplacePrefabPanel = false;
		

		std::vector<Entity> entitiesToDelete;
		std::vector<Entity> parentlessChildren;

	
		Entity entityToAttach = Entity();
		Entity parentOfPrefabEntity = Entity();
		Entity entityToReplace = Entity();

	public:
		EditorHierarchyPanel(Editor* editor) : m_Editor(editor) {};
		~EditorHierarchyPanel() = default;

		void HierarchyPanel();
		void EntitiesList();
		void DrawEntityTree(Entity& entity); // Original Code from DrawEntityParentAndChildren
		void DeleteEntityTree(Scene* scene);
		void CheckParentlessChildren(Scene* scene);
		void ClearParentlessChildren(Scene* scene);
		void CreateEntityFromPrefabPanel();
		void OpenReplacePefabPanel();

		Entity FindPrefabRoot(Entity entity);
		std::string SerializeEntityForRevert(Entity entity);

		
	};
}

#endif // EDITOR_HIERARCHYPANEL_H