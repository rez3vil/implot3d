//--------------------------------------------------
// ImPlot3D v0.1
// implot3d_items.cpp
// Date: 2024-11-26
// By brenocq
//
// Acknowledgments:
//  This library is heavily inspired by ImPlot
//  (https://github.com/epezent/implot) by Evan Pezent,
//  and follows a similar code style and structure to
//  maintain consistency with ImPlot's API.
//--------------------------------------------------

// Table of Contents:
// [SECTION] Includes
// [SECTION] Macros & Defines
// [SECTION] Template instantiation utility
// [SECTION] Item Utils
// [SECTION] Draw Utils
// [SECTION] Transformer
// [SECTION] Renderers
// [SECTION] Indexers
// [SECTION] Getters
// [SECTION] RenderPrimitives
// [SECTION] Markers
// [SECTION] ScatterPlot

//-----------------------------------------------------------------------------
// [SECTION] Includes
//-----------------------------------------------------------------------------

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "implot3d.h"
#include "implot3d_internal.h"

#ifndef IMGUI_DISABLE

//-----------------------------------------------------------------------------
// [SECTION] Macros & Defines
//-----------------------------------------------------------------------------

#define SQRT_1_2 0.70710678118f
#define SQRT_3_2 0.86602540378f

// clang-format off
#ifndef IMPLOT3D_NO_FORCE_INLINE
    #ifdef _MSC_VER
        #define IMPLOT3D_INLINE __forceinline
    #elif defined(__GNUC__)
        #define IMPLOT3D_INLINE inline __attribute__((__always_inline__))
    #elif defined(__CLANG__)
        #if __has_attribute(__always_inline__)
            #define IMPLOT3D_INLINE inline __attribute__((__always_inline__))
        #else
            #define IMPLOT3D_INLINE inline
        #endif
    #else
        #define IMPLOT3D_INLINE inline
    #endif
#else
    #define IMPLOT3D_INLINE inline
#endif
// clang-format on

#define IMPLOT3D_NORMALIZE2F(VX, VY)     \
    do {                                 \
        float d2 = VX * VX + VY * VY;    \
        if (d2 > 0.0f) {                 \
            float inv_len = ImRsqrt(d2); \
            VX *= inv_len;               \
            VY *= inv_len;               \
        }                                \
    } while (0)

// Calculate maximum index size of ImDrawIdx
template <typename T>
struct MaxIdx {
    static const unsigned int Value;
};
template <> const unsigned int MaxIdx<unsigned short>::Value = 65535;
template <> const unsigned int MaxIdx<unsigned int>::Value = 4294967295;

//-----------------------------------------------------------------------------
// [SECTION] Template instantiation utility
//-----------------------------------------------------------------------------

// By default, templates are instantiated for `float`, `double`, and for the following integer types, which are defined in imgui.h:
//     signed char         ImS8;   // 8-bit signed integer
//     unsigned char       ImU8;   // 8-bit unsigned integer
//     signed short        ImS16;  // 16-bit signed integer
//     unsigned short      ImU16;  // 16-bit unsigned integer
//     signed int          ImS32;  // 32-bit signed integer == int
//     unsigned int        ImU32;  // 32-bit unsigned integer
//     signed   long long  ImS64;  // 64-bit signed integer
//     unsigned long long  ImU64;  // 64-bit unsigned integer
// (note: this list does *not* include `long`, `unsigned long` and `long double`)
//
// You can customize the supported types by defining IMPLOT3D_CUSTOM_NUMERIC_TYPES at compile time to define your own type list.
//    As an example, you could use the compile time define given by the line below in order to support only float and double.
//        -DIMPLOT3D_CUSTOM_NUMERIC_TYPES="(float)(double)"
//    In order to support all known C++ types, use:
//        -DIMPLOT3D_CUSTOM_NUMERIC_TYPES="(signed char)(unsigned char)(signed short)(unsigned short)(signed int)(unsigned int)(signed long)(unsigned long)(signed long long)(unsigned long long)(float)(double)(long double)"

#ifdef IMPLOT3D_CUSTOM_NUMERIC_TYPES
#define IMPLOT3D_NUMERIC_TYPES IMPLOT3D_CUSTOM_NUMERIC_TYPES
#else
#define IMPLOT3D_NUMERIC_TYPES (ImS8)(ImU8)(ImS16)(ImU16)(ImS32)(ImU32)(ImS64)(ImU64)(float)(double)
#endif

