#include "Game.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Utility/Logger.h"
#include "Utility/AssetPath.h"
#include "ECS/Components.h"
#ifndef DISABLE_EDITOR
#include "Editor/Editor.h"
#endif
#include "Serialization/ComponentRegistry.h"
#include "Audio/AudioSystem.h"
#include "Audio/AudioEffectSystem.h"
#include "Asset/AssetManager.h"
#include "Asset/ResourceManager.h"
#include "Asset/ResourceHelpers.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

#include "Component/TagComponent.h"
#include "Component/TransformComponent.h"
#include "Component/CameraComponent.h"
#include "Component/MeshRendererComponent.h"
#include "Component/RigidbodyComponent.h"
#include "Component/ScriptComponent.h"  // ADD THIS
#include "Component/LightComponent.h"  


// Adding systems
#include "Graphics/RenderSystem.h"
#include "Graphics/CameraSystem.h"
#include "Transform/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "Scripting/ScriptSystem.h"        // ADD THIS
#include "Scripting/MonoScriptEngine.h"    // ADD THIS
#include "BehaviourTree/BehaviourTreeSystem.h"
#include "ParticleSystem/ParticleSystem.h"
#include "Animation/AnimationSystem.h"
#include "Physics/CollisionSystem2D.h"
#include "ParticleSystem/TrailSystem.h"

#include "Event/EventSystem.h"

// KENNY TESTING: FOR MAINCAMERA "SCRIPT"
#include <glm/common.hpp>               // glm::clamp
#include <glm/gtc/type_ptr.hpp>         // glm::value_ptr
// Math Utility
#include "Utility/MathUtils.h"

#include "Serialization/PrefabSerializer.h"
#include "Serialization/PrefabInstantiator.h"
#include "Prefab/PrefabRegistry.h"

#include "Utility/AssetPath.h"

#ifndef DEBUG
//this is for release
Game::Game()
	: Application("Guardian of The MotherBoard", 1280, 720)
	, m_ActiveScene(nullptr)
#ifndef DISABLE_EDITOR
	, m_Editor(nullptr)
#endif
	, m_ColorShift(0.0f)
{
	LOG_INFO("Game constructor body executing");
}

#endif

#ifndef NDEBUG
//this is for debug
Game::Game()
	: Application("Property-Based ECS Engine", 1280, 720)
	, m_ActiveScene(nullptr)
#ifndef DISABLE_EDITOR
	, m_Editor(nullptr)
#endif

	, m_ColorShift(0.0f)
{
	LOG_INFO("Game constructor body executing");
}
#endif

