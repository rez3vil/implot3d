//--------------------------------------------------
// ImPlot3D v0.1
// implot3d.cpp
// Date: 2024-11-16
// By brenocq
//--------------------------------------------------

// Table of Contents:
// [SECTION] Includes
// [SECTION] Macros
// [SECTION] Context
// [SECTION] Begin/End Plot
// [SECTION] ImPlot3DStyle
// [SECTION] Context Utils

//-----------------------------------------------------------------------------
// [SECTION] Includes
//-----------------------------------------------------------------------------

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "implot3d.h"
#include "implot3d_internal.h"

#ifndef IMGUI_DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Macros
//-----------------------------------------------------------------------------

#define IMPLOT3D_CHECK_CTX() IM_ASSERT_USER_ERROR(GImPlot3D != nullptr, "No current context. Did you call ImPlot3D::CreateContext() or ImPlot3D::SetCurrentContext()?")
#define IMPLOT3D_CHECK_PLOT() IM_ASSERT_USER_ERROR(GImPlot3D->CurrentPlot != nullptr, "No active plot. Did you call ImPlot3D::BeginPlot()?")

//-----------------------------------------------------------------------------
// [SECTION] Context
//-----------------------------------------------------------------------------

// Global plot context
#ifndef GImPlot3D
ImPlot3DContext* GImPlot3D = nullptr;
#endif

ImPlot3DContext* ImPlot3D::CreateContext() {
    ImPlot3DContext* ctx = IM_NEW(ImPlot3DContext)();
    if (GImPlot3D == nullptr)
        SetCurrentContext(ctx);
    InitializeContext(ctx);
    return ctx;
}

void ImPlot3D::DestroyContext(ImPlot3DContext* ctx) {
    if (ctx == nullptr)
        ctx = GImPlot3D;
    if (GImPlot3D == ctx)
        SetCurrentContext(nullptr);
    IM_DELETE(ctx);
}

ImPlot3DContext* ImPlot3D::GetCurrentContext() { return GImPlot3D; }

void ImPlot3D::SetCurrentContext(ImPlot3DContext* ctx) { GImPlot3D = ctx; }

//-----------------------------------------------------------------------------
// [SECTION] Begin/End Plot
//-----------------------------------------------------------------------------
bool ImPlot3D::BeginPlot(const char* title_id, const ImVec2& size, ImPlot3DFlags flags) {
    IMPLOT3D_CHECK_CTX();
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot == nullptr, "Mismatched BeginPlot()/EndPlot()!");

    // Get globals
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Skip if needed
    if (window->SkipItems)
        return false;

    // Get or create plot
    const ImGuiID ID = window->GetID(title_id);
    gp.CurrentPlot = gp.Plots.GetOrAddByKey(ID);
    ImPlot3DPlot& plot = *gp.CurrentPlot;

    // Populate plot ID/flags
    plot.ID = ID;
    plot.Flags = flags;

    // Populate title
    if (title_id && ImGui::FindRenderedTextEnd(title_id, nullptr) != title_id && !(plot.Flags & ImPlot3DFlags_NoTitle))
        plot.TextBuffer.append(title_id, title_id + strlen(title_id) + 1);
    else
        plot.TextBuffer.clear();

    // Calculate frame size
    ImVec2 frame_size = ImGui::CalcItemSize(size, gp.Style.PlotDefaultSize.x, gp.Style.PlotDefaultSize.y);
    if (frame_size.x < gp.Style.PlotMinSize.x && size.x < 0.0f)
        frame_size.x = gp.Style.PlotMinSize.x;
    if (frame_size.y < gp.Style.PlotMinSize.y && size.y < 0.0f)
        frame_size.y = gp.Style.PlotMinSize.y;

    plot.FrameRect = ImRect(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    ImGui::ItemSize(plot.FrameRect);
    if (!ImGui::ItemAdd(plot.FrameRect, plot.ID, &plot.FrameRect)) {
        gp.CurrentPlot = nullptr;
        return false;
    }

    return true;
}

void AddTextCentered(ImDrawList* DrawList, ImVec2 top_center, ImU32 col, const char* text_begin, const char* text_end) {
    float txt_ht = ImGui::GetTextLineHeight();
    const char* title_end = ImGui::FindRenderedTextEnd(text_begin, text_end);
    ImVec2 text_size;
    float y = 0;
    while (const char* tmp = (const char*)memchr(text_begin, '\n', title_end - text_begin)) {
        text_size = ImGui::CalcTextSize(text_begin, tmp, true);
        DrawList->AddText(ImVec2(top_center.x - text_size.x * 0.5f, top_center.y + y), col, text_begin, tmp);
        text_begin = tmp + 1;
        y += txt_ht;
    }
    text_size = ImGui::CalcTextSize(text_begin, title_end, true);
    DrawList->AddText(ImVec2(top_center.x - text_size.x * 0.5f, top_center.y + y), col, text_begin, title_end);
}

void ImPlot3D::EndPlot() {
    IMPLOT3D_CHECK_CTX();
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "Mismatched BeginPlot()/EndPlot()!");

    ImGuiContext& g = *GImGui;
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    ImGuiWindow* window = g.CurrentWindow;
    ImDrawList& draw_list = *window->DrawList;

    // ImGui::PushClipRect(plot.FrameRect.Min, plot.FrameRect.Max, true);

    // Plot title
    if (!plot.TextBuffer.empty()) {
        ImU32 col = ImGui::GetColorU32(ImVec4(0.9, 0.9, 0.9, 1.0)); // GetStyleColorU32(ImPlotCol_TitleText);
        AddTextCentered(&draw_list, ImVec2(plot.PlotRect.GetCenter().x, plot.PlotRect.Min.y), col, plot.TextBuffer.c_str());
    }
    ImGui::TextUnformatted(plot.TextBuffer.c_str());
    ImGui::Text("Hello");

    // ImGui::PopClipRect();

    // Reset current plot
    gp.CurrentPlot = nullptr;
}

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DStyle
//-----------------------------------------------------------------------------

ImPlot3DStyle::ImPlot3DStyle() {
    PlotDefaultSize = ImVec2(400, 400);
    PlotMinSize = ImVec2(200, 200);
};

//-----------------------------------------------------------------------------
// [SECTION] Context Utils
//-----------------------------------------------------------------------------

void ImPlot3D::InitializeContext(ImPlot3DContext* ctx) {
    ResetContext(ctx);
}

void ImPlot3D::ResetContext(ImPlot3DContext* ctx) {
    ctx->CurrentPlot = nullptr;
}

#endif // #ifndef IMGUI_DISABLE
