//--------------------------------------------------
// ImPlot3D v0.1
// implot3d_internal.h
// Date: 2024-11-17
// By brenocq
//--------------------------------------------------

// Table of Contents:
// [SECTION] Context Utils
// [SECTION] Style Utils
// [SECTION] Structs

#pragma once

#ifndef IMPLOT3D_VERSION
#include "implot3d.h"
#endif

#ifndef IMGUI_DISABLE
#include "imgui_internal.h"

namespace ImPlot3D {

//-----------------------------------------------------------------------------
// [SECTION] Context Utils
//-----------------------------------------------------------------------------

IMPLOT3D_API void InitializeContext(ImPlot3DContext* ctx); // Initialize ImPlot3DContext
IMPLOT3D_API void ResetContext(ImPlot3DContext* ctx);      // Reset ImPlot3DContext

//-----------------------------------------------------------------------------
// [SECTION] Style Utils
//-----------------------------------------------------------------------------

IMPLOT3D_API bool IsColorAuto(const ImVec4& col);
IMPLOT3D_API bool IsColorAuto(ImPlot3DCol idx);
IMPLOT3D_API ImVec4 GetAutoColor(ImPlot3DCol idx);
IMPLOT3D_API const char* GetStyleColorName(ImPlot3DCol idx);

} // namespace ImPlot3D

//-----------------------------------------------------------------------------
// [SECTION] Structs
//-----------------------------------------------------------------------------

// Holds Plot state information that must persist after EndPlot
struct ImPlot3DPlot {
    ImGuiID ID;
    ImPlot3DFlags Flags;
    ImGuiTextBuffer TextBuffer;
    ImRect FrameRect;  // Outermost bounding rectangle that encapsulates whole the plot/title/padding/etc
    ImRect CanvasRect; // Frame rectangle reduced by padding
    ImRect PlotRect;   // Bounding rectangle for the actual plot area
};

struct ImPlot3DContext {
    ImPool<ImPlot3DPlot> Plots;
    ImPlot3DPlot* CurrentPlot;
    ImPlot3DStyle Style;
};

#endif // #ifndef IMGUI_DISABLE
