//--------------------------------------------------
// ImPlot3D v0.1
// implot3d_demo.cpp
// Date: 2024-11-17
// By brenocq
//--------------------------------------------------

// Table of Contents:
// [SECTION] Helpers
// [SECTION] Plots
// [SECTION] Demo Window
// [SECTION] Style Editor

#include "implot3d.h"

namespace ImPlot3D {

//-----------------------------------------------------------------------------
// [SECTION] Helpers
//-----------------------------------------------------------------------------

static void HelpMarker(const char* desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip()) {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
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

void DemoHeader(const char* label, void (*demo)()) {
    if (ImGui::TreeNodeEx(label)) {
        demo();
        ImGui::TreePop();
    }
}

void ShowDemoWindow(bool* p_open) {
    static bool show_implot3d_style_editor = false;
    static bool show_imgui_metrics = false;
    static bool show_imgui_style_editor = false;
    static bool show_imgui_demo = false;

    if (show_implot3d_style_editor) {
        ImGui::Begin("Style Editor (ImPlot3D)", &show_implot3d_style_editor);
        ImPlot3D::ShowStyleEditor();
        ImGui::End();
    }

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
            ImGui::MenuItem("Style Editor", nullptr, &show_implot3d_style_editor);
            ImGui::Separator();
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

//-----------------------------------------------------------------------------
// [SECTION] Style Editor
//-----------------------------------------------------------------------------

void ShowStyleEditor(ImPlot3DStyle* ref) {
    static ImPlot3DStyle style;
    static bool initialized = false; // Default to using internal storage as reference

    if (!initialized)
        style = ref == nullptr ? ImPlot3D::GetStyle() : *ref;
    initialized = true;
    if (ref == nullptr)
        ref = &ImPlot3D::GetStyle();

    // Save/Revert button
    if (ImGui::Button("Save Ref"))
        *ref = style;
    ImGui::SameLine();
    if (ImGui::Button("Revert Ref"))
        style = *ref;
    ImGui::SameLine();
    HelpMarker(
        "Save/Revert in local non-persistent storage. Default Colors definition are not affected. "
        "Use \"Export\" below to save them somewhere.");

    ImGui::Separator();

    if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Sizes")) {
            ImGui::Text("Sizes");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Colors")) {
            ImGui::Text("Colors");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

} // namespace ImPlot3D
