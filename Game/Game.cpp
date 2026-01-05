#include "Game.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Utility/Logger.h"
#include "Utility/AssetPath.h"
#include "ECS/Components.h"
#include "Editor/Editor.h"
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
#include "Scripting/ScriptReloader.h" 
#include "BehaviourTree/BehaviourTreeSystem.h"
#include "ParticleSystem/ParticleSystem.h"
#include "Animation/AnimationSystem.h"

#include "Event/EventSystem.h"

// KENNY TESTING: FOR MAINCAMERA "SCRIPT"
#include <glm/common.hpp>               // glm::clamp
#include <glm/gtc/type_ptr.hpp>         // glm::value_ptr
// Math Utility
#include "Utility/MathUtils.h"

// FOR BT TESTING
//#include "BehaviourTree/BTNodeRegistry.h"
//#include "Serialization/BehaviourTreeSerializer.h"
//#include "BehaviourTree/BehaviourTreeEditor.h"
//#include "Prefab/BehaviourTreePrefab.h"
//#include "BehaviourTree/BTNode.h"

#include "Serialization/PrefabSerializer.h"
#include "Serialization/PrefabInstantiator.h"
#include "Prefab/PrefabRegistry.h"

#include "Utility/AssetPath.h"

#ifndef DEBUG
//this is for release
Game::Game()
	: Application("Guardian of The MotherBoard", 1280, 720)
	, m_ActiveScene(nullptr)
	, m_Editor(nullptr)
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
	, m_Editor(nullptr)
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
	for (const auto &root : config.sourceRoots)
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
		LOG_INFO("Performing initial asset scan...");
		Engine::AM.scanAndProcess();

		LOG_INFO("Initial asset scan complete - found ",
			Engine::AM.db().Count(), " assets");
	}

	Engine::RM.startUp();

	// Step 1: Register components for serialization
	LOG_INFO("Step 1: Registering components...");
	try
	{
		Engine::ComponentRegistry::RegisterAllComponents();
		LOG_INFO("  -> Components registered successfully");
	}
	catch (const std::exception &e)
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
	catch (const std::exception &e)
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
	catch (const std::exception &e)
	{
		LOG_CRITICAL("  -> Exception while initializing Audio Manager: ", e.what());
		return;
	}

	// Step 4: Create scene
	LOG_INFO("Step 4: Creating scene object...");
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
		// Create initial scenes
		try
		{
			Engine::Scene* mainScene = CreateScene("Main Scene");
			if (!mainScene)
			{
				LOG_CRITICAL("Failed to create initial scene!");
				return;
			}
			m_Editor->SetActiveScene(mainScene);
			LOG_INFO("  -> Scene created at address: ", (void*)mainScene);

		}
		catch (const std::exception& e)
		{
			LOG_CRITICAL("Failed to create scenes: ", e.what());
			return;
		}

		//LOG_INFO("  -> Scene created at address: ", (void *)m_Scene.get());
	}
	catch (const std::exception &e)
	{
		LOG_CRITICAL("  -> Exception while creating scene: ", e.what());
		return;
	}

	// Step 5: Add systems to the scene
	LOG_INFO("Step 5: Adding systems to scene...");
	try
	{
		if (m_ActiveScene)  // Use m_ActiveScene instead of m_Scene
		{
			AddAllSystems();  // CHANGED: Replace all manual AddSystem calls with helper function
		}

		LOG_INFO("  -> Systems added successfully");
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("  -> Exception while adding systems: ", e.what());
	}

	// Step 6: Initialize all systems
	LOG_INFO("Step 6: Initializing systems...");
	try
	{
		if (m_ActiveScene)  // Use m_ActiveScene instead of m_Scene
		{
			m_ActiveScene->InitializeSystems();
			LOG_INFO("  -> Systems initialized successfully");
		}
		//LOG_INFO("  -> Systems initialized successfully");
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("  -> Exception while initializing systems: ", e.what());
	}

	// Step 7: Load scene from file or create default
	LOG_INFO("Step 7: Loading scene content...");
	bool loadedFromFile = false;

	try
	{
		if (m_ActiveScene)
		{

			loadedFromFile = m_ActiveScene->LoadFromFile("Resources/Sources/Scenes/ExampeScene.json");
		}

		if (loadedFromFile)
		{
			LOG_INFO("  -> Scene loaded from file successfully");

			// Update settings from loaded scene
			m_Renderer->getBloomToggle() = m_ActiveScene->GetSceneSetting().s_BloomToggle;
			m_Renderer->getBloomStrength() = m_ActiveScene->GetSceneSetting().s_BloomStrength;
			m_Renderer->getBloomFilterRadius() = m_ActiveScene->GetSceneSetting().s_BloomFilterRadius;
			m_Renderer->getExposure() = m_ActiveScene->GetSceneSetting().s_Exposure;
		}
		else
		{
			LOG_WARNING("  -> Could not load scene file (file may not exist)");
		}
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("  -> Exception while loading: ", e.what());
		loadedFromFile = false;
	}

	if (!loadedFromFile)
	{
		LOG_INFO("Step 7: Creating default scene...");
		try
		{
			if (m_ActiveScene)  // Use m_ActiveScene instead of m_Scene
			{
				CreateDefaultScene();  // Update CreateDefaultScene to take a parameter
				LOG_INFO("  -> Default scene created successfully");
			}
			else
			{
				LOG_ERROR("  -> No active scene to create default content in!");
			}
		}
		catch (const std::exception &e)
		{
			LOG_CRITICAL("  -> Failed to create default scene: ", e.what());
			return;
		}
	}

	// Final verification
	if (!m_ActiveScene)
	{
		LOG_CRITICAL("CRITICAL: Scene is null at end of OnInit()!");
		return;
	}

	// Step 8: Initialize Tracy Profiler
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
	catch (const std::exception &e)
	{
		LOG_ERROR("  -> Exception while initializing Tracy Profiler: ", e.what());
	}

  // ====================================
  // Step 8: Initialize Mono Scripting Engine
  // ====================================
	LOG_INFO("Step 8: Initializing Mono Scripting Engine...");
	try
	{
		std::string assemblyPath = "GameScripts.dll";

		if (std::filesystem::exists(assemblyPath))
		{
			Engine::MonoScriptEngine::GetInstance().Initialize(assemblyPath);
			LOG_INFO("  -> Mono Scripting Engine initialized successfully");
		}

		WCHAR exePath[MAX_PATH] = { 0 };
		GetModuleFileNameW(NULL, exePath, MAX_PATH);
		std::filesystem::path exeDir = std::filesystem::path(exePath).parent_path();

		std::filesystem::path projectRoot = exeDir.parent_path().parent_path().parent_path();
		std::filesystem::path scriptSourcePath = projectRoot / "Scripts" / "Game";
		std::filesystem::path scriptProjectPath = projectRoot / "Scripts" / "GameScripts.csproj";
		std::string outputDllPath = (exeDir / "GameScripts.dll").string();

		Engine::ScriptReloader::GetInstance().Initialize(
			scriptSourcePath.string(),
			scriptProjectPath.string(),
			outputDllPath
		);
	}
	catch (const std::exception &e)
	{
		LOG_ERROR("  -> Exception while initializing Mono: ", e.what());
	}

	LOG_INFO("=== Game::OnInit() COMPLETED SUCCESSFULLY ===");
	LOG_INFO("Scene status: VALID at ", (void *)m_ActiveScene);
	LOG_INFO("");
	LOG_INFO("=== CONTROLS ===");
	LOG_INFO("  WASD: Test movement (hold to move continuously)");
	LOG_INFO("  Space: Test action input");
	LOG_INFO("  Mouse: Click to test mouse input");
	LOG_INFO("  Scroll: Test scroll wheel");
	LOG_INFO("  F1: Toggle cursor visibility");
	LOG_INFO("  F2: Create test entity with velocity");
	LOG_INFO("  F5: Save scene to file");
	LOG_INFO("  F9: Load scene from file");
	//LOG_INFO("  P: Play Audio");
	//LOG_INFO("  O: Pause Audio");
	//LOG_INFO("  L: Stop Audio");
	LOG_INFO("  ESC: Exit");
	LOG_INFO("================");
	LOG_INFO("");
}

