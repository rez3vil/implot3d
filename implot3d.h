//--------------------------------------------------
// ImPlot3D v0.1
// implot3d.h
// Date: 2024-11-16
// By brenocq
//
// Acknowledgments:
//  This library is heavily inspired by ImPlot
//  (https://github.com/epezent/implot) by Evan Pezent,
//  and follows a similar code style and structure to
//  maintain consistency with ImPlot's API.
//--------------------------------------------------

// Table of Contents:
// [SECTION] Macros and Defines
// [SECTION] Forward declarations and basic types
// [SECTION] Context
// [SECTION] Begin/End Plot
// [SECTION] Plot Items
// [SECTION] Miscellaneous
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

// Enums
typedef int ImPlot3DCol;    // -> ImPlot3DCol_                 // Enum: Styling colors
typedef int ImPlot3DMarker; // -> ImPlot3DMarker_              // Enum: Marker styles

// Flags
typedef int ImPlot3DFlags;        // -> ImPlot3DFlags_         // Flags: for BeginPlot()
typedef int ImPlot3DScatterFlags; // -> ImPlot3DScatterFlags_  // Flags: Scatter plot flags
typedef int ImPlot3DItemFlags;    // -> ImPlot3DItemFlags_     // Flags: Item flags

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
// [SECTION] Plot Items
//-----------------------------------------------------------------------------

IMPLOT3D_TMP void PlotScatter(const char* label_id, const T* xs, const T* ys, const T* zs, int count, ImPlot3DScatterFlags flags = 0, int offset = 0, int stride = sizeof(T));

//-----------------------------------------------------------------------------
// [SECTION] Miscellaneous
//-----------------------------------------------------------------------------

IMPLOT3D_API ImDrawList* GetPlotDrawList();

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
    // Item colors
    ImPlot3DCol_Line,          // Line color
    ImPlot3DCol_MarkerOutline, // Marker outline color
    ImPlot3DCol_MarkerFill,    // Marker fill color
    // Plot colors
    ImPlot3DCol_TitleText,    // Title color
    ImPlot3DCol_FrameBg,      // Frame background color
    ImPlot3DCol_PlotBg,       // Plot area background color
    ImPlot3DCol_PlotBorder,   // Plot area border color
    ImPlot3DCol_LegendBg,     // Legend background color
    ImPlot3DCol_LegendBorder, // Legend border color
    ImPlot3DCol_LegendText,   // Legend text color
    ImPlot3DCol_COUNT,
};

enum ImPlot3DMarker_ {
    ImPlot3DMarker_None = -1, // No marker
    ImPlot3DMarker_Circle,    // Circle marker (default)
    ImPlot3DMarker_Square,    // Square maker
    ImPlot3DMarker_Diamond,   // Diamond marker
    ImPlot3DMarker_Up,        // Upward-pointing triangle marker
    ImPlot3DMarker_Down,      // Downward-pointing triangle marker
    ImPlot3DMarker_Left,      // Leftward-pointing triangle marker
    ImPlot3DMarker_Right,     // Rightward-pointing triangle marker
    ImPlot3DMarker_Cross,     // Cross marker (not fillable)
    ImPlot3DMarker_Plus,      // Plus marker (not fillable)
    ImPlot3DMarker_Asterisk,  // Asterisk marker (not fillable)
    ImPlot3DMarker_COUNT
};

// Flags for PlotScatter
enum ImPlot3DScatterFlags_ {
    ImPlot3DScatterFlags_None = 0,        // Default
    ImPlot3DScatterFlags_NoClip = 1 << 0, // Markers on the edge of a plot will not be clipped
};

enum ImPlot3DItemFlags_ {
    ImPlot3DItemFlags_None = 0,          // Default
    ImPlot3DItemFlags_NoLegend = 1 << 0, // The item won't have a legend entry displayed
    ImPlot3DItemFlags_NoFit = 1 << 1,    // The item won't be considered for plot fits
};

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DStyle
//-----------------------------------------------------------------------------

struct ImPlot3DStyle {
    // Item style
    float LineWeight;   // Line weight in pixels
    int Marker;         // Default marker type (ImPlot3DMarker_Circle)
    float MarkerSize;   // Marker size in pixels (roughly the marker's "radius")
    float MarkerWeight; // Marker outline weight in pixels
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
