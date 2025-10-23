#include "Game.h"
#include "Core/Application.h"
#include "Core/Input.h"
#include "Utility/Logger.h"
#include "ECS/Components.h"
#include "Editor/Editor.h"
#include "Serialization/ComponentRegistry.h"
#include "Audio/AudioSystem.h"
#include "Asset/AssetManager.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cmath>

// Adding systems
#include "Graphics/RenderSystem.h"
#include "Transform/TransformSystem.h"

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

    auto config = AM.createDefaultConfig();

    //debug 
    LOG_INFO("Asset Manager Configuration:");
    LOG_INFO("  Source Roots:");
    for (const auto& root : config.sourceRoots) {
        LOG_INFO("    - ", root);
    }
    LOG_INFO("  Descriptor Root: ", config.descriptorRoot);
    LOG_INFO("  Database File: ", config.databaseFile);

    AM.setConfig(config);

    if (AM.startUp() != 0) {
        LOG_ERROR("Failed to initialize Asset Manager!");
        return;
    }
    else {

    LOG_INFO("Performing initial asset scan...");
    AM.scanAndProcess();

    LOG_INFO("Initial asset scan complete - found ",
        AM.db().Count(), " assets");
    }



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

	// Step 2: Create Audio Manager
	LOG_INFO("Step 2: Initializing Audio Manager...");
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

    // Step 3: Create scene
    LOG_INFO("Step 3: Creating scene object...");
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
            m_Editor->OnInit();
            LOG_INFO("Editor initialized successfully.");

        }

        LOG_INFO("  -> Scene created at address: ", (void*)m_Scene.get());
    }
    catch (const std::exception& e) {
        LOG_CRITICAL("  -> Exception while creating scene: ", e.what());
        return;
    }

    // Step 4: Add systems to the scene
    LOG_INFO("Step 4: Adding systems to scene...");
    try {
        // TODO: Add more systems here as they're created by team members:
        // m_Scene->AddSystem<Engine::PhysicsSystem>();
        // m_Scene->AddSystem<Engine::RenderSystem>(GetWidth(), GetHeight());
        m_Scene->AddSystem<Engine::AudioSystem>(m_AudioManager.get());

        m_Scene->AddSystem<Engine::TransformSystem>();
        m_Scene->AddSystem<Engine::RenderSystem>(*m_Renderer);
       
        LOG_INFO("  -> Systems added successfully");
    }
    catch (const std::exception& e) {
        LOG_ERROR("  -> Exception while adding systems: ", e.what());
    }

    // Step 5: Initialize all systems
    LOG_INFO("Step 5: Initializing systems...");
    try {
        m_Scene->InitializeSystems();
        LOG_INFO("  -> Systems initialized successfully");
    }
    catch (const std::exception& e) {
        LOG_ERROR("  -> Exception while initializing systems: ", e.what());
    }

    // Step 6: Load scene from file or create default
    LOG_INFO("Step 6: Loading scene content...");
    bool loadedFromFile = false;

    try {
        loadedFromFile = m_Scene->LoadFromFile("Resources/Sources/Scenes/ExampleScene.json");

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

    auto& rb = player.AddComponent<Engine::RigidbodyComponent>();
    rb.Mass = 1.0f;
    rb.UseGravity = true;
    rb.IsKinematic = false;
    rb.Velocity = glm::vec3(0, 0, 0);  // Will fall due to gravity

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

    player.AddComponent<Engine::MeshRendererComponent>();
    LOG_TRACE("  -> Player created (will fall and demonstrate MovementSystem)");

    LOG_TRACE("  Creating Camera entity...");
    auto camera = m_Scene->CreateEntity("Camera");
    camera.AddComponent<Engine::TagComponent>("MainCamera");

    auto& camTransform = camera.AddComponent<Engine::TransformComponent>();
    camTransform.Position = glm::vec3(0, 2, 5);
    camTransform.Rotation = glm::vec3(-15, 0, 0);
    camTransform.Scale = glm::vec3(1, 1, 1);

    auto& camComponent = camera.AddComponent<Engine::CameraComponent>();
    camComponent.Primary = true;
    camComponent.FOV = 60.0f;
    camComponent.NearClip = 0.1f;
    camComponent.FarClip = 1000.0f;
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
            m_Scene->InitializeSystems();
            LOG_INFO("Scene loaded and systems reinitialized!");
        }
        else {
            LOG_ERROR("Load failed!");
        }
    }

    //m_Editor->StartImguiFrame();

    // Update Editor To Do
    //m_Editor->OnUpdate(Engine::Timestep ts);
    m_Editor->OnUpdate(ts);
    m_Editor->RenderEditor();
}

void Game::OnShutdown() {
    LOG_INFO("Game shutting down...");

    if (m_Scene) {
        // Shutdown all systems before destroying scene
        m_Scene->ShutdownSystems();
    }

    //============= Audio =============
    LOG_INFO("Shutting down Audio Manager...");
    try {
		m_AudioManager->Shutdown();
        LOG_INFO("  -> Audio Manager shut down successfully");
    }
    catch (const std::exception& e) {
        LOG_ERROR("  -> Exception while shutting down Audio Manager: ", e.what());
    }

    //============= Asset =============
    LOG_INFO("Shutting Down Asset");
    AM.shutDown();

    m_Scene.reset();
    m_Editor.reset();
    LOG_INFO("Game shutdown complete");
}