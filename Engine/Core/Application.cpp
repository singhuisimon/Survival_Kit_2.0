#include "Application.h"
#include "Input.h"
#include "Utility/Logger.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tracy/Tracy.hpp>
#include <sstream>
#include <iomanip>

namespace Engine {

    static void GLFWErrorCallback(int error, const char* description) {
        LOG_ERROR("GLFW Error (", error, "): ", description);
    }

    static void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);

        Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
        if (app) {
            app->SetWindowSize(width, height);
        }
    }

    Application::Application(const std::string& name, int width, int height)
        : m_Name(name)
        , m_WindowWidth(width)
        , m_WindowHeight(height) {
        Init();
    }

    Application::~Application() {
        Shutdown();
    }

    void Application::Init() {
        LOG_INFO("===========================================");
        LOG_INFO("  ", m_Name);
        LOG_INFO("===========================================");

        // Initialize GLFW
        glfwSetErrorCallback(GLFWErrorCallback);

        if (!glfwInit()) {
            LOG_CRITICAL("Failed to initialize GLFW!");
            return;
        }

        LOG_INFO("GLFW initialized");

        // Create window
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

        m_Window = glfwCreateWindow(m_WindowWidth, m_WindowHeight, m_Name.c_str(), nullptr, nullptr);

        if (!m_Window) {
            LOG_CRITICAL("Failed to create window!");
            glfwTerminate();
            return;
        }

        glfwSetWindowUserPointer(m_Window, this);
        glfwMakeContextCurrent(m_Window);
        glfwSetFramebufferSizeCallback(m_Window, FramebufferSizeCallback);
        glfwSwapInterval(1); // VSync

        // Initialize Renderer
        m_Renderer = std::make_unique<Renderer>(m_Editor_camera, m_Editor_light);
        m_Renderer->setup();
        LOG_INFO("Renderer setup initialized");

        glViewport(0, 0, m_WindowWidth, m_WindowHeight);
        glEnable(GL_DEPTH_TEST);

        // Initialize Input system
        m_Input = std::make_unique<Input>();
        m_Input->Init(m_Window);
        LOG_INFO("Input system initialized");

        // DO NOT call OnInit() here - it will be called in Run() instead!

        LOG_INFO("Application initialized successfully");
    }

    void Application::Run() {
        LOG_INFO("Starting application...");

        // NOW call OnInit() after the derived class is fully constructed
        LOG_INFO("Calling OnInit()...");
        OnInit();
        LOG_INFO("OnInit() completed");

        LOG_INFO("Press ESC to exit");

        m_LastFrameTime = (float)glfwGetTime();

        while (m_Running && !glfwWindowShouldClose(m_Window)) {
            ZoneScoped;
            FrameMark;

            // Calculate delta time
            float time = (float)glfwGetTime();
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            // Update FPS counter and window title
            m_FrameCount++;
            m_FpsUpdateTimer += timestep;

            if (m_FpsUpdateTimer >= 0.25f) {
                m_CurrentFPS = m_FrameCount / m_FpsUpdateTimer;
                UpdateWindowTitle(m_CurrentFPS);

                m_FrameCount = 0;
                m_FpsUpdateTimer = 0.0f;
            }

            // Poll events first to get latest input
            {
                ZoneScopedN("Events");
                glfwPollEvents();
            }

            // Update
            {
                ZoneScopedN("Update");

                // Update input system (after events are polled)
                m_Input->Update();

                // Then update game
                OnUpdate(timestep);
            }

            // Swap buffers
            {
                ZoneScopedN("Render");
                glfwSwapBuffers(m_Window);
            }

            // Check for ESC key
            if (m_Input->IsKeyPressed(GLFW_KEY_ESCAPE)) {
                LOG_INFO("ESC pressed - closing application");
                Close();
            }
        }

        LOG_INFO("Application loop ended");
    }

    void Application::Close() {
        m_Running = false;
    }

    void Application::UpdateWindowTitle(float fps) {
        std::stringstream ss;
        ss << m_Name << " | FPS: " << std::fixed << std::setprecision(1) << fps;
        glfwSetWindowTitle(m_Window, ss.str().c_str());
    }

    void Application::Shutdown() {
        LOG_INFO("Shutting down application...");

        OnShutdown();

        // Cleanup Input system
        m_Input.reset();

        if (m_Window) {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
        }

        glfwTerminate();

        LOG_INFO("Application shutdown complete");
    }

} // namespace Engine