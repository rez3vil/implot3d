//--------------------------------------------------
// ImPlot3D v0.1
// implot3d_demo.cpp
// Date: 2024-11-17
// Author: Breno Cunha Queiroz (brenocq.com)
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

// Utility structure for realtime plot
struct ScrollingBuffer {
    int MaxSize;
    int Offset;
    ImVector<float> Data;
    ScrollingBuffer(int max_size = 2000) {
        MaxSize = max_size;
        Offset = 0;
        Data.reserve(MaxSize);
    }
    void AddPoint(float x) {
        if (Data.size() < MaxSize)
            Data.push_back(x);
        else {
            Data[Offset] = x;
            Offset = (Offset + 1) % MaxSize;
        }
    }
    void Erase() {
        if (Data.size() > 0) {
            Data.shrink(0);
            Offset = 0;
        }
    }
};

//-----------------------------------------------------------------------------
// [SECTION] Plots
//-----------------------------------------------------------------------------

void DemoLinePlots() {
    static float xs1[1001], ys1[1001], zs1[1001];
    for (int i = 0; i < 1001; i++) {
        xs1[i] = i * 0.001f;
        ys1[i] = 0.5f + 0.5f * cosf(50 * (xs1[i] + (float)ImGui::GetTime() / 10));
        zs1[i] = 0.5f + 0.5f * sinf(50 * (xs1[i] + (float)ImGui::GetTime() / 10));
    }
    static double xs2[20], ys2[20], zs2[20];
    for (int i = 0; i < 20; i++) {
        xs2[i] = i * 1 / 19.0f;
        ys2[i] = xs2[i] * xs2[i];
        zs2[i] = xs2[i] * ys2[i];
    }
    if (ImPlot3D::BeginPlot("Line Plots")) {
        ImPlot3D::SetupAxes("x", "y", "z");
        ImPlot3D::PlotLine("f(x)", xs1, ys1, zs1, 1001);
        ImPlot3D::SetNextMarkerStyle(ImPlot3DMarker_Circle);
        ImPlot3D::PlotLine("g(x)", xs2, ys2, zs2, 20, ImPlot3DLineFlags_Segments);
        ImPlot3D::EndPlot();
    }
}

void DemoScatterPlots() {
    srand(0);
    static float xs1[100], ys1[100], zs1[100];
    for (int i = 0; i < 100; ++i) {
        xs1[i] = i * 0.01f;
        ys1[i] = xs1[i] + 0.1f * ((float)rand() / (float)RAND_MAX);
        zs1[i] = xs1[i] + 0.1f * ((float)rand() / (float)RAND_MAX);
    }
    static float xs2[50], ys2[50], zs2[50];
    for (int i = 0; i < 50; i++) {
        xs2[i] = 0.25f + 0.2f * ((float)rand() / (float)RAND_MAX);
        ys2[i] = 0.50f + 0.2f * ((float)rand() / (float)RAND_MAX);
        zs2[i] = 0.75f + 0.2f * ((float)rand() / (float)RAND_MAX);
    }

    if (ImPlot3D::BeginPlot("Scatter Plots")) {
        ImPlot3D::PlotScatter("Data 1", xs1, ys1, zs1, 100);
        ImPlot3D::PushStyleVar(ImPlot3DStyleVar_FillAlpha, 0.25f);
        ImPlot3D::SetNextMarkerStyle(ImPlot3DMarker_Square, 6, ImPlot3D::GetColormapColor(1), IMPLOT3D_AUTO, ImPlot3D::GetColormapColor(1));
        ImPlot3D::PlotScatter("Data 2", xs2, ys2, zs1, 50);
        ImPlot3D::PopStyleVar();
        ImPlot3D::EndPlot();
    }
}

