#include "../Engine/Editor/EditorAudioTrackerPanel.h"
#include "../Engine/Editor/Editor.h"
#include "../ECS/Components.h"
#include "../Component/TagComponent.h"
#include <vector>
#include <string>
#include <algorithm>

namespace Engine
{
    void EditorAudioTrackerPanel::AudioTrackerPanel()
    {
        if (!m_Editor->GetAudioTrackerWindowRef()) {
            return;
        }

        if (!ImGui::Begin("Audio File Tracker", &m_Editor->GetAudioTrackerWindowRef()))
        {
            ImGui::End();
            return;
        }

		// Add in the mixer controls at the top of the panel
        AudioManager* audioManager = m_Editor ? m_Editor->GetAudioManager() : nullptr;
        if (!audioManager)
        {
            LOG_DEBUG("AudioManager not found in EditorAudioTrackerPanel.");
            ImGui::TextDisabled("AudioManager not available.");
            ImGui::End();
            return;
        }

        ImGui::Text("Audio Mixer");
        ImGui::Separator();

        auto DrawVolumeControl = [&](const char* label, AudioType type)
            {
                float value = audioManager->GetEditorCap(type);
                bool changed = false;

                ImGui::PushID(label);

                ImGui::TextUnformatted(label);

                ImGui::SetNextItemWidth(200.0f);
                changed |= ImGui::SliderFloat("##slider", &value, 0.0f, 2.0f, "%.2f");

                ImGui::SameLine();

                ImGui::SetNextItemWidth(100.0f);
                changed |= ImGui::DragFloat("##drag", &value, 0.01f, 0.0f, 2.0f, "%.3f");

                value = std::clamp(value, 0.0f, 2.0f);

                if (changed)
                {
                    audioManager->SetEditorCap(type, value);
                }

                ImGui::PopID();
            };

        ImGui::Columns(2, "AudioMixerColumns", false);

        DrawVolumeControl("Master", AudioType::MASTER);
        ImGui::NextColumn();

        DrawVolumeControl("BGM", AudioType::BGM);
        ImGui::NextColumn();

        DrawVolumeControl("SFX", AudioType::SFX);
        ImGui::NextColumn();

        DrawVolumeControl("GameSFX", AudioType::GAMESFX);
        ImGui::NextColumn();

        DrawVolumeControl("UI", AudioType::UI);
        ImGui::NextColumn();

        DrawVolumeControl("VO", AudioType::VO);

        ImGui::Columns(1);

        ImGui::Separator();

        ////add in sliders here
        //float masterCap = audioManager->GetEditorCap(AudioType::MASTER);
        //if (ImGui::SliderFloat("Master", &masterCap, 0.0f, 1.0f, "%.2f"))
        //{
        //    audioManager->SetEditorCap(AudioType::MASTER, masterCap);
        //}

        //float bgmCap = audioManager->GetEditorCap(AudioType::BGM);
        //if (ImGui::SliderFloat("BGM", &bgmCap, 0.0f, 1.0f, "%.2f"))
        //{
        //    audioManager->SetEditorCap(AudioType::BGM, bgmCap);
        //}

        ImGui::Separator();

        bool openAll = ImGui::Button("Expand All");
        ImGui::SameLine();
        bool closeAll = ImGui::Button("Collapse All");

		auto audios = m_Editor->getAssetsInFolder(getAssetFilePath("Sources/Audio/"));

		for (auto& audio : audios)
		{
            if (openAll) { 
                ImGui::SetNextItemOpen(true);
            }
            if (closeAll) { 
                ImGui::SetNextItemOpen(false); 
            }

            std::vector<std::pair<std::string, int>> filteredEntities = FilterEntitiesByAudio(audio.name);
            std::string audioNameAndCount = audio.name;

            if (filteredEntities.size() != 0) {
                audioNameAndCount = audio.name + " (" + std::to_string(filteredEntities.size()) + ")";
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
            }

            if (ImGui::TreeNodeEx(audioNameAndCount.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {

                for (auto& entityName : filteredEntities)
                {
                    std::string entityLabel = entityName.first + " (ID :" + std::to_string(entityName.second) + ") ";
                    ImGui::TreeNodeEx(entityLabel.c_str(), ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen);
                }
                ImGui::TreePop();
            }

            if (filteredEntities.size() != 0) {
                ImGui::PopStyleColor();
            }
		}

        ImGui::End();
    }

    std::vector<std::pair<std::string, int>> EditorAudioTrackerPanel::FilterEntitiesByAudio(std::string audioFileName) {
        
        //LOG_DEBUG("audioFileName: ", audioFileName, "; Entities ->");

        std::vector<std::pair<std::string, int>> entitiesWithAudio;
        
        Scene* m_Scene = m_Editor->GetActiveScene();
        auto viewEntities = m_Scene->GetRegistry().view<TagComponent>();
       
        for (auto currentEntity : viewEntities)
        {
            Entity entity(currentEntity, &m_Scene->GetRegistry());
            if (entity.HasComponent<AudioComponent>())
            {
                //LOG_DEBUG("entity: ", entity.GetComponent<TagComponent>().Name, " has Audio");

                auto& audio = entity.GetComponent<AudioComponent>();
                if (!audio.AudioFilePath.empty()) {

                    //LOG_DEBUG("Audio: ", audio.AudioFilePath);

                    if (audio.AudioFilePath == audioFileName) {
                        entitiesWithAudio.push_back(std::make_pair(entity.GetComponent<TagComponent>().Name, static_cast<int>(entity.GetHandle())));
                    }
                }

            }
        }

        return entitiesWithAudio;
    }
}