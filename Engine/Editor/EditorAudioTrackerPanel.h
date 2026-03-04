#pragma once
#ifndef EDITOR_AUDIOTRACKER_H
#define EDITOR_AUDIOTRACKER_H

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "../Editor/Editor.h"
#include "../Utility/AssetPath.h"
#include "../Asset/AssetManager.h"
#include "../Asset/ResourceManager.h"
#include "../Asset/ResourceHelpers.h"

#include <vector>
#include <string>

namespace Engine
{
    class EditorAudioTrackerPanel
    {
    private:

        Editor* m_Editor = nullptr;

    public:

        EditorAudioTrackerPanel(Editor* editor) : m_Editor(editor) {}
        ~EditorAudioTrackerPanel() = default;

        void AudioTrackerPanel();
        std::vector<std::pair<std::string, int>> FilterEntitiesByAudio(std::string audioFileName);

    };
}

#endif