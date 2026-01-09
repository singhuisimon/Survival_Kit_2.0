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

		enum class PlayState
		{
			STOP,
			PLAY,
			PAUSE
		};
		PlayState m_PlayState = PlayState::STOP;

		std::string m_OriginalScenePath;
		std::string m_OriginalSceneName;

	public:
		EditorViewportPanel(Editor* editor) : m_Editor(editor) {};
		~EditorViewportPanel() = default;

		void ManipulateEntityTransform(Entity& entity, EditorViewport m_ImGuizmoViewportData);
		void HandleGizmoPicked(EditorViewport m_ImGuizmoViewportData);
		void ViewportClickAndTeleport();

		void Play();
		void Pause();
		void Stop();
		void ViewportButtons();

		bool IsPlaying() const { return m_PlayState == PlayState::PLAY; }
		bool IsPause() const {  return  m_PlayState == PlayState::PAUSE; }
		bool IsStop() const { return  m_PlayState == PlayState::STOP; }

		PlayState GetPlayState() const { return m_PlayState; }

		void SetOriginalScene(const std::string& path, const std::string& name) {
			m_OriginalScenePath = path;
			m_OriginalSceneName = name;
		}

		ImGuizmo::OPERATION GetOperation() { return m_Operation; }
		void SetOperation(ImGuizmo::OPERATION operation) { m_Operation = operation; }
	};
}
#endif