// CALL_INSTANTIATE_FOR_NUMERIC_TYPES will duplicate the template instantiation code `INSTANTIATE_MACRO(T)` on supported types.
#define _CAT(x, y) _CAT_(x, y)
#define _CAT_(x, y) x##y
#define _INSTANTIATE_FOR_NUMERIC_TYPES(chain) _CAT(_INSTANTIATE_FOR_NUMERIC_TYPES_1 chain, _END)
#define _INSTANTIATE_FOR_NUMERIC_TYPES_1(T) INSTANTIATE_MACRO(T) _INSTANTIATE_FOR_NUMERIC_TYPES_2
#define _INSTANTIATE_FOR_NUMERIC_TYPES_2(T) INSTANTIATE_MACRO(T) _INSTANTIATE_FOR_NUMERIC_TYPES_1
#define _INSTANTIATE_FOR_NUMERIC_TYPES_1_END
#define _INSTANTIATE_FOR_NUMERIC_TYPES_2_END
#define CALL_INSTANTIATE_FOR_NUMERIC_TYPES() _INSTANTIATE_FOR_NUMERIC_TYPES(IMPLOT3D_NUMERIC_TYPES)

//-----------------------------------------------------------------------------
// [SECTION] Item Utils
//-----------------------------------------------------------------------------

namespace ImPlot3D {

bool BeginItem(const char* label_id, ImPlot3DItemFlags flags, ImPlot3DCol recolor_from) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "PlotX() needs to be called between BeginPlot() and EndPlot()!");

    return true;
}

void EndItem() {}

ImPlot3DItem* RegisterOrGetItem(const char* label_id, ImPlot3DItemFlags flags, bool* just_created) {
    ImPlot3DContext& gp = *GImPlot3D;
    ImPlot3DItemGroup& Items = *gp.CurrentItems;
    ImGuiID id = Items.GetItemID(label_id);
    if (just_created != nullptr)
        *just_created = Items.GetItem(id) == nullptr;
    ImPlot3DItem* item = Items.GetOrAddItem(id);
    item->ID = id;
    if ((flags & ImPlot3DItemFlags_NoLegend) != 0 && ImGui::FindRenderedTextEnd(label_id, nullptr) != label_id) {
        // TODO
        item->Show = false;
    } else {
        item->Show = true;
    }
    return item;
}

//-----------------------------------------------------------------------------
// [SECTION] Draw Utils
//-----------------------------------------------------------------------------

IMPLOT3D_INLINE void PrimLine(ImDrawList& draw_list, const ImVec2& P1, const ImVec2& P2, float half_weight, ImU32 col, const ImVec2& tex_uv0, const ImVec2 tex_uv1) {
    float dx = P2.x - P1.x;
    float dy = P2.y - P1.y;
    IMPLOT3D_NORMALIZE2F(dx, dy);
    dx *= half_weight;
    dy *= half_weight;
    draw_list._VtxWritePtr[0].pos.x = P1.x + dy;
    draw_list._VtxWritePtr[0].pos.y = P1.y - dx;
    draw_list._VtxWritePtr[0].uv = tex_uv0;
    draw_list._VtxWritePtr[0].col = col;
    draw_list._VtxWritePtr[1].pos.x = P2.x + dy;
    draw_list._VtxWritePtr[1].pos.y = P2.y - dx;
    draw_list._VtxWritePtr[1].uv = tex_uv0;
    draw_list._VtxWritePtr[1].col = col;
    draw_list._VtxWritePtr[2].pos.x = P2.x - dy;
    draw_list._VtxWritePtr[2].pos.y = P2.y + dx;
    draw_list._VtxWritePtr[2].uv = tex_uv1;
    draw_list._VtxWritePtr[2].col = col;
    draw_list._VtxWritePtr[3].pos.x = P1.x - dy;
    draw_list._VtxWritePtr[3].pos.y = P1.y + dx;
    draw_list._VtxWritePtr[3].uv = tex_uv1;
    draw_list._VtxWritePtr[3].col = col;
    draw_list._VtxWritePtr += 4;
    draw_list._IdxWritePtr[0] = (ImDrawIdx)(draw_list._VtxCurrentIdx);
    draw_list._IdxWritePtr[1] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 1);
    draw_list._IdxWritePtr[2] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 2);
    draw_list._IdxWritePtr[3] = (ImDrawIdx)(draw_list._VtxCurrentIdx);
    draw_list._IdxWritePtr[4] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 2);
    draw_list._IdxWritePtr[5] = (ImDrawIdx)(draw_list._VtxCurrentIdx + 3);
    draw_list._IdxWritePtr += 6;
    draw_list._VtxCurrentIdx += 4;
}

