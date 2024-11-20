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
// [SECTION] ImQuat
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

//-----------------------------------------------------------------------------
// [SECTION] ImVec3
//-----------------------------------------------------------------------------

// ImVec3: 3D vector to store points in 3D
struct ImVec3 {
    float x, y, z;
    constexpr ImVec3() : x(0.0f), y(0.0f), z(0.0f) {}
    constexpr ImVec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}

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
    ImVec3 operator*(float rhs) const;
    ImVec3 operator/(float rhs) const;
    ImVec3 operator+(const ImVec3& rhs) const;
    ImVec3 operator-(const ImVec3& rhs) const;
    ImVec3 operator*(const ImVec3& rhs) const;
    ImVec3 operator/(const ImVec3& rhs) const;

    // Unary operator
    ImVec3 operator-() const;

    // Compound assignment operators
    ImVec3& operator*=(float rhs);
    ImVec3& operator/=(float rhs);
    ImVec3& operator+=(const ImVec3& rhs);
    ImVec3& operator-=(const ImVec3& rhs);
    ImVec3& operator*=(const ImVec3& rhs);
    ImVec3& operator/=(const ImVec3& rhs);

    // Comparison operators
    bool operator==(const ImVec3& rhs) const;
    bool operator!=(const ImVec3& rhs) const;

    // Dot product
    float Dot(const ImVec3& rhs) const;

    // Cross product
    ImVec3 Cross(const ImVec3& rhs) const;

    // Get vector magnitude
    float Magnitude() const;

    // Friend binary operators to allow commutative behavior
    friend ImVec3 operator*(float lhs, const ImVec3& rhs);
};

//-----------------------------------------------------------------------------
// [SECTION] ImQuat
//-----------------------------------------------------------------------------

struct ImQuat {
    float x, y, z, w;

    // Constructors
    constexpr ImQuat();
    constexpr ImQuat(float _x, float _y, float _z, float _w);
    ImQuat(float _angle, const ImVec3& _axis);

    // Get quaternion magnitude
    float Magnitude() const;

    // Get normalized quaternion
    ImQuat Normalized() const;

    // Conjugate of the quaternion
    ImQuat Conjugate() const;

    // Inverse of the quaternion
    ImQuat Inverse() const;

    // Binary operators
    ImQuat operator*(const ImQuat& rhs) const;

    // Normalize the quaternion in place
    ImQuat& Normalize();

    // Rotate a 3D point using the quaternion
    ImVec3 operator*(const ImVec3& point) const;

    // Comparison operators
    bool operator==(const ImQuat& rhs) const;
    bool operator!=(const ImQuat& rhs) const;

    // Friend binary operators to allow commutative behavior
    friend ImQuat operator*(float lhs, const ImQuat& rhs);
};

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
