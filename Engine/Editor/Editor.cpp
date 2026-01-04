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
#include "../Component/MeshRendererComponent.h"
#include "../Component/TransformComponent.h"
#include "../Graphics/Renderer.h"

// Include other necessary 
#include <GLFW/glfw3.h>
#include <cctype>
#include <ImGuizmo.h>


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


        //ImGui::SetWindowFocus();

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

            // Save editor viewport data for OBJECT PICKING
            editorViewportData.tl = tl_client;
            editorViewportData.size = viewportSize;

            // Sync with renderer 
            if (m_Renderer)
            {
                m_Renderer->getEditorViewport() = editorViewportData;
            }

            // ==================== GIZMO INTEGRATION ====================
            m_ImGuizmoViewportData.tl = tl_screen;
            m_ImGuizmoViewportData.size = actualSize;


            if (m_Renderer && m_Renderer->getEditorCamToggle())
            {
                HandleGizmoPicked();
            }
        }
        ImGui::End();
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
        std::cout << "=== CreateNewScene called ===" << std::endl;
        std::cout << "Before clearing - m_CurrentScenePath: " << m_CurrentScenePath << std::endl;
        SetCurrSelectedEntity(Entity{});
        RetrievePickedID(0xFFFFFFFFu);
        std::cout << "After clearing selection" << std::endl;

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
        std::cout << "After clearing - m_CurrentScenePath: " << m_CurrentScenePath << std::endl;
        std::cout << "=== CreateNewScene End ===" << std::endl;
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

    void Editor::HandleGizmoPicked()
    {
        static Entity doubleClickCandidate;
        static uint32_t doubleClickCandidateID = 0xFFFFFFFFu;
        static double lastClickTime = 0.0;
        static const double DOUBLE_CLICK_TIME = 0.3;

        if (ImGui::IsItemHovered())
        {


            if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                if (doubleClickCandidate && doubleClickCandidateID != 0xFFFFFFFFu)
                {
                    std::cout << "DOUBLE-CLICK teleport to ID: " << doubleClickCandidateID << "\n";
                    ViewportClickAndTeleport();

                }
            }
            else
            {
                std::cout << "CLICK DETECTED!" << std::endl;
                bool gizmoUsing = ImGuizmo::IsUsing();
                bool gizmoOver = ImGuizmo::IsOver();

                // force selection regardless of ImGuizmo state
                if (!m_SelectedEntity || (!gizmoUsing && !gizmoOver))
                {
                    if (m_PickedID != 0xFFFFFFFFu && m_ActiveScene)
                    {
                        Entity newEntity = Entity{ (entt::entity)m_PickedID, &m_ActiveScene->GetRegistry() };
                        m_SelectedEntity = newEntity;
                        SetCurrSelectedEntity(m_SelectedEntity);
                        //m_Operation = static_cast<ImGuizmo::OPERATION>(-1);

                        doubleClickCandidate = newEntity;
                        doubleClickCandidateID = m_PickedID;

                    }
                    else
                    {
                        m_SelectedEntity = Entity{};
                        SetCurrSelectedEntity(m_SelectedEntity);
                        doubleClickCandidate = Entity{};
                        doubleClickCandidateID = 0xFFFFFFFFu;
                        std::cout << "[GIZMO] Deselected" << std::endl;
                    }
                }
            }
        }
        
     
        if (m_SelectedEntity)
        {
            ManipulateEntityTransform(m_SelectedEntity);
        }

        // Handle right-click context menu for gizmo operations
        if (ImGui::BeginPopupContextWindow("GizmoContextMenu", ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Move", "W"))
                m_Operation = ImGuizmo::TRANSLATE;
            if (ImGui::MenuItem("Rotate", "E"))
                m_Operation = ImGuizmo::ROTATE;
            if (ImGui::MenuItem("Scale", "R"))
                m_Operation = ImGuizmo::SCALE;
            ImGui::Separator();
            if (ImGui::MenuItem("Disable", "Q"))
                m_Operation = static_cast<ImGuizmo::OPERATION>(-1);
            ImGui::EndPopup();
        }

        // Handle gizmo manipulation for selected entity
        if (ImGui::IsWindowFocused())
        {
            if (ImGui::IsKeyPressed(ImGuiKey_W))
            {
                m_Operation = ImGuizmo::TRANSLATE;
                std::cout << "GIZMO: TRANSLATE mode activated" << std::endl;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_E))
            {
                m_Operation = ImGuizmo::ROTATE;
                std::cout << "GIZMO: ROTATE mode activated" << std::endl;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_R))
            {
                m_Operation = ImGuizmo::SCALE;
                std::cout << "GIZMO: SCALE mode activated" << std::endl;
            }
            if (ImGui::IsKeyPressed(ImGuiKey_Q))
            {
                m_Operation = static_cast<ImGuizmo::OPERATION>(-1);
                std::cout << "GIZMO: DISABLED" << std::endl;
            }
        }
        

        //std::cout << "=== FRAME END ===\n" << std::endl;
    }

    void Editor::ManipulateEntityTransform(Entity& entity)
    {
        if (!entity || !m_Renderer || !entity.HasComponent<TransformComponent>()) return;

        // Get the camera
        Camera3D& camera = m_Renderer->getEditorCamera();

        // Get the transform component
        auto& tc = entity.GetComponent<TransformComponent>();

        // Build transform matrix from position, rotation, and scale
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), tc.Position);
        transform = transform * glm::mat4_cast(tc.Rotation);
        transform = glm::scale(transform, tc.Scale);

        // Set up ImGuizmo
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist(ImGui::GetWindowDrawList());

        // Use the stored viewport coordinates
        float x = m_ImGuizmoViewportData.tl.x;
        float y = m_ImGuizmoViewportData.tl.y;
        float width = m_ImGuizmoViewportData.size.x;
        float height = m_ImGuizmoViewportData.size.y;
        ImGuizmo::SetRect(x, y, width, height);

        // Get camera matrices
        float aspect_ratio = (height > 0) ? (width / height) : 1.0f;
        glm::mat4 view = camera.getLookAt();
        glm::mat4 proj = camera.getPerspective(aspect_ratio);

        // Only manipulate if an operation is selected
        if (m_Operation != (ImGuizmo::OPERATION)-1)
        {
            // Perform the manipulation
            ImGuizmo::Manipulate(
                glm::value_ptr(view),
                glm::value_ptr(proj),
                m_Operation,
                ImGuizmo::WORLD,
                glm::value_ptr(transform)
            );

            // Apply the changes if the gizmo is being used
            if (ImGuizmo::IsUsing())
            {
                if (m_Operation == ImGuizmo::TRANSLATE)
                {
                    // Extract and set new position
                    glm::vec3 newPosition = glm::vec3(transform[3]);
                    tc.SetPosition(newPosition);
                }
                else if (m_Operation == ImGuizmo::ROTATE)
                {
                    // Extract rotation matrix and convert to quaternion
                    glm::mat3 rotationMatrix;
                    rotationMatrix[0] = glm::normalize(glm::vec3(transform[0]));
                    rotationMatrix[1] = glm::normalize(glm::vec3(transform[1]));
                    rotationMatrix[2] = glm::normalize(glm::vec3(transform[2]));

                    glm::quat newRotation = glm::quat_cast(rotationMatrix);
                    tc.Rotation = newRotation;
                    tc.IsDirty = true;
                }
                else if (m_Operation == ImGuizmo::SCALE)
                {
                    // Extract scale from matrix columns
                    glm::vec3 newScale;
                    newScale.x = glm::length(glm::vec3(transform[0]));
                    newScale.y = glm::length(glm::vec3(transform[1]));
                    newScale.z = glm::length(glm::vec3(transform[2]));

                    tc.SetScale(newScale);
                }
            }
        }
    }
