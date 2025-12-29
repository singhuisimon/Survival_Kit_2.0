#pragma once
#ifndef EDITOR_PROPERTYPANEL_H
#define EDITOR_PROPERTYPANEL_H
// include necessary file
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// include header file
#include "../Utility/AssetPath.h"
#include "../Editor/Editor.h"

namespace Engine
{
	class Editor;
	class EditorPropertyPanel
	{
		
	private:
		Editor* m_Editor = nullptr;

		//Entity m_SelectedEntity{};

	public:
		EditorPropertyPanel(Editor* editor) : m_Editor(editor) {};
		~EditorPropertyPanel() = default;

		void PropertyPanel();
	};
}
#endif // END OF EDITOR_PROPERTYPANEL_H