void Game::OnInit()
{
	LOG_INFO("=== Game::OnInit() STARTED ===");

	//==========INITIALIZING ASSET ==============

	LOG_INFO("Initializing Asset...");

	auto config = Engine::AM.createDefaultConfig();

	//debug 
	LOG_INFO("Asset Manager Configuration:");
	LOG_INFO("  Source Roots:");
	for (const auto& root : config.sourceRoots)
	{
		LOG_INFO("    - ", root);
	}
	LOG_INFO("  Descriptor Root: ", config.descriptorRoot);
	LOG_INFO("  Database File: ", config.databaseFile);
	LOG_INFO("  Compiled Root: ", config.compiledPath);

	Engine::AM.setConfig(config);

	if (Engine::AM.startUp() != 0)
	{
		LOG_ERROR("Failed to initialize Asset Manager!");
		return;
	}
	else
	{
#ifndef DISABLE_EDITOR
		LOG_INFO("Performing initial asset scan...");
		Engine::AM.scanAndProcess();

		LOG_INFO("Initial asset scan complete - found ",
			Engine::AM.db().Count(), " assets");
#endif // !DISABLE_EDITOR
	}

	Engine::RM.startUp();

	// Step 1: Register components for serialization
	LOG_INFO("Step 1: Registering components...");
	try
	{
		Engine::ComponentRegistry::RegisterAllComponents();
		LOG_INFO("  -> Components registered successfully");
	}
	catch (const std::exception& e)
	{
		LOG_CRITICAL("  -> FAILED to register components: ", e.what());
		return;
	}

	// Step 2: Register nodes for serialization
	LOG_INFO("Step 2: Registering components...");
	try
	{
		// Register all built-in behavior tree node types
		Engine::BTNodeRegistry::RegisterBuiltInNodes();
		LOG_INFO("  -> Components registered successfully");
	}
	catch (const std::exception& e)
	{
		LOG_CRITICAL("  -> FAILED to register components: ", e.what());
		return;
	}

	// Step 3: Create Audio Manager
	LOG_INFO("Step 3: Initializing Audio Manager...");
	try
	{
		m_AudioManager = std::make_unique<Engine::AudioManager>();
		if (!m_AudioManager->Init())
		{
			LOG_CRITICAL("  -> Audio Manager initialization failed!");
			return;
		}

		Engine::SetScriptingAudioManager(m_AudioManager.get());

		LOG_INFO("  -> Audio Manager initialized successfully");
	}
	catch (const std::exception& e)
	{
		LOG_CRITICAL("  -> Exception while initializing Audio Manager: ", e.what());
		return;
	}
#ifndef DISABLE_EDITOR
	// Step 4:  Initialize Editor BEFORE creating scene
	LOG_INFO("Step 4: Initializing Editor...");

	try
	{
		//m_Scenes.push_back(std::make_unique<Engine::Scene>("Main Scene"));
		// Editor get scene
		if (!m_Editor)
		{
			m_Editor = std::make_unique<Engine::Editor>(GetWindow());
			//m_Editor->SetScene(m_Scene.get());
			m_Editor->SetRenderer(m_Renderer.get());
			m_Editor->SetGame(this);
			m_Editor->OnInit();
			LOG_INFO("Editor initialized successfully.");

		}

		//// Create initial scenes
		//try
		//{
		//	Engine::Scene* mainScene = CreateScene("Main Scene");
		//	if (!mainScene)
		//	{
		//		LOG_CRITICAL("Failed to create initial scene!");
		//		return;
		//	}
		//	if (mainScene) {
		//		m_Editor->SetActiveScene(mainScene);
		//	}
		//	LOG_INFO("  -> Scene created at address: ", (void*)mainScene);

		//}
		//catch (const std::exception& e)
		//{
		//	LOG_CRITICAL("Failed to create scenes: ", e.what());
		//	return;
		//}

		//LOG_INFO("  -> Scene created at address: ", (void *)m_Scene.get());
	}
	catch (const std::exception& e)
	{
		LOG_CRITICAL("  -> Exception while initializing Audio Manager: ", e.what());
		return;
	}
#endif

	
	// Step 5: Load or Create Initial Scene
	LOG_INFO("Step 5: Creating initial scene...");
	try {
		Engine::Scene* mainScene = CreateScene("Main Scene");

		if (!mainScene) {
			LOG_CRITICAL("Failed to create initial scene!");
			return;
		}

#ifndef DISABLE_EDITOR
		// Notify editor of the scene
		if (m_Editor) {
			m_Editor->SetActiveScene(mainScene);
		}
#endif
		LOG_INFO("  -> Scene created: ", mainScene->GetName());
		SetScriptingCurrentScene(mainScene);
	}
	catch (const std::exception& e) {
		LOG_CRITICAL("Failed to create scene: ", e.what());
		return;
	}

	// Step 6: Load scene from file
	LOG_INFO("Step 6: Loading scene content from file...");
	m_SceneNeedsLoading = true;

	


	// Step 8: Initialize Tracy Profiler
#ifndef DISABLE_EDITOR
	LOG_INFO("Step 8: Initializing Tracy Profiler...");
	try
	{
		m_TracyProfiler = std::make_shared<Engine::TracyProfiler>();

		// Get the directory where the executable is running from
		std::filesystem::path exeDir = Engine::getAssetsPath();

		// Move up one level (from 'resources/' to project root)
		std::filesystem::path projectRoot = exeDir.parent_path().parent_path();

		std::filesystem::path tracyExe = projectRoot / "tracy-profiler.exe";
		std::string result = tracyExe.generic_string();
		LOG_INFO("tracy-profiler path: %s", result.c_str());

		m_TracyProfiler->SetTracyPath(result);

		m_Editor->SetTracy(m_TracyProfiler);
	}
	catch (const std::exception& e)
	{
		LOG_ERROR("  -> Exception while initializing Tracy Profiler: ", e.what());
	}
#endif
}



void Game::AddAllSystemsToScene(Engine::Scene* scene)
{
	if (!scene) return;
	LOG_INFO("Adding systems to scene: ", scene->GetName());
	scene->AddSystem<Engine::AudioSystem>(m_AudioManager.get());
	scene->AddSystem<Engine::AudioEffectSystem>(m_AudioManager.get());
	scene->AddSystem<Engine::PhysicsSystem>();
	scene->AddSystem<Engine::TransformSystem>();
	scene->AddSystem<Engine::CameraSystem>();
	scene->AddSystem<Engine::ScriptSystem>();
	scene->AddSystem<Engine::RenderSystem>(*m_Renderer);
	scene->AddSystem<Engine::BehaviourTreeSystem>();
	scene->AddSystem<Engine::ParticleSystem>();
	scene->AddSystem<Engine::AnimationSystem>();
	scene->AddSystem<Engine::CollisionSystem2D>(m_Renderer->getMeshData2DStorage(), m_Renderer->GetUIViewport(), m_Renderer->GetUIProjection());
	scene->AddSystem<Engine::TrailSystem>();
}

