#pragma once
#ifndef EDITOR_LOGPANEL_H
#define EDITOR_LOGPANEL_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../Editor/Editor.h"
#include "../Utility/Logger.h"

namespace Engine
{
    class Editor;

    struct LogEntry {
        LogLevel level;
        std::string message;
    };

    class EditorLogPanel
    {
    private:

        Editor* m_Editor = nullptr;
        std::vector<LogEntry>  m_Entries;
        bool m_ScrollToBottom = true;
        bool m_AutoScroll = true;

        // search function in log
        char m_SearchBuffer[256] = {};
        std::string m_SearchQuery = "";
        std::vector<int>m_MatchIndices;
        int m_CurrentMatchIndex = 0;
        int  m_GotoIndex = 1;
        int  m_JumpToEntry = -1;
        bool m_SearchDirty = false;
        bool m_JumpMatch = false;

        ImVec4 GetLevelColor(LogLevel level)
        {
            switch (level) {
            case LogLevel::Trace:    return { 0.5f, 0.5f, 0.5f, 1.f };
            case LogLevel::Debug:    return { 0.2f, 0.8f, 0.8f, 1.f };
            case LogLevel::Info:     return { 1.0f, 1.0f, 1.0f, 1.f };
            case LogLevel::Warning:  return { 1.0f, 0.8f, 0.0f, 1.f };
            case LogLevel::Error:    return { 1.0f, 0.3f, 0.3f, 1.f };
            case LogLevel::Critical: return { 1.0f, 0.0f, 0.5f, 1.f };
            default:                 return { 1.0f, 1.0f, 1.0f, 1.f };
            }
        }

    public:

        EditorLogPanel(Editor* editor) : m_Editor(editor) {
            // Register ourselves as the logger callback
            Logger::Get().SetCallback([this](LogLevel level, const std::string& msg) {
                m_Entries.push_back({ level, msg });
                if (!m_SearchQuery.empty())
                {
                    std::string msgLower = msg;
                    std::string queryLower = m_SearchQuery;
                    std::transform(msgLower.begin(), msgLower.end(), msgLower.begin(), ::tolower);
                    std::transform(queryLower.begin(), queryLower.end(), queryLower.begin(), ::tolower);
                    if (msgLower.find(queryLower) != std::string::npos)
                        m_MatchIndices.push_back(static_cast<int>(m_Entries.size()) - 1);
                }
            });
        }

        ~EditorLogPanel()
        {
            Logger::Get().ClearCallback();
        }

        void Clear() { 
            m_Entries.clear();
            m_MatchIndices.clear();
            m_CurrentMatchIndex = 0;
            m_SearchBuffer[0] = '\0';
            m_SearchQuery.clear();
        }
        void LogPanel();

    };
}

#endif