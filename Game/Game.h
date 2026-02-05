#pragma once
#include "Editor/Editor.h"
#include "Core/Application.h"
#include "ECS/Scene.h"
#include "Audio/AudioManager.h"

#include "Utility/Logger.h"
#include "Profiler/Profiler.h"
#include <memory>

/**
 * @brief Your specific game implementation
 * @details This is where you write YOUR game code using the engine
 */
class Game : public Engine::Application {
public:
    Game();

    ~Game() override {
        Engine::Logger::Get().Info("Game destructor called");
    }

    Engine::AudioManager* GetAudioManager() { return m_AudioManager.get(); }

    Engine::Scene* CreateScene(const std::string& name);

    void RequestNewSceneFromEditor(const std::string& name);
protected:
    /**
     * @brief Initialize your game
     * @details Create scenes, spawn entities, load assets
     */
    void OnInit() override;

    /**
     * @brief Update your game logic every frame
     * @param ts Time elapsed since last frame
     */
    void OnUpdate(Engine::Timestep ts) override;

    /**
     * @brief Cleanup your game resources
     */
    void OnShutdown() override;

private:
    std::vector<std::unique_ptr<Engine::Scene>> m_Scenes;
    Engine::Scene* m_ActiveScene = nullptr;
    // Editor 

    std::unique_ptr<Engine::Editor> m_Editor; 
    std::shared_ptr<Engine::TracyProfiler> m_TracyProfiler;
    float m_ColorShift = 0.0f;
    bool m_EditorEnable = false;
    bool m_EditorJustPaused = false;
    bool m_IsFirstPausedFrame = true;

	std::unique_ptr<Engine::AudioManager> m_AudioManager;
	std::string m_CurrentScenePath;

    void AddAllSystems();
    void AddAllSystemsToScene(Engine::Scene* scene);

    /**
     * @brief Create a default scene if loading from file fails
     */
    void CreateDefaultScene();

    /**
	* @brief Load a scene from the given file path
    */
    void LoadSceneFromEvent(const std::string& scenePath);
};