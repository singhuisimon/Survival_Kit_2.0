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
#include "../Component/PrefabComponent.h"
#include "../Transform/TransformSystem.h"
#include "../Component/SpriteRendererComponent.h"

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

		bool m_AnimatorWindow = false;			// Animator/dopesheet window toggle
		bool m_FocusAnimatorNextFrame = false;	// Request Animator window focus next frame

		// --- Animator / dopesheet editor state ---
		enum class DopesheetTrackType
		{
			None,
			Position,
			Rotation,
			Scale,
			UVTiling,
			UVOffset
		};

		enum class AnimatorViewMode
		{
			Dopesheet,
			Curves
		};

		enum class AnimatorComponentTrack
		{
			Transform,
			UVTransform
		};

		DopesheetTrackType m_DopesheetSelectedTrack = DopesheetTrackType::None;
		int                m_DopesheetSelectedKey = -1;  // index into that track’s key array
		AnimatorViewMode    m_AnimatorViewMode = AnimatorViewMode::Dopesheet;
		AnimatorComponentTrack m_SelectedComponentTrack = AnimatorComponentTrack::Transform;

		bool IsComponentOverridden(ComponentTypeID componentType);
		void MarkComponentOverridden(ComponentTypeID componentType, const std::string& propertyName = "");
		// bool IsComponentRemoved(ComponentTypeID componentType);
		//void MarkComponentRemoved(ComponentTypeID componentType);

		bool IsComponentAddedToInstance(ComponentTypeID type);
		bool WasComponentInPrefab(ComponentTypeID type);

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
		
		void DisplayBTComponent(ImVec2& buttonSize);
		void DisplayParticleComponent(ImVec2& buttonSize);
		void DisplayScriptComponent(ImVec2& buttonSize);
		
		void DisplayLightComponent(ImVec2& buttonSize);
		void DisplayCameraComponent(ImVec2& buttonSize);
		void DisplayAnimatorComponent(ImVec2& buttonSize);
		void DisplaySpriteRendererComponent(ImVec2& buttonSize);
		void DisplayAssetField(const char* label, xresource::instance_guid& guid, ResourceType expectedType, bool& errorFlag, ComponentTypeID type);
		
			   
		void AddComponent();
		void AnimatorWindow();
		const char* ColliderTypeToString(ColliderType& colliderType);
		void ReplaceChildNode(std::shared_ptr<BTNode> parent,
			std::shared_ptr<BTNode> oldChild,
			std::shared_ptr<BTNode> newChild);
		void DrawBTNodeEditor(std::shared_ptr<BTNode> node, std::shared_ptr<BTNode> parent = nullptr);
		static void DrawCurveLegendRow(const char* label,
			const char* c0Label, ImU32 c0,
			const char* c1Label, ImU32 c1,
			const char* c2Label, ImU32 c2);
	};
}
#endif // END OF EDITOR_PROPERTYPANEL_H