#include "EditorViewportPanel.h"
#include "Editor.h"

#include <algorithm>
#include <ImGuizmo.h>

namespace Engine
{
    void EditorViewportPanel::ManipulateEntityTransform(Entity& entity, EditorViewport m_ImGuizmoViewportData)
    {
        if (m_PlayState != PlayState::STOP) return;
        Renderer* m_Renderer = m_Editor->GetRenderer();
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

    void EditorViewportPanel::HandleGizmoPicked(EditorViewport m_ImGuizmoViewportData)
    {
        if (m_PlayState != PlayState::STOP) return;
        Entity m_SelectedEntity = m_Editor->GetSelectedEntity();
        u32 m_PickedID = m_Editor->GetPickedID();
        Scene* m_ActiveScene = m_Editor->GetActiveScene();
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
                bool gizmoUsing = ImGuizmo::IsUsing();
                bool gizmoOver = ImGuizmo::IsOver();
                std::cout << "CLICK DETECTED!" << std::endl;

                // force selection regardless of ImGuizmo state
                if (!m_SelectedEntity || (!gizmoUsing && !gizmoOver))
                {
                    if (m_PickedID != 0xFFFFFFFFu && m_ActiveScene)
                    {
                        Entity newEntity = Entity{ (entt::entity)m_PickedID, &m_ActiveScene->GetRegistry() };
                        m_SelectedEntity = newEntity;
                        m_Editor->SetCurrSelectedEntity(m_SelectedEntity);
                        //m_Operation = static_cast<ImGuizmo::OPERATION>(-1);

                        doubleClickCandidate = newEntity;
                        doubleClickCandidateID = m_PickedID;

                    }
                    else
                    {
                        m_SelectedEntity = Entity{};
                        m_Editor->SetCurrSelectedEntity(m_SelectedEntity);
                        doubleClickCandidate = Entity{};
                        doubleClickCandidateID = 0xFFFFFFFFu;
                        std::cout << "[GIZMO] Deselected" << std::endl;
                    }
                }
            }
        }


