#pragma once

#ifndef EDITOR_VIEWPORTPANEL_H
#define EDITOR_VIEWPORTPANEL_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <algorithm>
#include <ImGuizmo.h>

#include "../Editor/Editor.h"
#include "../Graphics/GraphicsLoader.h"
#include "Graphics/Renderer.h"

namespace Engine
{
	class Editor;
	class EditorViewportPanel
	{
	private:
		Editor* m_Editor = nullptr;
		ImGuizmo::OPERATION m_Operation = static_cast<ImGuizmo::OPERATION>(-1);

		// Editor Viewport 
		/*EditorViewport editorViewportData;
		EditorViewport m_ImGuizmoViewportData;*/
	public:
		EditorViewportPanel(Editor* editor) : m_Editor(editor) {};
		~EditorViewportPanel() = default;

		void ManipulateEntityTransform(Entity& entity, EditorViewport m_ImGuizmoViewportData);
		void HandleGizmoPicked(EditorViewport m_ImGuizmoViewportData);
		void ViewportClickAndTeleport();

	};
}
#endif