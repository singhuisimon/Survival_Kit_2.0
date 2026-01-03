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
#include "../Component/TransformComponent.h"
#include "../Transform/TransformSystem.h"

namespace Engine
{
	class Editor;
	class EditorPropertyPanel
	{
		
	private:
		Editor* m_Editor = nullptr;
		Scene* m_Scene = nullptr;
		Entity m_SelectedEntity{};
		std::string m_ScenePath{};
		std::string m_SceneName{};

	public:
		EditorPropertyPanel(Editor* editor) : m_Editor(editor) {};
		~EditorPropertyPanel() = default;

		void PropertyPanel();

		void DisplayTagComponent();
		void DisplayPrefabComponent(ImVec2& buttonSize);
		void DisplayTransformComponent(ImVec2& buttonSize);

		void DisplayRigidBodyComponent(ImVec2& buttonSize);
		void DisplayMeshRendererComponent(ImVec2& buttonSize);
		
		void DisplayAudioComponent(ImVec2& buttonSize);
		void DisplayReverbZoneComponent(ImVec2& buttonSize);
		void DisplayListenerComponent(ImVec2& buttonSize);
		
		//void DisplayBTComponent(ImVec2& buttonSize);
		void DisplayParticleComponent(ImVec2& buttonSize);
		//void DisplayScriptComponent(ImVec2& buttonSize);
		
		void DisplayLightComponent(ImVec2& buttonSize);
		void DisplayCameraComponent(ImVec2& buttonSize);
		//void DisplayAnimatorComponent(ImVec2& buttonSize);
			   
		void AddComponent();
		const char* ColliderTypeToString(ColliderType& colliderType);
	};
}
#endif // END OF EDITOR_PROPERTYPANEL_H