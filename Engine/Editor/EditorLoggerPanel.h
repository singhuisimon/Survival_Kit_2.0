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
    class EditorLogPanel
    {
    private:

        Editor* m_Editor = nullptr;
        
    public:

        EditorLogPanel(Editor* editor) : m_Editor(editor) {}
        ~EditorLogPanel() = default;

        void LogPanel();

    };
}

#endif