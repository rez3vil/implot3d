//--------------------------------------------------
// ImPlot3D
// implot3d.h
// Date: 2024-11-16
// By Breno Cunha Queiroz
//--------------------------------------------------

// Table of Contents:
// [SECTION] Macros and Defines
// [SECTION] Enums and Types
// [SECTION] Context

#pragma once
#include "imgui.h"
#ifndef IMGUI_DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Macros and Defines
//-----------------------------------------------------------------------------
#ifndef IMPLOT3D_API
#define IMPLOT3D_API
#endif

#define IMPLOT3D_VERSION "v0.1"
#define IMPLOT3D_AUTO -1
#define IMPLOT3D_AUTO_COL ImVec4(0, 0, 0, -1)
#define IMPLOT3D_TMP template <typename T> IMPLOT3D_API

//-----------------------------------------------------------------------------
// [SECTION] Enums and Types
//-----------------------------------------------------------------------------

// Forward declarations
struct ImPlot3DContext;

namespace ImPlot3D {

//-----------------------------------------------------------------------------
// [SECTION] Context
//-----------------------------------------------------------------------------
IMPLOT3D_API ImPlot3DContext* CreateContext();
IMPLOT3D_API void DestroyContext(ImPlot3DContext* ctx = nullptr);
IMPLOT3D_API ImPlot3DContext* GetCurrentContext();
IMPLOT3D_API void SetCurrentContext(ImPlot3DContext* ctx);

} // namespace ImPlot3D

#endif // #ifndef IMGUI_DISABLE
