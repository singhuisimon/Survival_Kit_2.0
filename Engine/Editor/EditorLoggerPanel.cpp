#include <windows.h>
#include <algorithm>
#include "EditorLoggerPanel.h"
#include "../Utility/Logger.h"
#include "../Engine/Editor/Editor.h"

namespace Engine
{
	void EditorLogPanel::LogPanel()
    {
        if (!m_Editor->GetLoggerWindowRef()) return;

        if (!ImGui::Begin("Log", &m_Editor->GetLoggerWindowRef()))
        {
            ImGui::End();
            return;
        }

        ImGui::End();
    }
}