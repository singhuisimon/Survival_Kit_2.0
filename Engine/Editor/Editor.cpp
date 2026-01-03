/**
* @file Editor.cpp
* @brief Implementation of the functions of Editor class for running the IMGUI level editor.
* @author Liliana Hanawardani (47%), Saw Hui Shan (47%), Rio Shannon Yvon Leonardo (2%), Tan Jun Rui (2%), Wai Lwin Thit (2%)
* @date September 8, 2025
* Copyright (C) 2025 DigiPen Institute of Technology.
* Reproduction or disclosure of this file or its contents without the
* prior written consent of DigiPen Institute of Technology is prohibited.
*/


// Include Header Files
#include "Editor.h"
#include "../Game/Game.h" 

// Include other necessary 
#include <GLFW/glfw3.h>
#include <cctype>

namespace Engine
{
    void Editor::OnInit()
    {
        if (m_Initialized)
        {
            LOG_INFO("Editor: Editor Already Initialized.");
            return;
        }
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        io = &ImGui::GetIO(); (void)io;

        io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io->ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
        io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
        io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup scaling
        ImGuiStyle& style = ImGui::GetStyle();

        // Set WindowRounding and ImGuiCol_WindowBg when viewport is enabled
        if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;
        }

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForOpenGL(m_Window, true);
        ImGui_ImplOpenGL3_Init("#version 410");

        // Set default pickedID
        m_PickedID = 0xFFFFFFFFu;

        m_Initialized = true;
    }

    void Editor::OnUpdate(Timestep ts, GLuint texhandle)
    {
        if (!m_Initialized) return;

        StartImguiFrame();

        // Enable Docking Function
        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        m_EditorMenu->EditorTopMenu();

        m_EditorHierarchy->HierarchyPanel();
        
        m_EditorProperty->PropertyPanel();

        m_EditorPerformance->PerformanceProfilePanel(ts);

        RenderViewport(texhandle);

        CompleteFrame();

        m_EditorHierarchy->DeleteEntityTree(m_ActiveScene);

    }

    void Editor::StartImguiFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }

    void Editor::RenderViewport(GLuint texhandle)
    {
        ImVec2 texture_pos = ImGui::GetCursorScreenPos();

        ImVec2 viewportSize = { 600, 600 };
        if (m_Window)
        {
            int width = 0;
            int height = 0;
            glfwGetWindowSize(m_Window, &width, &height);
            viewportSize = {
                static_cast<float>(width) / 2.0f,
                static_cast<float>(height) / 2.0f
            };
        }

        ImGui::Begin("Viewport");
        if (texhandle)
        {
            ImVec2 imagePos = ImGui::GetCursorScreenPos();
            ImGui::Image((ImTextureID)(intptr_t)texhandle, viewportSize, ImVec2(0, 1), ImVec2(1, 0));
            ImVec2 tl_screen = ImGui::GetItemRectMin();    // Top left of image wrt SCREEN space
            ImVec2 actualSize = ImGui::GetItemRectSize();  // Get ACTUAL rendered size

            ImGuiViewport* vp = ImGui::GetWindowViewport();
            // Convert to CLIENT-WINDOW coords (origin = top-left of that GLFW window's content area)
            ImVec2 tl_client;
            if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                tl_client = { tl_screen.x - vp->Pos.x, tl_screen.y - vp->Pos.y }; // subtract OS window's top-left in screen coords
            }
            else
            {
                // Single viewport: ImGui "screen" origin coincides with your main client window
                tl_client = tl_screen;
            }

            // Save editor viewport data for OBJECT PICKING (client coordinates)
            editorViewportData.tl = tl_client;
            editorViewportData.size = viewportSize;

            // Sync with renderer using the existing getEditorViewport() method
            if (m_Renderer)
            {
                m_Renderer->getEditorViewport() = editorViewportData;
            }
            ImGui::End();
        
        }
    }

    void Editor::CompleteFrame()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Update and Render additional Platform Windows
        if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        }
    }

    Scene* Editor::CreateNewScene(const std::string& name)
    {
        if (!m_Game)
        {
            LOG_ERROR("Editor::CreateNewScene failed: Game is null");
            return nullptr;
        }

        Scene* newScene = m_Game->CreateScene(name);
        if (!newScene)
        {
            LOG_ERROR("Editor::CreateNewScene failed: Scene creation failed");
            return nullptr;
        }
        
        SetActiveScene(newScene);
        
        return newScene;
    }

    void Editor::SetActiveScene(Scene* scene)
    {
        m_ActiveScene = scene;
    }

    void Editor::SetSceneName(const std::string& name)
    {
        m_CurrentSceneName = name;

        // Also update the active scene's name if it exists
        if (m_ActiveScene)
        {
            m_ActiveScene->SetName(name);
        }

        LOG_INFO("Scene name set to: ", name);
    }

    std::string Editor::GetSceneName() const
    {
        if (!m_CurrentSceneName.empty())
        {
            return m_CurrentSceneName;
        }

        if (m_ActiveScene)
        {
            return m_ActiveScene->GetName();
        }

        if (!m_CurrentScenePath.empty())
        {
            std::filesystem::path p(m_CurrentScenePath);
            return p.stem().string();  
        }

        // Default
        return "Untitled Scene";
    }

    std::vector<Editor::AssetEntry> Editor::getAssetsInFolder(const std::string& folderPath)
    {
        std::vector<AssetEntry> entries;

        if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath))
            return entries;

        for (const auto& entry : std::filesystem::directory_iterator(folderPath))
        {
            entries.push_back({
                entry.path().filename().string(),
                entry.path().generic_string(),
                });
        }
        return entries;
    }

    void Editor::SaveActiveSceneToPath(const std::string& path)
    {
        Scene* scene = GetActiveScene();
        Renderer* renderer = GetRenderer();

        if (!scene || !renderer)
        {
            LOG_ERROR("SaveActiveSceneToPath failed: invalid state");
            return;
        }

        std::string finalPath = path;

        if (!std::filesystem::path(finalPath).has_extension())
            finalPath += ".json";

        // Sync renderer settings scene
        auto& settings = scene->GetSceneSetting();
        settings.s_BloomToggle = renderer->getBloomToggle();
        settings.s_BloomStrength = renderer->getBloomStrength();
        settings.s_BloomFilterRadius = renderer->getBloomFilterRadius();
        settings.s_Exposure = renderer->getExposure();

        scene->SaveToFile(finalPath);
        scene->SaveToFile(convertAssetPathToRootResources(finalPath));

        scene->SetName(std::filesystem::path(finalPath).stem().string());
        SetScenePath(finalPath);

        LOG_INFO("Scene saved to: ", finalPath);
    }
}

