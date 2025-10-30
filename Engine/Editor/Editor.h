#pragma once
/**
 * @file ImguiManager.h
 * @brief Declaration of the IMGUI_Manager class for running the IMGUI level editor.
 * @author Liliana Hanawardani (45%), Saw Hui Shan (45%), Rio Shannon Yvon Leonardo (10%)
 * @date September 8, 2025
 * Copyright (C) 2025 DigiPen Institute of Technology.
 * Reproduction or disclosure of this file or its contents without the
 * prior written consent of DigiPen Institute of Technology is prohibited.
 */

#ifndef SK_IMGUI_MANAGER_H
#define SK_IMGUI_MANAGER_H

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

// Include other necessary headers
#include "../Profiler/Profiler.h"
#include "../ECS/Scene.h"
#include "../ECS/Components.h"
#include "../Utility/Timestep.h"
#include "../Utility/Logger.h"
#include "../Utility/AssetPath.h"
#include "../Prefab/Prefab.h"
#include "../Prefab/PrefabRegistry.h"
#include "../Serialization/PrefabInstantiator.h"

// Temporary inclusion to access EditorViewport data struct
#include "Graphics/GraphicsLoader.h"


namespace Engine
{
	/**
	* @class Editor
	* @brief Displaying and editing information, and the internal logic in level editor.
	*/
	class Editor
	{
	private:
		GLFWwindow* m_Window = nullptr;
		bool m_Initialized = false;
		ImGuiIO* io;
		Scene* m_Scene;
		Entity m_SelectedEntity{};
		GLuint m_FBOTextureHandle;
		std::weak_ptr<TracyProfiler> m_Profiler;
		u32 m_PickedID;

		// ImGui Window functionality
		bool inspectorWindow = true;
		bool hierachyWindow = true;
		bool assetsWindow = true;
		bool performanceProfileWindow = true;

		// ImGui Top Menu Panel
		bool openScenePanel = false; // for top menu open file 
		bool saveAsPanel = false; // pop up save as panel
		bool openScript = false; // pop up open script option
		bool createScript = false; // pop up panel to create new script
		bool isNewScene = false;  // to check if is new scene

		// ImGui other helper variable
		std::string currScenePath{}; // to store current scene path 
		char saveAsDefaultSceneName[128] = {}; // default new scene path (in SaveAsScenePanel)
		int selectedResourcesIndex = -1; // for the selected index in the assets browser

		// Prefab helper variables
		std::unordered_set<std::string> m_TemporaryPrefabPaths; // only save the prefab file if save the scene 
		std::string currPrefabPath{};
		bool isPrefabEditor = false;
		bool replacePrefabPending = false;
		std::string selectedPrefabPath{};
		std::string currFileName{};
		Prefab* m_CurrentPrefab = nullptr;
		//std::string loadPrefabNextFrame{}; // store prefab path to load if it is at scene
		//bool prefabModified = false;

	
		// Helper struct to get resources folder/files 
		struct AssetEntry
		{
			std::string name;
			std::string fullPath;
		};

		bool raw_asset = false;

		// Editor viewport's data storage
		EditorViewport editorViewportData;

	public:
		// Default contructor 
		Editor(GLFWwindow* window) : m_Window(window), io(nullptr), m_Scene(nullptr) {};

		// Deconstuctor
		~Editor() {
			CleanupTemporaryPrefabs();
		};

		// Delected copy constructor
		//Editor(const Editor&) = delete;

		// Deleted copy assignment operator
		Editor& operator=(const Editor&) = delete;

		// Set scene for editor
		void SetScene(Engine::Scene* scene);
		
		// Initialise Imgui
		void OnInit();

		// still figure out
		void OnUpdate(Timestep ts, GLuint texhandle);

		void StartImguiFrame();

		// display top menu 
		void displayTopMenu();

		// display properties list
		void displayPropertiesPanel();

		// display hierarchy list
		void displayHierarchyPanel();

		// display assets browser list
		void displayAssetsBrowserPanel();

		// display performance profile
		void displayPerformanceProfilePanel(Timestep ts);

		// Render Viewport with given texture handle
		void renderViewport(GLuint texhandle);

		// Helper function for searching and return the files
		// std::vector <std::pair<std::string, std::string>> getFilesInFolder(const std::string& folderName);

		// ========================= Helper Function ======================================
		// helper function to open scene files from top menu
		void sceneOpenPanel();

		// open save as panel after select from top menu
		void saveAsScenePanel();

		// Helper function for searching and return the files/folder
		std::vector<AssetEntry> getAssetsInFolder(const std::string& folderPath);

		// Complete the ImGui frame
		void CompleteFrame();

		// to clean unsave prefab
		void CleanupTemporaryPrefabs();

		void SetTracy(const std::shared_ptr<TracyProfiler>& profiler) {
			m_Profiler = profiler; // still increases refcount, no extra copy on call
		}

		// Set Editor viewport's data for object picking
		void SetEditorViewport(EditorViewport& vp) const { vp = editorViewportData; }

		void RetrievePickedID(u32 id) { m_PickedID = id; /* Comment/delete output if needed*/std::cout << "Selected Entity: " << m_PickedID << std::endl; }
	};


} // end of gam300


#endif // LOF_IMGUI_MANAGER_h