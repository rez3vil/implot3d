//--------------------------------------------------
// ImPlot3D v0.1
// implot3d_demo.cpp
// Date: 2024-11-17
// By brenocq
//
// Acknowledgments:
//  ImPlot3D is heavily inspired by ImPlot
//  (https://github.com/epezent/implot) by Evan Pezent,
//  and follows a similar code style and structure to
//  maintain consistency with ImPlot's API.
//--------------------------------------------------

// Table of Contents:
// [SECTION] Helpers
// [SECTION] Plots
// [SECTION] Demo Window
// [SECTION] Style Editor

#include "implot3d.h"
#include "implot3d_internal.h"

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
    static float x1[] = {0.0f, 0.1f, NAN, 0.1f};
    static float y1[] = {0.0f, 0.0f, NAN, 0.5f};
    static float z1[] = {0.0f, 0.1f, NAN, -0.5f};
    static float x2[] = {-0.5f, -0.4f, 0.5f, -0.5f};
    static float y2[] = {-0.5f, -0.4f, 0.5f, 0.5f};
    static float z2[] = {-0.5f, -0.4f, 0.5f, -0.5f};
    if (ImPlot3D::BeginPlot("Line Plots", ImVec2(-1, 400))) {
        ImPlot3D::SetupLegend(ImPlot3DLocation_NorthWest, ImPlot3DLegendFlags_Horizontal);
        ImPlot3D::SetupAxis(ImAxis3D_X, "X-Axis");
        ImPlot3D::SetupAxis(ImAxis3D_Y, "Y-Axis");
        ImPlot3D::SetupAxis(ImAxis3D_Z, "Z-Axis");

        ImPlot3D::SetNextMarkerStyle(ImPlot3DMarker_Circle, 2, ImVec4(1, 0, 0, 1), 1, ImVec4(0, 1, 0, 1));
        ImPlot3D::SetNextLineStyle(ImVec4(0.2f, 0.8f, 0.4f, 1));
        ImPlot3D::PlotLine("Line loop", x1, y1, z1, 4, ImPlot3DLineFlags_Loop | ImPlot3DLineFlags_SkipNaN);

        ImPlot3D::SetNextLineStyle(ImVec4(0.8f, 0.2f, 0.4f, 1));
        ImPlot3D::PlotLine("Red line", x2, y2, z2, 2);

        ImPlot3D::EndPlot();
    }
}

