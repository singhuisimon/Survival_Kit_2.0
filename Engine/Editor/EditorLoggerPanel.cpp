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

        // Search function
        ImGui::Text("Search:");
        ImGui::SameLine();
        static float inputWidth = 0.0f;
        static float lastWindowWidth = 0.0f;
        float currentWindowWidth = ImGui::GetWindowSize().x;

        float btnW = ImGui::CalcTextSize("Clear").x + ImGui::GetStyle().FramePadding.x * 2.0f;
        float arrowW = ImGui::GetFrameHeight() * 2.0f + ImGui::GetStyle().ItemSpacing.x;
        //float inputWidth = ImGui::GetContentRegionAvail().x - btnW - arrowW - ImGui::GetStyle().ItemSpacing.x * 3.0f - 180.0f;
        //ImGui::PushItemWidth(inputWidth > 180.0f ? inputWidth : 180.0f);
        if (currentWindowWidth != lastWindowWidth)
        {
            lastWindowWidth = currentWindowWidth;

            float btnW = ImGui::CalcTextSize("Clear").x + ImGui::GetStyle().FramePadding.x * 2.0f;
            float arrowW = ImGui::GetFrameHeight() * 2.0f + ImGui::GetStyle().ItemSpacing.x;

            inputWidth = ImGui::GetContentRegionAvail().x
                - btnW
                - arrowW
                - ImGui::GetStyle().ItemSpacing.x * 3.0f
                - 200.0f;
        }


        ImGui::PushItemWidth(inputWidth);

        if (ImGui::InputText("##LogSearch", m_SearchBuffer, sizeof(m_SearchBuffer)))
        {
            m_SearchQuery = m_SearchBuffer;
            m_CurrentMatchIndex = 0;
            m_SearchDirty = true;
            m_AutoScroll = false;
        }
        ImGui::PopItemWidth();


        // Rebuild match list whenever query or entries change
        if (m_SearchDirty)
        {
            m_MatchIndices.clear();
            if (!m_SearchQuery.empty())
            {
                std::string queryLower = m_SearchQuery;
                std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);
                for (int i = 0; i < static_cast<int>(m_Entries.size()); ++i)
                {
                    std::string msgLower = m_Entries[i].message;
                    std::transform(msgLower.begin(), msgLower.end(), msgLower.begin(), ::tolower);
                    if (msgLower.find(queryLower) != std::string::npos)
                        m_MatchIndices.push_back(i);
                }
            }
            m_SearchDirty = false;
        }

        // Prev / Next buttons
        ImGui::SameLine();
        bool hasMatches = !m_MatchIndices.empty();

        ImGui::BeginDisabled(!hasMatches);
        if (ImGui::ArrowButton("##PrevMatch", ImGuiDir_Up))
        {
            m_CurrentMatchIndex = (m_CurrentMatchIndex - 1
                + static_cast<int>(m_MatchIndices.size()))
                % static_cast<int>(m_MatchIndices.size());
            m_JumpMatch = true;
        }
        ImGui::SameLine();
        if (ImGui::ArrowButton("##NextMatch", ImGuiDir_Down))
        {
            m_CurrentMatchIndex = (m_CurrentMatchIndex + 1)
                % static_cast<int>(m_MatchIndices.size());
            m_JumpMatch = true;
        }
        ImGui::EndDisabled();

        // Match counter  
        ImGui::SameLine();
        if (!m_SearchQuery.empty())
        {
            if (hasMatches)
            {
                int lineNumber = m_MatchIndices[m_CurrentMatchIndex] + 1;
                ImGui::Text("%d / %d  (line %d)", m_CurrentMatchIndex + 1, static_cast<int>(m_MatchIndices.size()), lineNumber);

            }
            else
            {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "No results");
            }
        }

        ImGui::SameLine();
        if (ImGui::Button("Clear##Search"))
        {
            m_SearchBuffer[0] = '\0';
            m_SearchQuery.clear();
            m_MatchIndices.clear();
            m_CurrentMatchIndex = 0;
            m_AutoScroll = true;
        }

        ImGui::Separator();

        // jump to index
        int totalEntries = static_cast<int>(m_Entries.size());
        ImGui::Text("Go to line:");
        ImGui::SameLine();
        ImGui::PushItemWidth(60.0f);
        // clamp input so it can't exceed the current entry count
        if (ImGui::InputInt("##GotoLine", &m_GotoIndex, 0, 0))
        {
            m_GotoIndex = std::max(1, std::min(m_GotoIndex, totalEntries));
        }
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::BeginDisabled(totalEntries == 0);
        if (ImGui::Button("Go##GotoLine"))
        {
            // clamp once more on button press in case user typed out-of-range
            m_GotoIndex = std::max(1, std::min(m_GotoIndex, totalEntries));
            m_JumpToEntry = m_GotoIndex - 1;  // convert 1-based UI to 0-based index
            m_JumpMatch = false;             // cancel any pending search jump
            m_AutoScroll = false;
        }
        ImGui::EndDisabled();
        ImGui::SameLine();
        ImGui::TextDisabled("/ %d", totalEntries);

        ImGui::Separator();

        // Scrollable message region
        ImGui::BeginChild("LogRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        int  scrollTargetEntry = -1;
        if (m_JumpMatch && hasMatches)
        {
            scrollTargetEntry = m_MatchIndices[m_CurrentMatchIndex];
        }
        else if (m_JumpToEntry >= 0)
        {
            scrollTargetEntry = m_JumpToEntry;
        }

        std::string queryLower = m_SearchQuery;
        std::transform(queryLower.begin(), queryLower.end(),
            queryLower.begin(), ::tolower);
        hasMatches = !m_MatchIndices.empty();
        int focusedEntry = hasMatches ? m_MatchIndices[m_CurrentMatchIndex] : -1;

        for (int i = 0; i < totalEntries; ++i)
        {
            const auto& entry = m_Entries[i];
            bool isFocused = (i == focusedEntry && !m_SearchQuery.empty());
            bool isGotoTarget = (i == m_JumpToEntry && m_JumpToEntry >= 0
                && m_SearchQuery.empty());

            // Blue highlight for focused search match
            if (isFocused)
            {
                ImVec2 p = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled(
                    p,
                    ImVec2(p.x + ImGui::GetContentRegionAvail().x,
                        p.y + ImGui::GetTextLineHeightWithSpacing()),
                    IM_COL32(80, 160, 255, 60));
            }

            // Orange highlight for goto-line target
            if (isGotoTarget)
            {
                ImVec2 p = ImGui::GetCursorScreenPos();
                ImGui::GetWindowDrawList()->AddRectFilled(
                    p,
                    ImVec2(p.x + ImGui::GetContentRegionAvail().x,
                        p.y + ImGui::GetTextLineHeightWithSpacing()),
                    IM_COL32(255, 160, 40, 70));
            }

            // Line number prefix
            ImGui::TextDisabled("%4d  ", i + 1);
            ImGui::SameLine(0.0f, 0.0f);

            // Render text with inline keyword highlight when searching
            if (queryLower.empty())
            {
                ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(entry.level));
                ImGui::TextUnformatted(entry.message.c_str());
                ImGui::PopStyleColor();
            }
            else
            {
                std::string msgLower = entry.message;
                std::transform(msgLower.begin(), msgLower.end(),
                    msgLower.begin(), ::tolower);

                size_t searchLen = queryLower.size();
                size_t pos = 0;
                bool   anyPart = false;

                while (pos <= entry.message.size())
                {
                    size_t found = msgLower.find(queryLower, pos);
                    size_t end = (found == std::string::npos)
                        ? entry.message.size() : found;

                    // Normal text before match
                    if (end > pos)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(entry.level));
                        ImGui::TextUnformatted(entry.message.c_str() + pos,
                            entry.message.c_str() + end);
                        ImGui::PopStyleColor();
                        ImGui::SameLine(0.0f, 0.0f);
                        anyPart = true;
                    }
                    if (found == std::string::npos) break;

                    // Keyword in yellow
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.0f, 1.0f));
                    ImGui::TextUnformatted(entry.message.c_str() + found,
                        entry.message.c_str() + found + searchLen);
                    ImGui::PopStyleColor();
                    ImGui::SameLine(0.0f, 0.0f);
                    anyPart = true;
                    pos = found + searchLen;
                }
                if (anyPart) ImGui::NewLine();
            }

            // Scroll to target
            if (i == scrollTargetEntry)
            {
                ImGui::SetScrollHereY(0.5f);    // centre the target line
                m_JumpMatch = false;
                m_JumpToEntry = -1;             // consume the jump
            }
        }

        if (m_AutoScroll && m_SearchQuery.empty())
            ImGui::SetScrollHereY(1.0f);

        /*int focusedEntry = hasMatches ? m_MatchIndices[m_CurrentMatchIndex] : -1;
        std::string queryLower = m_SearchQuery;
        std::transform(queryLower.begin(), queryLower.end(),
            queryLower.begin(), ::tolower);
        for (const auto& entry : m_Entries)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, GetLevelColor(entry.level));
            ImGui::TextUnformatted(entry.message.c_str());
            ImGui::PopStyleColor();
        }

        if (m_AutoScroll && m_SearchQuery.empty()) {
            ImGui::SetScrollHereY(1.0f);
        }*/

        ImGui::EndChild();

        ImGui::End();
    }
}