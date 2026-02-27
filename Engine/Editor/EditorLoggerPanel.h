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
            });
        }

        ~EditorLogPanel()
        {
            Logger::Get().ClearCallback();
        }

        void Clear() { m_Entries.clear(); }
        void LogPanel();

    };
}

#endif