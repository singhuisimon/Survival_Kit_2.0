#include "EditorViewportPanel.h"
#include "Editor.h"
#include "../Scripting/MonoScriptEngine.h"
#include <algorithm>
#include <ImGuizmo.h>
#include <glm/gtc/matrix_inverse.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>  

namespace Engine
{
    namespace {
        struct ScriptFieldBackup {
            std::string ScriptClassName;
            std::unordered_map<std::string, std::vector<std::uint8_t>> Fields;
        };

        // Backed-up edit-mode serialized field data captured at the moment Play is pressed.
        // Stop() reloads the scene from disk, so without this, inspector-authored
        // [SerializeField] edits revert to whatever is stored in the scene file.
        static std::unordered_map<std::uint32_t, ScriptFieldBackup> s_EditModeScriptFieldBackup;

        static void CacheEditModeScriptFieldOverrides(Scene *scene) {
            s_EditModeScriptFieldBackup.clear();
            if(!scene) return;

            auto &registry = scene->GetRegistry();
            auto view = registry.view<ScriptComponent>();

            for(auto entity : view) {
                auto &sc = view.get<ScriptComponent>(entity);
                if(sc.ScriptClassName.empty())
                    continue;

                ScriptFieldBackup backup;
                backup.ScriptClassName = sc.ScriptClassName;
                backup.Fields = sc.SerializedFields; // copy
                s_EditModeScriptFieldBackup[static_cast<std::uint32_t>(entity)] = std::move(backup);
            }
        }

        static void RestoreEditModeScriptFieldOverrides(Scene *scene) {
            if(!scene || s_EditModeScriptFieldBackup.empty())
                return;

            auto &registry = scene->GetRegistry();
            auto view = registry.view<ScriptComponent>();
            auto &se = MonoScriptEngine::GetInstance();

            for(auto entity : view) {
                const std::uint32_t eid = static_cast<std::uint32_t>(entity);
                auto it = s_EditModeScriptFieldBackup.find(eid);
                if(it == s_EditModeScriptFieldBackup.end())
                    continue;

                auto &sc = view.get<ScriptComponent>(entity);

                // If the scene file didn't persist the script binding, keep the edit-time one.
                if(sc.ScriptClassName.empty() && !it->second.ScriptClassName.empty())
                    sc.ScriptClassName = it->second.ScriptClassName;

                sc.SerializedFields = it->second.Fields;

                // Best-effort: if an instance already exists, push restored values into it.
                MonoObject *inst = nullptr;
                if(sc.GCHandle != 0)
                    inst = se.GetObjectFromGCHandle(sc.GCHandle);
                if(!inst)
                    inst = reinterpret_cast<MonoObject *>(sc.ScriptInstance);

                if(inst)
                    se.ApplySerializedFieldsFromComponent(eid, inst);
            }
        }
    }

    void EditorViewportPanel::ManipulateEntityTransform(Entity& entity, EditorViewport m_ImGuizmoViewportData)
    {
        //if (m_PlayState != PlayState::STOP) return;
        Renderer* m_Renderer = m_Editor->GetRenderer();
        Scene* scene = m_Editor->GetActiveScene();
        if (!entity || !m_Renderer || !entity.HasComponent<TransformComponent>()) return;

        // Get the camera
        Camera3D& camera = m_Renderer->getEditorCamera();

        // Get the transform component
        auto& tc = entity.GetComponent<TransformComponent>();

        // Build transform matrix from position, rotation, and scale
        glm::mat4 transform = tc.WorldTransform;
        //transform = transform * glm::mat4_cast(tc.Rotation);
        //transform = glm::scale(transform, tc.Scale);

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
                    /*glm::vec3 newPosition = glm::vec3(transform[3]);
                    tc.SetPosition(newPosition);*/
                    // Extract new world position
                    glm::vec3 newWorldPosition = glm::vec3(transform[3]);

                    if (tc.Parent == u32_max) {
                        // Root entity: world position = local position
                        tc.SetPosition(newWorldPosition);
                    }
                    else {
                        // Child entity: convert world position to local space
                        auto& registry = scene->GetRegistry();
                        auto view = registry.view<TransformComponent>();

                        // Check if parent exists
                        entt::entity parentEntity = static_cast<entt::entity>(tc.Parent);
                        if (registry.valid(parentEntity) && registry.all_of<TransformComponent>(parentEntity)) {
                            auto& parent_transform = registry.get<TransformComponent>(parentEntity);

                            // Convert world to local: local = parent_inverse × world
                            glm::mat4 parent_inverse = glm::inverse(parent_transform.WorldTransform);
                            glm::mat4 newLocalTransform = parent_inverse * transform;

                            // Decompose to get new local position
                            glm::vec3 skew;
                            glm::vec4 perspective;
                            glm::vec3 newScale;
                            glm::quat newRotation;
                            glm::vec3 newLocalPosition;

                            glm::decompose(newLocalTransform, newScale, newRotation,
                                newLocalPosition, skew, perspective);

                            // Update transform
                            tc.Position = newLocalPosition;
                            tc.Rotation = newRotation;
                            tc.Scale = newScale;
                            tc.IsDirty = true;
                        }
                    }
                }
                else if (m_Operation == ImGuizmo::ROTATE)
                {
                    //// Extract rotation matrix and convert to quaternion
                    //glm::mat3 rotationMatrix;
                    //rotationMatrix[0] = glm::normalize(glm::vec3(transform[0]));
                    //rotationMatrix[1] = glm::normalize(glm::vec3(transform[1]));
                    //rotationMatrix[2] = glm::normalize(glm::vec3(transform[2]));

                    //glm::quat newRotation = glm::quat_cast(rotationMatrix);
                    //tc.Rotation = newRotation;
                    //tc.IsDirty = true;
                    glm::mat3 rotationMatrix;
                    rotationMatrix[0] = glm::normalize(glm::vec3(transform[0]));
                    rotationMatrix[1] = glm::normalize(glm::vec3(transform[1]));
                    rotationMatrix[2] = glm::normalize(glm::vec3(transform[2]));

                    glm::quat newWorldRotation = glm::quat_cast(rotationMatrix);

                    if (tc.Parent == u32_max) {
                        // Root entity: world rotation = local rotation
                        tc.Rotation = newWorldRotation;
                    }
                    else {
                        // Child entity: convert world rotation to local
                        auto& registry = scene->GetRegistry();
                        auto view = registry.view<TransformComponent>();

                        entt::entity parentEntity = static_cast<entt::entity>(tc.Parent);
                        if (registry.valid(parentEntity) && registry.all_of<TransformComponent>(parentEntity)) {
                            auto& parent_transform = registry.get<TransformComponent>(parentEntity);

                            // Get parent's world rotation from its WorldTransform
                            glm::mat3 parentRotationMatrix;
                            parentRotationMatrix[0] = glm::normalize(glm::vec3(parent_transform.WorldTransform[0]));
                            parentRotationMatrix[1] = glm::normalize(glm::vec3(parent_transform.WorldTransform[1]));
                            parentRotationMatrix[2] = glm::normalize(glm::vec3(parent_transform.WorldTransform[2]));
                            glm::quat parentWorldRotation = glm::quat_cast(parentRotationMatrix);

                            // Local rotation = parent_world_inverse × world_rotation
                            tc.Rotation = glm::inverse(parentWorldRotation) * newWorldRotation;
                        }
                    }

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
        //if (m_PlayState != PlayState::STOP) return;
        Entity m_SelectedEntity = m_Editor->GetSelectedEntity();
        u32 m_PickedID = m_Editor->GetPickedID();
        Scene* m_ActiveScene = m_Editor->GetActiveScene();
        static Entity doubleClickCandidate;
        static uint32_t doubleClickCandidateID = 0xFFFFFFFFu;
        static double lastClickTime = 0.0;
        static const double DOUBLE_CLICK_TIME = 0.3;

        if (ImGui::IsItemHovered() &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Left))
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
                //std::cout << "CLICK DETECTED!" << std::endl;

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
                        //std::cout << "[GIZMO] Deselected" << std::endl;
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
        //if (m_PlayState != PlayState::STOP) return;
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
    