//-----------------------------------------------------------------------------
// [SECTION] Transformer
//-----------------------------------------------------------------------------

struct Transformer3 {
    Transformer3(ImPlot3DQuat rotation, ImPlot3DVec3 rangeMin, ImPlot3DVec3 rangeMax) : Rotation(rotation),
                                                                                        RangeMin(rangeMin),
                                                                                        RangeMax(rangeMax) {}
    template <typename T> IMPLOT3D_INLINE ImVec2 operator()(T p) const {
        // TODO
    }
    ImPlot3DQuat Rotation;
    ImPlot3DVec3 RangeMin;
    ImPlot3DVec3 RangeMax;
};

//-----------------------------------------------------------------------------
// [SECTION] Renderers
//-----------------------------------------------------------------------------

struct RendererBase {
    RendererBase(int prims, int idx_consumed, int vtx_consumed) : Prims(prims),
                                                                  Transformer(ImPlot3DQuat{}, ImPlot3DVec3{}, ImPlot3DVec3{}),
                                                                  IdxConsumed(idx_consumed),
                                                                  VtxConsumed(vtx_consumed) {}
    const unsigned int Prims;       // Number of primitives to render
    Transformer3 Transformer;       // Transformer from 3D point to screen point
    const unsigned int IdxConsumed; // Number of indices consumed per primitive
    const unsigned int VtxConsumed; // Number of vertices consumed per primitive
};

//-----------------------------------------------------------------------------
// [SECTION] Indexers
//-----------------------------------------------------------------------------

template <typename T>
IMPLOT3D_INLINE T IndexData(const T* data, int idx, int count, int offset, int stride) {
    const int s = ((offset == 0) << 0) | ((stride == sizeof(T)) << 1);
    switch (s) {
        case 3: return data[idx];
        case 2: return data[(offset + idx) % count];
        case 1: return *(const T*)(const void*)((const unsigned char*)data + (size_t)((idx)) * stride);
        case 0: return *(const T*)(const void*)((const unsigned char*)data + (size_t)((offset + idx) % count) * stride);
        default: return T(0);
    }
}

template <typename T>
struct IndexerIdx {
    IndexerIdx(const T* data, int count, int offset = 0, int stride = sizeof(T)) : Data(data),
                                                                                   Count(count),
                                                                                   Offset(offset),
                                                                                   Stride(stride) {}
    template <typename I> IMPLOT3D_INLINE double operator()(I idx) const {
        return (double)IndexData(Data, idx, Count, Offset, Stride);
    }
    const T* Data;
    int Count;
    int Offset;
    int Stride;
};

//-----------------------------------------------------------------------------
// [SECTION] Getters
//-----------------------------------------------------------------------------

template <typename _IndexerX, typename _IndexerY, typename _IndexerZ>
struct GetterXYZ {
    GetterXYZ(_IndexerX x, _IndexerY y, _IndexerZ z, int count) : IndexerX(x), IndexerY(y), IndexerZ(z), Count(count) {}
    template <typename I> IMPLOT3D_INLINE ImPlot3DVec3 operator()(I idx) const {
        return ImPlot3DVec3(IndexerX(idx), IndexerY(idx), IndexerZ(idx));
    }
    const _IndexerX IndexerX;
    const _IndexerY IndexerY;
    const _IndexerZ IndexerZ;
    const int Count;
};

//-----------------------------------------------------------------------------
// [SECTION] RenderPrimitives
//-----------------------------------------------------------------------------

/// Renders primitive shapes
template <class _Renderer>
void RenderPrimitivesEx(const _Renderer& renderer, ImDrawList& draw_list) {
    // Initialize renderer
    renderer.Init(draw_list);
    // Find how many can be reserved up to end of current draw command's limit
    unsigned int prims_to_render = ImMin(renderer.Prims, (MaxIdx<ImDrawIdx>::Value - draw_list._VtxCurrentIdx) / renderer.VtxConsumed);
    // Reserve vertices and indices to render the primitives
    draw_list.PrimReserve(prims_to_render * renderer.IdxConsumed, prims_to_render * renderer.VtxConsumed);
    // Render primitives
    for (unsigned int i = 0; i < prims_to_render; ++i)
        renderer.Render(draw_list, i);
}

