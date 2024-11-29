//--------------------------------------------------
// ImPlot3D v0.1
// implot3d_internal.h
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
// [SECTION] Generic Helpers
// [SECTION] Structs
// [SECTION] Context Pointer
// [SECTION] Context Utils
// [SECTION] Style Utils
// [SECTION] Item Utils
// [SECTION] Plot Utils
// [SECTION] Setup Utils

#pragma once

#ifndef IMPLOT3D_VERSION
#include "implot3d.h"
#endif

#ifndef IMGUI_DISABLE
#include "imgui_internal.h"

//-----------------------------------------------------------------------------
// [SECTION] Generic Helpers
//-----------------------------------------------------------------------------

#ifndef IMPLOT_VERSION
// Returns true if flag is set
template <typename TSet, typename TFlag>
static inline bool ImHasFlag(TSet set, TFlag flag) { return (set & flag) == flag; }
// Returns true if val is NAN
static inline bool ImNan(double val) { return isnan(val); }
// Returns true if val is NAN or INFINITY
static inline bool ImNanOrInf(double val) { return !(val >= -DBL_MAX && val <= DBL_MAX) || ImNan(val); }
// True if two numbers are approximately equal using units in the last place.
static inline bool ImAlmostEqual(double v1, double v2, int ulp = 2) { return ImAbs(v1 - v2) < DBL_EPSILON * ImAbs(v1 + v2) * ulp || ImAbs(v1 - v2) < DBL_MIN; }
// Set alpha channel of 32-bit color from float in range [0.0 1.0]
static inline ImU32 ImAlphaU32(ImU32 col, float alpha) {
    return col & ~((ImU32)((1.0f - alpha) * 255) << IM_COL32_A_SHIFT);
}
// Mix color a and b by factor s in [0 256]
static inline ImU32 ImMixU32(ImU32 a, ImU32 b, ImU32 s) {
#ifdef IMPLOT_MIX64
    const ImU32 af = 256 - s;
    const ImU32 bf = s;
    const ImU64 al = (a & 0x00ff00ff) | (((ImU64)(a & 0xff00ff00)) << 24);
    const ImU64 bl = (b & 0x00ff00ff) | (((ImU64)(b & 0xff00ff00)) << 24);
    const ImU64 mix = (al * af + bl * bf);
    return ((mix >> 32) & 0xff00ff00) | ((mix & 0xff00ff00) >> 8);
#else
    const ImU32 af = 256 - s;
    const ImU32 bf = s;
    const ImU32 al = (a & 0x00ff00ff);
    const ImU32 ah = (a & 0xff00ff00) >> 8;
    const ImU32 bl = (b & 0x00ff00ff);
    const ImU32 bh = (b & 0xff00ff00) >> 8;
    const ImU32 ml = (al * af + bl * bf);
    const ImU32 mh = (ah * af + bh * bf);
    return (mh & 0xff00ff00) | ((ml & 0xff00ff00) >> 8);
#endif
}
#endif

//-----------------------------------------------------------------------------
// [SECTION] Structs
//-----------------------------------------------------------------------------

struct ImPlot3DNextItemData {
    ImVec4 Colors[3]; // ImPlot3DCol_Line, ImPlot3DCol_MarkerOutline, ImPlot3DCol_MarkerFill,
    float LineWeight;
    ImPlot3DMarker Marker;
    float MarkerSize;
    float MarkerWeight;
    bool RenderLine;
    bool RenderMarkerLine;
    bool RenderMarkerFill;
    bool Hidden;

    ImPlot3DNextItemData() { Reset(); }
    void Reset() {
        for (int i = 0; i < 3; ++i)
            Colors[i] = IMPLOT3D_AUTO_COL;
        LineWeight = IMPLOT3D_AUTO;
        Marker = IMPLOT3D_AUTO;
        MarkerSize = IMPLOT3D_AUTO;
        MarkerWeight = IMPLOT3D_AUTO;
        RenderLine = false;
        RenderMarkerLine = true;
        RenderMarkerFill = true;
        Hidden = false;
    }
};

// State information for plot items
struct ImPlot3DItem {
    ImGuiID ID;
    ImU32 Color;
    int NameOffset;
    bool Show;
    bool LegendHovered;

    ImPlot3DItem() {
        ID = 0;
        Color = IM_COL32_WHITE;
        NameOffset = -1;
        Show = true;
        LegendHovered = false;
    }
    ~ImPlot3DItem() { ID = 0; }
};

// Holds legend state
struct ImPlot3DLegend {
    ImPlot3DLegendFlags Flags;
    ImPlot3DLocation Location;
    ImVector<int> Indices;
    ImGuiTextBuffer Labels;
    ImRect Rect;
    bool Hovered;
    bool Held;

    ImPlot3DLegend() {
        Flags = ImPlot3DLegendFlags_None;
        Hovered = Held = false;
        Location = ImPlot3DLocation_NorthWest;
    }

    void Reset() {
        Indices.shrink(0);
        Labels.Buf.shrink(0);
    }
};

// Holds items
struct ImPlot3DItemGroup {
    ImPool<ImPlot3DItem> ItemPool;
    ImPlot3DLegend Legend;

    int GetItemCount() const { return ItemPool.GetBufSize(); }
    ImGuiID GetItemID(const char* label_id) { return ImGui::GetID(label_id); }
    ImPlot3DItem* GetItem(ImGuiID id) { return ItemPool.GetByKey(id); }
    ImPlot3DItem* GetItem(const char* label_id) { return GetItem(GetItemID(label_id)); }
    ImPlot3DItem* GetOrAddItem(ImGuiID id) { return ItemPool.GetOrAddByKey(id); }
    ImPlot3DItem* GetItemByIndex(int i) { return ItemPool.GetByIndex(i); }
    int GetItemIndex(ImPlot3DItem* item) { return ItemPool.GetIndex(item); }
    int GetLegendCount() const { return Legend.Indices.size(); }
    ImPlot3DItem* GetLegendItem(int i) { return ItemPool.GetByIndex(Legend.Indices[i]); }
    const char* GetLegendLabel(int i) { return Legend.Labels.Buf.Data + GetLegendItem(i)->NameOffset; }
    void Reset() {
        ItemPool.Clear();
        Legend.Reset();
    }
};

