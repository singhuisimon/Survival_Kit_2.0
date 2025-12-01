#pragma once
/**
 * @file ImguiManager.h
 * @brief Declaration of the Editor class for running the IMGUI level editor.
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
#include "../Asset/AssetManager.h"

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

		static constexpr float m_DoublePickedTime = 0.5f;
		float m_LastClickedTime = 0.f;
		u32 m_LastClickedID = 0xFFFFFFFFu;

		// ImGui Window functionality
		bool inspectorWindow = true;
		bool hierachyWindow = true;
		bool assetsWindow = true;
		bool performanceProfileWindow = true;
		bool animatorWindow = false;			// Animator/dopesheet window toggle
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

		// Helper to draw legend in Animator Curve display
		static void DrawCurveLegendRow(const char* label,
			const char* c0Label, ImU32 c0,
			const char* c1Label, ImU32 c1,
			const char* c2Label, ImU32 c2);

		DopesheetTrackType m_DopesheetSelectedTrack = DopesheetTrackType::None;
		int                m_DopesheetSelectedKey = -1;  // index into that track’s key array
		AnimatorViewMode    m_AnimatorViewMode = AnimatorViewMode::Dopesheet;
		AnimatorComponentTrack m_SelectedComponentTrack = AnimatorComponentTrack::Transform;

		// ImGui Top Menu Panel
		bool openScenePanel = false; // for top menu open file 
		bool saveAsPanel = false; // pop up save as panel
		bool openScript = false; // pop up open script option
		bool createScript = false; // pop up panel to create new script
		bool isNewScene = false;  // to check if is new scene

		// ImGui other helper variable
		std::string currScenePath{}; // to store current scene path 
		char saveAsDefaultSceneName[128] = {}; // default new scene path (in SaveAsScenePanel)
		std::string selectedFolder = ""; // for the selected folder in asset browser
		int selectedResourcesIndex = -1; // for the selected index in the assets browser
		ResourceType selectedType = ResourceType::UNKNOWN;//for the selected index in the assets browser

		// Prefab helper variables
		std::unordered_set<std::string> m_TemporaryPrefabPaths; // only save the prefab file if save the scene 
		std::string currPrefabPath{}; // track selected perfab path
		bool isPrefabEditor = false; // track if is at prefab scenes
		bool replacePrefabPending = false;
		std::string selectedPrefabPath{};
		std::string currFileName{}; // track file name
		Prefab* m_CurrentPrefab = nullptr;
		bool createEttFromPrfab = false;
		//bool ApplyOverrideButtonTriggle = false;

		ImGuizmo::OPERATION m_Operation = ImGuizmo::TRANSLATE; // for ImGuizmo 
		bool m_PreviousEditorCamToggle = false; // track if is in camera mode


		// helper for saveAsPanel()
	/*	bool m_ShouldRefreshAssets = false;
		std::string m_SceneToSelect = "";*/

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
		bool m_ShouldApplyOverrides = false;

		std::unordered_map<xresource::instance_guid, std::vector<Entity>> m_PrefabEntities;

		std::unordered_map<xresource::instance_guid, std::time_t> m_PrefabLastModifiedTimes;
		std::unordered_map<std::string, std::filesystem::file_time_type> m_PrefabFileTimes;

		std::unordered_set<xresource::instance_guid> m_UpdatedPrefabsThisSession;
		std::unordered_map<std::string, std::unordered_set<xresource::instance_guid>> m_SceneUpdateHistory;
		bool isPlaying = true;

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
			//CleanupTemporaryPrefabs();
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
		*   Displays the Animator window.
		*   Left side: entity/controller/clip, time and tracks.
		*   Right side: dopesheet or curves (switchable).
		**************************************************************************/
		void displayAnimatorPanel();

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
		* 	Display the prefab component when prefab component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayPrefabComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the camera component when camera component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayCameraComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the rigid body component when rigid body component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayRigidBodyComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the mesh renderer component when mesh renderer component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayMeshRendererComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the audio component when audio component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayAudioComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the revertZoneComp component when revertZoneComp component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayReverbZoneComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the listerner component when listerner component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayListenerComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the behaviour tree component when behaviour tree component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayBTComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the particle component when particle component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayParticleComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the light component when light component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayLightComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the script component when script component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayScriptComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the animator component when animator component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void displayAnimatorComp(ImVec2& buttonSize);

		/**************************************************************************
		* @brief
		* 	Display the add component when prefab component is added to the
		*	selected entity.
		* @param buttonSize
		*	The size of the button used for removing the component.
		**************************************************************************/
		void addComponents();

		/**************************************************************************
		* @brief
		* 	To revert back the selected entity that contain prefab component 
		to the default setting of the prefab file .
		**************************************************************************/
		void RevertSelectedEntityToPrefab();

		/**************************************************************************
		* @brief
		* 	To load all the prefab that is registry. 
		**************************************************************************/
		void LoadAllPrefabsIntoRegistry();

	
		//void UpdateAllInstancesOfPrefab(xresource::instance_guid prefabGUID, Entity modifiedEntity);

		// void CheckAndUpdatePrefabInstances();

		//void UpdateAllPrefabInstancesInScene(xresource::instance_guid prefabGUID);

		/**************************************************************************
		* @brief
		* 	Display the HDR setting for user to adjust
		**************************************************************************/
		void displayHDRSettingsPanel();

		/*void MarkPrefabAsUpdated(xresource::instance_guid prefabGUID)
		{
			m_UpdatedPrefabsThisSession.insert(prefabGUID);
		}*/

		/**************************************************************************
		* @brief
		* 	To track if the editor is playing or stop
		**************************************************************************/
		bool getIsPlaying() const { return isPlaying; }

		/**************************************************************************
		* @brief
		* 	Applies all overridden component values from a prefab instance
		*   in the scene back to its source prefab file. 
		**************************************************************************/
		void ApplyPrefabOverrides(Entity entity);

		/**************************************************************************
		* @brief
		* 	 Collects all descendant entities of the given parent entity using an
		*    iterative breadth-first traversal. All children, grandchildren, and
		*    deeper nested entities are added to the output vector.
		* @param parentEntity
		*    The root entity whose hierarchy will be scanned.
		* @param outEntities
		*	The list to append discovered child entities to.
		**************************************************************************/
		void CollectChildEntitiesIterative(Entity parentEntity, std::vector<Entity>& outEntities);

		/**************************************************************************
		* @brief
		* 	Update all instantiated entities in the scene that were create from 
		*	the specified entity prefab. Called after applying overrides so that 
		*	every instance reflects the updated prefab data. 
		* @param prefabGUID
		*	The GUID of the prefab whose instances should be updated.
		* @param prefab
		*	The updated prefab asset containing the new component data.
		**************************************************************************/
		void UpdateEntityPrefabInstances(xresource::instance_guid prefabGUID, std::shared_ptr<Prefab> prefab);

		/**************************************************************************
		* @brief
		* 	Update all instantiated entities in the scene that were create from
		*	the specified scene prefab. Called after applying overrides so that
		*	every instance reflects the updated prefab data.
		* @param prefabGUID
		*	The GUID of the prefab whose instances should be updated.
		* @param prefab
		*	The updated prefab asset containing the new component data.
		**************************************************************************/
		void UpdateScenePrefabInstances(xresource::instance_guid prefabGUID, std::shared_ptr<Prefab> prefab);

		/**************************************************************************
		* @brief
		* 	Recursively collects the entt::entity handles of all descendants of the 
		*	given parent entity.
		* @param parentEntity
		*	The root entity from which to start collecting child handles.
		* @param outHandles
		*	Vector that will be populated with all descendant entt entity handles.
		**************************************************************************/
		void CollectChildHandles(Entity parentEntity, std::vector<entt::entity>& outHandles);
		void setCurrScenePathAndFilename(std::string scenePath, std::string fileName) {
			currScenePath = scenePath;
			currFileName = fileName;
		}

		void immediatelyCompile() {
			AM.CompileAllAsset(0);
		}
	};


} // end of gam300


#endif // LOF_IMGUI_MANAGER_h