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
// [SECTION] Context Utils

//-----------------------------------------------------------------------------
// [SECTION] Includes
//-----------------------------------------------------------------------------

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

    return true;
}

void ImPlot3D::EndPlot() {
    IMPLOT3D_CHECK_CTX();
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "Mismatched BeginPlot()/EndPlot()!");

    ImGuiContext& g = *GImGui;
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    ImGuiWindow* window = g.CurrentWindow;

    // Plot title
    if (!plot.TextBuffer.empty())
        ImGui::TextUnformatted(plot.TextBuffer.c_str());

    // Reset current plot
    gp.CurrentPlot = nullptr;
}

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
