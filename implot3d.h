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
// [SECTION] Styles
// [SECTION] Demo
// [SECTION] Debugging
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

#define IMPLOT3D_VERSION "0.1"                // ImPlot3D version
#define IMPLOT3D_AUTO -1                      // Deduce variable automatically
#define IMPLOT3D_AUTO_COL ImVec4(0, 0, 0, -1) // Deduce color automatically
#define IMPLOT3D_TMP template <typename T> IMPLOT3D_API

//-----------------------------------------------------------------------------
// [SECTION] Forward declarations and basic types
//-----------------------------------------------------------------------------

// Forward declarations
struct ImPlot3DContext;
struct ImPlot3DStyle;
struct ImPlot3DVec3;
struct ImPlot3DQuat;

// Enums/Flags
typedef int ImPlot3DFlags; // -> enum ImPlot3DFlags_           // Flags: for BeginPlot()
typedef int ImPlot3DCol;   // -> enum ImPlot3DCol_             // Enum: Styling colors

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
// [SECTION] Styles
//-----------------------------------------------------------------------------

// Get current style
IMPLOT3D_API ImPlot3DStyle& GetStyle();

// Set color styles
IMPLOT3D_API void StyleColorsAuto(ImPlot3DStyle* dst = nullptr);    // Set colors with ImGui style
IMPLOT3D_API void StyleColorsDark(ImPlot3DStyle* dst = nullptr);    // Set colors with dark style
IMPLOT3D_API void StyleColorsLight(ImPlot3DStyle* dst = nullptr);   // Set colors with light style
IMPLOT3D_API void StyleColorsClassic(ImPlot3DStyle* dst = nullptr); // Set colors with classic style

// Get color
IMPLOT3D_API ImVec4 GetStyleColorVec4(ImPlot3DCol idx);
IMPLOT3D_API ImU32 GetStyleColorU32(ImPlot3DCol idx);

//-----------------------------------------------------------------------------
// [SECTION] Demo
//-----------------------------------------------------------------------------
// Add implot3d_demo.cpp to your sources to use methods in this section

// Shows the ImPlot3D demo window
IMPLOT3D_API void ShowDemoWindow(bool* p_open = nullptr);

// Shows ImPlot3D style editor block (not a window)
IMPLOT3D_API void ShowStyleEditor(ImPlot3DStyle* ref = nullptr);

} // namespace ImPlot3D

//-----------------------------------------------------------------------------
// [SECTION] Flags & Enumerations
//-----------------------------------------------------------------------------

// Flags for ImPlot3D::BeginPlot()
enum ImPlot3DFlags_ {
    ImPlot3DFlags_None = 0,         // default
    ImPlot3DFlags_NoTitle = 1 << 0, // hide plot title
};

enum ImPlot3DCol_ {
    ImPlot3DCol_TitleText,    // Title color
    ImPlot3DCol_FrameBg,      // Frame background color
    ImPlot3DCol_PlotBg,       // Plot area background color
    ImPlot3DCol_PlotBorder,   // Plot area border color
    ImPlot3DCol_LegendBg,     // Legend background color
    ImPlot3DCol_LegendBorder, // Legend border color
    ImPlot3DCol_LegendText,   // Legend text color
    ImPlot3DCol_COUNT,
};

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DStyle
//-----------------------------------------------------------------------------

struct ImPlot3DStyle {
    // Plot style
    ImVec2 PlotDefaultSize;
    ImVec2 PlotMinSize;
    ImVec2 PlotPadding;
    ImVec2 LabelPadding;
    // Colors
    ImVec4 Colors[ImPlot3DCol_COUNT];
    // Constructor
    IMPLOT3D_API ImPlot3DStyle();
};

#endif // #ifndef IMGUI_DISABLE