#if 0
    void Editor::ViewportClickAndTeleport()
    {
        if (!m_SelectedEntity.HasComponent<TransformComponent>()) return;

        TransformComponent& targetCamPos = m_SelectedEntity.GetComponent<TransformComponent>();
        Camera3D& editorCam = m_Renderer->getEditorCamera();

        glm::vec3 entityPos = targetCamPos.Position;
        float offsetDistance = 5.f;
        if (m_SelectedEntity.HasComponent<MeshRendererComponent>()) {
            glm::vec3 scale = targetCamPos.Scale;
            float maxScale = glm::max<float>(glm::max<float>(scale.x, scale.y), scale.z);
            offsetDistance = maxScale * 1.5f;
        }

        glm::vec3 cameraPos = editorCam.getCamPos();
        glm::vec3 cameraTarget = editorCam.getEditorCamTarget();

        glm::vec3 viewDir = cameraTarget - cameraPos;
        if (glm::dot(viewDir, viewDir) < 1e-8f) {

            viewDir = glm::vec3(0.0f, 0.0f, -1.0f);
        }
        else {
            viewDir = glm::normalize(viewDir);
        }

        glm::vec3 newCamPos = entityPos - viewDir * offsetDistance;

        editorCam.setEditorCamPosition(newCamPos);
        editorCam.setEditorCamTarget(entityPos);
        LOG_DEBUG("Moved to Clicked ID {", newCamPos.x, ", ", newCamPos.y, ", ", newCamPos.z, "}");
        //}

    }
#endif
    void Editor::ViewportClickAndTeleport()
    {
        if (!m_SelectedEntity.HasComponent<TransformComponent>()) return;

        TransformComponent& targetTransform = m_SelectedEntity.GetComponent<TransformComponent>();
        Camera3D& editorCam = m_Renderer->getEditorCamera();

        glm::vec3 entityPos = targetTransform.Position;
        glm::quat entityRotation = targetTransform.Rotation; // Quaternion

        // Calculate offset distance based on entity size
        float offsetDistance = 5.0f;
        if (m_SelectedEntity.HasComponent<MeshRendererComponent>()) {
            glm::vec3 scale = targetTransform.Scale;
            float maxScale = glm::max<float>(glm::max<float>(scale.x, scale.y), scale.z);

            float minDistance = 3.0f;
            float maxDistance = 15.0f;
            offsetDistance = glm::clamp(maxScale * 1.5f, minDistance, maxDistance);
        }

        glm::vec3 entityForward = glm::normalize(entityRotation * glm::vec3(0.0f, 0.0f, -1.0f));

        // Position camera in front of entity 
        glm::vec3 cameraOffset = -entityForward * offsetDistance + glm::vec3(0.0f, offsetDistance * 0.15f, 0.0f);
        glm::vec3 newCamPos = entityPos + cameraOffset;

        editorCam.setEditorCamPosition(newCamPos);
        editorCam.setEditorCamTarget(entityPos);

        LOG_DEBUG("Camera focused on entity front at {", newCamPos.x, ", ", newCamPos.y, ", ", newCamPos.z, "}");
    }
}