template <template <class> class _Renderer, class _Getter, typename... Args>
void RenderPrimitives1(const _Getter& getter, Args... args) {
    ImDrawList& draw_list = *GetPlotDrawList();
    RenderPrimitivesEx(_Renderer<_Getter>(getter, args...), draw_list);
}

//-----------------------------------------------------------------------------
// [SECTION] Markers
//-----------------------------------------------------------------------------

template <class _Getter>
struct RendererMarkersLine : RendererBase {
    RendererMarkersLine(const _Getter& getter, const ImVec2* marker, int count, float size, float weight, ImU32 col) : RendererBase(getter.Count, count / 2 * 6, count / 2 * 4), Getter(getter), Marker(marker), Count(count), HalfWeight(ImMax(1.0f, weight) * 0.5f), Size(size), Col(col) {}

    void Init(ImDrawList& draw_list) const {
        // TODO anti-aliasing
        UV0 = draw_list._Data->TexUvWhitePixel;
        UV1 = draw_list._Data->TexUvWhitePixel;
    }

    IMPLOT3D_INLINE void Render(ImDrawList& draw_list, int prim) const {
        ImVec2 p = this->Transformer(Getter(prim));
        for (int i = 0; i < Count; i = i + 2) {
            ImVec2 p1(p.x + Marker[i].x * Size, p.y + Marker[i].y * Size);
            ImVec2 p2(p.x + Marker[i + 1].x * Size, p.y + Marker[i + 1].y * Size);
            PrimLine(draw_list, p1, p2, HalfWeight, Col, UV0, UV1);
        }
    }

    const _Getter& Getter;
    const ImVec2* Marker;
    const int Count;
    mutable float HalfWeight;
    const float Size;
    const ImU32 Col;
    mutable ImVec2 UV0;
    mutable ImVec2 UV1;
};

static const ImVec2 MARKER_FILL_CIRCLE[10] = {ImVec2(1.0f, 0.0f), ImVec2(0.809017f, 0.58778524f), ImVec2(0.30901697f, 0.95105654f), ImVec2(-0.30901703f, 0.9510565f), ImVec2(-0.80901706f, 0.5877852f), ImVec2(-1.0f, 0.0f), ImVec2(-0.80901694f, -0.58778536f), ImVec2(-0.3090171f, -0.9510565f), ImVec2(0.30901712f, -0.9510565f), ImVec2(0.80901694f, -0.5877853f)};
static const ImVec2 MARKER_FILL_SQUARE[4] = {ImVec2(SQRT_1_2, SQRT_1_2), ImVec2(SQRT_1_2, -SQRT_1_2), ImVec2(-SQRT_1_2, -SQRT_1_2), ImVec2(-SQRT_1_2, SQRT_1_2)};
static const ImVec2 MARKER_FILL_DIAMOND[4] = {ImVec2(1, 0), ImVec2(0, -1), ImVec2(-1, 0), ImVec2(0, 1)};
static const ImVec2 MARKER_FILL_UP[3] = {ImVec2(SQRT_3_2, 0.5f), ImVec2(0, -1), ImVec2(-SQRT_3_2, 0.5f)};
static const ImVec2 MARKER_FILL_DOWN[3] = {ImVec2(SQRT_3_2, -0.5f), ImVec2(0, 1), ImVec2(-SQRT_3_2, -0.5f)};
static const ImVec2 MARKER_FILL_LEFT[3] = {ImVec2(-1, 0), ImVec2(0.5, SQRT_3_2), ImVec2(0.5, -SQRT_3_2)};
static const ImVec2 MARKER_FILL_RIGHT[3] = {ImVec2(1, 0), ImVec2(-0.5, SQRT_3_2), ImVec2(-0.5, -SQRT_3_2)};

static const ImVec2 MARKER_LINE_CIRCLE[20] = {
    ImVec2(1.0f, 0.0f),
    ImVec2(0.809017f, 0.58778524f),
    ImVec2(0.809017f, 0.58778524f),
    ImVec2(0.30901697f, 0.95105654f),
    ImVec2(0.30901697f, 0.95105654f),
    ImVec2(-0.30901703f, 0.9510565f),
    ImVec2(-0.30901703f, 0.9510565f),
    ImVec2(-0.80901706f, 0.5877852f),
    ImVec2(-0.80901706f, 0.5877852f),
    ImVec2(-1.0f, 0.0f),
    ImVec2(-1.0f, 0.0f),
    ImVec2(-0.80901694f, -0.58778536f),
    ImVec2(-0.80901694f, -0.58778536f),
    ImVec2(-0.3090171f, -0.9510565f),
    ImVec2(-0.3090171f, -0.9510565f),
    ImVec2(0.30901712f, -0.9510565f),
    ImVec2(0.30901712f, -0.9510565f),
    ImVec2(0.80901694f, -0.5877853f),
    ImVec2(0.80901694f, -0.5877853f),
    ImVec2(1.0f, 0.0f)};