void DemoRealtimePlots() {
    ImGui::BulletText("Move your mouse to change the data!");
    static ScrollingBuffer sdata1, sdata2, sdata3;
    static ImPlot3DAxisFlags flags = ImPlot3DAxisFlags_NoTickLabels;
    static float t = 0.0f;
    static float last_t = -1.0f;

    if (ImPlot3D::BeginPlot("Scrolling Plot", ImVec2(-1, 400))) {
        // Pool mouse data every 10 ms
        t += ImGui::GetIO().DeltaTime;
        if (t - last_t > 0.01f) {
            last_t = t;
            ImVec2 mouse = ImGui::GetMousePos();
            if (ImAbs(mouse.x) < 1e4f && ImAbs(mouse.y) < 1e4f) {
                ImVec2 plot_center = ImPlot3D::GetFramePos();
                plot_center.x += ImPlot3D::GetFrameSize().x / 2;
                plot_center.y += ImPlot3D::GetFrameSize().y / 2;
                sdata1.AddPoint(t);
                sdata2.AddPoint(mouse.x - plot_center.x);
                sdata3.AddPoint(mouse.y - plot_center.y);
            }
        }

        ImPlot3D::SetupAxes("Time", "Mouse X", "Mouse Y", flags, flags, flags);
        ImPlot3D::SetupAxisLimits(ImAxis3D_X, t - 10.0f, t, ImPlot3DCond_Always);
        ImPlot3D::SetupAxisLimits(ImAxis3D_Y, -400, 400, ImPlot3DCond_Once);
        ImPlot3D::SetupAxisLimits(ImAxis3D_Z, -400, 400, ImPlot3DCond_Once);
        ImPlot3D::PlotLine("Mouse", &sdata1.Data[0], &sdata2.Data[0], &sdata3.Data[0], sdata1.Data.size(), 0, sdata1.Offset, sizeof(float));
        ImPlot3D::EndPlot();
    }
}

void DemoMarkersAndText() {
    static float mk_size = ImPlot3D::GetStyle().MarkerSize;
    static float mk_weight = ImPlot3D::GetStyle().MarkerWeight;
    ImGui::DragFloat("Marker Size", &mk_size, 0.1f, 2.0f, 10.0f, "%.2f px");
    ImGui::DragFloat("Marker Weight", &mk_weight, 0.05f, 0.5f, 3.0f, "%.2f px");

    if (ImPlot3D::BeginPlot("##MarkerStyles", ImVec2(-1, 0), ImPlot3DFlags_CanvasOnly)) {

        ImPlot3D::SetupAxes(nullptr, nullptr, nullptr, ImPlot3DAxisFlags_NoDecorations, ImPlot3DAxisFlags_NoDecorations, ImPlot3DAxisFlags_NoDecorations);
        ImPlot3D::SetupAxesLimits(-0.5, 1.5, -0.5, 1.5, 0, ImPlot3DMarker_COUNT + 1);

        float xs[2] = {0, 0};
        float ys[2] = {0, 0};
        float zs[2] = {ImPlot3DMarker_COUNT, ImPlot3DMarker_COUNT + 1};

        // Filled markers
        for (int m = 0; m < ImPlot3DMarker_COUNT; ++m) {
            xs[1] = xs[0] + ImCos(zs[0] / float(ImPlot3DMarker_COUNT) * 2 * M_PI) * 0.5;
            ys[1] = ys[0] + ImSin(zs[0] / float(ImPlot3DMarker_COUNT) * 2 * M_PI) * 0.5;

            ImGui::PushID(m);
            ImPlot3D::SetNextMarkerStyle(m, mk_size, IMPLOT3D_AUTO_COL, mk_weight);
            ImPlot3D::PlotLine("##Filled", xs, ys, zs, 2);
            ImGui::PopID();
            zs[0]--;
            zs[1]--;
        }

        xs[0] = 1;
        ys[0] = 1;
        zs[0] = ImPlot3DMarker_COUNT;
        zs[1] = zs[0] + 1;

        // Open markers
        for (int m = 0; m < ImPlot3DMarker_COUNT; ++m) {
            xs[1] = xs[0] + ImCos(zs[0] / float(ImPlot3DMarker_COUNT) * 2 * M_PI) * 0.5;
            ys[1] = ys[0] - ImSin(zs[0] / float(ImPlot3DMarker_COUNT) * 2 * M_PI) * 0.5;

            ImGui::PushID(m);
            ImPlot3D::SetNextMarkerStyle(m, mk_size, ImVec4(0, 0, 0, 0), mk_weight);
            ImPlot3D::PlotLine("##Open", xs, ys, zs, 2);
            ImGui::PopID();
            zs[0]--;
            zs[1]--;
        }

        // ImPlot3D::PlotText("Filled Markers", 2.5f, 6.0f);
        // ImPlot3D::PlotText("Open Markers", 7.5f, 6.0f);

        // ImPlot3D::PushStyleColor(ImPlot3DCol_InlayText, ImVec4(1, 0, 1, 1));
        // ImPlot3D::PlotText("Vertical Text", 5.0f, 6.0f, ImVec2(0, 0), ImPlot3DTextFlags_Vertical);
        // ImPlot3D::PopStyleColor();

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
            DemoHeader("Realtime Plots", DemoRealtimePlots);
            DemoHeader("Markers and Text", DemoMarkersAndText);
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
            ImGui::SliderFloat("FillAlpha", &style.FillAlpha, 0.0f, 1.0f, "%.2f");
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