void Game::CreateDefaultScene()
{
	if (!m_ActiveScene)
	{
		throw std::runtime_error("Scene is null in CreateDefaultScene");
	}

	namespace fs = std::filesystem;
	Engine::m_AnimationClipStorage.clear();
	Engine::m_AnimatorControllerStorage.clear();

	// Update settings 
	m_Renderer->getBloomToggle() = m_ActiveScene->GetSceneSetting().s_BloomToggle;
	m_Renderer->getBloomStrength() = m_ActiveScene->GetSceneSetting().s_BloomStrength;
	m_Renderer->getBloomFilterRadius() = m_ActiveScene->GetSceneSetting().s_BloomFilterRadius;
	m_Renderer->getExposure() = m_ActiveScene->GetSceneSetting().s_Exposure;

	// ---------------------------------------------------------------------
	// Load animation clips
	// ---------------------------------------------------------------------
	const std::string clipsDir = Engine::getAssetFilePath("Sources/AnimationClips");

	if (fs::exists(clipsDir))
	{
		for (const auto& entry : fs::directory_iterator(clipsDir))
		{
			if (!entry.is_regular_file())
				continue;

			if (entry.path().extension() == ".animclip")
			{
				Engine::AnimationClip clip;
				if (Engine::DeserializeAnimationClip(entry.path().string(), clip))
				{
					//Engine::u32 handle = static_cast<Engine::u32>(Engine::m_AnimationClipStorage.size());
					//clip.id = handle;
					Engine::m_AnimationClipStorage[clip.id] = clip;
				}
			}
		}
	}

	// ---------------------------------------------------------------------
	// Load animator controllers
	// ---------------------------------------------------------------------
	const std::string ctrlDir = Engine::getAssetFilePath("Sources/AnimationControllers");

	if (fs::exists(ctrlDir))
	{
		for (const auto& entry : fs::directory_iterator(ctrlDir))
		{
			if (!entry.is_regular_file())
				continue;

			if (entry.path().extension() == ".animcontroller")
			{
				Engine::AnimatorController ctrl;
				if (Engine::DeserializeAnimationController(entry.path().string(), ctrl))
				{
					//Engine::u32 handle = static_cast<Engine::u32>(Engine::m_AnimatorControllerStorage.size());
					//ctrl.id = handle;
					Engine::m_AnimatorControllerStorage[ctrl.id] = ctrl;
				}
			}
		}
	}

	LOG_TRACE("  Creating Player entity...");
	auto player = m_ActiveScene->CreateEntity("Player");
	player.AddComponent<Engine::TagComponent>("Player");

	auto& transform = player.AddComponent<Engine::TransformComponent>();
	transform.Position = glm::vec3(1, 2, 0);  // Start above ground
	//transform.Scale    = glm::vec3(1.f, 1.f, 1.f);
	transform.Scale = glm::vec3(0.0005f, 0.0005f, 0.0005f);

	auto& mesh = player.AddComponent<Engine::MeshRendererComponent>();
	mesh.Material = 1;
	//mesh.CastType = Engine::ShadowCastType::On;

	std::string meshName = "E004_botnet_v001.fbx";
	xresource::instance_guid inst_guid = Engine::AM.getAssetIdByFilename(meshName);
	mesh.MeshGuid = inst_guid;
	//mesh.MaterialGuid = Engine::AM.getAssetIdByFilename("test.mat");
	//if (meshName == "E004_botnet_v001.fbx") { transform.SetRotation(glm::vec3(0, 90.0f, 0)); }

	std::cout << inst_guid.m_Value << "\n"; //this is the main value - amanda

	std::string meshName_ = "E005_loveletter_v001.fbx";
	xresource::instance_guid inst_guid_ = Engine::AM.getAssetIdByFilename(meshName_);
	//mesh.MeshGuid = inst_guid_;
	//if (meshName == "E004_botnet_v001.fbx") { transform.SetRotation(glm::vec3(0, 90.0f, 0)); }

	std::cout << inst_guid_.m_Value << "\n";
	//mesh.MeshResource = mesh_rsc;
	LOG_INFO("Mesh GUID for ", meshName_, ": ", inst_guid_.m_Value);

	auto& script = player.AddComponent<Engine::ScriptComponent>();
	script.ScriptClassName = "Game.TestScript";
	LOG_TRACE("  -> SCRIPT IS CREATED SCRIPT IS CREATED");
	xresource::instance_guid tex_inst_guid = Engine::AM.getAssetIdByFilename("rabbit_kenny.png");

	auto& playerAudio = player.AddComponent<Engine::AudioComponent>();
	playerAudio.AudioFilePath = "laserSmall_001.ogg";
	playerAudio.Type = Engine::AudioType::SFX;
	playerAudio.State = Engine::PlayState::STOP;
	playerAudio.Volume = 0.8f;
	playerAudio.Pitch = 1.0f;
	playerAudio.Loop = false;
	playerAudio.Mute = false;
	playerAudio.ReverbProperties = 1.0f;
	playerAudio.Is3D = true;
	playerAudio.MinDistance = 1.0f;
	playerAudio.MaxDistance = 50.0f;

	LOG_TRACE("  -> Player created (will fall and demonstrate MovementSystem)");

	LOG_TRACE("  Creating Camera entity...");
	auto camera = m_ActiveScene->CreateEntity("MainCamera");
	camera.AddComponent<Engine::TagComponent>("MainCamera");

	auto& camTransform = camera.AddComponent<Engine::TransformComponent>();
	camTransform.Position = glm::vec3(0, 5, 5);
	camTransform.Rotation = glm::vec3(-15, 0, 0);
	camTransform.Scale = glm::vec3(1, 1, 1);

	auto& camComponent = camera.AddComponent<Engine::CameraComponent>();
	camComponent.Enabled = true;
	camComponent.autoAspect = true;
	camComponent.SetProjection(false);
	camComponent.Depth = 0; // 0 is the main camera
	camComponent.SetAspect(static_cast<float>(GetWidth()) / static_cast<float>(GetHeight()));
	camComponent.SetFOV(45.0f);
	camComponent.SetNearPlane(0.5f);
	camComponent.SetFarPlane(100.0f);
	camComponent.SetTarget(glm::vec3(0.0f));
	LOG_TRACE("  -> Camera created");

	LOG_TRACE("  Creating Camera entity...");
	auto cam2 = m_ActiveScene->CreateEntity("SecondCamera");
	cam2.AddComponent<Engine::TagComponent>("SecondCamera");

	auto& cam2Transform = cam2.AddComponent<Engine::TransformComponent>();
	cam2Transform.Position = glm::vec3(0, 5, 5);
	cam2Transform.Rotation = glm::vec3(-15, 0, 0);
	cam2Transform.Scale = glm::vec3(1, 1, 1);

	auto& cam2Component = cam2.AddComponent<Engine::CameraComponent>();
	cam2Component.Enabled = true;
	cam2Component.autoAspect = true;
	cam2Component.SetProjection(true);
	cam2Component.Depth = 1; // 0 is the main camera
	cam2Component.SetAspect(static_cast<float>(GetWidth()) / static_cast<float>(GetHeight()));
	cam2Component.SetFOV(45.0f);
	cam2Component.SetNearPlane(0.5f);
	cam2Component.SetFarPlane(100.0f);
	cam2Component.SetTarget(glm::vec3(0.0f));
	LOG_TRACE("  -> SecondCamera created");

	auto& listener = camera.AddComponent<Engine::ListenerComponent>();
	listener.Active = true;
	LOG_TRACE("  -> Camera created with listenerComponent");

	LOG_TRACE("  Creating Ground entity...");
	auto ground = m_ActiveScene->CreateEntity("Ground");
	ground.AddComponent<Engine::TagComponent>("Ground");

	auto& groundTransform = ground.AddComponent<Engine::TransformComponent>();
	groundTransform.Position = glm::vec3(0, -1, 0);
	groundTransform.Scale = glm::vec3(20, 0.1f, 20);

	auto& groundRb = ground.AddComponent<Engine::RigidbodyComponent>();
	groundRb.Mass = 0.0f;
	groundRb.IsKinematic = true;
	groundRb.UseGravity = false;
	groundRb.Velocity = glm::vec3(0, 0, 0);

	auto& g_mesh = ground.AddComponent<Engine::MeshRendererComponent>();
	g_mesh.MaterialGuid = Engine::AM.getAssetIdByFilename("test.mat");
	//g_mesh.ShadowReceive = true;  
	LOG_TRACE("  -> Ground created");

	LOG_TRACE("  Creating Sphere entity...");
	auto sphere = m_ActiveScene->CreateEntity("Sphere");
	sphere.AddComponent<Engine::TagComponent>("Sphere");

	auto& sphereTransform = sphere.AddComponent<Engine::TransformComponent>();
	sphereTransform.Position = glm::vec3(-5.0f, 1.0f, 1.0f);
	//sphereTransform.Scale = glm::vec3(1.0f);
	sphereTransform.Scale = glm::vec3(1.0f);

	auto& sphereRb = sphere.AddComponent<Engine::RigidbodyComponent>();
	sphereRb.Mass = 0.0f;
	sphereRb.IsKinematic = true;
	sphereRb.UseGravity = false;
	sphereRb.Velocity = glm::vec3(0, 0, 0);

	auto& spheremesh = sphere.AddComponent<Engine::MeshRendererComponent>();
	spheremesh.MeshType = 0; // Sphere
	//spheremesh.CastType = Engine::ShadowCastType::On;

	auto& sphereAnimation = sphere.AddComponent<Engine::AnimatorComponent>();
	sphereAnimation.playing = true;
	sphereAnimation.respectClipLoop = true;
	sphereAnimation.controller = 0;
	sphereAnimation.currentClipIndex = 0;
	sphereAnimation.currentTime = 0.0f;
	sphereAnimation.playbackSpeed = 1.0f;

	LOG_TRACE("  -> Sphere created");

	LOG_TRACE("  Creating TimerBar entity...");
	auto TimerBar = m_ActiveScene->CreateEntity("TimerBar");
	TimerBar.AddComponent<Engine::TagComponent>("TimerBar");

	auto& TimerBarTransform = TimerBar.AddComponent<Engine::TransformComponent>();
	TimerBarTransform.Position = glm::vec3(1.0f, 4.0f, 0.0f);
	//TimerBarTransform.Scale = glm::vec3(1.0f);
	TimerBarTransform.Scale = glm::vec3(4.0f, 0.05f, 0.05f);

	auto& TimerBarmesh = TimerBar.AddComponent<Engine::MeshRendererComponent>();
	TimerBarmesh.MeshType = 0; // Square

	auto& TimerBarAnimation = TimerBar.AddComponent<Engine::AnimatorComponent>();
	TimerBarAnimation.playing = true;
	TimerBarAnimation.respectClipLoop = true;
	TimerBarAnimation.controller = 0;
	TimerBarAnimation.currentClipIndex = 0;
	TimerBarAnimation.currentTime = 0.0f;
	TimerBarAnimation.playbackSpeed = 1.0f;

	LOG_TRACE("  -> TimerBar created");

	LOG_TRACE("  Creating ReverbZone entity...");
	auto reverbZone = m_ActiveScene->CreateEntity("CaveReverb");
	reverbZone.AddComponent<Engine::TagComponent>("CaveReverb");

	auto& rzTransform = reverbZone.AddComponent<Engine::TransformComponent>();
	rzTransform.Position = glm::vec3(0, 0, 0); // center of world
	rzTransform.Scale = glm::vec3(1, 1, 1);

	auto& reverb = reverbZone.AddComponent<Engine::ReverbZoneComponent>();
	reverb.Preset = Engine::ReverbPreset::Cave;
	reverb.MinDistance = 1.0f;
	reverb.MaxDistance = 50.0f;
	reverb.DecayTime = 2500.0f; // long decay
	reverb.HfDecayRatio = 80.0f;
	reverb.WetLevel = 0.0f;
	reverb.IsDirty = true;

	LOG_TRACE("  -> Reverb zone created");

	LOG_TRACE("  Creating AI entity...");
	auto ai = m_ActiveScene->CreateEntity("AI");

	auto& aiTransform = reverbZone.GetComponent<Engine::TransformComponent>();
	aiTransform.Position = glm::vec3(0, 0, 0); // center of world
	aiTransform.Scale = glm::vec3(1, 1, 1);

	auto& bt = ai.AddComponent<Engine::BehaviourTreeComponent>();
	bt.Active = true;
	bt.ResetOnComplete = false;
	bt.TreeAssetPath = "CreateEnemeyCube.json";

	LOG_TRACE("  -> ai created");

	LOG_TRACE("  Creating Sunlight entity...");
	auto sunlight = m_ActiveScene->CreateEntity("Sunlight");
	sunlight.AddComponent<Engine::TagComponent>("Sunlight");

	auto& sunlightTransform = sunlight.AddComponent<Engine::TransformComponent>();
	sunlightTransform.Position = glm::vec3(0, 10, 0);
	sunlightTransform.Rotation = glm::quatLookAt(
		glm::normalize(glm::vec3(-0.3f, -1.0f, -0.5f)), // forward vector (light direction)
		glm::vec3(0.0f, 1.0f, 0.0f));
	sunlightTransform.Scale = glm::vec3(1.0f);

	auto& sunlightLight = sunlight.AddComponent<Engine::LightComponent>();
	sunlightLight.Type = Engine::LightType::Directional;
	sunlightLight.Enabled = true;
	sunlightLight.Color = glm::vec3(1.0f, 0.956f, 0.839f); // warm white (~5500K)
	sunlightLight.Intensity = 0.5f;     // good balance for full-scene lighting
	sunlightLight.Range = 0.0f;         // ignored for directional
	sunlightLight.SpotAngleDeg = 0.0f;  // ignored for directional
	sunlightLight.IndirectMultiplier = 1.0f;
	sunlightLight.Mode = Engine::LightMode::Realtime;


	LOG_TRACE("  -> Sunlight created");

	LOG_TRACE("  Creating Lamp entity...");
	auto lamp = m_ActiveScene->CreateEntity("Lamp");
	lamp.AddComponent<Engine::TagComponent>("Lamp");

	auto& lampTransform = lamp.AddComponent<Engine::TransformComponent>();
	lampTransform.Position = glm::vec3(5, 5, 0);

	auto& lampLight = lamp.AddComponent<Engine::LightComponent>();
	lampLight.Type = Engine::LightType::Point;
	lampLight.Enabled = true;
	lampLight.Color = glm::vec3(0.0f, 0.67f, 0.0f);  // green
	lampLight.Intensity = 1.0f;     // 1.0 = neutral brightness
	lampLight.Range = 15.0f;         // meters / world units
	lampLight.SpotAngleDeg = 0.0f;  // ignored for point light
	lampLight.IndirectMultiplier = 1.0f;
	lampLight.Mode = Engine::LightMode::Realtime;
	LOG_TRACE("  -> Lamp created");

	LOG_TRACE("  Creating Spotlight entity...");
	auto spotlight = m_ActiveScene->CreateEntity("Spotlight");
	spotlight.AddComponent<Engine::TagComponent>("Spotlight");

	auto& spotlightTransform = spotlight.AddComponent<Engine::TransformComponent>();
	spotlightTransform.Position = glm::vec3(-5, 5, 0);
	spotlightTransform.Rotation = glm::quatLookAt(
		glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::vec3(0.0f, 1.0f, 0.0f));

	auto& spotlightLight = spotlight.AddComponent<Engine::LightComponent>();
	spotlightLight.Type = Engine::LightType::Spot;
	spotlightLight.Enabled = true;
	spotlightLight.Color = glm::vec3(1.0f); // Bright white
	spotlightLight.Intensity = 2.0f;
	spotlightLight.Range = 15.0f;           // meters / world units
	spotlightLight.SpotAngleDeg = 40.0f;
	spotlightLight.IndirectMultiplier = 1.0f;
	spotlightLight.Mode = Engine::LightMode::Realtime;
	LOG_TRACE("  -> Spotlight created");
}