        if (m_SelectedEntity)
        {
            //ManipulateEntityTransform(m_SelectedEntity);
            ManipulateEntityTransform(m_SelectedEntity, m_ImGuizmoViewportData);
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

    void EditorViewportPanel::ViewportClickAndTeleport()
    {
        if (m_PlayState != PlayState::STOP) return;
        Entity m_SelectedEntity = m_Editor->GetSelectedEntity();
        Renderer* m_Renderer = m_Editor->GetRenderer();

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
    
    void EditorViewportPanel::Play()
    {
        // ONLY allow Play from STOP state
        if (m_PlayState == PlayState::STOP)
        {
            // Store the original scene state before playing
            Scene* activeScene = m_Editor->GetActiveScene();
            if (activeScene && m_Editor->HasScenePath())
            {
                m_OriginalScenePath = m_Editor->GetScenePath();
                m_OriginalSceneName = m_Editor->GetSceneName();
                std::cout << "[VIEWPORT] Saved scene state: " << m_OriginalSceneName << std::endl;
            }

            m_PlayState = PlayState::PLAY;
            m_Editor->SetCurrSelectedEntity(Entity{});
            m_Editor->RetrievePickedID(0xFFFFFFFFu);
            std::cout << "[VIEWPORT] State changed: STOP to PLAY" << std::endl;
        }
        else
        {
            std::cout << "[VIEWPORT] Play() ignored - not in STOP state" << std::endl;
        }
    }

    void EditorViewportPanel::Pause()
    {
        if (m_PlayState == PlayState::PLAY)
        {
            m_PlayState = PlayState::PAUSE;
            std::cout << "[VIEWPORT] State changed: PLAY to PAUSE" << std::endl;
        }
        else if (m_PlayState == PlayState::PAUSE)
        {
            // Resume from pause
            m_PlayState = PlayState::PLAY;
            std::cout << "[VIEWPORT] State changed: PAUSE to PLAY (Resume)" << std::endl;
        }
        else
        {
            std::cout << "[VIEWPORT] Pause() ignored - not in PLAY or PAUSE state" << std::endl;
        }
    }

    void EditorViewportPanel::Stop()
    {
        // ONLY allow Stop from PLAY or PAUSE states
        if (m_PlayState == PlayState::PLAY || m_PlayState == PlayState::PAUSE)
        {
            Scene* activeScene = m_Editor->GetActiveScene();
            if (activeScene && !m_OriginalScenePath.empty())
            {
                // Clear current scene
                activeScene->GetRegistry().clear();

                // Load the original scene
                activeScene->LoadFromFile(m_OriginalScenePath);

                // Reset scene name and path in Editor
                m_Editor->SetScenePath(m_OriginalScenePath);
                m_Editor->SetSceneName(m_OriginalSceneName);

                std::cout << "[VIEWPORT] Scene reset to: " << m_OriginalSceneName << std::endl;
            }

            m_PlayState = PlayState::STOP;
            m_Editor->SetCurrSelectedEntity(Entity{});
            m_Editor->RetrievePickedID(0xFFFFFFFFu);
            std::cout << "[VIEWPORT] State changed: to STOP" << std::endl;
        }
        else
        {
            std::cout << "[VIEWPORT] Stop() ignored - not in PLAY or PAUSE state" << std::endl;
        }
    }

    void EditorViewportPanel::ViewportButtons()
    {
        Scene* scene = m_Editor->GetActiveScene();
        Entity selectedEntity = m_Editor->GetSelectedEntity();
        uint32_t pickedID = m_Editor->GetPickedID();
        std::string scenePath = m_Editor->GetScenePath();
        std::string sceneName = m_Editor->GetSceneName();

        // Display current state
        switch (m_PlayState)
        {
        case PlayState::PLAY:
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::Text("PLAYING");
            ImGui::PopStyleColor();
            break;
        case PlayState::PAUSE:
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
            ImGui::Text("PAUSED");
            ImGui::PopStyleColor();
            break;
        case PlayState::STOP:
        default:
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGui::Text("STOPPED");
            ImGui::PopStyleColor();
            break;
        }

        float spacing = ImGui::GetStyle().ItemSpacing.x;

        // ALWAYS 3 buttons with FIXED text
        float playWidth = ImGui::CalcTextSize("Play").x + ImGui::GetStyle().FramePadding.x * 2;
        float pauseWidth = ImGui::CalcTextSize("Pause").x + ImGui::GetStyle().FramePadding.x * 2;
        float stopWidth = ImGui::CalcTextSize("Stop").x + ImGui::GetStyle().FramePadding.x * 2;

        float totalButtonsWidth = playWidth + pauseWidth + stopWidth + spacing * 2;

        float availWidth = ImGui::GetContentRegionAvail().x;
        ImGui::SameLine(availWidth - totalButtonsWidth);

        // ===== PLAY BUTTON (ALWAYS says "Play") =====
        if (m_PlayState == PlayState::STOP)
        {
            // Green Play button - ENABLED only in STOP state
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(144, 238, 144, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(120, 220, 120, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(100, 200, 100, 255));

            if (ImGui::Button("Play")) {
                Play();
            }

            ImGui::PopStyleColor(3);
        }
        else
        {
            // Gray Play button - DISABLED in PLAY or PAUSE
            ImGui::BeginDisabled();
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 200, 200, 255));
            ImGui::Button("Play");
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
        }

        ImGui::SameLine();

        // ===== PAUSE BUTTON (ALWAYS says "Pause") =====
        if (m_PlayState == PlayState::STOP)
        {
            // Gray Pause button - DISABLED in STOP
            ImGui::BeginDisabled();
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 200, 200, 255));
            ImGui::Button("Pause");
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
        }
        else if (m_PlayState == PlayState::PLAY)
        {
            // Yellow Pause button - ENABLED in PLAY
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 210, 100, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(240, 190, 80, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(220, 170, 60, 255));

            if (ImGui::Button("Pause")) {
                Pause();
            }

            ImGui::PopStyleColor(3);
        }
        else // PAUSE state
        {
            // Yellow Pause button - ENABLED in PAUSE (acts as "Resume")
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 210, 100, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(240, 190, 80, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(220, 170, 60, 255));

            if (ImGui::Button("Pause")) {
                Pause(); // Will resume from pause
            }

            ImGui::PopStyleColor(3);
        }

        ImGui::SameLine();

        // ===== STOP BUTTON (ALWAYS says "Stop") =====
        if (m_PlayState == PlayState::PLAY || m_PlayState == PlayState::PAUSE)
        {
            // Red Stop button - ENABLED in PLAY or PAUSE
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(255, 0, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(200, 0, 0, 255));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(180, 0, 0, 255));

            if (ImGui::Button("Stop")) {
                Stop();
            }

            ImGui::PopStyleColor(3);
        }
        else
        {
            // Gray Stop button - DISABLED in STOP
            ImGui::BeginDisabled();
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 200, 200, 255));
            ImGui::Button("Stop");
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
        }

        ImGui::SeparatorText("Viewport");
    }
}