void DemoScatterPlots() {
    static float x[] = {0.0f, 0.1f, 0.5f};
    static float y[] = {0.0f, 0.1f, 0.5f};
    static float z[] = {0.0f, 0.1f, 0.5f};
    if (ImPlot3D::BeginPlot("Scatter Plots", ImVec2(-1, 400), ImPlot3DFlags_NoClip)) {
        ImPlot3D::SetNextMarkerStyle(ImPlot3DMarker_Asterisk, 4, ImVec4(0.3, 0.7, 0.6, 1), 1, ImVec4(0.3, 0.8, 0.9, 1));
        ImPlot3D::PlotScatter("Points", x, y, z, 3);
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

    ImGui::Text("ImPlot3D says olá! (%s)", IMPLOT3D_VERSION);

    ImGui::Spacing();

    if (ImGui::BeginTabBar("ImPlot3DDemoTabs")) {
        if (ImGui::BeginTabItem("Plots")) {
            DemoHeader("Line Plots", DemoLinePlots);
            DemoHeader("Scatter Plots", DemoScatterPlots);
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

bool ShowStyleSelector(const char* label) {
    static int style_idx = -1;
    if (ImGui::Combo(label, &style_idx, "Auto\0Classic\0Dark\0Light\0")) {
        switch (style_idx) {
            case 0: StyleColorsAuto(); break;
            case 1: StyleColorsClassic(); break;
            case 2: StyleColorsDark(); break;
            case 3: StyleColorsLight(); break;
        }
        return true;
    }
    return false;
}

void ShowStyleEditor(ImPlot3DStyle* ref) {
    // Handle style internal storage
    ImPlot3DStyle& style = GetStyle();
    static ImPlot3DStyle ref_saved_style;
    static bool init = true;
    if (init && ref == nullptr)
        ref_saved_style = style;
    init = false;
    if (ref == nullptr)
        ref = &ref_saved_style;

    // Handle flash style color
    static float flash_color_time = 0.5f;
    static ImPlot3DCol flash_color_idx = ImPlot3DCol_COUNT;
    static ImVec4 flash_color_backup = ImVec4(0, 0, 0, 0);
    if (flash_color_idx != ImPlot3DCol_COUNT) {
        // Flash color
        ImVec4& color = style.Colors[flash_color_idx];
        ImGui::ColorConvertHSVtoRGB(ImCos(flash_color_time * 6.0f) * 0.5f + 0.5f, 0.5f, 0.5f, color.x, color.y, color.z);
        color.w = 1.0f;

        // Decrease timer until zero
        if ((flash_color_time -= ImGui::GetIO().DeltaTime) <= 0.0f) {
            // When timer reaches zero, restore the backup color
            style.Colors[flash_color_idx] = flash_color_backup;
            flash_color_idx = ImPlot3DCol_COUNT;
            flash_color_time = 0.5f;
        }
    }

    // Style selector
    if (ImPlot3D::ShowStyleSelector("Colors##Selector"))
        ref_saved_style = style;

    // Save/Revert button
    if (ImGui::Button("Save Ref"))
        *ref = ref_saved_style = style;
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
            ImGui::Text("Item Styling");
            ImGui::SliderFloat("LineWeight", &style.LineWeight, 0.0f, 5.0f, "%.1f");
            ImGui::SliderFloat("MarkerSize", &style.MarkerSize, 2.0f, 10.0f, "%.1f");
            ImGui::SliderFloat("MarkerWeight", &style.MarkerWeight, 0.0f, 5.0f, "%.1f");
            ImGui::Text("Plot Styling");
            ImGui::SliderFloat2("PlotDefaultSize", (float*)&style.PlotDefaultSize, 0.0f, 1000, "%.0f");
            ImGui::SliderFloat2("PlotMinSize", (float*)&style.PlotMinSize, 0.0f, 300, "%.0f");
            ImGui::SliderFloat2("PlotPadding", (float*)&style.PlotPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("LabelPadding", (float*)&style.LabelPadding, 0.0f, 20.0f, "%.0f");
            ImGui::Text("Legend Styling");
            ImGui::SliderFloat2("LegendPadding", (float*)&style.LegendPadding, 0.0f, 20.0f, "%.0f");
            ImGui::SliderFloat2("LegendInnerPadding", (float*)&style.LegendInnerPadding, 0.0f, 10.0f, "%.0f");
            ImGui::SliderFloat2("LegendSpacing", (float*)&style.LegendSpacing, 0.0f, 5.0f, "%.0f");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Colors")) {
            static int output_dest = 0;
            static bool output_only_modified = true;
            if (ImGui::Button("Export")) {
                if (output_dest == 0)
                    ImGui::LogToClipboard();
                else
                    ImGui::LogToTTY();
                ImGui::LogText("ImVec4* colors = ImPlot3D::GetStyle().Colors;\n");
                for (int i = 0; i < ImPlot3DCol_COUNT; i++) {
                    const ImVec4& col = style.Colors[i];
                    const char* name = ImPlot3D::GetStyleColorName(i);
                    if (!output_only_modified || memcmp(&col, &ref->Colors[i], sizeof(ImVec4)) != 0)
                        ImGui::LogText("colors[ImPlot3DCol_%s]%*s= ImVec4(%.2ff, %.2ff, %.2ff, %.2ff);\n",
                                       name, 15 - (int)strlen(name), "", col.x, col.y, col.z, col.w);
                }
                ImGui::LogFinish();
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(120);
            ImGui::Combo("##output_type", &output_dest, "To Clipboard\0To TTY\0");
            ImGui::SameLine();
            ImGui::Checkbox("Only Modified Colors", &output_only_modified);

            static ImGuiTextFilter filter;
            filter.Draw("Filter colors", ImGui::GetFontSize() * 16);

            static ImGuiColorEditFlags alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf;
            if (ImGui::RadioButton("Opaque", alpha_flags == ImGuiColorEditFlags_None))
                alpha_flags = ImGuiColorEditFlags_None;
            ImGui::SameLine();
            if (ImGui::RadioButton("Alpha", alpha_flags == ImGuiColorEditFlags_AlphaPreview))
                alpha_flags = ImGuiColorEditFlags_AlphaPreview;
            ImGui::SameLine();
            if (ImGui::RadioButton("Both", alpha_flags == ImGuiColorEditFlags_AlphaPreviewHalf))
                alpha_flags = ImGuiColorEditFlags_AlphaPreviewHalf;
            ImGui::SameLine();
            HelpMarker(
                "In the color list:\n"
                "Left-click on color square to open color picker,\n"
                "Right-click to open edit options menu.");

            ImGui::Separator();

            for (int i = 0; i < ImPlot3DCol_COUNT; i++) {
                const char* name = ImPlot3D::GetStyleColorName(i);
                if (!filter.PassFilter(name))
                    continue;
                ImGui::PushID(i);

                // Flash color
                if (ImGui::Button("?")) {
                    if (flash_color_idx != ImPlot3DCol_COUNT)
                        style.Colors[flash_color_idx] = flash_color_backup;
                    flash_color_time = 0.5f;
                    flash_color_idx = (ImPlot3DCol)i;
                    flash_color_backup = style.Colors[i];
                }
                ImGui::SetItemTooltip("Flash given color to identify places where it is used.");
                ImGui::SameLine();

                // Handle auto color selection
                const bool is_auto = IsColorAuto(style.Colors[i]);
                if (is_auto)
                    ImGui::BeginDisabled();
                if (ImGui::Button("Auto"))
                    style.Colors[i] = IMPLOT3D_AUTO_COL;
                if (is_auto)
                    ImGui::EndDisabled();

                // Color selection
                ImGui::SameLine();
                if (ImGui::ColorEdit4("##Color", (float*)&style.Colors[i], ImGuiColorEditFlags_NoInputs | alpha_flags)) {
                    if (style.Colors[i].w == -1)
                        style.Colors[i].w = 1;
                }

                // Save/Revert buttons if color changed
                if (memcmp(&style.Colors[i], &ref->Colors[i], sizeof(ImVec4)) != 0) {
                    ImGui::SameLine();
                    if (ImGui::Button("Save"))
                        ref->Colors[i] = style.Colors[i];
                    ImGui::SameLine();
                    if (ImGui::Button("Revert"))
                        style.Colors[i] = ref->Colors[i];
                }
                ImGui::SameLine();
                ImGui::TextUnformatted(name);
                ImGui::PopID();
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
}

} // namespace ImPlot3D