void Game::OnUpdate(Engine::Timestep ts)
{
	if (m_SceneNeedsLoading)
	{
		LOG_INFO("=== FIRST FRAME: Loading scene data ===");

		bool loadedFromFile = false;
		try {
			if (m_ActiveScene) {
#ifndef DISABLE_EDITOR
				loadedFromFile = m_ActiveScene->LoadFromFile("Resources/Sources/Scenes/EpilepsyWarning.json");
#else 
				loadedFromFile = m_ActiveScene->LoadFromFile(Engine::getAssetFilePath("Sources/Scenes/EpilepsyWarning.json"));
#endif
			}

			if (loadedFromFile) {
				LOG_INFO("  -> Scene content loaded from file successfully");
#ifndef DISABLE_EDITOR
				m_CurrentScenePath = "Resources/Sources/Scenes/EpilepsyWarning.json";
#else
				m_CurrentScenePath = "Sources/Scenes/EpilepsyWarning.json";
#endif

				// Update renderer settings from loaded scene
				m_Renderer->getBloomToggle() = m_ActiveScene->GetSceneSetting().s_BloomToggle;
				m_Renderer->getBloomStrength() = m_ActiveScene->GetSceneSetting().s_BloomStrength;
				m_Renderer->getBloomFilterRadius() = m_ActiveScene->GetSceneSetting().s_BloomFilterRadius;
				m_Renderer->getExposure() = m_ActiveScene->GetSceneSetting().s_Exposure;
				m_Renderer->getGlobalBias() = m_ActiveScene->GetSceneSetting().s_GlobalBias;

				LOG_INFO("  -> Renderer settings updated from scene");
			}
			else {
				LOG_WARNING("  -> Could not load scene file, using empty scene");
			}
		}
		catch (const std::exception& e) {
			LOG_ERROR("  -> Exception while loading scene file: ", e.what());
			loadedFromFile = false;
		}

		m_SceneNeedsLoading = false;
		LOG_INFO("=== Scene loading complete, starting normal updates ===");
	}
	// Check scene validity
	if (!m_ActiveScene)
	{
		static bool errorLogged = false;
		if (!errorLogged)
		{
			LOG_ERROR("ERROR: Scene is null in OnUpdate!");
			LOG_ERROR("This means OnInit() failed to create the scene properly");
			LOG_ERROR("Check the logs above for initialization errors");
			errorLogged = true;
		}

		// Still render something so window doesn't freeze
		glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		return;
	}

	// Get input reference
	auto& input = GetInput();

	// Get editor camera toggle and editor mode for renderer reference
	auto& editorCamToggle = m_Renderer->getEditorCamToggle();
	auto& editorModeToggle = m_Renderer->getEditorModeToggle();
#ifndef DISABLE_EDITOR
	// Add this somewhere in your input handling:
	if (input.IsKeyJustPressed(GLFW_KEY_F3))
	{
		m_EditorEnable = !m_EditorEnable;
		editorModeToggle = m_EditorEnable;
		editorCamToggle = false;
		LOG_INFO("Editor toggled: ", m_EditorEnable);
	}

	// Editor camera toggle
	if (input.IsKeyJustPressed(GLFW_KEY_TAB) && m_EditorEnable)
	{
		editorCamToggle = !editorCamToggle;
		LOG_INFO("Editor camera toggled: ", editorCamToggle);
	}
#endif
	
	// When Editor is turned OFF OR Editor is ON but gameplay is PLAYING: Update Everything
#ifdef DISABLE_EDITOR
	if (true)
#else
	if (!m_EditorEnable || (m_EditorEnable && m_Editor->GetEditorIsPlaying()))
#endif
	{

		if (m_EditorJustPaused)
		{
			m_AudioManager->PauseAll(false);
			m_EditorJustPaused = false;
		}

		if (!m_IsFirstPausedFrame)
		{
			m_IsFirstPausedFrame = true;
		}

		// Update scene (this will call all systems in priority order)
		m_ActiveScene->OnUpdate(ts);  // Convert Timestep to float

		// Update audio manager if exists
		m_AudioManager->OnUpdate(ts);
	}
	else
	{ // When Editor is ON but gameplay is NOT PLAYING: Update Transform, Camera and Render systems

		m_EditorJustPaused = true;

		auto& sceneSystems = m_ActiveScene->GetSystemRegistry();

		Engine::TransformSystem* transformSystem = sceneSystems.GetSystem<Engine::TransformSystem>();
		transformSystem->OnUpdate(m_ActiveScene, ts);

		if (m_IsFirstPausedFrame)
		{
			Engine::BehaviourTreeSystem* BTSystem = sceneSystems.GetSystem<Engine::BehaviourTreeSystem>();
			BTSystem->LoadBehaviourTrees(m_ActiveScene);
			m_IsFirstPausedFrame = false;
		}

		Engine::CameraSystem* camSystem = sceneSystems.GetSystem<Engine::CameraSystem>();
		camSystem->OnUpdate(m_ActiveScene, ts);

		Engine::RenderSystem* renderSystem = sceneSystems.GetSystem<Engine::RenderSystem>();
		renderSystem->OnUpdate(m_ActiveScene, ts);

		m_AudioManager->PauseAll(true);

	}
	
	// Editor camera controls
#ifndef DISABLE_EDITOR
	if (input.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && editorCamToggle)
	{

		auto& editorCam = m_Renderer->getEditorCamera();

		// Check for left or right mouse click
		uint32_t mouse = 2;
		if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
		{
			mouse = GLFW_MOUSE_BUTTON_LEFT;
		}
		else if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT))
		{
			mouse = GLFW_MOUSE_BUTTON_RIGHT;
		}

		// Cursor orbiting
		editorCam.cameraOnCursor(input.GetMouseDelta().x, input.GetMouseDelta().y, mouse);

		// Zooming in-and-out scrolling
		double scrollY_offset = input.GetScrollDelta().y;
		if (scrollY_offset != 0)
		{
			editorCam.cameraOnScroll(scrollY_offset);
		}

		// Check moving input
		if (input.IsKeyPressed(GLFW_KEY_W))
		{
			editorCam.moveCamForward();
		}
		if (input.IsKeyPressed(GLFW_KEY_A))
		{
			editorCam.moveCamLeft();
		}
		if (input.IsKeyPressed(GLFW_KEY_S))
		{
			editorCam.moveCamBack();
		}
		if (input.IsKeyPressed(GLFW_KEY_D))
		{
			editorCam.moveCamRight();
		}

	}

