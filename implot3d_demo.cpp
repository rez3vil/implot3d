//--------------------------------------------------
// ImPlot3D v0.1
// implot3d_demo.cpp
// Date: 2024-11-17
// By brenocq
//--------------------------------------------------

// Table of Contents:
// [SECTION] Plots
// [SECTION] Demo Window

#include "implot3d.h"

namespace ImPlot3D {

void DemoHelp() {
    ImGui::SeparatorText("ABOUT THIS DEMO:");
    ImGui::BulletText("The other tabs are demonstrating many aspects of the library.");

    ImGui::SeparatorText("PROGRAMMER GUIDE:");
    ImGui::BulletText("See the ShowDemoWindow() code in implot3d_demo.cpp. <- you are here!");
    ImGui::BulletText("See comments in implot3d_demo.cpp.");
    ImGui::BulletText("See example application in example/ folder.");

    ImGui::SeparatorText("USER GUIDE:");
    ImGui::BulletText("TODO");
}

//-----------------------------------------------------------------------------
// [SECTION] Plots
//-----------------------------------------------------------------------------

void DemoLinePlots() {
    if (ImPlot3D::BeginPlot("Line Plot", ImVec2(400, 400), ImPlot3DFlags_None)) {
        ImPlot3D::EndPlot();
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Demo Window
//-----------------------------------------------------------------------------

void DemoHeader(const char* label, void (*demo)()) {
    if (ImGui::TreeNodeEx(label)) {
        demo();
        ImGui::TreePop();
    }
}

void ShowDemoWindow(bool* p_open) {
    static bool show_imgui_metrics = false;
    static bool show_imgui_style_editor = false;
    static bool show_imgui_demo = false;

    if (show_imgui_style_editor) {
        ImGui::Begin("Style Editor (ImGui)", &show_imgui_style_editor);
        ImGui::ShowStyleEditor();
        ImGui::End();
    }
    if (show_imgui_metrics)
        ImGui::ShowMetricsWindow(&show_imgui_metrics);
    if (show_imgui_demo)
        ImGui::ShowDemoWindow(&show_imgui_demo);

    ImGui::Begin("ImPlot3D Demo", p_open, ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Tools")) {
            ImGui::MenuItem("ImGui Metrics", nullptr, &show_imgui_metrics);
            ImGui::MenuItem("ImGui Style Editor", nullptr, &show_imgui_style_editor);
            ImGui::MenuItem("ImGui Demo", nullptr, &show_imgui_demo);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    ImGui::Text("ImPlot3D says hello! (%s)", IMPLOT3D_VERSION);

    ImGui::Spacing();

    if (ImGui::BeginTabBar("ImPlot3DDemoTabs")) {
        if (ImGui::BeginTabItem("Plots")) {
            DemoHeader("Line Plots", DemoLinePlots);
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Help")) {
            DemoHelp();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

} // namespace ImPlot3D