static const ImVec2 MARKER_LINE_SQUARE[8] = {ImVec2(SQRT_1_2, SQRT_1_2), ImVec2(SQRT_1_2, -SQRT_1_2), ImVec2(SQRT_1_2, -SQRT_1_2), ImVec2(-SQRT_1_2, -SQRT_1_2), ImVec2(-SQRT_1_2, -SQRT_1_2), ImVec2(-SQRT_1_2, SQRT_1_2), ImVec2(-SQRT_1_2, SQRT_1_2), ImVec2(SQRT_1_2, SQRT_1_2)};
static const ImVec2 MARKER_LINE_DIAMOND[8] = {ImVec2(1, 0), ImVec2(0, -1), ImVec2(0, -1), ImVec2(-1, 0), ImVec2(-1, 0), ImVec2(0, 1), ImVec2(0, 1), ImVec2(1, 0)};
static const ImVec2 MARKER_LINE_UP[6] = {ImVec2(SQRT_3_2, 0.5f), ImVec2(0, -1), ImVec2(0, -1), ImVec2(-SQRT_3_2, 0.5f), ImVec2(-SQRT_3_2, 0.5f), ImVec2(SQRT_3_2, 0.5f)};
static const ImVec2 MARKER_LINE_DOWN[6] = {ImVec2(SQRT_3_2, -0.5f), ImVec2(0, 1), ImVec2(0, 1), ImVec2(-SQRT_3_2, -0.5f), ImVec2(-SQRT_3_2, -0.5f), ImVec2(SQRT_3_2, -0.5f)};
static const ImVec2 MARKER_LINE_LEFT[6] = {ImVec2(-1, 0), ImVec2(0.5, SQRT_3_2), ImVec2(0.5, SQRT_3_2), ImVec2(0.5, -SQRT_3_2), ImVec2(0.5, -SQRT_3_2), ImVec2(-1, 0)};
static const ImVec2 MARKER_LINE_RIGHT[6] = {ImVec2(1, 0), ImVec2(-0.5, SQRT_3_2), ImVec2(-0.5, SQRT_3_2), ImVec2(-0.5, -SQRT_3_2), ImVec2(-0.5, -SQRT_3_2), ImVec2(1, 0)};
static const ImVec2 MARKER_LINE_ASTERISK[6] = {ImVec2(-SQRT_3_2, -0.5f), ImVec2(SQRT_3_2, 0.5f), ImVec2(-SQRT_3_2, 0.5f), ImVec2(SQRT_3_2, -0.5f), ImVec2(0, -1), ImVec2(0, 1)};
static const ImVec2 MARKER_LINE_PLUS[4] = {ImVec2(-1, 0), ImVec2(1, 0), ImVec2(0, -1), ImVec2(0, 1)};
static const ImVec2 MARKER_LINE_CROSS[4] = {ImVec2(-SQRT_1_2, -SQRT_1_2), ImVec2(SQRT_1_2, SQRT_1_2), ImVec2(SQRT_1_2, -SQRT_1_2), ImVec2(-SQRT_1_2, SQRT_1_2)};