    void EditorViewportPanel::Play() {
        // Allow entering play mode from STOP. When PAUSE, Play acts as Resume.
        if(m_PlayState == PlayState::PAUSE) {
            m_PlayState = PlayState::PLAY;
            std::cout << "[VIEWPORT] State changed: PAUSE to PLAY (Resume)" << std::endl;
            return;
        }

        // ONLY allow Play from STOP state
        if(m_PlayState != PlayState::STOP) {
            std::cout << "[VIEWPORT] Play() ignored - not in STOP state" << std::endl;
            return;
        }

        Scene *activeScene = m_Editor->GetActiveScene();

        // Capture edit-mode script field overrides BEFORE any hot-reload or scene changes.
        CacheEditModeScriptFieldOverrides(activeScene);

        auto &se = Engine::MonoScriptEngine::GetInstance();
        se.HotReloadOnPlay(true);

        // Store the original scene state before playing
        if(activeScene && m_Editor->HasScenePath()) {
            m_OriginalScenePath = m_Editor->GetScenePath();
            m_OriginalSceneName = m_Editor->GetSceneName();
            std::cout << "[VIEWPORT] Saved scene state: " << m_OriginalSceneName << std::endl;
        }

        m_PlayState = PlayState::PLAY;
        std::cout << "[VIEWPORT] State changed: STOP to PLAY" << std::endl;
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
                RestoreEditModeScriptFieldOverrides(activeScene);

                // Reset scene name and path in Editor
                m_Editor->SetScenePath(m_OriginalScenePath);
                m_Editor->SetSceneName(m_OriginalSceneName);

                std::cout << "[VIEWPORT] Scene reset to: " << m_OriginalSceneName << std::endl;
            }

            m_PlayState = PlayState::STOP;
            //m_Editor->SetCurrSelectedEntity(Entity{});
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
        if (m_PlayState == PlayState::PAUSE || m_PlayState == PlayState::STOP)
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
        if (m_PlayState == PlayState::PAUSE || m_PlayState == PlayState::STOP)
        {
            // Gray Pause button - DISABLED in STOP
            ImGui::BeginDisabled();
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(200, 200, 200, 255));
            ImGui::Button("Pause");
            ImGui::PopStyleColor();
            ImGui::EndDisabled();
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

    void EditorViewportPanel::CameraControl(Renderer* renderer) {
        ImGui::SeparatorText("Camera Controls");

        Camera3D& editorCam = renderer->getEditorCamera();
        float& camSpeed = editorCam.getEditorCamSpeed();
        if (ImGui::DragFloat("Editor Camera Movement Speed", &camSpeed, 0.01f, 0.0f, 1e10f)) {
            editorCam.setEditorCamSpeed(camSpeed);
        }
        float& zoomSpeed = editorCam.getEditorZoomSpeed();
        if (ImGui::DragFloat("Editor Camera Zoom Speed", &zoomSpeed, 0.01f, 0.0f, 1e10f)) {
            editorCam.setEditorZoomSpeed(zoomSpeed);
        }
    }
}