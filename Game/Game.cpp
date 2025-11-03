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
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

#include "Component/TagComponent.h"
#include "Component/TransformComponent.h"
#include "Component/CameraComponent.h"
#include "Component/MeshRendererComponent.h"
#include "Component/RigidbodyComponent.h"

// Adding systems
#include "Graphics/RenderSystem.h"
#include "Graphics/CameraSystem.h"
#include "Transform/TransformSystem.h"
#include "Physics/PhysicsSystem.h"
#include "BehaviourTree/BehaviourTreeSystem.h"

// KEPT FOR BT TEST <WILL REMOVE BY THIS WEEKEND PLS DUN TOUCH>
#include "BehaviourTree/BTNodeRegistry.h"
#include "Serialization/BehaviourTreeSerializer.h"
#include "Prefab/BehaviourTreePrefab.h"
#include "Serialization/PrefabSerializer.h"
#include "Serialization/PrefabInstantiator.h"
#include "Prefab/PrefabRegistry.h"
#include "BehaviourTree/BTNode.h"

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
    m_Scene->AddSystem<Engine::RenderSystem>(*m_Renderer);
    m_Scene->AddSystem<Engine::BehaviourTreeSystem>();
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
    transform.Scale    = glm::vec3(1.f, 1.f, 1.f);

    auto& mesh = player.AddComponent<Engine::MeshRendererComponent>();
    mesh.ComponentGUID = Engine::AM.getAssetIdByFilename("E005_loveletter_v001.fbx");

    //auto& rb = player.AddComponent<Engine::RigidbodyComponent>();
    //rb.Mass = 1.0f;
    //rb.UseGravity = true;
    //rb.IsKinematic = false;
    //rb.Velocity = glm::vec3(0, 0, 0);  // Will fall due to gravity

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
    auto camera = m_Scene->CreateEntity("Camera");
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
    groundTransform.Scale = glm::vec3(1, 0.1f, 1);

    auto& groundRb = ground.AddComponent<Engine::RigidbodyComponent>();
    groundRb.Mass = 0.0f;
    groundRb.IsKinematic = true;
    groundRb.UseGravity = false;
    groundRb.Velocity = glm::vec3(0, 0, 0);

    ground.AddComponent<Engine::MeshRendererComponent>();
    LOG_TRACE("  -> Ground created");

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

    // Update scene (this will call all systems in priority order)
    m_Scene->OnUpdate(ts);

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
        m_AudioManager->SetGroupVolume(Engine::AudioType::SFX, volume-0.1f);
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

    if (found && foundEntity.HasComponent<Engine::TransformComponent>()) {

        auto& transform = foundEntity.GetComponent<Engine::TransformComponent>();

        if (input.IsKeyPressed(GLFW_KEY_W)) transform.Position.z -= 0.1f; // move forward
        if (input.IsKeyPressed(GLFW_KEY_S)) transform.Position.z += 0.1f; // move backward
        if (input.IsKeyPressed(GLFW_KEY_A)) transform.Position.x -= 0.1f; // move left
        if (input.IsKeyPressed(GLFW_KEY_D)) transform.Position.x += 0.1f; // move right
    }

    // Change Camera type
    if (input.IsKeyJustPressed(GLFW_KEY_TAB)) {

        auto& editorCam = m_Renderer->getEditorCamera();
        if (editorCam.getCamType() == Engine::CameraType::ORBITING) {
            editorCam.setCamType(Engine::CameraType::WALKING);
        }
        else {
            editorCam.setCamType(Engine::CameraType::ORBITING);
        }

    }

    // Editor camera controls
    if (input.IsKeyPressed(GLFW_KEY_LEFT_SHIFT)) {
        
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
    if (found && foundEntity.HasComponent<Engine::TransformComponent>()) {
        auto& tf = foundEntity.GetComponent<Engine::TransformComponent>();
        if (input.IsKeyPressed(GLFW_KEY_Q)) tf.Position.y += 0.05f;
        if (input.IsKeyPressed(GLFW_KEY_E)) tf.Position.y -= 0.05f;
    }
    
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

    // F10 -> Create a BehaviourTree, attach to entity, and save to JSON
    // F10 -> Create a BehaviourTree, attach to entity, and save to JSON
    if (input.IsKeyJustPressed(GLFW_KEY_F10)) {
        LOG_INFO("=== [F10] Create Behaviour Tree and attach to entity ===");

        // 1. Create a simple behaviour tree
        auto tree = std::make_shared<Engine::BehaviourTree>();
        tree->SetName("SimpleWaitTree");

        // Create nodes
        auto root = std::make_shared<Engine::BTSequence>();
        auto waitNode = std::make_shared<Engine::BTWait>(2.0f);
        auto logNode = std::make_shared<Engine::BTLog>("HELLO");
            //= std::make_shared<Engine::BTAction>([](Engine::BTContext& ctx) {
            //LOG_INFO("[AI Action] Hello from Behaviour Tree!");
            //return Engine::BTStatus::Success;
            //});

        root->AddChild(waitNode);
        root->AddChild(logNode);
        tree->SetRootNode(root);

        // 2. Serialize the tree to file
        std::string btPath = "SimpleWaitTree.json";
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