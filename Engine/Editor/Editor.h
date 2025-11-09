#pragma once
/**
 * @file ImguiManager.h
 * @brief Declaration of the IMGUI_Manager class for running the IMGUI level editor.
 * @author Liliana Hanawardani (50%), Saw Hui Shan (50%)
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
#include <ImGuizmo.h>


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
#include "../BehaviourTree/BehaviourTreeEditor.h"
#include "../Asset/DescriptorEditor.h"
#include "../Scripting/MonoScriptEngine.h"
#include "../Scripting/ScriptReloader.h"
#include "../Component/ScriptComponent.h"

// Temporary inclusion to access EditorViewport data struct
#include "Graphics/GraphicsLoader.h"
#include "Graphics/Renderer.h"

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
		Entity m_SelectedEntity{}; // track which entity is selected
		std::weak_ptr<TracyProfiler> m_Profiler;
		u32 m_PickedID = 0xFFFFFFFFu;
		
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
		std::string currPrefabPath{}; // track selected perfab path
		bool isPrefabEditor = false; // track if is at prefab scenes
		bool replacePrefabPending = false;
		std::string selectedPrefabPath{};
		std::string currFileName{}; // track file name
		Prefab* m_CurrentPrefab = nullptr;
		bool createEttFromPrfab = false;
	
		ImGuizmo::OPERATION m_Operation = ImGuizmo::TRANSLATE; // for ImGuizmo 
		bool m_PreviousEditorCamToggle = false; // track if is in camera mode
	
		// Helper struct to get resources folder/files 
		struct AssetEntry
		{
			std::string name;
			std::string fullPath;
		};

		bool raw_asset = false;

		// Editor viewport's data storage
		EditorViewport editorViewportData;
		Renderer* m_Renderer = nullptr;
		EditorViewport m_ImGuizmoViewportData;
		bool m_WasUsingGizmoLastFrame = false;
		bool m_WasOverGizmoLastFrame = false;

		DescriptorEditor descriptorEditor;
		bool showDescriptorEditorPanel = false;
		xresource::instance_guid currentEditingGuid;
		std::string editedAsset{};

	public:
		/**************************************************************************
		* @brief 
		* 	Constructs an Editor instance with a GLFW window reference.
		* @param window 
		* 	Pointer to the GLFW window.
		**************************************************************************/
		Editor(GLFWwindow* window) : m_Window(window), io(nullptr), m_Scene(nullptr) {};

		/**************************************************************************
		* @brief
		* 	Assigns a pointer to a Renderer object to this editor instance.
		* @param renderer
		* 	Pointer to Renderer.
		**************************************************************************/
		void SetRenderer(Renderer* renderer) { m_Renderer = renderer; }

		/**************************************************************************
		* @brief
		* 	Destructor for the Editor class. Cleans up any temporary prefabs or
		* 	resources created during editing.
		**************************************************************************/
		~Editor() {
			CleanupTemporaryPrefabs();
		};

		/**************************************************************************
		* @brief
		* 	Deleted copy assignment operator.
		**************************************************************************/
		Editor& operator=(const Editor&) = delete;

		/**************************************************************************
		* @brief
		* 	Set scene for editor.
		* @param scene
		* 	Pointer to Scene.
		**************************************************************************/
		void SetScene(Engine::Scene* scene);
		
		/**************************************************************************
		* @brief
		* 	Initialise Editor.
		**************************************************************************/
		void OnInit();

		/**************************************************************************
		* @brief
		* 	Update Editor for each frame.
		* @param ts 
		* 	Delta time of the current frame.
		* @param texhandle
		* 	OpenGL texture handle used during rendering
		**************************************************************************/
		void OnUpdate(Timestep ts, GLuint texhandle);

		/**************************************************************************
		* @brief
		* 	Start a new ImGui Editor frame. This function prepares the ImGui context
		* 	for rendering UI elements in the current frame.
		**************************************************************************/
		void StartImguiFrame();

		/**************************************************************************
		* @brief
		* 	Displays top menu bar of the Editor. The menu provides options such as 
		* 	saving and opening scenes, creating new scene or script, editing 
		* 	existing script and exiting the Editor.
		**************************************************************************/
		void displayTopMenu();

		/**************************************************************************
		* @brief
		* 	Displays the property panel for the selected game object. The panel 
		* 	allows users to view and edit component properties, adding and removing
		* 	existing components. 
		**************************************************************************/
		void displayPropertiesPanel();

		/**************************************************************************
		* @brief 
		* 	Displays the hierarchy panel for all created game objects.
		* 	The panel allows users to view all existing game objects, create new game 
		* 	objects, replace prefabs, and create a new prefab based on the selected entity.
		**************************************************************************/
		void displayHierarchyPanel();

		/**************************************************************************
		* @brief 
		* 	Displays the assets browser panel.
		* 	The panel allows users to view all existing files, edit files, switch 
		* 	between scenes and prefabs, edit asset descriptor through asset pipeline. 
		**************************************************************************/
		void displayAssetsBrowserPanel();

		/**************************************************************************
		* @brief 
		* 	Displays the descriptor panel.
		* 	The panel allows users to view and edit the properties of a selected
		* 	asset's descriptor, including settings, tags, and validation options.
		**************************************************************************/
		void displayDescriptorEditorPanel();

		/**************************************************************************
		* @brief 
		* 	Displays the performance profile panel.
		* 	The panel allows users to view the performance details and launch tracy.
		* @param ts
		* 	Delta time of the current frame.
		**************************************************************************/
		void displayPerformanceProfilePanel(Timestep ts);

		/**************************************************************************
		* @brief
		* 	Render Viewport with given texture handle.
		* @param texhandle
		* 	OpenGL texture handle used during rendering.
		**************************************************************************/
		void renderViewport(GLuint texhandle);

		// ========================= Helper Function ======================================

		/**************************************************************************
		* @brief
		* 	Helper function to open scene files from top menu panel
		**************************************************************************/
		void sceneOpenPanel();

		/**************************************************************************
		* @brief
		* 	Opens the "Save As" panel after selecting the option from the top menu.
		**************************************************************************/
		void saveAsScenePanel();

		
		/**************************************************************************
		* @brief
		* 	Helper function for searching a folder and returning its contents.
		* @param folderPath
		* 	The path of the folder to search.
		* @return 
		*	Vector of AssetEntry representing the files/folders found in the folder.
		**************************************************************************/
		std::vector<AssetEntry> getAssetsInFolder(const std::string& folderPath);

		/**************************************************************************
		* @brief
		* 	Completes the current ImGui frame.
		**************************************************************************/
		void CompleteFrame();

		/**************************************************************************
		* @brief
		* 	Cleans up any unsaved temporary prefabs when closing the program.
		**************************************************************************/
		void CleanupTemporaryPrefabs();

		/**************************************************************************
		* @brief
		* 	Sets the Tracy Profiler.
		* @param profiler
		*	Shared pointer to a Tracy profiler instance.
		**************************************************************************/
		void SetTracy(const std::shared_ptr<TracyProfiler>& profiler) {
			m_Profiler = profiler; // still increases refcount, no extra copy on call
		}
		
		/**************************************************************************
		* @brief
		* 	Copies the editor viewport data to the provided EditorViewport reference.
		* @param vp
		*	Reference to an EditorViewport instance to receive the data.
		**************************************************************************/
		void SetEditorViewport(EditorViewport& vp) const { vp = editorViewportData; }

		/**************************************************************************
		* @brief
		* 	Retrieve the picked object in viewport from Renderer.
		* @param id
		*	Return the id of the game objected that is picked.
		**************************************************************************/
		void RetrievePickedID(u32 id) { m_PickedID = id; }

		/**************************************************************************
		* @brief
		* 	Recursively draws an entity and all its children in an ImGui tree view.
		* @param entity
		*	The entity to draw.
		* @param registry
		*	The EnTT registry containing all entities and components.
		**************************************************************************/
		void DrawEntityRecursive(Entity entity, entt::registry& registry);

		/**************************************************************************
		* @brief
		* 	Activate ImGuizmo to manipulate the selected entity's transformation,
		*	rotate and scale in editor viewport. 
		* @param entity
		*	Reference to the entity.
		**************************************************************************/
		void ManipulateEntityTransform(Entity& entity);

		/**************************************************************************
		* @brief
		* 	Display the create script panel when top menu "New Script" being selected. 
		**************************************************************************/
		void CreateScriptPanel();

		/**************************************************************************
		* @brief
		* 	Display the create script panel when top menu "Open Script" being selected.
		**************************************************************************/
		void OpenScriptPanel();

		/**************************************************************************
		* @brief
		* 	Opens the specified script file in the system editor.
		* @param scriptName
		*	The name of the script to open.
		* @return
		*	True if the script was successfully opened, false if the file does not
		*   exist or an error occurred while opening it.
		**************************************************************************/
		bool OpenScriptInEditor(const std::string& scriptName);

		/**************************************************************************
		* @brief
		* 	Display the camera component when camera component is added to the 
		*	selected entity. 
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayCameraComp(ImVec2& buttonSize);
	};


} // end of gam300


#endif // LOF_IMGUI_MANAGER_h