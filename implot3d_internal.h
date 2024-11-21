//--------------------------------------------------
// ImPlot3D v0.1
// implot3d_internal.h
// Date: 2024-11-17
// By brenocq
//--------------------------------------------------

// Table of Contents:
// [SECTION] Context Utils
// [SECTION] Style Utils
// [SECTION] ImPlot3DVec3
// [SECTION] ImPlot3DQuat
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
        IM_ASSERT(idx == 0 || idx == 1 || idx == 3);
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

// Holds Plot state information that must persist after EndPlot
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
};

struct ImPlot3DContext {
    ImPool<ImPlot3DPlot> Plots;
    ImPlot3DPlot* CurrentPlot;
    ImPlot3DStyle Style;
};

#endif // #ifndef IMGUI_DISABLE