#endif
	if (input.IsMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT))
	{
		auto mousePos = input.GetMousePosition();
		LOG_DEBUG("Left mouse clicked at: (", mousePos.x, ", ", mousePos.y, ")");
#ifndef DISABLE_EDITOR
		// Retrieve picked ID and send it to editor
		m_Editor->RetrievePickedID(m_Renderer->getPickedID());
#endif
	}
	if (input.IsMouseButtonJustPressed(GLFW_MOUSE_BUTTON_RIGHT))
	{
		auto mousePos = input.GetMousePosition();
		LOG_DEBUG("Right mouse clicked at: (", mousePos.x, ", ", mousePos.y, ")");
	}

	// Scroll wheel
	auto scrollDelta = input.GetScrollDelta();
	if (std::abs(scrollDelta.y) > 0.01f)
	{
		LOG_DEBUG("Mouse scrolled: ", scrollDelta.y > 0 ? "UP" : "DOWN");
	}

	// Function keys
	if (input.IsKeyJustPressed(GLFW_KEY_F1))
	{
		bool newVisibility = !input.IsCursorVisible();
		input.SetCursorVisible(newVisibility);
		LOG_INFO("Cursor visibility toggled: ", newVisibility ? "VISIBLE" : "HIDDEN");
	}

	
#ifndef DISABLE_EDITOR
	if (m_EditorEnable)
	{
		m_Editor->OnUpdate(ts, m_Renderer->get_imgui_texture());
	}

	m_Editor->SetEditorViewport(m_Renderer->getEditorViewport());
	m_TracyProfiler->OnUpdate();
