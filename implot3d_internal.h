//--------------------------------------------------
// ImPlot3D v0.1
// implot3d_internal.h
// Date: 2024-11-17
// By brenocq
//
// Acknowledgments:
//  This library is heavily inspired by ImPlot
//  (https://github.com/epezent/implot) by Evan Pezent,
//  and follows a similar code style and structure to
//  maintain consistency with ImPlot's API.
//--------------------------------------------------

// Table of Contents:
// [SECTION] ImPlot3DVec3
// [SECTION] ImPlot3DQuat
// [SECTION] Structs
// [SECTION] Context Pointer
// [SECTION] Context Utils
// [SECTION] Style Utils
// [SECTION] Item Utils

#pragma once

#ifndef IMPLOT3D_VERSION
#include "implot3d.h"
#endif

#ifndef IMGUI_DISABLE
#include "imgui_internal.h"

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DVec3
//-----------------------------------------------------------------------------

// ImPlot3DVec3: 3D vector to store points in 3D
struct ImPlot3DVec3 {
    float x, y, z;
    constexpr ImPlot3DVec3() : x(0.0f), y(0.0f), z(0.0f) {}
    constexpr ImPlot3DVec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    // Accessors
    float& operator[](size_t idx) {
        IM_ASSERT(idx == 0 || idx == 1 || idx == 2);
        return ((float*)(void*)(char*)this)[idx];
    }
    float operator[](size_t idx) const {
        IM_ASSERT(idx == 0 || idx == 1 || idx == 2);
        return ((const float*)(const void*)(const char*)this)[idx];
    }

    // Binary operators
    ImPlot3DVec3 operator*(float rhs) const;
    ImPlot3DVec3 operator/(float rhs) const;
    ImPlot3DVec3 operator+(const ImPlot3DVec3& rhs) const;
    ImPlot3DVec3 operator-(const ImPlot3DVec3& rhs) const;
    ImPlot3DVec3 operator*(const ImPlot3DVec3& rhs) const;
    ImPlot3DVec3 operator/(const ImPlot3DVec3& rhs) const;

    // Unary operator
    ImPlot3DVec3 operator-() const;

    // Compound assignment operators
    ImPlot3DVec3& operator*=(float rhs);
    ImPlot3DVec3& operator/=(float rhs);
    ImPlot3DVec3& operator+=(const ImPlot3DVec3& rhs);
    ImPlot3DVec3& operator-=(const ImPlot3DVec3& rhs);
    ImPlot3DVec3& operator*=(const ImPlot3DVec3& rhs);
    ImPlot3DVec3& operator/=(const ImPlot3DVec3& rhs);

    // Comparison operators
    bool operator==(const ImPlot3DVec3& rhs) const;
    bool operator!=(const ImPlot3DVec3& rhs) const;

    // Dot product
    float Dot(const ImPlot3DVec3& rhs) const;

    // Cross product
    ImPlot3DVec3 Cross(const ImPlot3DVec3& rhs) const;

    // Get vector magnitude
    float Magnitude() const;

    // Friend binary operators to allow commutative behavior
    friend ImPlot3DVec3 operator*(float lhs, const ImPlot3DVec3& rhs);
};

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DQuat
//-----------------------------------------------------------------------------

struct ImPlot3DQuat {
    float x, y, z, w;

    // Constructors
    constexpr ImPlot3DQuat();
    constexpr ImPlot3DQuat(float _x, float _y, float _z, float _w);
    ImPlot3DQuat(float _angle, const ImPlot3DVec3& _axis);

    // Get quaternion magnitude
    float Magnitude() const;

    // Get normalized quaternion
    ImPlot3DQuat Normalized() const;

    // Conjugate of the quaternion
    ImPlot3DQuat Conjugate() const;

    // Inverse of the quaternion
    ImPlot3DQuat Inverse() const;

    // Binary operators
    ImPlot3DQuat operator*(const ImPlot3DQuat& rhs) const;

    // Normalize the quaternion in place
    ImPlot3DQuat& Normalize();

    // Rotate a 3D point using the quaternion
    ImPlot3DVec3 operator*(const ImPlot3DVec3& point) const;

    // Comparison operators
    bool operator==(const ImPlot3DQuat& rhs) const;
    bool operator!=(const ImPlot3DQuat& rhs) const;
};

//-----------------------------------------------------------------------------
// [SECTION] Structs
//-----------------------------------------------------------------------------

struct ImPlot3DNextItemData {
    ImVec4 Colors[3]; // ImPlot3DCol_Line, ImPlot3DCol_MarkerOutline, ImPlot3DCol_MarkerFill,
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
        Marker = IMPLOT3D_AUTO;
        MarkerSize = IMPLOT3D_AUTO;
        MarkerWeight = IMPLOT3D_AUTO;
        RenderLine = false;
        RenderMarkerLine = false;
        RenderMarkerFill = false;
        Hidden = false;
    }
};

// State information for plot items
struct ImPlot3DItem {
    ImGuiID ID;
    ImU32 Color;
    bool Show;
    bool LegendHovered;

    ImPlot3DItem() {
        ID = 0;
        Color = IM_COL32_WHITE;
        Show = true;
        LegendHovered = false;
    }
    ~ImPlot3DItem() { ID = 0; }
};

// Holds items
struct ImPlot3DItemGroup {
    ImPool<ImPlot3DItem> ItemPool;

    int GetItemCount() const { return ItemPool.GetBufSize(); }
    ImGuiID GetItemID(const char* label_id) { return ImGui::GetID(label_id); }
    ImPlot3DItem* GetItem(ImGuiID id) { return ItemPool.GetByKey(id); }
    ImPlot3DItem* GetItem(const char* label_id) { return GetItem(GetItemID(label_id)); }
    ImPlot3DItem* GetOrAddItem(ImGuiID id) { return ItemPool.GetOrAddByKey(id); }
    ImPlot3DItem* GetItemByIndex(int i) { return ItemPool.GetByIndex(i); }
    int GetItemIndex(ImPlot3DItem* item) { return ItemPool.GetIndex(item); }
    void Reset() { ItemPool.Clear(); }
};

// Holds plot state information that must persist after EndPlot
struct ImPlot3DPlot {
    ImGuiID ID;
    ImPlot3DFlags Flags;
    ImGuiTextBuffer TextBuffer;
    ImRect FrameRect;  // Outermost bounding rectangle that encapsulates whole the plot/title/padding/etc
    ImRect CanvasRect; // Frame rectangle reduced by padding
    ImRect PlotRect;   // Bounding rectangle for the actual plot area
    ImPlot3DQuat Rotation;
    ImPlot3DVec3 RangeMin;
    ImPlot3DVec3 RangeMax;
    bool Hovered;
    bool Held;
    ImPlot3DItemGroup Items;
    ImPlot3DItem* CurrentItem;
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

} // namespace ImPlot3D

#endif // #ifndef IMGUI_DISABLE
