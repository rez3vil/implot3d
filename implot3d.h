//--------------------------------------------------
// ImPlot3D v0.1
// implot3d.h
// Date: 2024-11-16
// By brenocq
//--------------------------------------------------

// Table of Contents:
// [SECTION] Macros and Defines
// [SECTION] Forward declarations and basic types
// [SECTION] Context
// [SECTION] Begin/End Plot
// [SECTION] Demo
// [SECTION] Flags & Enumerations
// [SECTION] ImPlot3DStyle

#pragma once
#include "imgui.h"
#ifndef IMGUI_DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Macros and Defines
//-----------------------------------------------------------------------------

#ifndef IMPLOT3D_API
#define IMPLOT3D_API
#endif

#define IMPLOT3D_VERSION "0.1"
#define IMPLOT3D_TMP template <typename T> IMPLOT3D_API

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

// Forward declarations
struct ImPlot3DContext;
struct ImPlot3DStyle;

// Enums/Flags
typedef int ImPlot3DFlags; // -> enum ImPlot3DFlags_             // Flags: for BeginPlot()

namespace ImPlot3D {

//-----------------------------------------------------------------------------
// [SECTION] Context
//-----------------------------------------------------------------------------
IMPLOT3D_API ImPlot3DContext* CreateContext();
IMPLOT3D_API void DestroyContext(ImPlot3DContext* ctx = nullptr);
IMPLOT3D_API ImPlot3DContext* GetCurrentContext();
IMPLOT3D_API void SetCurrentContext(ImPlot3DContext* ctx);

//-----------------------------------------------------------------------------
// [SECTION] Begin/End Plot
//-----------------------------------------------------------------------------

// Starts a 3D plotting context. If this function returns true, EndPlot() MUST
// be called! You are encouraged to use the following convention:
//
// if (ImPlot3D::BeginPlot(...)) {
//     ImPlot3D::PlotLine(...);
//     ...
//     ImPlot3D::EndPlot();
// }
//
// Important notes:
// - #title_id must be unique to the current ImGui ID scope. If you need to avoid ID
//   collisions or don't want to display a title in the plot, use double hashes
//   (e.g. "MyPlot##HiddenIdText" or "##NoTitle").
// - #size is the **frame** size of the plot widget, not the plot area.
IMPLOT3D_API bool BeginPlot(const char* title_id, const ImVec2& size = ImVec2(-1, 0), ImPlot3DFlags flags = 0);
IMPLOT3D_API void EndPlot(); // Only call if BeginPlot() returns true!

//-----------------------------------------------------------------------------
// [SECTION] Demo
//-----------------------------------------------------------------------------

// Shows the ImPlot3D demo window (add implot3d_demo.cpp to your sources!)
IMPLOT3D_API void ShowDemoWindow(bool* p_open = nullptr);

} // namespace ImPlot3D

//-----------------------------------------------------------------------------
// [SECTION] Flags & Enumerations
//-----------------------------------------------------------------------------

// Flags for ImPlot3D::BeginPlot()
enum ImPlot3DFlags_ {
    ImPlot3DFlags_None = 0,         // default
    ImPlot3DFlags_NoTitle = 1 << 0, // hide plot title

};

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DStyle
//-----------------------------------------------------------------------------

struct ImPlot3DStyle {
    ImVec2 PlotDefaultSize;
    ImVec2 PlotMinSize;
    IMPLOT3D_API ImPlot3DStyle();
};

#endif // #ifndef IMGUI_DISABLE