#endif
	Engine::EventSystem::Instance().DispatchQueued();
}

void Game::OnShutdown()
{
	LOG_INFO("Game shutting down...");

	if (m_ActiveScene)
	{
		LOG_DEBUG("SHUTTING DOWN SCENE");
		// Shutdown all systems before destroying scene
		m_ActiveScene->ShutdownSystems();
	}

	//============= Audio =============
	if (m_AudioManager)
	{
		LOG_INFO("Shutting down Audio Manager...");
		try
		{
			m_AudioManager->Shutdown();
			LOG_INFO("  -> Audio Manager shut down successfully");
		}
		catch (const std::exception& e)
		{
			LOG_ERROR("  -> Exception while shutting down Audio Manager: ", e.what());
		}
	}
	Engine::EventSystem::Instance().Clear();
	Engine::MonoScriptEngine::GetInstance().Shutdown();
	LOG_INFO("[Game] Mono shutdown");

	//============= Asset =============
	Engine::RM.shutDown();

	LOG_INFO("Shutting Down Asset");
	Engine::AM.shutDown();
	m_Scenes.clear();
	m_ActiveScene = nullptr;

	m_AudioManager.reset();
#ifndef DISABLE_EDITOR
	m_Editor.reset();
#endif
	m_TracyProfiler.reset();

	LOG_INFO("Game shutdown complete");
}

