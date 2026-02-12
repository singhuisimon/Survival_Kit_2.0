#include "EditorPerformancePanel.h"
#include "../Editor/Editor.h"
#include "../Utility/Timestep.h"
namespace Engine
{
    void EditorPerformancePanel::PerformanceProfilePanel(Timestep ts)
    {
        if (!m_Editor->GetPerformanceProfileWindowRef()) return;

        std::weak_ptr<TracyProfiler> profilerWeak = m_Editor->GetProfiler();
        ImGui::SetNextWindowSize(ImVec2(500, 420), ImGuiCond_FirstUseEver);

        // Begin MUST be called, and End MUST be called regardless of the return value
        bool isOpen = ImGui::Begin("Performance Profile", &m_Editor->GetPerformanceProfileWindowRef(), ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

        if (isOpen) // Only render content if window is not collapsed
        {
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Editor Settings:");

            float currentScale = m_Editor->GetFontScale();
            if (ImGui::SliderFloat("UI Scale", &currentScale, 0.5f, 2.0f, "%.2f"))
            {
                m_Editor->SetFontScale(currentScale);
                ImGui::GetIO().FontGlobalScale = currentScale;
                ImGui::GetStyle() = m_Editor->GetBaseStyle();
                ImGui::GetStyle().ScaleAllSizes(currentScale);
            }

            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Adjusts the size of all text and UI elements.");

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // Theme Editor
            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Text("Theme Settings:");

            static int currentTheme = 0;
            const char* themes[] = { "Dark", "Light", "Classic", "Red"};

            if (ImGui::Combo("Editor Theme", &currentTheme, themes, IM_ARRAYSIZE(themes)))
            {
                switch (currentTheme)
                {
                case 0: ImGui::StyleColorsDark(); break;
                case 1: ImGui::StyleColorsLight(); break;
                case 2: ImGui::StyleColorsClassic(); break;
                case 3: ApplyCustomTheme(); break;
                }

                float currentScale = m_Editor->GetFontScale();
                ImGui::GetStyle().ScaleAllSizes(currentScale);
            }

            ImGui::Text("Tracy Window:");
            if (ImGui::Button("Launch Tracy Window"))
            {
#ifdef TRACY_ENABLE
                if (auto profiler = profilerWeak.lock())
                {
                    profiler->LaunchTracy();
                    LOG_INFO("  -> Tracy profiler launched successfully");
                }
                else
                {
                    LOG_WARNING("  -> Tracy profiler reference expired.");
                }
#else
                LOG_WARNING("  -> TRACY_ENABLE not defined. Skipping profiler launch.");
#endif
            }

            ImGui::Separator();

            // Frame statistics
            float deltaTime = ts.GetSeconds();
            float currFPS = (deltaTime > 0.0f) ? 1.0f / deltaTime : 0.0f;
            float currFrameTime = ts.GetMilliseconds();

            static const int FPS_HISTORY_SIZE = 90;
            static float fpsHistory[FPS_HISTORY_SIZE] = {};
            static float frameTimeHistory[FPS_HISTORY_SIZE] = {};
            static int fpsHistoryOffset = 0;
            static int frameCount = 0;

            fpsHistory[fpsHistoryOffset] = currFPS;
            frameTimeHistory[fpsHistoryOffset] = currFrameTime;
            fpsHistoryOffset = (fpsHistoryOffset + 1) % FPS_HISTORY_SIZE;
            frameCount = std::min(frameCount + 1, FPS_HISTORY_SIZE);

            static float minFPS = FLT_MAX;
            static float maxFPS = 0.0f;
            static float minFrameTime = FLT_MAX;
            static float maxFrameTime = 0.0f;

            minFPS = std::min(minFPS, currFPS);
            maxFPS = std::max(maxFPS, currFPS);
            minFrameTime = std::min(minFrameTime, currFrameTime);
            maxFrameTime = std::max(maxFrameTime, currFrameTime);

            float avgFPS = 0.0f;
            float avgFrameTime = 0.0f;

            for (int i = 0; i < frameCount; i++)
            {
                avgFPS += fpsHistory[i];
                avgFrameTime += frameTimeHistory[i];
            }

            avgFPS /= frameCount;
            avgFrameTime /= frameCount;

            ImGui::Text("Frame Statistics:");
            ImGui::Spacing();

            if (ImGui::BeginTable("StatsTable", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
            {
                ImGui::TableSetupColumn("Metric", ImGuiTableColumnFlags_WidthFixed, 120.0f);
                ImGui::TableSetupColumn("Value", ImGuiTableColumnFlags_WidthStretch);
                ImGui::TableSetupColumn("Unit", ImGuiTableColumnFlags_WidthFixed, 40.0f);
                ImGui::TableHeadersRow();

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Average FPS:");
                ImGui::TableNextColumn();
                ImGui::Text("%.1f", avgFPS);
                ImGui::TableNextColumn();
                ImGui::Text("fps");

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Avg Frame Time:");
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", avgFrameTime);
                ImGui::TableNextColumn();
                ImGui::Text("ms");

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Min Frame Time:");
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", minFrameTime);
                ImGui::TableNextColumn();
                ImGui::Text("ms");

                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Max Frame Time:");
                ImGui::TableNextColumn();
                ImGui::Text("%.2f", maxFrameTime);
                ImGui::TableNextColumn();
                ImGui::Text("ms");

                ImGui::EndTable();
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Performance Graphs:");
            ImGui::Spacing();
            float graphWidth = ImGui::GetContentRegionAvail().x;

            char fpsOverlay[64];
            sprintf_s(fpsOverlay, sizeof(fpsOverlay), "FPS - avg %.1f", avgFPS);

            float fpsMinScale = (avgFPS - 30.0f > 0.0f) ? (avgFPS - 30.0f) : 0.0f;
            float fpsMaxScale = avgFPS + 30.0f;

            ImGui::PlotLines(
                "##FPS",
                fpsHistory,
                FPS_HISTORY_SIZE,
                fpsHistoryOffset,
                fpsOverlay,
                fpsMinScale,
                fpsMaxScale,
                ImVec2(graphWidth, 100.0f),
                sizeof(float)
            );

            char frameTimeOverlay[64];
            sprintf_s(frameTimeOverlay, sizeof(frameTimeOverlay), "Frame Time (ms) - avg %.2f", avgFrameTime);

            float ftMinScale = std::max(avgFrameTime - 5.0f, 0.0f);
            float ftMaxScale = avgFrameTime + 5.0f;

            ImGui::PlotLines(
                "##FrameTime",
                frameTimeHistory,
                FPS_HISTORY_SIZE,
                fpsHistoryOffset,
                frameTimeOverlay,
                ftMinScale,
                ftMaxScale,
                ImVec2(graphWidth, 100.0f),
                sizeof(float)
            );

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            if (currFPS >= 60.0f)
            {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Performance: Excellent");
            }
            else if (currFPS >= 30.0f)
            {
                ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Performance: Good");
            }
            else
            {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Performance: Poor");
            }

            ImGui::Spacing();
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("Frame Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
        }

        ImGui::End(); 
    }


    void EditorPerformancePanel::ApplyCustomTheme()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        auto& colors = style.Colors;

      
        // --- Window & Background ---
        colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.08f, 0.08f, 1.00f); // Deep Charcoal-Red
        colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_PopupBg] = ImVec4(0.14f, 0.10f, 0.10f, 0.94f);
        colors[ImGuiCol_Border] = ImVec4(0.35f, 0.20f, 0.20f, 0.50f); // Muted Red-Brown Border

        // --- Headers (The blue bars you wanted to align) ---
        colors[ImGuiCol_Header] = ImVec4(0.65f, 0.10f, 0.10f, 0.70f); // Crimson Header
        colors[ImGuiCol_HeaderHovered] = ImVec4(0.85f, 0.15f, 0.15f, 0.80f); // Brighter Red on Hover
        colors[ImGuiCol_HeaderActive] = ImVec4(0.95f, 0.20f, 0.20f, 1.00f);

        // --- Buttons ---
        colors[ImGuiCol_Button] = ImVec4(0.45f, 0.10f, 0.10f, 0.60f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(0.70f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_ButtonActive] = ImVec4(0.90f, 0.10f, 0.10f, 1.00f);

        // --- Tabs ---
        colors[ImGuiCol_Tab] = ImVec4(0.20f, 0.10f, 0.10f, 0.80f);
        colors[ImGuiCol_TabHovered] = ImVec4(0.75f, 0.15f, 0.15f, 0.80f);
        colors[ImGuiCol_TabActive] = ImVec4(0.55f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_TabUnfocused] = ImVec4(0.15f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.30f, 0.12f, 0.12f, 1.00f);

        // --- Frame & Widgets (Inputs, Checkboxes) ---
        colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.12f, 0.12f, 0.54f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(0.45f, 0.15f, 0.15f, 0.40f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(0.60f, 0.15f, 0.15f, 0.67f);
        colors[ImGuiCol_CheckMark] = ImVec4(0.95f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 0.25f, 0.25f, 1.00f);

        // --- Title ---
        colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.05f, 0.05f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.35f, 0.10f, 0.10f, 1.00f);

        // --- Styling ---
        style.WindowRounding = 4.0f;
        style.FrameRounding = 3.0f;
        style.PopupRounding = 4.0f;
        style.GrabRounding = 3.0f;
        style.TabRounding = 4.0f;
        style.ScrollbarRounding = 9.0f;
    }
}