#include <windows.h>
#include <algorithm>
#include "EditorLoggerPanel.h"
#include "../Utility/Logger.h"
#include "../Engine/Editor/Editor.h"

namespace Engine
{
    void EditorLogPanel::LogPanel()
    {
        if (!m_Editor->GetLoggerWindowRef()) { 
            return; 
        }

        if (!ImGui::Begin("Log", &m_Editor->GetLoggerWindowRef()))
        {
            ImGui::End();
            return;
        }

        if (ImGui::Button("Clear")) Clear();
        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_AutoScroll);

        ImGui::Separator();

        // Scrollable message region
        ImGui::BeginChild("LogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        for (const auto& entry : m_Entries)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(entry.level));
            ImGui::TextUnformatted(entry.message.c_str());
            ImGui::PopStyleColor();
        }

        if (m_AutoScroll) {
            ImGui::SetScrollHereY(1.0f);
        }

        ImGui::EndChild();

        ImGui::End();
    }
}