Engine::Scene* Game::CreateScene(const std::string& name)
{
	LOG_INFO("=== CreateScene: ", name, " ===");
	if (m_ActiveScene)
	{
		//m_ActiveScene = nullptr;
		m_ActiveScene->ShutdownSystems();
		m_ActiveScene->GetRegistry().clear();
		m_ActiveScene = nullptr;
	}

	// ===== Create NEW scene =====
	auto newScene = std::make_unique<Engine::Scene>(name);
	Engine::Scene* scenePtr = newScene.get();

	AddAllSystemsToScene(scenePtr);
	scenePtr->InitializeSystems();
	m_Scenes.push_back(std::move(newScene));
	m_ActiveScene = scenePtr;
	m_CurrentScenePath.clear();

	m_Renderer->getBloomToggle() = m_ActiveScene->GetSceneSetting().s_BloomToggle;
	m_Renderer->getBloomStrength() = m_ActiveScene->GetSceneSetting().s_BloomStrength;
	m_Renderer->getBloomFilterRadius() = m_ActiveScene->GetSceneSetting().s_BloomFilterRadius;
	m_Renderer->getExposure() = m_ActiveScene->GetSceneSetting().s_Exposure;
	m_Renderer->getGlobalBias() = m_ActiveScene->GetSceneSetting().s_GlobalBias;

	LOG_INFO("Scene created successfully");
	return m_ActiveScene;
}