void Game::AddAllSystems()
{
	if (!m_ActiveScene) return;

	m_ActiveScene->AddSystem<Engine::AudioSystem>(m_AudioManager.get());
	m_ActiveScene->AddSystem<Engine::AudioEffectSystem>(m_AudioManager.get());
	m_ActiveScene->AddSystem<Engine::PhysicsSystem>();
	m_ActiveScene->AddSystem<Engine::TransformSystem>();
	m_ActiveScene->AddSystem<Engine::CameraSystem>();
	m_ActiveScene->AddSystem<Engine::ScriptSystem>();

	m_ActiveScene->AddSystem<Engine::RenderSystem>(*m_Renderer);
	m_ActiveScene->AddSystem<Engine::BehaviourTreeSystem>();
	m_ActiveScene->AddSystem<Engine::ParticleSystem>();
	m_ActiveScene->AddSystem<Engine::AnimationSystem>();
}

void Game::AddAllSystemsToScene(Engine::Scene* scene)
{
	if (!scene) return;

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
		for (const auto &entry : fs::directory_iterator(clipsDir))
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
		for (const auto &entry : fs::directory_iterator(ctrlDir))
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

	auto &transform = player.AddComponent<Engine::TransformComponent>();
	transform.Position = glm::vec3(1, 2, 0);  // Start above ground
	//transform.Scale    = glm::vec3(1.f, 1.f, 1.f);
	transform.Scale = glm::vec3(0.0005f, 0.0005f, 0.0005f);

	auto &mesh = player.AddComponent<Engine::MeshRendererComponent>();
	mesh.Material = 1;

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

	auto &script = player.AddComponent<Engine::ScriptComponent>();
	script.ScriptClassName = "Game.TestScript";
	LOG_TRACE("  -> SCRIPT IS CREATED SCRIPT IS CREATED");


	//auto& rb = player.AddComponent<Engine::RigidbodyComponent>();
	//rb.Mass = 1.0f;
	//rb.UseGravity = true;
	//rb.IsKinematic = false;
	//rb.Velocity = glm::vec3(0, 0, 0);  // Will fall due to gravity
	xresource::instance_guid tex_inst_guid = Engine::AM.getAssetIdByFilename("rabbit_kenny.png");

	auto &playerAudio = player.AddComponent<Engine::AudioComponent>();
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

	auto &camTransform = camera.AddComponent<Engine::TransformComponent>();
	camTransform.Position = glm::vec3(0, 5, 5);
	camTransform.Rotation = glm::vec3(-15, 0, 0);
	camTransform.Scale = glm::vec3(1, 1, 1);

	auto &camComponent = camera.AddComponent<Engine::CameraComponent>();
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

	auto &cam2Transform = cam2.AddComponent<Engine::TransformComponent>();
	cam2Transform.Position = glm::vec3(0, 5, 5);
	cam2Transform.Rotation = glm::vec3(-15, 0, 0);
	cam2Transform.Scale = glm::vec3(1, 1, 1);

	auto &cam2Component = cam2.AddComponent<Engine::CameraComponent>();
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

	auto &listener = camera.AddComponent<Engine::ListenerComponent>();
	listener.Active = true;
	LOG_TRACE("  -> Camera created with listenerComponent");

	LOG_TRACE("  Creating Ground entity...");
	auto ground = m_ActiveScene->CreateEntity("Ground");
	ground.AddComponent<Engine::TagComponent>("Ground");

	auto &groundTransform = ground.AddComponent<Engine::TransformComponent>();
	groundTransform.Position = glm::vec3(0, -1, 0);
	groundTransform.Scale = glm::vec3(20, 0.1f, 20);

	auto &groundRb = ground.AddComponent<Engine::RigidbodyComponent>();
	groundRb.Mass = 0.0f;
	groundRb.IsKinematic = true;
	groundRb.UseGravity = false;
	groundRb.Velocity = glm::vec3(0, 0, 0);

	auto &g_mesh = ground.AddComponent<Engine::MeshRendererComponent>();
	g_mesh.MaterialGuid = Engine::AM.getAssetIdByFilename("test.mat");
	LOG_TRACE("  -> Ground created");

	LOG_TRACE("  Creating Sphere entity...");
	auto sphere = m_ActiveScene->CreateEntity("Sphere");
	sphere.AddComponent<Engine::TagComponent>("Sphere");

	auto &sphereTransform = sphere.AddComponent<Engine::TransformComponent>();
	sphereTransform.Position = glm::vec3(-5.0f, 1.0f, 1.0f);
	//sphereTransform.Scale = glm::vec3(1.0f);
	sphereTransform.Scale = glm::vec3(1.0f);

	auto &sphereRb = sphere.AddComponent<Engine::RigidbodyComponent>();
	sphereRb.Mass = 0.0f;
	sphereRb.IsKinematic = true;
	sphereRb.UseGravity = false;
	sphereRb.Velocity = glm::vec3(0, 0, 0);

	auto &spheremesh = sphere.AddComponent<Engine::MeshRendererComponent>();
	spheremesh.MeshType = 0; // Sphere

	auto &sphereAnimation = sphere.AddComponent<Engine::AnimatorComponent>();
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

	auto &TimerBarTransform = TimerBar.AddComponent<Engine::TransformComponent>();
	TimerBarTransform.Position = glm::vec3(1.0f, 4.0f, 0.0f);
	//TimerBarTransform.Scale = glm::vec3(1.0f);
	TimerBarTransform.Scale = glm::vec3(4.0f, 0.05f, 0.05f);

	auto &TimerBarmesh = TimerBar.AddComponent<Engine::MeshRendererComponent>();
	TimerBarmesh.MeshType = 0; // Square

	auto &TimerBarAnimation = TimerBar.AddComponent<Engine::AnimatorComponent>();
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

	auto &rzTransform = reverbZone.AddComponent<Engine::TransformComponent>();
	rzTransform.Position = glm::vec3(0, 0, 0); // center of world
	rzTransform.Scale = glm::vec3(1, 1, 1);

	auto &reverb = reverbZone.AddComponent<Engine::ReverbZoneComponent>();
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

	auto &aiTransform = reverbZone.GetComponent<Engine::TransformComponent>();
	aiTransform.Position = glm::vec3(0, 0, 0); // center of world
	aiTransform.Scale = glm::vec3(1, 1, 1);

	auto &bt = ai.AddComponent<Engine::BehaviourTreeComponent>();
	bt.Active = true;
	bt.ResetOnComplete = false;
	bt.TreeAssetPath = "CreateEnemeyCube.json";

	LOG_TRACE("  -> ai created");

	LOG_TRACE("  Creating Sunlight entity...");
	auto sunlight = m_ActiveScene->CreateEntity("Sunlight");
	sunlight.AddComponent<Engine::TagComponent>("Sunlight");

	auto &sunlightTransform = sunlight.AddComponent<Engine::TransformComponent>();
	sunlightTransform.Position = glm::vec3(0, 10, 0);
	sunlightTransform.Rotation = glm::quatLookAt(
		glm::normalize(glm::vec3(-0.3f, -1.0f, -0.5f)), // forward vector (light direction)
		glm::vec3(0.0f, 1.0f, 0.0f));
	sunlightTransform.Scale = glm::vec3(1.0f);

	auto &sunlightLight = sunlight.AddComponent<Engine::LightComponent>();
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

	auto &lampTransform = lamp.AddComponent<Engine::TransformComponent>();
	lampTransform.Position = glm::vec3(5, 5, 0);

	auto &lampLight = lamp.AddComponent<Engine::LightComponent>();
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

	auto &spotlightTransform = spotlight.AddComponent<Engine::TransformComponent>();
	spotlightTransform.Position = glm::vec3(-5, 5, 0);
	spotlightTransform.Rotation = glm::quatLookAt(
		glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f)),
		glm::vec3(0.0f, 1.0f, 0.0f));

	auto &spotlightLight = spotlight.AddComponent<Engine::LightComponent>();
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
	auto &input = GetInput();

	// Get editor camera toggle and editor mode for renderer reference
	auto &editorCamToggle = m_Renderer->getEditorCamToggle();
	auto &editorModeToggle = m_Renderer->getEditorModeToggle();

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

		if (!editorCamToggle)
		{
			// Lock & hide cursor (free-look mode)
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
		else
		{
			// Restore normal cursor
			glfwSetInputMode(m_Window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}

		LOG_INFO("Editor camera toggled: ", editorCamToggle);
	}


	Engine::ScriptReloader::GetInstance().Update();

	//if (Engine::ScriptReloader::GetInstance().IsReloadRequested())
	//{
	//	LOG_INFO("[Game] Hot-reload requested, clearing script instances...");

	//	if (m_Scene)
	//	{
	//		auto& registry = m_Scene->GetRegistry();
	//		auto view = registry.view<Engine::ScriptComponent>();

	//		for (auto entity : view)
	//		{
	//			auto& script = registry.get<Engine::ScriptComponent>(entity);
	//			script.ScriptInstance = nullptr;  //  NULL the pointer
	//		}

	//		LOG_INFO("[Game] Cleared all script instances");
	//	}

	//	// NOW reload safely - no stale pointers exist
	//	Engine::MonoScriptEngine::GetInstance().ReloadAssembly();
	//	Engine::ScriptReloader::GetInstance().ClearReloadFlag();
	//	LOG_INFO("[Game] Hot-reload complete!");
	//}

	// When Editor is turned OFF OR Editor is ON but gameplay is PLAYING: Update Everything
	if (!m_EditorEnable || (m_EditorEnable && m_Editor->GetEditorIsPlaying()))
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

		auto &sceneSystems = m_ActiveScene->GetSystemRegistry();

		Engine::TransformSystem *transformSystem = sceneSystems.GetSystem<Engine::TransformSystem>();
		transformSystem->OnUpdate(m_ActiveScene, ts);

		if (m_IsFirstPausedFrame)
		{
			Engine::BehaviourTreeSystem *BTSystem = sceneSystems.GetSystem<Engine::BehaviourTreeSystem>();
			BTSystem->LoadBehaviourTrees(m_ActiveScene);
			m_IsFirstPausedFrame = false;
		}

		Engine::CameraSystem *camSystem = sceneSystems.GetSystem<Engine::CameraSystem>();
		camSystem->OnUpdate(m_ActiveScene, ts);

		Engine::RenderSystem *renderSystem = sceneSystems.GetSystem<Engine::RenderSystem>();
		renderSystem->OnUpdate(m_ActiveScene, ts);

		m_AudioManager->PauseAll(true);

	}

	/*if (input.IsKeyJustPressed(GLFW_KEY_P))
	{
		LOG_DEBUG("Testing Audio Playback");

		auto &registry = m_Scene->GetRegistry();
		auto view = registry.view<Engine::AudioComponent>();
		for (auto entityHandle : view)
		{
			auto &audio = view.get<Engine::AudioComponent>(entityHandle);

			if (audio.AudioFilePath.empty())
			{
				audio.AudioFilePath = "laserSmall_001.ogg";
			}

			audio.State = Engine::PlayState::PLAY;
		}
	}

	if (input.IsKeyJustPressed(GLFW_KEY_O))
	{
		auto &registry = m_Scene->GetRegistry();
		for (auto entityHandle : registry.view<Engine::AudioComponent>())
		{
			auto &audio = registry.get<Engine::AudioComponent>(entityHandle);
			audio.State = Engine::PlayState::PAUSE;
		}
	}
	if (input.IsKeyJustPressed(GLFW_KEY_L))
	{
		auto &registry = m_Scene->GetRegistry();
		for (auto entityHandle : registry.view<Engine::AudioComponent>())
		{
			auto &audio = registry.get<Engine::AudioComponent>(entityHandle);
			audio.State = Engine::PlayState::STOP;
		}
	}
	if (input.IsKeyJustPressed(GLFW_KEY_BACKSLASH))
	{
		float volume = 0.0f;
		m_AudioManager->GetGroupVolume(Engine::AudioType::SFX, volume);
		m_AudioManager->SetGroupVolume(Engine::AudioType::SFX, volume - 0.1f);
		LOG_TRACE("Reducing Audio SFX Group Volume by 0.1 Currently it is: ", volume);
	}*/


	// Audio Testing if Attentuation works
	//LOG_INFO("[TEST] Searching for entity named 'Player'...");

	auto &registry = m_ActiveScene->GetRegistry();

	Engine::Entity foundEntity;
	bool found = false;

	auto view = registry.view<Engine::TagComponent>();
	for (auto entityHandle : view)
	{
		auto &tag = view.get<Engine::TagComponent>(entityHandle);
		if (tag.Tag == "Player")
		{ // change to whatever name you want
			foundEntity = Engine::Entity(entityHandle, &registry);
			found = true;
			break;
		}
	}

	// Get MainCamera to follow Player from the back
	Engine::Entity GameCam;
	bool GameCamFound = false;
	for (auto entityHandle : view)
	{
		auto &camTag = view.get<Engine::TagComponent>(entityHandle);
		if (camTag.Tag == "MainCamera")
		{
			GameCam = Engine::Entity(entityHandle, &registry);
			GameCamFound = true;
			break;
		}
	}

	Engine::Entity SecCam;
	bool SecCamFound = false;
	for (auto entityHandle : view)
	{
		auto &camTag = view.get<Engine::TagComponent>(entityHandle);
		if (camTag.Tag == "SecondCamera")
		{
			SecCam = Engine::Entity(entityHandle, &registry);
			SecCamFound = true;
			break;
		}
	}

	Engine::Entity timerBarUI;
	bool timerBarUIFound = false;
	for (auto entityHandle : view)
	{
		auto &timerBarUITag = view.get<Engine::TagComponent>(entityHandle);
		if (timerBarUITag.Tag == "TimerBar")
		{
			timerBarUI = Engine::Entity(entityHandle, &registry);
			timerBarUIFound = true;
			break;
		}
	}

	Engine::Entity healthBarUI;
	bool healthBarUIFound = false;
	for (auto entityHandle : view)
	{
		auto &healthUITag = view.get<Engine::TagComponent>(entityHandle);
		if (healthUITag.Tag == "HealthBar")
		{
			healthBarUI = Engine::Entity(entityHandle, &registry);
			healthBarUIFound = true;
			break;
		}
	}

	//// Editor camera toggle
	//if (input.IsKeyJustPressed(GLFW_KEY_C))
	//{
	//	if (GameCam.HasComponent<Engine::CameraComponent>() &&
	//		SecCam.HasComponent<Engine::CameraComponent>())
	//	{

	//		auto &GameCamComp = GameCam.GetComponent<Engine::CameraComponent>();
	//		auto &SecCamComp = SecCam.GetComponent<Engine::CameraComponent>();

	//		GameCamComp.Enabled = !GameCamComp.Enabled;
	//		SecCamComp.Enabled = !SecCamComp.Enabled;
	//	}
	//}

	if (found && foundEntity.HasComponent<Engine::TransformComponent>())
	{

		// Get player transform to control its movement
		auto &transform = foundEntity.GetComponent<Engine::TransformComponent>();

		// Update main game camera on player if it exists
		if (GameCamFound && GameCam.HasComponent<Engine::CameraComponent>()
			&& GameCam.HasComponent<Engine::TransformComponent>()
			&& !editorCamToggle)
		{

			// Get MainCamera transform and camera component
			auto &camTransform = GameCam.GetComponent<Engine::TransformComponent>();
			auto &camComp = GameCam.GetComponent<Engine::CameraComponent>();

			// Player head/aim point (slightly above)
			const glm::vec3 aimTarget(transform.Position.x, transform.Position.y + 2.0f, transform.Position.z);

			// Persistent orbit state
			static bool  initialized = false;
			static float pitch = 0.25f; // alpha
			static float yaw = 0.0f;    // betta
			static float radius = 7.5f;

			// Initialize yaw/pitch from current camera placement once
			if (!initialized)
			{
				const glm::vec3 rel = camTransform.Position - aimTarget;
				const float r = glm::length(rel);
				if (r > 1e-6f)
				{
					pitch = glm::asin(glm::clamp(rel.y / r, -1.0f, 1.0f));
					yaw = std::atan2(rel.x, rel.z);
					/* Radius is constant thru out the gameplay */
				}
				else
				{
					// fallback if camera starts at target: put it behind player
					pitch = 0.25f;
					yaw = 0.0f;
					radius = 7.5f;
				}
				initialized = true;
			}

			// Mouse deltas
			const float xOffset = input.GetMouseDelta().x;
			const float yOffset = input.GetMouseDelta().y;

			// Update orbit angles only when mouse moves
			if (xOffset != 0.0f || yOffset != 0.0f)
			{

				// Adjust angles based on cursor offset
				yaw += (xOffset < 0.0f) ? 0.05f : (xOffset > 0.0f ? -0.05f : 0.0f);
				pitch += (yOffset > 0.0f) ? 0.02f : (yOffset < 0.0f ? -0.02f : 0.0f);

				// Clamp pitch to avoid flipping
				pitch = glm::clamp(pitch, -Engine::MathUtils::HALF_PI + 0.01f, Engine::MathUtils::HALF_PI - 0.01f);
			}

			// Rebuild direction from yaw/pitch EVERY FRAME
			glm::vec3 dir;
			dir.x = glm::cos(pitch) * glm::sin(yaw);
			dir.y = glm::sin(pitch);
			dir.z = glm::cos(pitch) * glm::cos(yaw);
			dir = glm::normalize(dir);

			// Keep constant distance from the player (orbit)
			const glm::vec3 camPos = aimTarget + dir * radius;
			camTransform.SetPosition(camPos);

			// Always update camera target to the player's head/aim point
			camComp.SetTarget(aimTarget);

			///* Camera and Player rotations if all meshes face Z- as forward */
			//// Player face same horizontal direction as the camera (camera behind player)
			//glm::vec3 camFwd = glm::normalize(aimTarget - camPos);          // camera forward (cam -> target)

			//// Calculate yaw (rotation around Y axis)
			//const float yawDeg = glm::degrees(std::atan2(camFwd.x, camFwd.z));

			//// Calculate pitch (rotation around X axis)
			//// Pitch = angle between horizontal plane and forward vector
			//float pitchDeg = glm::degrees(std::asin(glm::clamp(-camFwd.y, -1.0f, 1.0f)));

			//transform.SetRotation(glm::vec3(pitchDeg, yawDeg/* - 90.0f*/, 0.0f));
			///* Camera and Player rotations if all meshes face Z- as forward */

			/* Temporary adjustments to Camera and Player rotations */
			glm::vec3 camFwd = glm::normalize(aimTarget - camPos);
			glm::vec3 camRight = glm::normalize(glm::cross(glm::vec3(0, 1, 0), camFwd));
			glm::vec3 camUp = glm::normalize(glm::cross(camFwd, camRight));

			// Build rotation from camera basis to match player orientation
			glm::mat3 camBasis(camRight, camUp, camFwd);

			// Your model’s forward = X+, so rotate 90° around Y to map +Z → +X
			glm::quat modelOffset = glm::angleAxis(glm::radians(-90.0f), glm::vec3(0, 1, 0));

			// Convert basis to quaternion and apply offset
			glm::quat q = glm::normalize(glm::quat_cast(camBasis) * modelOffset);

			transform.Rotation = q;
			transform.IsDirty = true;

			/* Temporary adjustments to Camera and Player rotations */

			/* Player controls begin here */
			// Movement speed 
			const float moveSpeed = 0.1f;

			// Get player's facing direction (derived from rotation quaternion) (For now adjust according to mesh's front)
			//glm::vec3 forward = transform.Rotation * glm::vec3(0.0f, 0.0f, 1.0f); // forward in local space (For cube) 
			glm::vec3 forward = glm::normalize(transform.Rotation * glm::vec3(1.0f, 0.0f, 0.0f)); // forward in local space (For botnet)

			// Compute right vector of player from forward and world up
			glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

			// TimerBar entity exists and has a TransformComponent
			if (timerBarUIFound && timerBarUI.HasComponent<Engine::TransformComponent>())
			{

				// Timerbar UI transform
				auto &timerBarTrans = timerBarUI.GetComponent<Engine::TransformComponent>();

				// Place timer bar above the player's local up
				const float timerBarDist = 3.0f; // How high above the player
				const float angleRad = glm::radians(20.0f);	// Diagonal angle to "fit" into the camera screen
				glm::vec3 angledDir = glm::normalize(camFwd * std::cos(angleRad) +
					camUp * std::sin(angleRad));
				timerBarTrans.Position = camPos + angledDir * timerBarDist;

				// Make timer bar "follow" camera rotation
				// Use only pitch and yaw so the timer bar stays upright 
				const float timerBarPitchDeg = glm::degrees(std::asin(glm::clamp(-camFwd.y, -1.0f, 1.0f)));
				const float timerBarYawDeg = glm::degrees(std::atan2(camFwd.x, camFwd.z));
				timerBarTrans.SetRotation(glm::vec3(timerBarPitchDeg, timerBarYawDeg, 0.0f));
			}

			if (healthBarUIFound && healthBarUI.HasComponent<Engine::TransformComponent>())
			{

				// Healthbar UI transform
				auto &healthBarTrans = healthBarUI.GetComponent<Engine::TransformComponent>();

				// Place timer bar above the player's local up
				const float healthBarDist = 3.0f;      // How far from camera/player
				const float angleRad = glm::radians(20.0f);
				glm::vec3 angledDir = glm::normalize(camFwd * std::cos(angleRad) +
					camUp * std::sin(angleRad));

				// Offset downward in camera's local up direction
				const float yOffset = -3.5f; // negative = lower
				glm::vec3 offset = camUp * yOffset;

				// Final position
				healthBarTrans.Position = camPos + angledDir * healthBarDist + offset;

				// Make timer bar "follow" camera rotation
				const float healthBarPitchDeg = glm::degrees(std::asin(glm::clamp(-camFwd.y, -1.0f, 1.0f)));
				const float healthBarYawDeg = glm::degrees(std::atan2(camFwd.x, camFwd.z));
				healthBarTrans.SetRotation(glm::vec3(healthBarPitchDeg, healthBarYawDeg, 0.0f));
			}


			// Movement accumulator
			glm::vec3 moveDir(0.0f);

			if (input.IsKeyPressed(GLFW_KEY_W)) moveDir += forward;  // move forward
			if (input.IsKeyPressed(GLFW_KEY_S)) moveDir -= forward;  // move backward
			if (input.IsKeyPressed(GLFW_KEY_A)) moveDir -= right;    // move left
			if (input.IsKeyPressed(GLFW_KEY_D)) moveDir += right;    // move right

			// Normalize to prevent faster diagonal movement
			if (glm::dot(moveDir, moveDir) > 0.0f)
				moveDir = glm::normalize(moveDir);

			// Apply movement to player
			transform.Position += moveDir * moveSpeed;

		}
		else
		{
			// Default player movement w/o MainCamera
			//if (input.IsKeyPressed(GLFW_KEY_W)) transform.Position.z -= 0.1f; // move forward
			//if (input.IsKeyPressed(GLFW_KEY_S)) transform.Position.z += 0.1f; // move backward
			//if (input.IsKeyPressed(GLFW_KEY_A)) transform.Position.x -= 0.1f; // move left
			//if (input.IsKeyPressed(GLFW_KEY_D)) transform.Position.x += 0.1f; // move right
		}
	}


	////sphereTrans.Position = glm::vec3(transform.Position.x, transform.Position.y + 2.0f, transform.Position.z);
	//sphereTrans.SetPosition(glm::vec3(transform.Position.x, transform.Position.y + 2.0f, transform.Position.z));
	//sphereTrans.Rotation = q;
	////sphereTrans.IsDirty = true;

	// Editor camera controls
	if (input.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && editorCamToggle)
	{

		auto &editorCam = m_Renderer->getEditorCamera();

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

	// Test the DSP Global Effects

	FMOD::DSP *dsp = nullptr;
	if (input.IsKeyJustPressed(GLFW_KEY_ENTER))
	{
		dsp = m_AudioManager->CreateDSP(Engine::DSPEffectType::LowPass, Engine::AudioType::SFX);
		m_AudioManager->SetDSPParameter(Engine::AudioType::SFX, Engine::DSPEffectType::LowPass,
			FMOD_DSP_LOWPASS_CUTOFF, 1000.0); //1kHz = muffled
	}

	if (input.IsKeyJustPressed(GLFW_KEY_LEFT_BRACKET))
	{
		m_AudioManager->EnableDSP(Engine::AudioType::SFX, Engine::DSPEffectType::LowPass, true);
	}
	if (input.IsKeyJustPressed(GLFW_KEY_RIGHT_BRACKET))
	{
		m_AudioManager->EnableDSP(Engine::AudioType::SFX, Engine::DSPEffectType::LowPass, false);
	}

	if (dsp)
	{
		float cutoff;
		dsp->getParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, &cutoff, nullptr, 0);
		LOG_INFO("LowPass cutoff currently: ", cutoff);
	}

	// Move player in/out of the reverb radius with QE to feel falloff
	/*if (found && foundEntity.HasComponent<Engine::TransformComponent>()) {
		auto& tf = foundEntity.GetComponent<Engine::TransformComponent>();
		if (input.IsKeyPressed(GLFW_KEY_Q)) tf.Position.y += 0.05f;
		if (input.IsKeyPressed(GLFW_KEY_E)) tf.Position.y -= 0.05f;
	}*/

	// === Test Input System ===

	// Movement keys - continuous input while held
	//if (input.IsKeyPressed(GLFW_KEY_W)) {
	//    LOG_DEBUG("W held - Moving forward");
	//}
	//if (input.IsKeyPressed(GLFW_KEY_S)) {
	//    LOG_DEBUG("S held - Moving backward");
	//}
	//if (input.IsKeyPressed(GLFW_KEY_A)) {
	//    LOG_DEBUG("A held - Moving left");
	//}
	//if (input.IsKeyPressed(GLFW_KEY_D)) {
	//    LOG_DEBUG("D held - Moving right");
	//}

	// Action keys - one-time press
	if (input.IsKeyJustPressed(GLFW_KEY_SPACE))
	{
		LOG_DEBUG("Space pressed - Jump action!");
	}

	// Mouse buttons
	if (input.IsMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT))
	{
		auto mousePos = input.GetMousePosition();
		LOG_DEBUG("Left mouse clicked at: (", mousePos.x, ", ", mousePos.y, ")");

		// Retrieve picked ID and send it to editor
		m_Editor->RetrievePickedID(m_Renderer->getPickedID());
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

	if (input.IsKeyJustPressed(GLFW_KEY_F2))
	{
		LOG_INFO("F2 pressed - Creating test entity with velocity...");
		static int entityCounter = 0;

		auto newEntity = m_ActiveScene->CreateEntity("DynamicEntity_" + std::to_string(entityCounter));
		newEntity.AddComponent<Engine::TagComponent>("DynamicEntity_" + std::to_string(entityCounter));

		auto &transform = newEntity.AddComponent<Engine::TransformComponent>();
		transform.Position = glm::vec3(entityCounter * 2.0f, 10.0f, 0);
		transform.Rotation = glm::vec3(0, 0, 0);
		transform.Scale = glm::vec3(1, 1, 1);

		// Add rigidbody with random velocity to demonstrate MovementSystem
		auto &rb = newEntity.AddComponent<Engine::RigidbodyComponent>();
		rb.Mass = 1.0f;
		rb.UseGravity = true;
		rb.IsKinematic = false;
		rb.Velocity = glm::vec3(
			(entityCounter % 2 == 0 ? 1.0f : -1.0f),
			0.0f,
			0.0f
		);

		newEntity.AddComponent<Engine::MeshRendererComponent>();

		entityCounter++;
		LOG_INFO("Created falling entity ID: ", (uint32_t)newEntity, " with velocity (will demonstrate MovementSystem)");
	}

	// Serialization controls
	if (input.IsKeyJustPressed(GLFW_KEY_F5))
	{
		LOG_INFO("=== SAVING SCENE ===");
		bool success = m_ActiveScene->SaveToFile("Resources/Sources/Scenes/SavedScene.json");
		LOG_INFO(success ? "Scene saved!" : "Save failed!");
	}

	if (input.IsKeyJustPressed(GLFW_KEY_F9))
	{
		LOG_INFO("=== LOADING SCENE ===");

		// Shutdown systems before loading new scene
		m_ActiveScene->ShutdownSystems();

		bool success = m_ActiveScene->LoadFromFile("Resources/Sources/Scenes/ExampleScene.json");

		// Reinitialize systems after loading
		if (success)
		{
			AddAllSystems();
			m_ActiveScene->InitializeSystems();
			LOG_INFO("Scene loaded and systems reinitialized!");
		}
		else
		{
			LOG_ERROR("Load failed!");
		}
	}

	//m_Editor->StartImguiFrame();

	// Update Editor To Do
	//m_Editor->OnUpdate(Engine::Timestep ts);
	//m_Renderer->get_imgui_texture();

	if (m_EditorEnable)
	{
		m_Editor->OnUpdate(ts, m_Renderer->get_imgui_texture());
	}

	m_Editor->SetEditorViewport(m_Renderer->getEditorViewport());
	m_TracyProfiler->OnUpdate();

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
		catch (const std::exception &e)
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
	m_Editor.reset();
	m_TracyProfiler.reset();

	LOG_INFO("Game shutdown complete");
}

Engine::Scene* Game::CreateScene(const std::string& name)
{
	if (m_ActiveScene)
	{
		m_ActiveScene->ShutdownSystems();
		m_ActiveScene->GetRegistry().clear();
	}

	// ===== Create NEW scene =====
	auto newScene = std::make_unique<Engine::Scene>(name);
	Engine::Scene* scenePtr = newScene.get();
	AddAllSystemsToScene(scenePtr);
	scenePtr->InitializeSystems();
	m_Scenes.push_back(std::move(newScene));
	m_ActiveScene = scenePtr;
	return scenePtr;
}

void Game::RequestNewSceneFromEditor(const std::string& name)
{
	Engine::Scene* newScene = CreateScene(name);
	m_Editor->SetActiveScene(newScene);
	LOG_INFO("New scene created and set as active: ", name);
}