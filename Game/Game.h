#pragma once
#include "Core/Application.h"
#include "ECS/Scene.h"
#include "Utility/Logger.h"
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
    std::unique_ptr<Engine::Scene> m_Scene;
    float m_ColorShift = 0.0f;

    /**
     * @brief Create a default scene if loading from file fails
     */
    void CreateDefaultScene();
};