//--------------------------------------------------
// ImPlot3D v0.1
// implot3d_internal.h
// Date: 2024-11-17
// By brenocq
//--------------------------------------------------

// Table of Contents:
// [SECTION] Context Utils
// [SECTION] Style Utils
// [SECTION] ImVec3
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
// [SECTION] ImVec3
//-----------------------------------------------------------------------------

// ImVec3: 3D vector to store points in 3D
struct ImVec3 {
    float x, y, z;
    constexpr ImVec3() : x(0.0f), y(0.0f), z(0.0f) {}
    constexpr ImVec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

    // Binary operators
    ImVec3 operator*(float rhs) const { return ImVec3(x * rhs, y * rhs, z * rhs); }
    ImVec3 operator/(float rhs) const { return ImVec3(x / rhs, y / rhs, z / rhs); }
    ImVec3 operator+(const ImVec3& rhs) const { return ImVec3(x + rhs.x, y + rhs.y, z + rhs.z); }
    ImVec3 operator-(const ImVec3& rhs) const { return ImVec3(x - rhs.x, y - rhs.y, z - rhs.z); }
    ImVec3 operator*(const ImVec3& rhs) const { return ImVec3(x * rhs.x, y * rhs.y, z * rhs.z); }
    ImVec3 operator/(const ImVec3& rhs) const { return ImVec3(x / rhs.x, y / rhs.y, z / rhs.z); }

    // Unary operator
    ImVec3 operator-() const { return ImVec3(-x, -y, -z); }

    // Compound assignment operators
    ImVec3& operator*=(float rhs) {
        x *= rhs;
        y *= rhs;
        z *= rhs;
        return *this;
    }
    ImVec3& operator/=(float rhs) {
        x /= rhs;
        y /= rhs;
        z /= rhs;
        return *this;
    }
    ImVec3& operator+=(const ImVec3& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }
    ImVec3& operator-=(const ImVec3& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        z -= rhs.z;
        return *this;
    }
    ImVec3& operator*=(const ImVec3& rhs) {
        x *= rhs.x;
        y *= rhs.y;
        z *= rhs.z;
        return *this;
    }
    ImVec3& operator/=(const ImVec3& rhs) {
        x /= rhs.x;
        y /= rhs.y;
        z /= rhs.z;
        return *this;
    }

    // Comparison operators
    bool operator==(const ImVec3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
    bool operator!=(const ImVec3& rhs) const { return !(*this == rhs); }

    // Friend binary operators to allow commutative behavior
    friend ImVec3 operator*(float lhs, const ImVec3& rhs) {
        return ImVec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
    }
};

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
