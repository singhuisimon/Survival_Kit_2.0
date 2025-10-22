#pragma once
#include <string>
#include <memory>

#include "../Graphics/Renderer.h"
#include "../Utility/Timestep.h"

// Forward declare GLFW types to avoid including GLFW in header
struct GLFWwindow;

namespace Engine {

    // Forward declarations
    class Input;

    /**
     * @brief Base application class - provides the core framework
     * @details Manages window, game loop timing, input system, and lifecycle hooks.
     *          Inherit from this to create your specific game.
     */
    class Application {
    public:
        Application(const std::string& name = "Engine", int width = 1280, int height = 720);
        virtual ~Application();

        // Delete copy/move (single instance owned by main())
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        /**
         * @brief Start the main application loop
         * @details Runs until window closes or Close() is called
         */
        void Run();

        /**
         * @brief Request to close the application
         */
        void Close();

        /**
         * @brief Get the GLFW window handle
         */
        GLFWwindow* GetWindow() const { return m_Window; }

        /**
         * @brief Get window dimensions
         */
        int GetWidth() const { return m_WindowWidth; }
        int GetHeight() const { return m_WindowHeight; }

        /**
         * @brief Set window dimensions (for internal use by callbacks)
         */
        void SetWindowSize(int width, int height) {
            m_WindowWidth = width;
            m_WindowHeight = height;
        }

        /**
         * @brief Get the Input system
         */
        Input& GetInput() { return *m_Input; }
        const Input& GetInput() const { return *m_Input; }

    protected:
        /**
         * @brief Called once at startup
         * @details Override to initialize your game-specific systems
         */
        virtual void OnInit() {}

        /**
         * @brief Called every frame
         * @param ts Time elapsed since last frame
         * @details Override to implement your game logic
         */
        virtual void OnUpdate(Timestep ts) {}

        /**
         * @brief Called once at shutdown
         * @details Override to cleanup your game-specific systems
         */
        virtual void OnShutdown() {}


    protected:
        // Renderer
        std::unique_ptr<Renderer> m_Renderer;

    private:
        void Init();
        void Shutdown();
        void UpdateWindowTitle(float fps);

        GLFWwindow* m_Window = nullptr;
        bool m_Running = true;
        float m_LastFrameTime = 0.0f;

        std::string m_Name;
        int m_WindowWidth;
        int m_WindowHeight;

        // Input system
        std::unique_ptr<Input> m_Input;

        // FPS tracking
        float m_FpsUpdateTimer = 0.0f;
        int m_FrameCount = 0;
        float m_CurrentFPS = 0.0f;


    };

} // namespace Engine