template <typename _Getter>
void RenderMarkers(const _Getter& getter, ImPlot3DMarker marker, float size, bool rend_fill, ImU32 col_fill, bool rend_line, ImU32 col_line, float weight) {
    // if (rend_fill) {
    //     switch (marker) {
    //         case ImPlot3DMarker_Circle: RenderPrimitives1<RendererMarkersFill>(getter, MARKER_FILL_CIRCLE, 10, size, col_fill); break;
    //         case ImPlot3DMarker_Square: RenderPrimitives1<RendererMarkersFill>(getter, MARKER_FILL_SQUARE, 4, size, col_fill); break;
    //         case ImPlot3DMarker_Diamond: RenderPrimitives1<RendererMarkersFill>(getter, MARKER_FILL_DIAMOND, 4, size, col_fill); break;
    //         case ImPlot3DMarker_Up: RenderPrimitives1<RendererMarkersFill>(getter, MARKER_FILL_UP, 3, size, col_fill); break;
    //         case ImPlot3DMarker_Down: RenderPrimitives1<RendererMarkersFill>(getter, MARKER_FILL_DOWN, 3, size, col_fill); break;
    //         case ImPlot3DMarker_Left: RenderPrimitives1<RendererMarkersFill>(getter, MARKER_FILL_LEFT, 3, size, col_fill); break;
    //         case ImPlot3DMarker_Right: RenderPrimitives1<RendererMarkersFill>(getter, MARKER_FILL_RIGHT, 3, size, col_fill); break;
    //     }
    // }
    if (rend_line) {
        switch (marker) {
            case ImPlot3DMarker_Circle: RenderPrimitives1<RendererMarkersLine>(getter, MARKER_LINE_CIRCLE, 20, size, weight, col_line); break;
            case ImPlot3DMarker_Square: RenderPrimitives1<RendererMarkersLine>(getter, MARKER_LINE_SQUARE, 8, size, weight, col_line); break;
            case ImPlot3DMarker_Diamond: RenderPrimitives1<RendererMarkersLine>(getter, MARKER_LINE_DIAMOND, 8, size, weight, col_line); break;
            case ImPlot3DMarker_Up: RenderPrimitives1<RendererMarkersLine>(getter, MARKER_LINE_UP, 6, size, weight, col_line); break;
            case ImPlot3DMarker_Down: RenderPrimitives1<RendererMarkersLine>(getter, MARKER_LINE_DOWN, 6, size, weight, col_line); break;
            case ImPlot3DMarker_Left: RenderPrimitives1<RendererMarkersLine>(getter, MARKER_LINE_LEFT, 6, size, weight, col_line); break;
            case ImPlot3DMarker_Right: RenderPrimitives1<RendererMarkersLine>(getter, MARKER_LINE_RIGHT, 6, size, weight, col_line); break;
            case ImPlot3DMarker_Asterisk: RenderPrimitives1<RendererMarkersLine>(getter, MARKER_LINE_ASTERISK, 6, size, weight, col_line); break;
            case ImPlot3DMarker_Plus: RenderPrimitives1<RendererMarkersLine>(getter, MARKER_LINE_PLUS, 4, size, weight, col_line); break;
            case ImPlot3DMarker_Cross: RenderPrimitives1<RendererMarkersLine>(getter, MARKER_LINE_CROSS, 4, size, weight, col_line); break;
        }
    }
}

//-----------------------------------------------------------------------------
// [SECTION] ScatterPlot
//-----------------------------------------------------------------------------

template <typename Getter>
void PlotScatterEx(const char* label_id, const Getter& getter, ImPlot3DScatterFlags flags) {
    if (BeginItem(label_id, flags, ImPlot3DCol_MarkerOutline)) {
        const ImPlot3DNextItemData& n = GetItemData();
        ImPlot3DMarker marker = n.Marker == ImPlot3DMarker_None ? ImPlot3DMarker_Circle : n.Marker;
        if (marker != ImPlot3DMarker_None) {
            const ImU32 col_line = ImGui::GetColorU32(n.Colors[ImPlot3DCol_MarkerOutline]);
            const ImU32 col_fill = ImGui::GetColorU32(n.Colors[ImPlot3DCol_MarkerFill]);
            RenderMarkers<Getter>(getter, marker, n.MarkerSize, n.RenderMarkerFill, col_fill, n.RenderMarkerLine, col_line, n.MarkerWeight);
        }
        EndItem();
    }
}

template <typename T>
void PlotScatter(const char* label_id, const T* xs, const T* ys, const T* zs, int count, ImPlot3DScatterFlags flags, int offset, int stride) {
    GetterXYZ<IndexerIdx<T>, IndexerIdx<T>, IndexerIdx<T>> getter(IndexerIdx<T>(xs, count, offset, stride), IndexerIdx<T>(ys, count, offset, stride), IndexerIdx<T>(zs, count, offset, stride), count);
    return PlotScatterEx(label_id, getter, flags);
}

#define INSTANTIATE_MACRO(T) \
    template IMPLOT3D_API void PlotScatter<T>(const char* label_id, const T* xs, const T* ys, const T* zs, int count, ImPlot3DScatterFlags flags, int offset, int stride);
CALL_INSTANTIATE_FOR_NUMERIC_TYPES()
#undef INSTANTIATE_MACRO

} // namespace ImPlot3D

#endif // #ifndef IMGUI_DISABLE