// Holds plot state information that must persist after EndPlot
struct ImPlot3DPlot {
    ImGuiID ID;
    ImPlot3DFlags Flags;
    ImGuiTextBuffer TextBuffer;
    // Bounding rectangles
    ImRect FrameRect;  // Outermost bounding rectangle that encapsulates whole the plot/title/padding/etc
    ImRect CanvasRect; // Frame rectangle reduced by padding
    ImRect PlotRect;   // Bounding rectangle for the actual plot area
    // Rotation and range
    ImPlot3DQuat Rotation;
    ImPlot3DPoint RangeMin;
    ImPlot3DPoint RangeMax;
    // User input
    bool SetupLocked;
    bool Hovered;
    bool Held;
    // Fit data
    bool FitThisFrame;
    ImPlot3DPoint FitMin;
    ImPlot3DPoint FitMax;
    // Items
    ImPlot3DItemGroup Items;
    ImPlot3DItem* CurrentItem;

    ImPlot3DPlot() {
        Flags = ImPlot3DFlags_None;
        Rotation = ImPlot3DQuat(0.0f, 0.0f, 0.0f, 1.0f);
        RangeMin = ImPlot3DPoint(0.0f, 0.0f, 0.0f);
        RangeMax = ImPlot3DPoint(1.0f, 1.0f, 1.0f);
        SetupLocked = false;
        Hovered = Held = false;
        FitThisFrame = true;
        FitMin = ImPlot3DPoint(HUGE_VAL, HUGE_VAL, HUGE_VAL);
        FitMax = ImPlot3DPoint(-HUGE_VAL, -HUGE_VAL, -HUGE_VAL);
        CurrentItem = nullptr;
    }

    inline void ExtendFit(const ImPlot3DPoint& point) {
        if (!ImNanOrInf(point.x) && !ImNanOrInf(point.y) && !ImNanOrInf(point.z)) {
            FitMin.x = point.x < FitMin.x ? point.x : FitMin.x;
            FitMin.y = point.y < FitMin.y ? point.y : FitMin.y;
            FitMin.z = point.z < FitMin.z ? point.z : FitMin.z;
            FitMax.x = point.x > FitMax.x ? point.x : FitMax.x;
            FitMax.y = point.y > FitMax.y ? point.y : FitMax.y;
            FitMax.z = point.z > FitMax.z ? point.z : FitMax.z;
        }
    }
};

struct ImPlot3DContext {
    ImPool<ImPlot3DPlot> Plots;
    ImPlot3DPlot* CurrentPlot;
    ImPlot3DItemGroup* CurrentItems;
    ImPlot3DNextItemData NextItemData;
    ImPlot3DStyle Style;
};

//-----------------------------------------------------------------------------
// [SECTION] Context Pointer
//-----------------------------------------------------------------------------

namespace ImPlot3D {

#ifndef GImPlot3D
extern IMPLOT3D_API ImPlot3DContext* GImPlot3D; // Current context pointer
#endif

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

// Get styling data for next item (call between BeginItem/EndItem)
IMPLOT3D_API const ImPlot3DNextItemData& GetItemData();

//-----------------------------------------------------------------------------
// [SECTION] Item Utils
//-----------------------------------------------------------------------------

IMPLOT3D_API bool BeginItem(const char* label_id, ImPlot3DItemFlags flags = 0, ImPlot3DCol recolor_from = IMPLOT3D_AUTO);
IMPLOT3D_API void EndItem();

// Register or get an existing item from the current plot
IMPLOT3D_API ImPlot3DItem* RegisterOrGetItem(const char* label_id, ImPlot3DItemFlags flags, bool* just_created = nullptr);
// Get a plot item from the current plot
IMPLOT3D_API ImPlot3DItem* GetItem(const char* label_id);
// Get the current item
IMPLOT3D_API ImPlot3DItem* GetCurrentItem();

//-----------------------------------------------------------------------------
// [SECTION] Plot Utils
//-----------------------------------------------------------------------------

// Gets the current plot from the current ImPlot3DContext
IMPLOT3D_API ImPlot3DPlot* GetCurrentPlot();

// Convert a position in the current plot's coordinate system to the current plot's normalized device coordinate system (NDC)
// When the cube aspect ratio is [1,1,1], the NDC varies from [-0.5, 0.5] in each axis
IMPLOT3D_API ImPlot3DPoint PlotToNDC(const ImPlot3DPoint& point);
IMPLOT3D_API ImPlot3DPoint NDCToPlot(const ImPlot3DPoint& point);
// Convert a position in the current plot's NDC to pixels
IMPLOT3D_API ImVec2 NDCToPixels(const ImPlot3DPoint& point);
// Convert a pixel coordinate to a ray in the NDC
IMPLOT3D_API ImPlot3DRay PixelsToNDCRay(const ImVec2& pix);
// Convert a ray in the NDC to a ray in the current plot's coordinate system
IMPLOT3D_API ImPlot3DRay NDCRayToPlotRay(const ImPlot3DRay& ray);

//-----------------------------------------------------------------------------
// [SECTION] Setup Utils
//-----------------------------------------------------------------------------

IMPLOT3D_API void SetupLock();

} // namespace ImPlot3D

#endif // #ifndef IMGUI_DISABLE
