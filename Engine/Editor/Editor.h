#pragma once
/**
 * @file Editor.h
 * @brief Declaration of the Editor class for running the IMGUI level editor.
 * @author Liliana Hanawardani (50%), Saw Hui Shan (50%)
 * @date September 8, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#ifndef SK_EDITOR_H
#define SK_EDITOR_H

 // Include Editor Header Files
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
#include <memory>
#include <unordered_map>
#include <vector>

// Include other necessary headers
#include "../ECS/Scene.h"
#include "../Editor/EditorMenu.h"
#include "../Editor/EditorHierarchyPanel.h"
#include "../Editor/EditorPropertyPanel.h"
#include "../Editor/EditorPerformancePanel.h"
#include "../Editor/EditorViewportPanel.h"
#include "../Editor/EditorAssetBrowserPanel.h"
#include "../Utility/Timestep.h"
#include "../Profiler/Profiler.h"
#include "Graphics/GraphicsLoader.h"
#include "Graphics/Renderer.h"

class Game;

namespace Engine
{
	// forward declare
	class Scene;
	class Renderer;
	class TracyProfiler;
	class EditorHierarchyPanel;
	class EditorPropertyPanel;
	class EditorPerformancePanel;
	class EditorViewportPanel;
	class EditorAssetBrowserPanel;

	/**
	* @class Editor
	* @brief Displaying and editing information, and the internal logic in level editor.
	*/
	class Editor
	{
	private:

		Scene* m_ActiveScene = nullptr;
		Renderer* m_Renderer = nullptr;
		GLFWwindow* m_Window = nullptr;
		Game* m_Game = nullptr;
		ImGuiIO* io = nullptr;

		std::weak_ptr<TracyProfiler> m_Profiler;

		std::unique_ptr<EditorMenu> m_EditorMenu;
		std::unique_ptr<EditorHierarchyPanel> m_EditorHierarchy;
		std::unique_ptr<EditorPropertyPanel> m_EditorProperty;
		std::unique_ptr<EditorPerformancePanel> m_EditorPerformance;
		std::unique_ptr<EditorViewportPanel> m_EditorViewport;
		std::unique_ptr<EditorAssetBrowserPanel> m_EditorAsset;
		// gizmo
		Entity m_SelectedEntity{};
		u32 m_PickedID = 0xFFFFFFFFu;

		// Editor Viewport 
		EditorViewport editorViewportData;
		EditorViewport m_ImGuizmoViewportData;

		// OnInit
		bool m_Initialized = false;
		bool m_HierarchyWindow = true;
		bool m_PropertyWindow = true;
		bool m_AnimatorWindow = true;
		bool m_PerformanceProfileWindow = true;

		std::string m_CurrentScenePath;
		std::string m_CurrentSceneName;

		std::string m_CurrentPrefabPath;
		std::string m_CurrentPrefabName;
	
	public:
		

		// For Getting AssetPath
		struct AssetEntry
		{
			std::string name;
			std::string fullPath;
		};

		
		Editor(GLFWwindow* window) :
			m_Window(window),
			io(nullptr),
			m_Initialized(false),
			m_ActiveScene(nullptr),
			m_Renderer(nullptr),
			m_Game(nullptr),
			m_PickedID(0xFFFFFFFFu)
		{
			m_EditorMenu = std::make_unique<EditorMenu>(this);
			m_EditorHierarchy = std::make_unique<EditorHierarchyPanel>(this);
			m_EditorProperty = std::make_unique<EditorPropertyPanel>(this);
			m_EditorPerformance = std::make_unique<EditorPerformancePanel>(this);
			m_EditorViewport = std::make_unique<EditorViewportPanel>(this);
			m_EditorAsset = std::make_unique<EditorAssetBrowserPanel>(this);
		};

		~Editor() = default;

		void OnInit();
		void OnUpdate(Timestep ts, GLuint texhandle);
		void StartImguiFrame();
		void RenderViewport(GLuint texhandle);
		void CompleteFrame();

		Scene* CreateNewScene(const std::string& name);

		void SetActiveScene(Scene* scene);
		Scene* GetActiveScene() const { return m_ActiveScene; }
		bool HasActiveScene() const { return m_ActiveScene != nullptr;};
		void SaveActiveSceneToPath(const std::string& path);

		void SetScenePath(const std::string& path) { m_CurrentScenePath = path; }
		const std::string& GetScenePath() const { return m_CurrentScenePath; }
		void ClearScenePath() { m_CurrentScenePath.clear(); }
		bool HasScenePath() const { return !m_CurrentScenePath.empty(); }

		void SetPrefabPath(const std::string& path) { m_CurrentPrefabPath = path; }
		const std::string& GetPrefabPath() const { return m_CurrentPrefabPath; }
		void ClearPrefabPath() { m_CurrentPrefabPath.clear(); }
		bool HasPrefabPath() const { return !m_CurrentPrefabPath.empty(); }

		void SetSceneName(const std::string& name);
		std::string GetSceneName() const;

		void SetPrefabName(const std::string& name);
		std::string GetPrefabName() const;

		void SetRenderer(Renderer* renderer) { m_Renderer = renderer; }
		Renderer* GetRenderer() const{ return m_Renderer; }

		void SetGame(Game* game) { m_Game = game; }
		Game* GetGame() const { return m_Game; }

		void SetCurrSelectedEntity(const Entity& entity) { m_SelectedEntity = entity; }
		const Entity GetSelectedEntity() const { return m_SelectedEntity; }

		ImGuizmo::OPERATION GetOperation();
		void SetOperation(ImGuizmo::OPERATION operation);

		// =================== Use In Game.cpp ===================
		void SetTracy(const std::shared_ptr<TracyProfiler>& profiler) {
			m_Profiler = profiler; // still increases refcount, no extra copy on call
		}
		std::weak_ptr<TracyProfiler> GetProfiler() const { return m_Profiler; }

		void RetrievePickedID(u32 id) { m_PickedID = id; }
		u32 GetPickedID() { return m_PickedID; }
		void SetEditorViewport(EditorViewport& vp) const { vp = editorViewportData; }

		void ViewportClickAndTeleport();
		bool GetEditorIsPlaying();
		// ================== Helper Function ======================
		std::vector<AssetEntry> getAssetsInFolder(const std::string& folderPath);

		bool& GetHierarchyWindowRef() { return m_HierarchyWindow; }
		bool& GetPropertyWindowRef() { return m_PropertyWindow; }
		bool& GetAnimatorWindowRef() { return m_AnimatorWindow; }
		bool& GetPerformanceProfileWindowRef() { return m_PerformanceProfileWindow; }

		void immediatelyCompile() {
			AM.CompileAllAsset(0);
		}
	};
}


#endif // LOF_EDITOR_H