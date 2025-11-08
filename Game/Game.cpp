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

// KENNY TESTING: FOR MAINCAMERA "SCRIPT"
#include <glm/common.hpp>               // glm::clamp
#include <glm/gtc/type_ptr.hpp>         // glm::value_ptr
// Math Utility
#include "Utility/MathUtils.h"


// KEPT FOR BT TEST <WILL REMOVE BY THIS WEEKEND PLS DUN TOUCH>
#include "BehaviourTree/BTNodeRegistry.h"
#include "Serialization/BehaviourTreeSerializer.h"
#include "BehaviourTree/BehaviourTreeEditor.h"
#include "Prefab/BehaviourTreePrefab.h"
#include "Serialization/PrefabSerializer.h"
#include "Serialization/PrefabInstantiator.h"
#include "Prefab/PrefabRegistry.h"
#include "BehaviourTree/BTNode.h"
#include "Utility/AssetPath.h"

Game::Game()
    : Application("Property-Based ECS Engine", 1280, 720)
    , m_Scene(nullptr)
    , m_Editor(nullptr)
    , m_ColorShift(0.0f) {
    LOG_INFO("Game constructor body executing");
}

void Game::OnInit() {
    LOG_INFO("=== Game::OnInit() STARTED ===");

    //==========INITIALIZING ASSET ==============

    LOG_INFO("Initializing Asset...");

    auto config = Engine::AM.createDefaultConfig();

    //debug 
    LOG_INFO("Asset Manager Configuration:");
    LOG_INFO("  Source Roots:");
    for (const auto& root : config.sourceRoots) {
        LOG_INFO("    - ", root);
    }
    LOG_INFO("  Descriptor Root: ", config.descriptorRoot);
    LOG_INFO("  Database File: ", config.databaseFile);
    LOG_INFO("  Compiled Root: ", config.compiledPath);

    Engine::AM.setConfig(config);

    if (Engine::AM.startUp() != 0) {
        LOG_ERROR("Failed to initialize Asset Manager!");
        return;
    }
    else {

        LOG_INFO("Performing initial asset scan...");
        Engine::AM.scanAndProcess();

        LOG_INFO("Initial asset scan complete - found ",
            Engine::AM.db().Count(), " assets");
    }

    Engine::RM.startUp();

    // Step 1: Register components for serialization
    LOG_INFO("Step 1: Registering components...");
    try {
        Engine::ComponentRegistry::RegisterAllComponents();
        LOG_INFO("  -> Components registered successfully");
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("  -> FAILED to register components: ", e.what());
        return;
    }

    // Step 2: Register nodes for serialization
    LOG_INFO("Step 2: Registering components...");
    try {
        // Register all built-in behavior tree node types
        Engine::BTNodeRegistry::RegisterBuiltInNodes();
        LOG_INFO("  -> Components registered successfully");
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("  -> FAILED to register components: ", e.what());
        return;
    }

    // Step 3: Create Audio Manager
    LOG_INFO("Step 3: Initializing Audio Manager...");
    try {
        m_AudioManager = std::make_unique<Engine::AudioManager>();
        if (!m_AudioManager->Init()) {
            LOG_CRITICAL("  -> Audio Manager initialization failed!");
            return;
        }
        LOG_INFO("  -> Audio Manager initialized successfully");
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("  -> Exception while initializing Audio Manager: ", e.what());
        return;
    }

    // Step 4: Create scene
    LOG_INFO("Step 4: Creating scene object...");
    try {
        m_Scene = std::make_unique<Engine::Scene>("Main Scene");


        if (!m_Scene) {
            LOG_CRITICAL("  -> Scene pointer is null after make_unique!");
            return;
        }

        // Editor get scene
        if (!m_Editor)
        {
            m_Editor = std::make_unique<Engine::Editor>(GetWindow());
            m_Editor->SetScene(m_Scene.get());
            m_Editor->SetRenderer(m_Renderer.get());
            m_Editor->OnInit();
            LOG_INFO("Editor initialized successfully.");

        }

        LOG_INFO("  -> Scene created at address: ", (void*)m_Scene.get());
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("  -> Exception while creating scene: ", e.what());
        return;
    }

    // Step 5: Add systems to the scene
    LOG_INFO("Step 5: Adding systems to scene...");
    try {
        AddAllSystems();  // CHANGED: Replace all manual AddSystem calls with helper function

        LOG_INFO("  -> Systems added successfully");
    }
    catch (const std::exception& e) {
        LOG_ERROR("  -> Exception while adding systems: ", e.what());
    }

    // Step 6: Initialize all systems
    LOG_INFO("Step 6: Initializing systems...");
    try {
        m_Scene->InitializeSystems();
        LOG_INFO("  -> Systems initialized successfully");
    }
    catch (const std::exception& e) {
        LOG_ERROR("  -> Exception while initializing systems: ", e.what());
    }

    // Step 7: Load scene from file or create default
    LOG_INFO("Step 7: Loading scene content...");
    bool loadedFromFile = false;

    try {
        loadedFromFile = m_Scene->LoadFromFile("Resources/Sources/Scenes/ExampeScene.json");

        if (loadedFromFile) {
            LOG_INFO("  -> Scene loaded from file successfully");
        }
        else {
            LOG_WARNING("  -> Could not load scene file (file may not exist)");
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR("  -> Exception while loading: ", e.what());
        loadedFromFile = false;
    }

    if (!loadedFromFile) {
        LOG_INFO("Step 7: Creating default scene...");
        try {
            CreateDefaultScene();
            LOG_INFO("  -> Default scene created successfully");
        }
        catch (const std::exception& e) {
            LOG_CRITICAL("  -> Failed to create default scene: ", e.what());
            m_Scene.reset();
            return;
        }
    }

    // Final verification
    if (!m_Scene) {
        LOG_CRITICAL("CRITICAL: Scene is null at end of OnInit()!");
        return;
    }

    // Step 8: Initialize Tracy Profiler
    LOG_INFO("Step 8: Initializing Tracy Profiler...");
    try {
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
    catch (const std::exception& e) {
        LOG_ERROR("  -> Exception while initializing Tracy Profiler: ", e.what());
    }


    // ADD THIS NEW STEP 8:
  // ====================================
  // Step 8: Initialize Mono Scripting Engine
  // ====================================
    LOG_INFO("Step 8: Initializing Mono Scripting Engine...");
    try {
        std::string assemblyPath = "GameScripts.dll";

        if (std::filesystem::exists(assemblyPath)) {
            Engine::MonoScriptEngine::GetInstance().Initialize(assemblyPath);
            LOG_INFO("  -> Mono Scripting Engine initialized successfully");
        }

        // NEW: Initialize hot-reload system
        Engine::ScriptReloader::GetInstance().Initialize(
            "../../../../Scripts",                      // 
            "../../../../Scripts/GameScripts.csproj",   // 
            "GameScripts.dll"                                 // Output DLL path
        );
    }
    catch (const std::exception& e) {
        LOG_ERROR("  -> Exception while initializing Mono: ", e.what());
    }

    LOG_INFO("=== Game::OnInit() COMPLETED SUCCESSFULLY ===");


    LOG_INFO("=== Game::OnInit() COMPLETED SUCCESSFULLY ===");
    LOG_INFO("Scene status: VALID at ", (void*)m_Scene.get());
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
    LOG_INFO("  P: Play Audio");
    LOG_INFO("  O: Pause Audio");
    LOG_INFO("  L: Stop Audio");
    LOG_INFO("  ESC: Exit");
    LOG_INFO("================");
    LOG_INFO("");
}

void Game::AddAllSystems() {


    m_Scene->AddSystem<Engine::AudioSystem>(m_AudioManager.get());
    m_Scene->AddSystem<Engine::AudioEffectSystem>(m_AudioManager.get());
    m_Scene->AddSystem<Engine::PhysicsSystem>();
    m_Scene->AddSystem<Engine::TransformSystem>();
    m_Scene->AddSystem<Engine::CameraSystem>();
    m_Scene->AddSystem<Engine::ScriptSystem>();

    m_Scene->AddSystem<Engine::RenderSystem>(*m_Renderer);
    m_Scene->AddSystem<Engine::BehaviourTreeSystem>();
    m_Scene->AddSystem<Engine::ParticleSystem>();
}

void Game::CreateDefaultScene() {
    if (!m_Scene) {
        throw std::runtime_error("Scene is null in CreateDefaultScene");
    }

    LOG_TRACE("  Creating Player entity...");
    auto player = m_Scene->CreateEntity("Player");
    player.AddComponent<Engine::TagComponent>("Player");

    auto& transform = player.AddComponent<Engine::TransformComponent>();
    transform.Position = glm::vec3(1, 2, 0);  // Start above ground
    //transform.Scale    = glm::vec3(1.f, 1.f, 1.f);
    transform.Scale = glm::vec3(0.0005f, 0.0005f, 0.0005f);

    auto& mesh = player.AddComponent<Engine::MeshRendererComponent>();
    mesh.Material = 1;

    std::string meshName = "E004_botnet_v001.fbx";
    xresource::instance_guid inst_guid = Engine::AM.getAssetIdByFilename(meshName);
    mesh.MeshGuid = inst_guid;
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


    //auto& rb = player.AddComponent<Engine::RigidbodyComponent>();
    //rb.Mass = 1.0f;
    //rb.UseGravity = true;
    //rb.IsKinematic = false;
    //rb.Velocity = glm::vec3(0, 0, 0);  // Will fall due to gravity
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
    auto camera = m_Scene->CreateEntity("MainCamera");
    camera.AddComponent<Engine::TagComponent>("MainCamera");

    auto& camTransform = camera.AddComponent<Engine::TransformComponent>();
    camTransform.Position = glm::vec3(0, 5, 5);
    camTransform.Rotation = glm::vec3(-15, 0, 0);
    camTransform.Scale = glm::vec3(1, 1, 1);

    auto& camComponent = camera.AddComponent<Engine::CameraComponent>();
    camComponent.Enabled = true;
    camComponent.autoAspect = true;
    camComponent.Depth = 0; // 0 is the main camera
    camComponent.Aspect = GetWidth() / GetHeight();
    camComponent.FOV = 45.0f;
    camComponent.NearPlane = 0.5f;
    camComponent.FarPlane = 100.0f;
    camComponent.Target = { 0.0f, 0.0f, 0.0f };
    LOG_TRACE("  -> Camera created");

    auto& listener = camera.AddComponent<Engine::ListenerComponent>();
    listener.Active = true;
    LOG_TRACE("  -> Camera created with listenerComponent");

    LOG_TRACE("  Creating Ground entity...");
    auto ground = m_Scene->CreateEntity("Ground");
    ground.AddComponent<Engine::TagComponent>("Ground");

    auto& groundTransform = ground.AddComponent<Engine::TransformComponent>();
    groundTransform.Position = glm::vec3(0, -1, 0);
    groundTransform.Scale = glm::vec3(20, 0.1f, 20);

    auto& groundRb = ground.AddComponent<Engine::RigidbodyComponent>();
    groundRb.Mass = 0.0f;
    groundRb.IsKinematic = true;
    groundRb.UseGravity = false;
    groundRb.Velocity = glm::vec3(0, 0, 0);

    auto& groundmesh = ground.AddComponent<Engine::MeshRendererComponent>();
    LOG_TRACE("  -> Ground created");


    groundmesh.TextureGuid = tex_inst_guid;

    LOG_TRACE("  Creating ReverbZone entity...");
    auto reverbZone = m_Scene->CreateEntity("CaveReverb");
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
    auto ai = m_Scene->CreateEntity("AI");

    auto& aiTransform = reverbZone.GetComponent<Engine::TransformComponent>();
    aiTransform.Position = glm::vec3(0, 0, 0); // center of world
    aiTransform.Scale = glm::vec3(1, 1, 1);

    auto& bt = ai.AddComponent<Engine::BehaviourTreeComponent>();
    bt.Active = true;
    bt.ResetOnComplete = false;
    bt.TreeAssetPath = "SimpleWaitTree.json";

    LOG_TRACE("  -> ai created");

    LOG_TRACE("  Creating Sunlight entity...");
    auto sunlight = m_Scene->CreateEntity("Sunlight");
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
    auto lamp = m_Scene->CreateEntity("Lamp");
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
    auto spotlight = m_Scene->CreateEntity("Spotlight");
    spotlight.AddComponent<Engine::TagComponent>("Spotlight");

    auto& spotlightTransform = spotlight.AddComponent<Engine::TransformComponent>();
    spotlightTransform.Position = glm::vec3(-5, 5, 0);

    auto& spotlightLight = spotlight.AddComponent<Engine::LightComponent>();
    spotlightLight.Type = Engine::LightType::Spot;
    spotlightLight.Enabled = true;
    spotlightLight.Color = glm::vec3(0.85f, 0.0f, 0.0f);  // red
    spotlightLight.Intensity = 2.0f;     // 1.0 = neutral brightness
    spotlightLight.Range = 10.0f;         // meters / world units
    spotlightLight.SpotAngleDeg = 45.0f;
    spotlightLight.IndirectMultiplier = 1.0f;
    spotlightLight.Mode = Engine::LightMode::Realtime;
    LOG_TRACE("  -> Spotlight created");
}

void Game::OnUpdate(Engine::Timestep ts) {
    // Check scene validity
    if (!m_Scene) {
        static bool errorLogged = false;
        if (!errorLogged) {
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
    Engine::ScriptReloader::GetInstance().Update();

    // Update scene (this will call all systems in priority order)
    m_Scene->OnUpdate(ts);  // Convert Timestep to float

    // Update audio manager if exists
    m_AudioManager->OnUpdate(ts);

    if (input.IsKeyJustPressed(GLFW_KEY_P)) {
        LOG_DEBUG("Testing Audio Playback");

        auto& registry = m_Scene->GetRegistry();
        auto view = registry.view<Engine::AudioComponent>();
        for (auto entityHandle : view) {
            auto& audio = view.get<Engine::AudioComponent>(entityHandle);

            if (audio.AudioFilePath.empty()) {
                audio.AudioFilePath = "laserSmall_001.ogg";
            }

            audio.State = Engine::PlayState::PLAY;
        }
    }

    if (input.IsKeyJustPressed(GLFW_KEY_O)) {
        auto& registry = m_Scene->GetRegistry();
        for (auto entityHandle : registry.view<Engine::AudioComponent>()) {
            auto& audio = registry.get<Engine::AudioComponent>(entityHandle);
            audio.State = Engine::PlayState::PAUSE;
        }
    }
    if (input.IsKeyJustPressed(GLFW_KEY_L)) {
        auto& registry = m_Scene->GetRegistry();
        for (auto entityHandle : registry.view<Engine::AudioComponent>()) {
            auto& audio = registry.get<Engine::AudioComponent>(entityHandle);
            audio.State = Engine::PlayState::STOP;
        }
    }
    if (input.IsKeyJustPressed(GLFW_KEY_BACKSLASH)) {
        float volume = 0.0f;
        m_AudioManager->GetGroupVolume(Engine::AudioType::SFX, volume);
        m_AudioManager->SetGroupVolume(Engine::AudioType::SFX, volume - 0.1f);
        LOG_TRACE("Reducing Audio SFX Group Volume by 0.1 Currently it is: ", volume);
    }


    // Audio Testing if Attentuation works
    //LOG_INFO("[TEST] Searching for entity named 'Player'...");

    auto& registry = m_Scene->GetRegistry();

    Engine::Entity foundEntity;
    bool found = false;

    auto view = registry.view<Engine::TagComponent>();
    for (auto entityHandle : view) {
        auto& tag = view.get<Engine::TagComponent>(entityHandle);
        if (tag.Tag == "Player") { // change to whatever name you want
            foundEntity = Engine::Entity(entityHandle, &registry);
            found = true;
            break;
        }
    }

    // Get MainCamera to follow Player from the back
    Engine::Entity GameCam;
    bool GameCamFound = false;
    for (auto entityHandle : view) {
        auto& camTag = view.get<Engine::TagComponent>(entityHandle);
        if (camTag.Tag == "MainCamera") {
            GameCam = Engine::Entity(entityHandle, &registry);
            GameCamFound = true;
            break;
        }
    }

    // Editor camera toggle
    auto& editorCamToggle = m_Renderer->getEditorCamToggle();
    if (input.IsKeyJustPressed(GLFW_KEY_TAB)) {
        if (editorCamToggle == true) {
            editorCamToggle = false;
        }
        else {
            editorCamToggle = true;
        }
    }

    if (found && foundEntity.HasComponent<Engine::TransformComponent>()) {

        // Get player transform to control its movement
        auto& transform = foundEntity.GetComponent<Engine::TransformComponent>();

        // Update main game camera on player if it exists
        if (GameCamFound && GameCam.HasComponent<Engine::CameraComponent>()
            && GameCam.HasComponent<Engine::TransformComponent>()
            && !editorCamToggle) {

            // Get MainCamera transform and camera component
            auto& camTransform = GameCam.GetComponent<Engine::TransformComponent>();
            auto& camComp = GameCam.GetComponent<Engine::CameraComponent>();

            // Player head/aim point (slightly above)
            const glm::vec3 aimTarget(transform.Position.x, transform.Position.y + 2.0f, transform.Position.z);

            // Persistent orbit state
            static bool  initialized = false;
            static float pitch = 0.25f; // alpha
            static float yaw = 0.0f;    // betta
            static float radius = 7.5f;

            // Initialize yaw/pitch from current camera placement once
            if (!initialized) {
                const glm::vec3 rel = camTransform.Position - aimTarget;
                const float r = glm::length(rel);
                if (r > 1e-6f) {
                    pitch = glm::asin(glm::clamp(rel.y / r, -1.0f, 1.0f));
                    yaw = std::atan2(rel.x, rel.z);
                    /* Radius is constant thru out the gameplay */
                }
                else {
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
            if (xOffset != 0.0f || yOffset != 0.0f) {

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

            /* Camera and Player rotations if all meshes face Z- as forward */
            //// Player face same horizontal direction as the camera (camera behind player)
            //glm::vec3 camFwd = glm::normalize(aimTarget - camPos);          // camera forward (cam -> target)

            //// Calculate yaw (rotation around Y axis)
            //const float yawDeg = glm::degrees(std::atan2(camFwd.x, camFwd.z));

            //// Calculate pitch (rotation around X axis)
            //// Pitch = angle between horizontal plane and forward vector
            //float pitchDeg = glm::degrees(std::asin(glm::clamp(-camFwd.y, -1.0f, 1.0f)));

            //transform.SetRotation(glm::vec3(pitchDeg, yawDeg/* - 90.0f*/, 0.0f));
            /* Camera and Player rotations if all meshes face Z- as forward */

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
            glm::vec3 forward = transform.Rotation * glm::vec3(1.0f, 0.0f, 0.0f); // forward in local space (For botnet)
            forward = glm::normalize(forward);

            // Compute right vector from forward and world up
            glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0.0f, 1.0f, 0.0f)));

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
        else {
            // Default player movement w/o MainCamera
            //if (input.IsKeyPressed(GLFW_KEY_W)) transform.Position.z -= 0.1f; // move forward
            //if (input.IsKeyPressed(GLFW_KEY_S)) transform.Position.z += 0.1f; // move backward
            //if (input.IsKeyPressed(GLFW_KEY_A)) transform.Position.x -= 0.1f; // move left
            //if (input.IsKeyPressed(GLFW_KEY_D)) transform.Position.x += 0.1f; // move right
        }
    }

    // Editor camera controls
    if (input.IsKeyPressed(GLFW_KEY_LEFT_SHIFT) && editorCamToggle) {

        auto& editorCam = m_Renderer->getEditorCamera();

        // Check for left or right mouse click
        uint32_t mouse = 2;
        if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            mouse = GLFW_MOUSE_BUTTON_LEFT;
        }
        else if (input.IsMouseButtonPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
            mouse = GLFW_MOUSE_BUTTON_RIGHT;
        }

        // Cursor orbiting
        editorCam.cameraOnCursor(input.GetMouseDelta().x, input.GetMouseDelta().y, mouse);

        // Zooming in-and-out scrolling
        double scrollY_offset = input.GetScrollDelta().y;
        if (scrollY_offset != 0) {
            editorCam.cameraOnScroll(scrollY_offset);
        }

        // Check moving input
        auto& camPos = editorCam.getCamPos();
        if (input.IsKeyPressed(GLFW_KEY_W)) {
            editorCam.moveCamForward();
        }
        if (input.IsKeyPressed(GLFW_KEY_A)) {
            editorCam.moveCamLeft();
        }
        if (input.IsKeyPressed(GLFW_KEY_S)) {
            editorCam.moveCamBack();
        }
        if (input.IsKeyPressed(GLFW_KEY_D)) {
            editorCam.moveCamRight();
        }

    }

    // Test the DSP Global Effects

    FMOD::DSP* dsp = nullptr;
    if (input.IsKeyJustPressed(GLFW_KEY_ENTER)) {
        dsp = m_AudioManager->CreateDSP(Engine::DSPEffectType::LowPass, Engine::AudioType::SFX);
        m_AudioManager->SetDSPParameter(Engine::AudioType::SFX, Engine::DSPEffectType::LowPass,
            FMOD_DSP_LOWPASS_CUTOFF, 1000.0); //1kHz = muffled
    }

    if (input.IsKeyJustPressed(GLFW_KEY_LEFT_BRACKET)) {
        m_AudioManager->EnableDSP(Engine::AudioType::SFX, Engine::DSPEffectType::LowPass, true);
    }
    if (input.IsKeyJustPressed(GLFW_KEY_RIGHT_BRACKET)) {
        m_AudioManager->EnableDSP(Engine::AudioType::SFX, Engine::DSPEffectType::LowPass, false);
    }

    if (dsp) {
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
    if (input.IsKeyPressed(GLFW_KEY_W)) {
        LOG_DEBUG("W held - Moving forward");
    }
    if (input.IsKeyPressed(GLFW_KEY_S)) {
        LOG_DEBUG("S held - Moving backward");
    }
    if (input.IsKeyPressed(GLFW_KEY_A)) {
        LOG_DEBUG("A held - Moving left");
    }
    if (input.IsKeyPressed(GLFW_KEY_D)) {
        LOG_DEBUG("D held - Moving right");
    }

    // Action keys - one-time press
    if (input.IsKeyJustPressed(GLFW_KEY_SPACE)) {
        LOG_DEBUG("Space pressed - Jump action!");
    }

    // Mouse buttons
    if (input.IsMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT)) {
        auto mousePos = input.GetMousePosition();
        LOG_DEBUG("Left mouse clicked at: (", mousePos.x, ", ", mousePos.y, ")");

        // Retrieve picked ID and send it to editor
        m_Editor->RetrievePickedID(m_Renderer->getPickedID());
    }
    if (input.IsMouseButtonJustPressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        auto mousePos = input.GetMousePosition();
        LOG_DEBUG("Right mouse clicked at: (", mousePos.x, ", ", mousePos.y, ")");
    }

    // Scroll wheel
    auto scrollDelta = input.GetScrollDelta();
    if (std::abs(scrollDelta.y) > 0.01f) {
        LOG_DEBUG("Mouse scrolled: ", scrollDelta.y > 0 ? "UP" : "DOWN");
    }

    // Function keys
    if (input.IsKeyJustPressed(GLFW_KEY_F1)) {
        bool newVisibility = !input.IsCursorVisible();
        input.SetCursorVisible(newVisibility);
        LOG_INFO("Cursor visibility toggled: ", newVisibility ? "VISIBLE" : "HIDDEN");
    }

    if (input.IsKeyJustPressed(GLFW_KEY_F2)) {
        LOG_INFO("F2 pressed - Creating test entity with velocity...");
        static int entityCounter = 0;

        auto newEntity = m_Scene->CreateEntity("DynamicEntity_" + std::to_string(entityCounter));
        newEntity.AddComponent<Engine::TagComponent>("DynamicEntity_" + std::to_string(entityCounter));

        auto& transform = newEntity.AddComponent<Engine::TransformComponent>();
        transform.Position = glm::vec3(entityCounter * 2.0f, 10.0f, 0);
        transform.Rotation = glm::vec3(0, 0, 0);
        transform.Scale = glm::vec3(1, 1, 1);

        // Add rigidbody with random velocity to demonstrate MovementSystem
        auto& rb = newEntity.AddComponent<Engine::RigidbodyComponent>();
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
    if (input.IsKeyJustPressed(GLFW_KEY_F5)) {
        LOG_INFO("=== SAVING SCENE ===");
        bool success = m_Scene->SaveToFile("Resources/Sources/Scenes/SavedScene.json");
        LOG_INFO(success ? "Scene saved!" : "Save failed!");
    }

    if (input.IsKeyJustPressed(GLFW_KEY_F9)) {
        LOG_INFO("=== LOADING SCENE ===");

        // Shutdown systems before loading new scene
        m_Scene->ShutdownSystems();

        bool success = m_Scene->LoadFromFile("Resources/Sources/Scenes/ExampleScene.json");

        // Reinitialize systems after loading
        if (success) {
            AddAllSystems();
            m_Scene->InitializeSystems();
            LOG_INFO("Scene loaded and systems reinitialized!");
        }
        else {
            LOG_ERROR("Load failed!");
        }
    }

    // === TEST BEHAVIOUR TREE SYSTEM ===

    if (input.IsKeyJustPressed(GLFW_KEY_F5)) {
        LOG_INFO("=== [F5] Create a new Behaviour Tree file: change_position.json ===");

        // Build: Sequence -> SetBlackboard(Key=TargetPosition, Type=Vec3, Value=1,2,3) -> Wait(Duration=2.0)
        auto tree = std::make_shared<Engine::BehaviourTree>();
        tree->SetName("ChangePositionTree");

        auto root = std::make_shared<Engine::BTSequence>();
        tree->SetRootNode(root);

        // Set a position in the blackboard (stored as text; editor/runner can parse as needed)
        auto setPos = std::make_shared<Engine::BTSetBlackboard>("TargetPosition", "1,2,3", "Vec3");
        root->AddChild(setPos);

        // Wait a bit
        auto waitNode = std::make_shared<Engine::BTWait>(2.0f);
        root->AddChild(waitNode);

        // Save to file (project + output)
        const std::string filePath = "change_position.json";
        if (Engine::BehaviourTreeEditor::SaveTree(*tree, filePath)) {
            LOG_INFO("[F5] Successfully created and saved: ", filePath);
        }
        else {
            LOG_ERROR("[F5] Failed to save: ", filePath);
        }
    }

    if (input.IsKeyJustPressed(GLFW_KEY_F6)) {
        LOG_INFO("=== [F6] Rename BT file ===");
        const std::string oldPath = "change_position.json";
        const std::string newPath = "change_position_renamed.json";

        // Pass m_Scene.get() if you want component references auto-updated
        bool ok = Engine::BehaviourTreeEditor::RenameFile(oldPath, newPath, m_Scene.get());
        if (ok) {
            LOG_INFO("[F6] Renamed '", oldPath, "' -> '", newPath, "'");
        }
        else {
            LOG_ERROR("[F6] Rename failed!");
        }
    }

    if (input.IsKeyJustPressed(GLFW_KEY_F7)) {
        LOG_INFO("=== [F7] SaveAs (duplicate file with new GUID) ===");

        const std::string sourcePath = "change_position.json";
        const std::string newPath = "change_position2.json";
        const std::string newName = "ChangePosition_Copy"; // Optional new internal name

        bool success = Engine::BehaviourTreeEditor::SaveAs(
            sourcePath,
            newPath,
            newName,
            /*generateNewGUID=*/true
        );

        if (success) {
            LOG_INFO("[F7] Successfully saved a copy as '", newPath, "' with a new GUID!");
        }
        else {
            LOG_ERROR("[F7] SaveAs failed!");
        }
    }


    //if (input.IsKeyJustPressed(GLFW_KEY_F7)) {
    //    LOG_INFO("=== [F7] SaveAs after editing a value ===");
    //    const std::string sourcePath = "change_position.json";
    //    const std::string outPath = "change_position2.json";

    //    auto tree = Engine::BehaviourTreeEditor::LoadTree(sourcePath);
    //    if (!tree) {
    //        LOG_ERROR("[F7] Could not load source: ", sourcePath);
    //    }
    //    else {
    //        // Find a node to tweak: prefer MoveEntity.Speed, else Wait.Duration
    //        std::function<std::shared_ptr<Engine::BTNode>(std::shared_ptr<Engine::BTNode>, const char*)> findType =
    //            [&](std::shared_ptr<Engine::BTNode> n, const char* typeName) -> std::shared_ptr<Engine::BTNode> {
    //            if (!n) return nullptr;
    //            if (std::string(n->GetTypeName()) == typeName) return n;
    //            for (auto& c : n->GetChildren()) {
    //                if (auto r = findType(c, typeName)) return r;
    //            }
    //            return nullptr;
    //            };

    //        auto root = tree->GetRootNode();
    //        bool changed = false;

    //        if (auto move = findType(root, "MoveEntity")) {
    //            move->SetProperty("Speed", "10.0");
    //            LOG_INFO("[F7] Changed MoveEntity.Speed to 10.0");
    //            changed = true;
    //        }
    //        else if (auto wt = findType(root, "Wait")) {
    //            wt->SetProperty("Duration", "3.0");
    //            LOG_INFO("[F7] Changed Wait.Duration to 3.0");
    //            changed = true;
    //        }
    //        else {
    //            LOG_WARNING("[F7] No MoveEntity or Wait node found; saving copy without edits.");
    //        }

    //        // Save to a new file so the original remains unchanged
    //        if (Engine::BehaviourTreeEditor::SaveTree(*tree, outPath)) {
    //            LOG_INFO("[F7] Saved edited copy as '", outPath, "'");
    //        }
    //        else {
    //            LOG_ERROR("[F7] Save copy failed!");
    //        }
    //    }
    //}



    // F10 -> Create a BehaviourTree, attach to entity, and save to JSON
    // F10 -> Create a BehaviourTree, attach to entity, and save to JSON
    if (input.IsKeyJustPressed(GLFW_KEY_F10)) {
        LOG_INFO("=== [F10] Create Behaviour Tree and attach to entity ===");

        // 1. Create a simple behaviour tree
        //auto tree = std::make_shared<Engine::BehaviourTree>();
        //tree->SetName("SimpleWaitTree");

        // Create nodes
        //auto root = std::make_shared<Engine::BTSequence>();
        //auto waitNode = std::make_shared<Engine::BTWait>(2.0f);
        //auto logNode = std::make_shared<Engine::BTLog>("HELLO");
            //= std::make_shared<Engine::BTAction>([](Engine::BTContext& ctx) {
            //LOG_INFO("[AI Action] Hello from Behaviour Tree!");
            //return Engine::BTStatus::Success;
            //});

        //root->AddChild(waitNode);
        //root->AddChild(logNode);
        //tree->SetRootNode(root);

        auto tree = std::make_shared<Engine::BehaviourTree>();
        tree->SetName("RotationColorChange");

        auto sequence = std::make_shared<Engine::BTSequence>();

        auto parallel = std::make_shared<Engine::BTParallel>();
        parallel->AddChild(std::make_shared<Engine::BTChangeColor>(0));
        parallel->AddChild(std::make_shared<Engine::BTRotateEntity>(30.0));

        auto parallel1 = std::make_shared<Engine::BTParallel>();
        parallel1->AddChild(std::make_shared<Engine::BTChangeColor>(1));
        parallel1->AddChild(std::make_shared<Engine::BTRotateEntity>(30.0));

        sequence->AddChild(parallel);
        sequence->AddChild(parallel1);

        tree->SetRootNode(sequence);

        // 2. Serialize the tree to file
        std::string btPath = "RotationColorChange.json";
        Engine::BehaviourTreeSerializer::SerializeToFile(*tree, btPath);

        // 3. Create entity & attach component
        Engine::Entity aiEntity = m_Scene->CreateEntity("TestAI");
        auto& btComp = aiEntity.AddComponent<Engine::BehaviourTreeComponent>(tree);
        btComp.Active = true;
        btComp.ResetOnComplete = true;

        // Set the file path so it persists to SavedScene.json
        btComp.TreeAssetPath = btPath;

        LOG_INFO("Created AI Entity with BehaviourTreeComponent!");
        LOG_INFO("Tree path set to: ", btComp.TreeAssetPath);
    }

    // F11 -> Create a BehaviourTreePrefab from the tree file
    if (input.IsKeyJustPressed(GLFW_KEY_F11)) {
        LOG_INFO("=== [F11] Create BehaviourTreePrefab from file ===");


        Engine::Entity foundEntity;
        bool found = false;

        auto view = registry.view<Engine::TagComponent>();
        for (auto entityHandle : view) {
            auto& tag = view.get<Engine::TagComponent>(entityHandle);
            if (tag.Tag == "TestAI") { // change to whatever name you want
                foundEntity = Engine::Entity(entityHandle, &registry);
                found = true;
                break;
            }
        }

        if (!foundEntity) {
            LOG_ERROR("No entity named 'TestAI' found. Create one first with F10.");
            return;
        }

        // Make sure it has a valid BehaviourTreeComponent
        if (!foundEntity.HasComponent<Engine::BehaviourTreeComponent>()) {
            LOG_ERROR("Entity 'TestAI' does not have a BehaviourTreeComponent!");
            return;
        }

        // Save the entity itself as a prefab (not just the tree)
        auto prefab = Engine::PrefabSerializer::CreateEntityPrefab(foundEntity, "AIPrefab_SimpleWaitTree");
        Engine::PrefabRegistry::Get().RegisterPrefab(prefab);

        LOG_INFO("Entity Prefab 'AIPrefab_SimpleWaitTree' created and registered.");
    }


    // F12 -> Instantiate entity from BehaviourTreePrefab
    if (input.IsKeyJustPressed(GLFW_KEY_K)) {
        auto prefab = Engine::PrefabRegistry::Get().GetPrefabByName("AIPrefab_SimpleWaitTree");
        if (prefab) {
            Engine::PrefabInstantiator::InstantiateEntityPrefab(m_Scene.get(), prefab->GetGUID());
            LOG_INFO("Instantiated AI entity from prefab 'AIPrefab_SimpleWaitTree'");
        }
        else {
            LOG_WARNING("Prefab 'AIPrefab_SimpleWaitTree' not found!");
        }
    }

    //m_Editor->StartImguiFrame();

    // Update Editor To Do
    //m_Editor->OnUpdate(Engine::Timestep ts);
    //m_Renderer->get_imgui_texture();
    m_Editor->OnUpdate(ts, m_Renderer->get_imgui_texture());
    m_Editor->SetEditorViewport(m_Renderer->getEditorViewport());
    m_TracyProfiler->OnUpdate();
}

void Game::OnShutdown() {
    LOG_INFO("Game shutting down...");

    if (m_Scene) {
        LOG_DEBUG("SHUTTING DOWN SCENE");
        // Shutdown all systems before destroying scene
        m_Scene->ShutdownSystems();
    }

    //============= Audio =============
    if (m_AudioManager) {
        LOG_INFO("Shutting down Audio Manager...");
        try {
            m_AudioManager->Shutdown();
            LOG_INFO("  -> Audio Manager shut down successfully");
        }
        catch (const std::exception& e) {
            LOG_ERROR("  -> Exception while shutting down Audio Manager: ", e.what());
        }
    }

    Engine::MonoScriptEngine::GetInstance().Shutdown();
    LOG_INFO("[Game] Mono shutdown");

    //============= Asset =============
    Engine::RM.shutDown();

    LOG_INFO("Shutting Down Asset");
    Engine::AM.shutDown();

    m_Scene.reset();
    m_AudioManager.reset();
    m_Editor.reset();
    m_TracyProfiler.reset();

    LOG_INFO("Game shutdown complete");
}
