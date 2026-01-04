#include "EditorViewportPanel.h"
#include "Editor.h"

#include <algorithm>
#include <ImGuizmo.h>

namespace Engine
{
    void EditorViewportPanel::ManipulateEntityTransform(Entity& entity, EditorViewport m_ImGuizmoViewportData)
    {
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
}