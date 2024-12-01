//--------------------------------------------------
// ImPlot3D v0.1
// implot3d.cpp
// Date: 2024-11-16
// Author: Breno Cunha Queiroz (brenocq.com)
//
// Acknowledgments:
//  ImPlot3D is heavily inspired by ImPlot
//  (https://github.com/epezent/implot) by Evan Pezent,
//  and follows a similar code style and structure to
//  maintain consistency with ImPlot's API.
//--------------------------------------------------

// Table of Contents:
// [SECTION] Includes
// [SECTION] Macros
// [SECTION] Context
// [SECTION] Text Utils
// [SECTION] Legend Utils
// [SECTION] Plot Box Utils
// [SECTION] Formatter
// [SECTION] Locator
// [SECTION] Begin/End Plot
// [SECTION] Setup
// [SECTION] Plot Utils
// [SECTION] Setup Utils
// [SECTION] Miscellaneous
// [SECTION] Styles
// [SECTION] Colormaps
// [SECTION] Context Utils
// [SECTION] Style Utils
// [SECTION] ImPlot3DPoint
// [SECTION] ImPlot3DBox
// [SECTION] ImPlot3DRange
// [SECTION] ImPlot3DQuat
// [SECTION] ImPlot3DAxis
// [SECTION] ImPlot3DPlot
// [SECTION] ImPlot3DStyle

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
// [SECTION] Macros
//-----------------------------------------------------------------------------

#define IMPLOT3D_CHECK_CTX() IM_ASSERT_USER_ERROR(GImPlot3D != nullptr, "No current context. Did you call ImPlot3D::CreateContext() or ImPlot3D::SetCurrentContext()?")
#define IMPLOT3D_CHECK_PLOT() IM_ASSERT_USER_ERROR(GImPlot3D->CurrentPlot != nullptr, "No active plot. Did you call ImPlot3D::BeginPlot()?")

//-----------------------------------------------------------------------------
// [SECTION] Context
//-----------------------------------------------------------------------------

namespace ImPlot3D {

// Global ImPlot3D context
#ifndef GImPlot3D
ImPlot3DContext* GImPlot3D = nullptr;
#endif

static ImPlot3DQuat init_rotation = ImPlot3DQuat(-0.513269, -0.212596, -0.318184, 0.76819);

ImPlot3DContext* CreateContext() {
    ImPlot3DContext* ctx = IM_NEW(ImPlot3DContext)();
    if (GImPlot3D == nullptr)
        SetCurrentContext(ctx);
    InitializeContext(ctx);
    return ctx;
}

void DestroyContext(ImPlot3DContext* ctx) {
    if (ctx == nullptr)
        ctx = GImPlot3D;
    if (GImPlot3D == ctx)
        SetCurrentContext(nullptr);
    IM_DELETE(ctx);
}

ImPlot3DContext* GetCurrentContext() { return GImPlot3D; }

void SetCurrentContext(ImPlot3DContext* ctx) { GImPlot3D = ctx; }

//-----------------------------------------------------------------------------
// [SECTION] Text Utils
//-----------------------------------------------------------------------------

void AddTextRotated(ImDrawList* draw_list, ImVec2 pos, float angle, ImU32 col, const char* text_begin, const char* text_end = nullptr) {
    if (!text_end)
        text_end = text_begin + strlen(text_begin);

    ImGuiContext& g = *GImGui;
    ImFont* font = g.Font;

    // Align to be pixel perfect
    pos.x = IM_FLOOR(pos.x);
    pos.y = IM_FLOOR(pos.y);

    const float scale = g.FontSize / font->FontSize;

    // Measure the size of the text in unrotated coordinates
    ImVec2 text_size = font->CalcTextSizeA(g.FontSize, FLT_MAX, 0.0f, text_begin, text_end, nullptr);

    // Precompute sine and cosine of the angle (note: angle should be positive for rotation in ImGui)
    float cos_a = cosf(-angle);
    float sin_a = sinf(-angle);

    const char* s = text_begin;
    int chars_total = (int)(text_end - s);
    int chars_rendered = 0;
    const int vtx_count_max = chars_total * 4;
    const int idx_count_max = chars_total * 6;
    draw_list->PrimReserve(idx_count_max, vtx_count_max);

    // Adjust pen position to center the text
    ImVec2 pen = ImVec2(-text_size.x * 0.5f, -text_size.y * 0.5f);

    while (s < text_end) {
        unsigned int c = (unsigned int)*s;
        if (c < 0x80) {
            s += 1;
        } else {
            s += ImTextCharFromUtf8(&c, s, text_end);
            if (c == 0) // Malformed UTF-8?
                break;
        }

        const ImFontGlyph* glyph = font->FindGlyph((ImWchar)c);
        if (glyph == nullptr) {
            continue;
        }

        // Glyph dimensions and positions
        ImVec2 glyph_offset = ImVec2(glyph->X0, glyph->Y0) * scale;
        ImVec2 glyph_size = ImVec2(glyph->X1 - glyph->X0, glyph->Y1 - glyph->Y0) * scale;

        // Corners of the glyph quad in unrotated space
        ImVec2 corners[4];
        corners[0] = pen + glyph_offset;
        corners[1] = pen + glyph_offset + ImVec2(glyph_size.x, 0);
        corners[2] = pen + glyph_offset + glyph_size;
        corners[3] = pen + glyph_offset + ImVec2(0, glyph_size.y);

        // Rotate and translate the corners
        for (int i = 0; i < 4; i++) {
            float x = corners[i].x;
            float y = corners[i].y;
            corners[i].x = x * cos_a - y * sin_a + pos.x;
            corners[i].y = x * sin_a + y * cos_a + pos.y;
        }

        // Texture coordinates
        ImVec2 uv0 = ImVec2(glyph->U0, glyph->V0);
        ImVec2 uv1 = ImVec2(glyph->U1, glyph->V1);

        // Render the glyph quad
        draw_list->PrimQuadUV(corners[0], corners[1], corners[2], corners[3],
                              uv0, ImVec2(glyph->U1, glyph->V0),
                              uv1, ImVec2(glyph->U0, glyph->V1),
                              col);

        // Advance the pen position
        pen.x += glyph->AdvanceX * scale;

        chars_rendered++;
    }

    // Return unused vertices
    int chars_skipped = chars_total - chars_rendered;
    draw_list->PrimUnreserve(chars_skipped * 6, chars_skipped * 4);
}

void AddTextCentered(ImDrawList* draw_list, ImVec2 top_center, ImU32 col, const char* text_begin) {
    const char* text_end = ImGui::FindRenderedTextEnd(text_begin);
    ImVec2 text_size = ImGui::CalcTextSize(text_begin, text_end, true);
    draw_list->AddText(ImVec2(top_center.x - text_size.x * 0.5f, top_center.y), col, text_begin, text_end);
}

//-----------------------------------------------------------------------------
// [SECTION] Legend Utils
//-----------------------------------------------------------------------------

ImVec2 GetLocationPos(const ImRect& outer_rect, const ImVec2& inner_size, ImPlot3DLocation loc, const ImVec2& pad) {
    ImVec2 pos;
    // Legend x coordinate
    if (ImHasFlag(loc, ImPlot3DLocation_West) && !ImHasFlag(loc, ImPlot3DLocation_East))
        pos.x = outer_rect.Min.x + pad.x;
    else if (!ImHasFlag(loc, ImPlot3DLocation_West) && ImHasFlag(loc, ImPlot3DLocation_East))
        pos.x = outer_rect.Max.x - pad.x - inner_size.x;
    else
        pos.x = outer_rect.GetCenter().x - inner_size.x * 0.5f;
    // Legend y coordinate
    if (ImHasFlag(loc, ImPlot3DLocation_North) && !ImHasFlag(loc, ImPlot3DLocation_South))
        pos.y = outer_rect.Min.y + pad.y;
    else if (!ImHasFlag(loc, ImPlot3DLocation_North) && ImHasFlag(loc, ImPlot3DLocation_South))
        pos.y = outer_rect.Max.y - pad.y - inner_size.y;
    else
        pos.y = outer_rect.GetCenter().y - inner_size.y * 0.5f;
    pos.x = IM_ROUND(pos.x);
    pos.y = IM_ROUND(pos.y);
    return pos;
}

ImVec2 CalcLegendSize(ImPlot3DItemGroup& items, const ImVec2& pad, const ImVec2& spacing, bool vertical) {
    const int nItems = items.GetLegendCount();
    const float txt_ht = ImGui::GetTextLineHeight();
    const float icon_size = txt_ht;
    // Get label max width
    float max_label_width = 0;
    float sum_label_width = 0;
    for (int i = 0; i < nItems; i++) {
        const char* label = items.GetLegendLabel(i);
        const float label_width = ImGui::CalcTextSize(label, nullptr, true).x;
        max_label_width = label_width > max_label_width ? label_width : max_label_width;
        sum_label_width += label_width;
    }
    // Compute legend size
    const ImVec2 legend_size = vertical ? ImVec2(pad.x * 2 + icon_size + max_label_width, pad.y * 2 + nItems * txt_ht + (nItems - 1) * spacing.y) : ImVec2(pad.x * 2 + icon_size * nItems + sum_label_width + (nItems - 1) * spacing.x, pad.y * 2 + txt_ht);
    return legend_size;
}

void ShowLegendEntries(ImPlot3DItemGroup& items, const ImRect& legend_bb, bool hovered, const ImVec2& pad, const ImVec2& spacing, bool vertical, ImDrawList& draw_list) {
    const float txt_ht = ImGui::GetTextLineHeight();
    const float icon_size = txt_ht;
    const float icon_shrink = 2;
    ImU32 col_txt = GetStyleColorU32(ImPlot3DCol_LegendText);
    ImU32 col_txt_dis = ImAlphaU32(col_txt, 0.25f);
    float sum_label_width = 0;

    const int num_items = items.GetLegendCount();
    if (num_items == 0)
        return;
    ImPlot3DContext& gp = *GImPlot3D;

    // Render legend items
    for (int i = 0; i < num_items; i++) {
        const int idx = i;
        ImPlot3DItem* item = items.GetLegendItem(idx);
        const char* label = items.GetLegendLabel(idx);
        const float label_width = ImGui::CalcTextSize(label, nullptr, true).x;
        const ImVec2 top_left = vertical ? legend_bb.Min + pad + ImVec2(0, i * (txt_ht + spacing.y)) : legend_bb.Min + pad + ImVec2(i * (icon_size + spacing.x) + sum_label_width, 0);
        sum_label_width += label_width;
        ImRect icon_bb;
        icon_bb.Min = top_left + ImVec2(icon_shrink, icon_shrink);
        icon_bb.Max = top_left + ImVec2(icon_size - icon_shrink, icon_size - icon_shrink);
        ImRect label_bb;
        label_bb.Min = top_left;
        label_bb.Max = top_left + ImVec2(label_width + icon_size, icon_size);
        ImU32 col_txt_hl;
        ImU32 col_item = ImAlphaU32(item->Color, 1);

        ImRect button_bb(icon_bb.Min, label_bb.Max);

        ImGui::KeepAliveID(item->ID);

        bool item_hov = false;
        bool item_hld = false;
        bool item_clk = ImHasFlag(items.Legend.Flags, ImPlot3DLegendFlags_NoButtons)
                            ? false
                            : ImGui::ButtonBehavior(button_bb, item->ID, &item_hov, &item_hld);

        if (item_clk)
            item->Show = !item->Show;

        const bool hovering = item_hov && !ImHasFlag(items.Legend.Flags, ImPlot3DLegendFlags_NoHighlightItem);

        if (hovering) {
            item->LegendHovered = true;
            col_txt_hl = ImMixU32(col_txt, col_item, 64);
        } else {
            item->LegendHovered = false;
            col_txt_hl = ImGui::GetColorU32(col_txt);
        }

        ImU32 col_icon;
        if (item_hld)
            col_icon = item->Show ? ImAlphaU32(col_item, 0.5f) : ImGui::GetColorU32(ImGuiCol_TextDisabled, 0.5f);
        else if (item_hov)
            col_icon = item->Show ? ImAlphaU32(col_item, 0.75f) : ImGui::GetColorU32(ImGuiCol_TextDisabled, 0.75f);
        else
            col_icon = item->Show ? col_item : col_txt_dis;

        draw_list.AddRectFilled(icon_bb.Min, icon_bb.Max, col_icon);
        const char* text_display_end = ImGui::FindRenderedTextEnd(label, nullptr);
        if (label != text_display_end)
            draw_list.AddText(top_left + ImVec2(icon_size, 0), item->Show ? col_txt_hl : col_txt_dis, label, text_display_end);
    }
}

void RenderLegend() {
    ImPlot3DContext& gp = *GImPlot3D;
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    if (ImHasFlag(plot.Flags, ImPlot3DFlags_NoLegend) || plot.Items.GetLegendCount() == 0)
        return;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImDrawList* draw_list = window->DrawList;
    const ImGuiIO& IO = ImGui::GetIO();

    ImPlot3DLegend& legend = plot.Items.Legend;
    const bool legend_horz = ImHasFlag(legend.Flags, ImPlot3DLegendFlags_Horizontal);
    const ImVec2 legend_size = CalcLegendSize(plot.Items, gp.Style.LegendInnerPadding, gp.Style.LegendSpacing, !legend_horz);
    const ImVec2 legend_pos = GetLocationPos(plot.PlotRect,
                                             legend_size,
                                             legend.Location,
                                             gp.Style.LegendPadding);
    legend.Rect = ImRect(legend_pos, legend_pos + legend_size);

    // Test hover
    legend.Hovered = ImGui::IsWindowHovered() && legend.Rect.Contains(IO.MousePos);

    // Render background
    ImU32 col_bg = GetStyleColorU32(ImPlot3DCol_LegendBg);
    ImU32 col_bd = GetStyleColorU32(ImPlot3DCol_LegendBorder);
    draw_list->AddRectFilled(legend.Rect.Min, legend.Rect.Max, col_bg);
    draw_list->AddRect(legend.Rect.Min, legend.Rect.Max, col_bd);

    // Render legends
    ShowLegendEntries(plot.Items, legend.Rect, legend.Hovered, gp.Style.LegendInnerPadding, gp.Style.LegendSpacing, !legend_horz, *draw_list);
}

//-----------------------------------------------------------------------------
// [SECTION] Plot Box Utils
//-----------------------------------------------------------------------------

// Faces of the box (defined by 4 corner indices)
static const int faces[6][4] = {
    {0, 3, 7, 4}, // X-min face
    {0, 4, 5, 1}, // Y-min face
    {0, 1, 2, 3}, // Z-min face
    {1, 2, 6, 5}, // X-max face
    {3, 7, 6, 2}, // Y-max face
    {4, 5, 6, 7}, // Z-max face
};

// Edges of the box (defined by 2 corner indices)
static const int edges[12][2] = {
    // Bottom face edges
    {0, 1},
    {1, 2},
    {2, 3},
    {3, 0},
    // Top face edges
    {4, 5},
    {5, 6},
    {6, 7},
    {7, 4},
    // Vertical edges
    {0, 4},
    {1, 5},
    {2, 6},
    {3, 7},
};

// Face edges (4 edge indices for each face)
static const int face_edges[6][4] = {
    {3, 11, 8, 7},  // X-min face
    {0, 8, 4, 9},   // Y-min face
    {0, 1, 2, 3},   // Z-min face
    {1, 9, 5, 10},  // X-max face
    {2, 10, 6, 11}, // Y-max face
    {4, 5, 6, 7},   // Z-max face
};

// Lookup table for axis_corners based on active_faces
static const int axis_corners_lookup[8][3][2] = {
    // Index 0: active_faces = {0, 0, 0}
    {{3, 2}, {1, 2}, {1, 5}},
    // Index 1: active_faces = {0, 0, 1}
    {{7, 6}, {5, 6}, {1, 5}},
    // Index 2: active_faces = {0, 1, 0}
    {{0, 1}, {1, 2}, {2, 6}},
    // Index 3: active_faces = {0, 1, 1}
    {{4, 5}, {5, 6}, {2, 6}},
    // Index 4: active_faces = {1, 0, 0}
    {{3, 2}, {0, 3}, {0, 4}},
    // Index 5: active_faces = {1, 0, 1}
    {{7, 6}, {4, 7}, {0, 4}},
    // Index 6: active_faces = {1, 1, 0}
    {{0, 1}, {0, 3}, {3, 7}},
    // Index 7: active_faces = {1, 1, 1}
    {{4, 5}, {4, 7}, {3, 7}},
};

void RenderPlotBackground(ImDrawList* draw_list, const ImPlot3DPlot& plot, const ImVec2* corners_pix, const bool* active_faces) {
    const ImU32 col_bg = GetStyleColorU32(ImPlot3DCol_PlotBg);
    for (int a = 0; a < 3; a++) {
        int idx[4]; // Corner indices
        for (int i = 0; i < 4; i++)
            idx[i] = faces[a + 3 * active_faces[a]][i];
        draw_list->AddQuadFilled(corners_pix[idx[0]], corners_pix[idx[1]], corners_pix[idx[2]], corners_pix[idx[3]], col_bg);
    }
}

void RenderPlotBorder(ImDrawList* draw_list, const ImVec2* corners_pix, const bool* active_faces) {
    bool render_edge[12] = {false};
    for (int a = 0; a < 3; a++) {
        int face_idx = a + 3 * active_faces[a];
        for (size_t i = 0; i < 4; i++)
            render_edge[face_edges[face_idx][i]] = true;
    }
    ImU32 col_bd = GetStyleColorU32(ImPlot3DCol_PlotBorder);
    for (int i = 0; i < 12; i++) {
        if (render_edge[i]) {
            int idx0 = edges[i][0];
            int idx1 = edges[i][1];
            draw_list->AddLine(corners_pix[idx0], corners_pix[idx1], col_bd);
        }
    }
}

void RenderPlotGrid(ImDrawList* draw_list, const ImPlot3DPlot& plot, const ImPlot3DPoint* corners, const bool* active_faces) {
    ImVec4 col_grid = GetStyleColorVec4(ImPlot3DCol_AxisGrid);
    ImU32 col_grid_minor = ImGui::GetColorU32(col_grid * ImVec4(1, 1, 1, 0.3f));
    ImU32 col_grid_major = ImGui::GetColorU32(col_grid * ImVec4(1, 1, 1, 0.6f));
    for (int face = 0; face < 3; face++) {
        int face_idx = face + 3 * active_faces[face];
        const ImPlot3DAxis& axis_u = plot.Axes[(face + 1) % 3];
        const ImPlot3DAxis& axis_v = plot.Axes[(face + 2) % 3];

        // Get the two axes (u and v) that define the face plane
        int idx0 = faces[face_idx][0];
        int idx1 = faces[face_idx][1];
        int idx2 = faces[face_idx][2];
        int idx3 = faces[face_idx][3];

        // Corners of the face in plot space
        ImPlot3DPoint p0 = corners[idx0];
        ImPlot3DPoint p1 = corners[idx1];
        ImPlot3DPoint p2 = corners[idx2];
        ImPlot3DPoint p3 = corners[idx3];

        // Vectors along the edges
        ImPlot3DPoint u_vec = p1 - p0;
        ImPlot3DPoint v_vec = p3 - p0;

        // Render grid lines along u axis (axis_u)
        for (int t = 0; t < axis_u.Ticker.TickCount(); ++t) {
            const ImPlot3DTick& tick = axis_u.Ticker.Ticks[t];

            // Compute position along u
            float t_u = (tick.PlotPos - axis_u.Range.Min) / (axis_u.Range.Max - axis_u.Range.Min);
            ImPlot3DPoint p_start = p0 + u_vec * t_u;
            ImPlot3DPoint p_end = p3 + u_vec * t_u;

            // Convert to pixel coordinates
            ImVec2 p_start_pix = PlotToPixels(p_start);
            ImVec2 p_end_pix = PlotToPixels(p_end);

            // Get color
            ImU32 col_line = tick.Major ? col_grid_major : col_grid_minor;

            // Draw the grid line
            draw_list->AddLine(p_start_pix, p_end_pix, col_line);
        }

        // Render grid lines along v axis (axis_v)
        for (int t = 0; t < axis_v.Ticker.TickCount(); ++t) {
            const ImPlot3DTick& tick = axis_v.Ticker.Ticks[t];

            // Compute position along v
            float t_v = (tick.PlotPos - axis_v.Range.Min) / (axis_v.Range.Max - axis_v.Range.Min);
            ImPlot3DPoint p_start = p0 + v_vec * t_v;
            ImPlot3DPoint p_end = p1 + v_vec * t_v;

            // Convert to pixel coordinates
            ImVec2 p_start_pix = PlotToPixels(p_start);
            ImVec2 p_end_pix = PlotToPixels(p_end);

            // Get color
            ImU32 col_line = tick.Major ? col_grid_major : col_grid_minor;

            // Draw the grid line
            draw_list->AddLine(p_start_pix, p_end_pix, col_line);
        }
    }
}

void RenderTickLabels(ImDrawList* draw_list, const ImPlot3DPlot& plot, const ImPlot3DPoint* corners, const ImVec2* corners_pix, const int axis_corners[3][2]) {
    ImVec2 box_center_pix = PlotToPixels(plot.RangeCenter());
    ImU32 col_tick_txt = GetStyleColorU32(ImPlot3DCol_AxisText);

    for (int a = 0; a < 3; a++) {
        const ImPlot3DAxis& axis = plot.Axes[a];

        // Corner indices for this axis
        int idx0 = axis_corners[a][0];
        int idx1 = axis_corners[a][1];

        // Start and end points of the axis in plot space
        ImPlot3DPoint axis_start = corners[idx0];
        ImPlot3DPoint axis_end = corners[idx1];

        // Direction vector along the axis
        ImPlot3DPoint axis_dir = axis_end - axis_start;

        // Convert axis start and end to screen space
        ImVec2 axis_start_pix = corners_pix[idx0];
        ImVec2 axis_end_pix = corners_pix[idx1];

        // Screen space axis direction
        ImVec2 axis_screen_dir = axis_end_pix - axis_start_pix;
        float axis_length = ImSqrt(ImLengthSqr(axis_screen_dir));
        if (axis_length != 0.0f)
            axis_screen_dir /= axis_length;
        else
            axis_screen_dir = ImVec2(1.0f, 0.0f); // Default direction if length is zero

        // Perpendicular direction in screen space
        ImVec2 offset_dir_pix = ImVec2(-axis_screen_dir.y, axis_screen_dir.x);

        // Make sure direction points away from cube center
        ImVec2 box_center_pix = PlotToPixels(plot.RangeCenter());
        ImVec2 axis_center_pix = (axis_start_pix + axis_end_pix) * 0.5f;
        ImVec2 center_to_axis_pix = axis_center_pix - box_center_pix;
        center_to_axis_pix /= ImSqrt(ImLengthSqr(center_to_axis_pix));
        if (ImDot(offset_dir_pix, center_to_axis_pix) < 0.0f)
            offset_dir_pix = -offset_dir_pix;

        // Adjust the offset magnitude
        float offset_magnitude = 20.0f; // Adjust as needed
        ImVec2 offset_pix = offset_dir_pix * offset_magnitude;

        // Compute angle perpendicular to axis in screen space
        float angle = atan2f(-axis_screen_dir.y, axis_screen_dir.x) + IM_PI * 0.5f;

        // Normalize angle to be between -π and π
        if (angle > IM_PI)
            angle -= 2 * IM_PI;
        if (angle < -IM_PI)
            angle += 2 * IM_PI;

        // Adjust angle to keep labels upright
        if (angle > IM_PI * 0.5f)
            angle -= IM_PI;
        if (angle < -IM_PI * 0.5f)
            angle += IM_PI;

        // Loop over ticks
        for (int t = 0; t < axis.Ticker.TickCount(); ++t) {
            const ImPlot3DTick& tick = axis.Ticker.Ticks[t];
            if (!tick.ShowLabel)
                continue;

            // Compute position along the axis
            float t_axis = (tick.PlotPos - axis.Range.Min) / (axis.Range.Max - axis.Range.Min);
            ImPlot3DPoint tick_pos = axis_start + axis_dir * t_axis;

            // Convert to pixel coordinates
            ImVec2 tick_pos_pix = PlotToPixels(tick_pos);

            // Get the tick label text
            const char* label = axis.Ticker.GetText(tick);

            // Adjust label position by offset
            ImVec2 label_pos_pix = tick_pos_pix + offset_pix;

            // Render the tick label
            AddTextRotated(draw_list, label_pos_pix, angle, col_tick_txt, label);
        }
    }
}

void RenderAxisLabels(ImDrawList* draw_list, const ImPlot3DPlot& plot, const ImPlot3DPoint* corners, const ImVec2* corners_pix, const int axis_corners[3][2]) {
    ImPlot3DPoint range_center = plot.RangeCenter();
    for (int a = 0; a < 3; a++) {
        const ImPlot3DAxis& axis = plot.Axes[a];
        if (!axis.HasLabel())
            continue;

        const char* label = plot.GetAxisLabel(axis);

        // Corner indices
        int idx0 = axis_corners[a][0];
        int idx1 = axis_corners[a][1];

        // Position at the end of the axis
        ImPlot3DPoint label_pos = (corners[idx0] + corners[idx1]) * 0.5f;
        // Add offset
        label_pos += (label_pos - range_center) * 0.4f;

        // Convert to pixel coordinates
        ImVec2 label_pos_pix = PlotToPixels(label_pos);

        // Adjust label position and angle
        ImU32 col_ax_txt = GetStyleColorU32(ImPlot3DCol_AxisText);

        // Compute text angle
        ImVec2 screen_delta = corners_pix[idx1] - corners_pix[idx0];
        float angle = atan2f(-screen_delta.y, screen_delta.x);
        if (angle > IM_PI * 0.5f)
            angle -= IM_PI;
        if (angle < -IM_PI * 0.5f)
            angle += IM_PI;

        AddTextRotated(draw_list, label_pos_pix, angle, col_ax_txt, label);
    }
}

void RenderPlotBox(ImDrawList* draw_list, const ImPlot3DPlot& plot) {
    // Get plot parameters
    const ImRect& plot_area = plot.PlotRect;
    const ImPlot3DQuat& rotation = plot.Rotation;
    ImPlot3DPoint range_min = plot.RangeMin();
    ImPlot3DPoint range_max = plot.RangeMax();
    ImPlot3DPoint range_center = plot.RangeCenter();

    // Compute zoom and center
    float zoom = ImMin(plot_area.GetWidth(), plot_area.GetHeight()) / 1.8f;
    ImVec2 center = plot_area.GetCenter();

    // Define rotated plot box face normals
    ImPlot3DPoint rot_face_n[3] = {
        rotation * ImPlot3DPoint(1.0f, 0.0f, 0.0f),
        rotation * ImPlot3DPoint(0.0f, 1.0f, 0.0f),
        rotation * ImPlot3DPoint(0.0f, 0.0f, 1.0f),
    };

    // Active faces to be plotted (visible from the viewer's perspective)
    bool active_faces[3] = {rot_face_n[0].z < 0, rot_face_n[1].z < 0, rot_face_n[2].z < 0};

    // Box corners in plot space
    ImPlot3DPoint corners[8] = {
        ImPlot3DPoint(range_min.x, range_min.y, range_min.z), // 0
        ImPlot3DPoint(range_max.x, range_min.y, range_min.z), // 1
        ImPlot3DPoint(range_max.x, range_max.y, range_min.z), // 2
        ImPlot3DPoint(range_min.x, range_max.y, range_min.z), // 3
        ImPlot3DPoint(range_min.x, range_min.y, range_max.z), // 4
        ImPlot3DPoint(range_max.x, range_min.y, range_max.z), // 5
        ImPlot3DPoint(range_max.x, range_max.y, range_max.z), // 6
        ImPlot3DPoint(range_min.x, range_max.y, range_max.z), // 7
    };

    // Box corners in pixel space
    ImVec2 corners_pix[8];
    for (int i = 0; i < 8; i++)
        corners_pix[i] = PlotToPixels(corners[i]);

    // Compute axes start and end corners (given current rotation)
    int axis_corners[3][2];
    int index = (active_faces[0] << 2) | (active_faces[1] << 1) | (active_faces[2]);
    for (int a = 0; a < 3; a++) {
        axis_corners[a][0] = axis_corners_lookup[index][a][0];
        axis_corners[a][1] = axis_corners_lookup[index][a][1];
    }

    // Render components
    RenderPlotBackground(draw_list, plot, corners_pix, active_faces);
    RenderPlotBorder(draw_list, corners_pix, active_faces);
    RenderPlotGrid(draw_list, plot, corners, active_faces);
    RenderTickLabels(draw_list, plot, corners, corners_pix, axis_corners);
    RenderAxisLabels(draw_list, plot, corners, corners_pix, axis_corners);
}

//-----------------------------------------------------------------------------
// [SECTION] Formatter
//-----------------------------------------------------------------------------

int Formatter_Default(float value, char* buff, int size, void* data) {
    char* fmt = (char*)data;
    return ImFormatString(buff, size, fmt, value);
}

//------------------------------------------------------------------------------
// [SECTION] Locator
//------------------------------------------------------------------------------

double NiceNum(double x, bool round) {
    double f;
    double nf;
    int expv = (int)floor(ImLog10(x));
    f = x / ImPow(10.0, (double)expv);
    if (round)
        if (f < 1.5)
            nf = 1;
        else if (f < 3)
            nf = 2;
        else if (f < 7)
            nf = 5;
        else
            nf = 10;
    else if (f <= 1)
        nf = 1;
    else if (f <= 2)
        nf = 2;
    else if (f <= 5)
        nf = 5;
    else
        nf = 10;
    return nf * ImPow(10.0, expv);
}

void Locator_Default(ImPlot3DTicker& ticker, const ImPlot3DRange& range, ImPlot3DFormatter formatter, void* formatter_data) {
    if (range.Min == range.Max)
        return;
    const int nMinor = 5;
    const int nMajor = 3;
    const int max_ticks_labels = 7;
    const double nice_range = NiceNum(range.Size() * 0.99, false);
    const double interval = NiceNum(nice_range / (nMajor - 1), true);
    const double graphmin = floor(range.Min / interval) * interval;
    const double graphmax = ceil(range.Max / interval) * interval;
    bool first_major_set = false;
    int first_major_idx = 0;
    const int idx0 = ticker.TickCount(); // ticker may have user custom ticks
    ImVec2 total_size(0, 0);
    for (double major = graphmin; major < graphmax + 0.5 * interval; major += interval) {
        // is this zero? combat zero formatting issues
        if (major - interval < 0 && major + interval > 0)
            major = 0;
        if (range.Contains(major)) {
            if (!first_major_set) {
                first_major_idx = ticker.TickCount();
                first_major_set = true;
            }
            total_size += ticker.AddTick(major, true, true, formatter, formatter_data).LabelSize;
        }
        for (int i = 1; i < nMinor; ++i) {
            double minor = major + i * interval / nMinor;
            if (range.Contains(minor)) {
                total_size += ticker.AddTick(minor, false, true, formatter, formatter_data).LabelSize;
            }
        }
    }

    // Prune tick labels
    if (ticker.TickCount() > max_ticks_labels) {
        for (int i = first_major_idx - 1; i >= idx0; i -= 2)
            ticker.Ticks[i].ShowLabel = false;
        for (int i = first_major_idx + 1; i < ticker.TickCount(); i += 2)
            ticker.Ticks[i].ShowLabel = false;
    }
}

//-----------------------------------------------------------------------------
// [SECTION] Begin/End Plot
//-----------------------------------------------------------------------------

bool BeginPlot(const char* title_id, const ImVec2& size, ImPlot3DFlags flags) {
    IMPLOT3D_CHECK_CTX();
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot == nullptr, "Mismatched BeginPlot()/EndPlot()!");

    // Get window
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;

    // Skip if needed
    if (window->SkipItems)
        return false;

    // Get or create plot
    const ImGuiID ID = window->GetID(title_id);
    const bool just_created = gp.Plots.GetByKey(ID) == nullptr;
    gp.CurrentPlot = gp.Plots.GetOrAddByKey(ID);
    gp.CurrentItems = &gp.CurrentPlot->Items;
    ImPlot3DPlot& plot = *gp.CurrentPlot;

    // Populate plot ID/flags
    plot.ID = ID;
    plot.Flags = flags;
    plot.JustCreated = just_created;
    if (just_created) {
        plot.Rotation = init_rotation;
        for (int i = 0; i < 3; i++)
            plot.Axes[i] = ImPlot3DAxis();
    }
    plot.SetupLocked = false;

    // Populate title
    if (title_id && ImGui::FindRenderedTextEnd(title_id, nullptr) != title_id && !ImHasFlag(plot.Flags, ImPlot3DFlags_NoTitle))
        plot.TextBuffer.append(title_id, title_id + strlen(title_id) + 1);
    else
        plot.TextBuffer.clear();

    // Calculate frame size
    ImVec2 frame_size = ImGui::CalcItemSize(size, gp.Style.PlotDefaultSize.x, gp.Style.PlotDefaultSize.y);
    if (frame_size.x < gp.Style.PlotMinSize.x && size.x < 0.0f)
        frame_size.x = gp.Style.PlotMinSize.x;
    if (frame_size.y < gp.Style.PlotMinSize.y && size.y < 0.0f)
        frame_size.y = gp.Style.PlotMinSize.y;

    plot.FrameRect = ImRect(window->DC.CursorPos, window->DC.CursorPos + frame_size);
    ImGui::ItemSize(plot.FrameRect);
    if (!ImGui::ItemAdd(plot.FrameRect, plot.ID, &plot.FrameRect)) {
        gp.CurrentPlot = nullptr;
        gp.CurrentItems = nullptr;
        return false;
    }

    // Reset legend
    plot.Items.Legend.Reset();

    // Push frame rect clipping
    ImGui::PushClipRect(plot.FrameRect.Min, plot.FrameRect.Max, true);

    return true;
}

void EndPlot() {
    IMPLOT3D_CHECK_CTX();
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "Mismatched BeginPlot()/EndPlot()!");
    ImPlot3DPlot& plot = *gp.CurrentPlot;

    // Handle data fitting
    if (plot.FitThisFrame) {
        plot.FitThisFrame = false;
        plot.Rotation = init_rotation;
        for (int i = 0; i < 3; i++) {
            if (plot.Axes[i].FitThisFrame) {
                plot.Axes[i].FitThisFrame = false;
                plot.Axes[i].ApplyFit();
            }
        }
    }

    // Lock setup if not already done
    SetupLock();

    // Reset legend hover
    plot.Items.Legend.Hovered = false;

    // Render legend
    RenderLegend();

    // Pop frame rect clipping
    ImGui::PopClipRect();

    // Reset current plot
    gp.CurrentPlot = nullptr;
    gp.CurrentItems = nullptr;
}

//-----------------------------------------------------------------------------
// [SECTION] Setup
//-----------------------------------------------------------------------------

void SetupAxis(ImAxis3D idx, const char* label, ImPlot3DAxisFlags flags) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr && !gp.CurrentPlot->SetupLocked,
                         "SetupAxis() needs to be called after BeginPlot() and before any setup locking functions (e.g. PlotX)!");

    // Get plot and axis
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    ImPlot3DAxis& axis = plot.Axes[idx];
    axis.Flags = flags;
    plot.SetAxisLabel(axis, label);
}

void SetupAxisLimits(ImAxis3D idx, double min_lim, double max_lim, ImPlot3DCond cond) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr && !gp.CurrentPlot->SetupLocked,
                         "SetupAxisLimits() needs to be called after BeginPlot and before any setup locking functions (e.g. PlotX)!"); // get plot and axis
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    ImPlot3DAxis& axis = plot.Axes[idx];
    if (plot.JustCreated || cond == ImPlot3DCond_Always) {
        axis.SetRange(min_lim, max_lim);
        axis.RangeCond = cond;
    }
}

void SetupAxes(const char* x_label, const char* y_label, const char* z_label, ImPlot3DAxisFlags x_flags, ImPlot3DAxisFlags y_flags, ImPlot3DAxisFlags z_flags) {
    SetupAxis(ImAxis3D_X, x_label, x_flags);
    SetupAxis(ImAxis3D_Y, y_label, y_flags);
    SetupAxis(ImAxis3D_Z, z_label, z_flags);
}

void SetupLegend(ImPlot3DLocation location, ImPlot3DLegendFlags flags) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr && !gp.CurrentPlot->SetupLocked,
                         "SetupLegend() needs to be called after BeginPlot() and before any setup locking functions (e.g. PlotX)!");
    IM_ASSERT_USER_ERROR(gp.CurrentItems != nullptr,
                         "SetupLegend() needs to be called within an itemized context!");
    ImPlot3DLegend& legend = gp.CurrentItems->Legend;
    legend.Location = location;
    legend.Flags = flags;
}

//-----------------------------------------------------------------------------
// [SECTION] Plot Utils
//-----------------------------------------------------------------------------

ImPlot3DPlot* GetCurrentPlot() {
    return GImPlot3D->CurrentPlot;
}

ImVec2 PlotToPixels(const ImPlot3DPoint& point) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "PlotToPixels() needs to be called between BeginPlot() and EndPlot()!");
    return NDCToPixels(PlotToNDC(point));
}

ImVec2 PlotToPixels(double x, double y, double z) {
    return PlotToPixels(ImPlot3DPoint(x, y, z));
}

ImPlot3DRay PixelsToPlotRay(const ImVec2& pix) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "PixelsToPlotRay() needs to be called between BeginPlot() and EndPlot()!");
    return NDCRayToPlotRay(PixelsToNDCRay(pix));
}

ImPlot3DRay PixelsToPlotRay(double x, double y) {
    return PixelsToPlotRay(ImVec2(x, y));
}

ImVec2 GetPlotPos() {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "GetPlotPos() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    return gp.CurrentPlot->PlotRect.Min;
}

ImVec2 GetPlotSize() {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "GetPlotSize() needs to be called between BeginPlot() and EndPlot()!");
    SetupLock();
    return gp.CurrentPlot->PlotRect.GetSize();
}

ImVec2 GetFramePos() {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "GetFramePos() needs to be called between BeginPlot() and EndPlot()!");
    return gp.CurrentPlot->FrameRect.Min;
}

ImVec2 GetFrameSize() {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "GetFrameSize() needs to be called between BeginPlot() and EndPlot()!");
    return gp.CurrentPlot->FrameRect.GetSize();
}

ImPlot3DPoint PlotToNDC(const ImPlot3DPoint& point) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "PlotToNDC() needs to be called between BeginPlot() and EndPlot()!");
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    SetupLock();

    ImPlot3DPoint ndc_point;
    for (int i = 0; i < 3; i++)
        ndc_point[i] = plot.Axes[i].PlotToNDC(point[i]);
    return ndc_point;
}

ImPlot3DPoint NDCToPlot(const ImPlot3DPoint& point) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "NDCToPlot() needs to be called between BeginPlot() and EndPlot()!");
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    SetupLock();

    ImPlot3DPoint plot_point;
    for (int i = 0; i < 3; i++)
        plot_point[i] = plot.Axes[i].NDCToPlot(point[i]);
    return plot_point;
}

ImVec2 NDCToPixels(const ImPlot3DPoint& point) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "NDCToPixels() needs to be called between BeginPlot() and EndPlot()!");
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    SetupLock();

    float zoom = ImMin(plot.PlotRect.GetWidth(), plot.PlotRect.GetHeight()) / 1.8f;
    ImVec2 center = plot.PlotRect.GetCenter();
    ImPlot3DPoint point_pix = zoom * (plot.Rotation * point);
    point_pix.y *= -1.0f; // Invert y-axis
    point_pix.x += center.x;
    point_pix.y += center.y;

    return {point_pix.x, point_pix.y};
}

ImPlot3DRay PixelsToNDCRay(const ImVec2& pix) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "PixelsToNDCRay() needs to be called between BeginPlot() and EndPlot()!");
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    SetupLock();

    // Calculate zoom factor and plot center
    float zoom = ImMin(plot.PlotRect.GetWidth(), plot.PlotRect.GetHeight()) / 1.8f;
    ImVec2 center = plot.PlotRect.GetCenter();

    // Undo screen transformations to get back to NDC space
    float x = (pix.x - center.x) / zoom;
    float y = -(pix.y - center.y) / zoom; // Invert y-axis

    // Define near and far points in NDC space along the z-axis
    ImPlot3DPoint ndc_near = plot.Rotation.Inverse() * ImPlot3DPoint(x, y, -0.5f);
    ImPlot3DPoint ndc_far = plot.Rotation.Inverse() * ImPlot3DPoint(x, y, 0.5f);

    // Create the ray in NDC space
    ImPlot3DRay ndc_ray;
    ndc_ray.Origin = ndc_near;
    ndc_ray.Direction = (ndc_far - ndc_near).Normalized();

    return ndc_ray;
}

ImPlot3DRay NDCRayToPlotRay(const ImPlot3DRay& ray) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "NDCRayToPlotRay() needs to be called between BeginPlot() and EndPlot()!");
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    SetupLock();

    // Convert NDC origin and a point along the ray to plot coordinates
    ImPlot3DPoint plot_origin = NDCToPlot(ray.Origin);
    ImPlot3DPoint ndc_point_along_ray = ray.Origin + ray.Direction;
    ImPlot3DPoint plot_point_along_ray = NDCToPlot(ndc_point_along_ray);

    // Compute the direction in plot coordinates
    ImPlot3DPoint plot_direction = (plot_point_along_ray - plot_origin).Normalized();

    // Create the ray in plot coordinates
    ImPlot3DRay plot_ray;
    plot_ray.Origin = plot_origin;
    plot_ray.Direction = plot_direction;

    return plot_ray;
}

//-----------------------------------------------------------------------------
// [SECTION] Setup Utils
//-----------------------------------------------------------------------------

void HandleInput(ImPlot3DPlot& plot) {
    ImGuiIO& IO = ImGui::GetIO();

    // clang-format off
    const ImGuiButtonFlags plot_button_flags = ImGuiButtonFlags_AllowOverlap
                                             | ImGuiButtonFlags_PressedOnClick
                                             | ImGuiButtonFlags_PressedOnDoubleClick
                                             | ImGuiButtonFlags_MouseButtonLeft
                                             | ImGuiButtonFlags_MouseButtonRight
                                             | ImGuiButtonFlags_MouseButtonMiddle;
    // clang-format on
    const bool plot_clicked = ImGui::ButtonBehavior(plot.PlotRect, plot.ID, &plot.Hovered, &plot.Held, plot_button_flags);
#if (IMGUI_VERSION_NUM < 18966)
    ImGui::SetItemAllowOverlap(); // Handled by ButtonBehavior()
#endif

    // Handle double click
    if (plot_clicked && ImGui::IsMouseDoubleClicked(0)) {
        plot.FitThisFrame = true;
        for (int i = 0; i < 3; i++)
            plot.Axes[i].FitThisFrame = true;
    }

    // Handle translation with right mouse button
    if (plot.Held && ImGui::IsMouseDown(0)) {
        ImVec2 delta(IO.MouseDelta.x, IO.MouseDelta.y);

        // Compute delta_pixels in 3D (invert y-axis)
        ImPlot3DPoint delta_pixels(delta.x, -delta.y, 0.0f);

        // Convert delta to NDC space
        float zoom = ImMin(plot.PlotRect.GetWidth(), plot.PlotRect.GetHeight()) / 1.8f;
        ImPlot3DPoint delta_NDC = plot.Rotation.Inverse() * (delta_pixels / zoom);

        // Convert delta to plot space
        ImPlot3DPoint delta_plot = delta_NDC * (plot.RangeMax() - plot.RangeMin());

        // Adjust plot range to translate the plot
        plot.SetRange(plot.RangeMin() - delta_plot, plot.RangeMax() - delta_plot);
    }

    // Handle rotation with left mouse dragging
    if (plot.Held && ImGui::IsMouseDown(1)) {
        ImVec2 delta(IO.MouseDelta.x, IO.MouseDelta.y);

        // Map delta to rotation angles (in radians)
        float angle_x = delta.x * (3.1415f / 180.0f);
        float angle_y = delta.y * (3.1415f / 180.0f);

        // Create quaternions for the rotations
        ImPlot3DQuat quat_x(angle_y, ImPlot3DPoint(1.0f, 0.0f, 0.0f));
        ImPlot3DQuat quat_z(angle_x, ImPlot3DPoint(0.0f, 0.0f, 1.0f));

        // Combine the new rotations with the current rotation
        plot.Rotation = quat_x * plot.Rotation * quat_z;
        plot.Rotation.Normalize();
    }

    // Handle zoom with mouse wheel
    if (plot.Hovered && (ImGui::IsMouseDown(2) || IO.MouseWheel != 0)) {
        float delta = ImGui::IsMouseDown(2) ? (-0.01f * IO.MouseDelta.y) : (-0.1f * IO.MouseWheel);
        float zoom = 1.0f + delta;
        ImPlot3DPoint center = (plot.RangeMin() + plot.RangeMax()) * 0.5f;
        ImPlot3DPoint size = plot.RangeMax() - plot.RangeMin();
        size *= zoom;
        plot.SetRange(center - size * 0.5f, center + size * 0.5f);
    }
}

void SetupLock() {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "SetupLock() needs to be called between BeginPlot() and EndPlot()!");
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    if (plot.SetupLocked)
        return;
    // Lock setup
    plot.SetupLocked = true;

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImDrawList* draw_list = window->DrawList;

    ImGui::PushClipRect(plot.FrameRect.Min, plot.FrameRect.Max, true);

    // Set default formatter/locator
    for (int i = 0; i < 3; i++) {
        ImPlot3DAxis& axis = plot.Axes[i];

        // Set formatter
        if (axis.Formatter == nullptr) {
            axis.Formatter = Formatter_Default;
            if (axis.FormatterData == nullptr)
                axis.FormatterData = (void*)IMPLOT3D_LABEL_FORMAT;
        }

        // Set locator
        if (axis.Locator == nullptr)
            axis.Locator = Locator_Default;
    }

    // Draw frame background
    ImU32 f_bg_color = GetStyleColorU32(ImPlot3DCol_FrameBg);
    draw_list->AddRectFilled(plot.FrameRect.Min, plot.FrameRect.Max, f_bg_color);

    // Compute canvas/canvas rectangle
    plot.CanvasRect = ImRect(plot.FrameRect.Min + gp.Style.PlotPadding, plot.FrameRect.Max - gp.Style.PlotPadding);
    plot.PlotRect = plot.CanvasRect;

    // Handle user input
    HandleInput(plot);

    // Compute ticks
    for (int i = 0; i < 3; i++) {
        ImPlot3DAxis& axis = plot.Axes[i];
        axis.Ticker.Reset();
        axis.Locator(axis.Ticker, axis.Range, axis.Formatter, axis.FormatterData);
    }

    // Render plot box
    RenderPlotBox(draw_list, plot);

    // Render title
    if (!plot.TextBuffer.empty()) {
        ImU32 col = GetStyleColorU32(ImPlot3DCol_TitleText);
        ImVec2 top_center = ImVec2(plot.FrameRect.GetCenter().x, plot.CanvasRect.Min.y);
        AddTextCentered(draw_list, top_center, col, plot.TextBuffer.c_str());
        plot.PlotRect.Min.y += ImGui::GetTextLineHeight() + gp.Style.LabelPadding.y;
    }

    ImGui::PopClipRect();
}

//-----------------------------------------------------------------------------
// [SECTION] Miscellaneous
//-----------------------------------------------------------------------------

ImDrawList* GetPlotDrawList() {
    return ImGui::GetWindowDrawList();
}

//-----------------------------------------------------------------------------
// [SECTION] Styles
//-----------------------------------------------------------------------------

struct ImPlot3DStyleVarInfo {
    ImGuiDataType Type;
    ImU32 Count;
    ImU32 Offset;
    void* GetVarPtr(ImPlot3DStyle* style) const { return (void*)((unsigned char*)style + Offset); }
};

static const ImPlot3DStyleVarInfo GPlot3DStyleVarInfo[] =
    {
        // Item style
        {ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlot3DStyle, LineWeight)},   // ImPlot3DStyleVar_LineWeight
        {ImGuiDataType_S32, 1, (ImU32)IM_OFFSETOF(ImPlot3DStyle, Marker)},         // ImPlot3DStyleVar_Marker
        {ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlot3DStyle, MarkerSize)},   // ImPlot3DStyleVar_MarkerSize
        {ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlot3DStyle, MarkerWeight)}, // ImPlot3DStyleVar_MarkerWeight
        {ImGuiDataType_Float, 1, (ImU32)IM_OFFSETOF(ImPlot3DStyle, FillAlpha)},    // ImPlot3DStyleVar_FillAlpha

        // Plot style
        {ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlot3DStyle, PlotDefaultSize)}, // ImPlot3DStyleVar_Plot3DDefaultSize
        {ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlot3DStyle, PlotMinSize)},     // ImPlot3DStyleVar_Plot3DMinSize
        {ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlot3DStyle, PlotPadding)},     // ImPlot3DStyleVar_Plot3DPadding

        // Label style
        {ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlot3DStyle, LabelPadding)},       // ImPlot3DStyleVar_LabelPaddine
        {ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlot3DStyle, LegendPadding)},      // ImPlot3DStyleVar_LegendPadding
        {ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlot3DStyle, LegendInnerPadding)}, // ImPlot3DStyleVar_LegendInnerPadding
        {ImGuiDataType_Float, 2, (ImU32)IM_OFFSETOF(ImPlot3DStyle, LegendSpacing)},      // ImPlot3DStyleVar_LegendSpacing
};

static const ImPlot3DStyleVarInfo* GetPlotStyleVarInfo(ImPlot3DStyleVar idx) {
    IM_ASSERT(idx >= 0 && idx < ImPlot3DStyleVar_COUNT);
    IM_ASSERT(IM_ARRAYSIZE(GPlot3DStyleVarInfo) == ImPlot3DStyleVar_COUNT);
    return &GPlot3DStyleVarInfo[idx];
}

ImPlot3DStyle& GetStyle() { return GImPlot3D->Style; }

void StyleColorsAuto(ImPlot3DStyle* dst) {
    ImPlot3DStyle* style = dst ? dst : &ImPlot3D::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImPlot3DCol_Line] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_MarkerOutline] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_MarkerFill] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_TitleText] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_FrameBg] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_PlotBg] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_PlotBorder] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_LegendBg] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_LegendBorder] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_LegendText] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_AxisText] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_AxisGrid] = IMPLOT3D_AUTO_COL;
}

void StyleColorsDark(ImPlot3DStyle* dst) {
    ImPlot3DStyle* style = dst ? dst : &ImPlot3D::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImPlot3DCol_Line] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_MarkerOutline] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_MarkerFill] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_TitleText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlot3DCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    colors[ImPlot3DCol_PlotBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    colors[ImPlot3DCol_PlotBorder] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImPlot3DCol_LegendBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImPlot3DCol_LegendBorder] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImPlot3DCol_LegendText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlot3DCol_AxisText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlot3DCol_AxisGrid] = ImVec4(1.00f, 1.00f, 1.00f, 0.25f);
}

void StyleColorsLight(ImPlot3DStyle* dst) {
    ImPlot3DStyle* style = dst ? dst : &ImPlot3D::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImPlot3DCol_Line] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_MarkerOutline] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_MarkerFill] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_TitleText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImPlot3DCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlot3DCol_PlotBg] = ImVec4(0.42f, 0.57f, 1.00f, 0.13f);
    colors[ImPlot3DCol_PlotBorder] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImPlot3DCol_LegendBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImPlot3DCol_LegendBorder] = ImVec4(0.82f, 0.82f, 0.82f, 0.80f);
    colors[ImPlot3DCol_LegendText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImPlot3DCol_AxisText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImPlot3DCol_AxisGrid] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
}

void StyleColorsClassic(ImPlot3DStyle* dst) {
    ImPlot3DStyle* style = dst ? dst : &ImPlot3D::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImPlot3DCol_Line] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_MarkerOutline] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_MarkerFill] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_TitleText] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImPlot3DCol_FrameBg] = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    colors[ImPlot3DCol_PlotBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
    colors[ImPlot3DCol_PlotBorder] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImPlot3DCol_LegendBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    colors[ImPlot3DCol_LegendBorder] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImPlot3DCol_LegendText] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImPlot3DCol_AxisText] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImPlot3DCol_AxisGrid] = ImVec4(0.90f, 0.90f, 0.90f, 0.25f);
}

void PushStyleVar(ImPlot3DStyleVar idx, float val) {
    ImPlot3DContext& gp = *GImPlot3D;
    const ImPlot3DStyleVarInfo* var_info = GetPlotStyleVarInfo(idx);
    if (var_info->Type == ImGuiDataType_Float && var_info->Count == 1) {
        float* pvar = (float*)var_info->GetVarPtr(&gp.Style);
        gp.StyleModifiers.push_back(ImGuiStyleMod((ImGuiStyleVar)idx, *pvar));
        *pvar = val;
        return;
    }
    IM_ASSERT(0 && "Called PushStyleVar() float variant but variable is not a float!");
}

void PushStyleVar(ImPlot3DStyleVar idx, int val) {
    ImPlot3DContext& gp = *GImPlot3D;
    const ImPlot3DStyleVarInfo* var_info = GetPlotStyleVarInfo(idx);
    if (var_info->Type == ImGuiDataType_S32 && var_info->Count == 1) {
        int* pvar = (int*)var_info->GetVarPtr(&gp.Style);
        gp.StyleModifiers.push_back(ImGuiStyleMod((ImGuiStyleVar)idx, *pvar));
        *pvar = val;
        return;
    } else if (var_info->Type == ImGuiDataType_Float && var_info->Count == 1) {
        float* pvar = (float*)var_info->GetVarPtr(&gp.Style);
        gp.StyleModifiers.push_back(ImGuiStyleMod((ImGuiStyleVar)idx, *pvar));
        *pvar = (float)val;
        return;
    }
    IM_ASSERT(0 && "Called PushStyleVar() int variant but variable is not a int!");
}

void PushStyleVar(ImPlot3DStyleVar idx, const ImVec2& val) {
    ImPlot3DContext& gp = *GImPlot3D;
    const ImPlot3DStyleVarInfo* var_info = GetPlotStyleVarInfo(idx);
    if (var_info->Type == ImGuiDataType_Float && var_info->Count == 2) {
        ImVec2* pvar = (ImVec2*)var_info->GetVarPtr(&gp.Style);
        gp.StyleModifiers.push_back(ImGuiStyleMod((ImGuiStyleVar)idx, *pvar));
        *pvar = val;
        return;
    }
    IM_ASSERT(0 && "Called PushStyleVar() ImVec2 variant but variable is not a ImVec2!");
}

void PopStyleVar(int count) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(count <= gp.StyleModifiers.Size, "You can't pop more modifiers than have been pushed!");
    while (count > 0) {
        ImGuiStyleMod& backup = gp.StyleModifiers.back();
        const ImPlot3DStyleVarInfo* info = GetPlotStyleVarInfo(backup.VarIdx);
        void* data = info->GetVarPtr(&gp.Style);
        if (info->Type == ImGuiDataType_Float && info->Count == 1) {
            ((float*)data)[0] = backup.BackupFloat[0];
        } else if (info->Type == ImGuiDataType_Float && info->Count == 2) {
            ((float*)data)[0] = backup.BackupFloat[0];
            ((float*)data)[1] = backup.BackupFloat[1];
        } else if (info->Type == ImGuiDataType_S32 && info->Count == 1) {
            ((int*)data)[0] = backup.BackupInt[0];
        }
        gp.StyleModifiers.pop_back();
        count--;
    }
}

ImVec4 GetStyleColorVec4(ImPlot3DCol idx) {
    return IsColorAuto(idx) ? GetAutoColor(idx) : GImPlot3D->Style.Colors[idx];
}

ImU32 GetStyleColorU32(ImPlot3DCol idx) {
    return ImGui::ColorConvertFloat4ToU32(ImPlot3D::GetStyleColorVec4(idx));
}

//------------------------------------------------------------------------------
// [SECTION] Colormaps
//------------------------------------------------------------------------------

ImPlot3DColormap AddColormap(const char* name, const ImVec4* colormap, int size, bool qual) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(size > 1, "The colormap size must be greater than 1!");
    IM_ASSERT_USER_ERROR(gp.ColormapData.GetIndex(name) == -1, "The colormap name has already been used!");
    ImVector<ImU32> buffer;
    buffer.resize(size);
    for (int i = 0; i < size; ++i)
        buffer[i] = ImGui::ColorConvertFloat4ToU32(colormap[i]);
    return gp.ColormapData.Append(name, buffer.Data, size, qual);
}

ImPlot3DColormap AddColormap(const char* name, const ImU32* colormap, int size, bool qual) {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(size > 1, "The colormap size must be greater than 1!");
    IM_ASSERT_USER_ERROR(gp.ColormapData.GetIndex(name) == -1, "The colormap name has already be used!");
    return gp.ColormapData.Append(name, colormap, size, qual);
}

int GetColormapCount() {
    ImPlot3DContext& gp = *GImPlot3D;
    return gp.ColormapData.Count;
}

const char* GetColormapName(ImPlot3DColormap colormap) {
    ImPlot3DContext& gp = *GImPlot3D;
    return gp.ColormapData.GetName(colormap);
}

ImPlot3DColormap GetColormapIndex(const char* name) {
    ImPlot3DContext& gp = *GImPlot3D;
    return gp.ColormapData.GetIndex(name);
}

ImU32 NextColormapColorU32() {
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentItems != nullptr, "NextColormapColor() needs to be called between BeginPlot() and EndPlot()!");
    int idx = gp.CurrentItems->ColormapIdx % gp.ColormapData.GetKeyCount(gp.Style.Colormap);
    ImU32 col = gp.ColormapData.GetKeyColor(gp.Style.Colormap, idx);
    gp.CurrentItems->ColormapIdx++;
    return col;
}

ImVec4 NextColormapColor() {
    return ImGui::ColorConvertU32ToFloat4(NextColormapColorU32());
}

ImU32 GetColormapColorU32(int idx, ImPlot3DColormap cmap) {
    ImPlot3DContext& gp = *GImPlot3D;
    cmap = cmap == IMPLOT3D_AUTO ? gp.Style.Colormap : cmap;
    IM_ASSERT_USER_ERROR(cmap >= 0 && cmap < gp.ColormapData.Count, "Invalid colormap index!");
    idx = idx % gp.ColormapData.GetKeyCount(cmap);
    return gp.ColormapData.GetKeyColor(cmap, idx);
}

ImVec4 GetColormapColor(int idx, ImPlot3DColormap cmap) {
    return ImGui::ColorConvertU32ToFloat4(GetColormapColorU32(idx, cmap));
}

ImU32 SampleColormapU32(float t, ImPlot3DColormap cmap) {
    ImPlot3DContext& gp = *GImPlot3D;
    cmap = cmap == IMPLOT3D_AUTO ? gp.Style.Colormap : cmap;
    IM_ASSERT_USER_ERROR(cmap >= 0 && cmap < gp.ColormapData.Count, "Invalid colormap index!");
    return gp.ColormapData.LerpTable(cmap, t);
}

ImVec4 SampleColormap(float t, ImPlot3DColormap cmap) {
    return ImGui::ColorConvertU32ToFloat4(SampleColormapU32(t, cmap));
}

//-----------------------------------------------------------------------------
// [SECTION] Context Utils
//-----------------------------------------------------------------------------

#define IMPLOT3D_APPEND_CMAP(name, qual) ctx->ColormapData.Append(#name, name, sizeof(name) / sizeof(ImU32), qual)
#define IM_RGB(r, g, b) IM_COL32(r, g, b, 255)

void InitializeContext(ImPlot3DContext* ctx) {
    ResetContext(ctx);

    const ImU32 Deep[] = {4289753676, 4283598045, 4285048917, 4283584196, 4289950337, 4284512403, 4291005402, 4287401100, 4285839820, 4291671396};
    const ImU32 Dark[] = {4280031972, 4290281015, 4283084621, 4288892568, 4278222847, 4281597951, 4280833702, 4290740727, 4288256409};
    const ImU32 Pastel[] = {4289639675, 4293119411, 4291161036, 4293184478, 4289124862, 4291624959, 4290631909, 4293712637, 4294111986};
    const ImU32 Paired[] = {4293119554, 4290017311, 4287291314, 4281114675, 4288256763, 4280031971, 4285513725, 4278222847, 4292260554, 4288298346, 4288282623, 4280834481};
    const ImU32 Viridis[] = {4283695428, 4285867080, 4287054913, 4287455029, 4287526954, 4287402273, 4286883874, 4285579076, 4283552122, 4280737725, 4280674301};
    const ImU32 Plasma[] = {4287039501, 4288480321, 4289200234, 4288941455, 4287638193, 4286072780, 4284638433, 4283139314, 4281771772, 4280667900, 4280416752};
    const ImU32 Hot[] = {4278190144, 4278190208, 4278190271, 4278190335, 4278206719, 4278223103, 4278239231, 4278255615, 4283826175, 4289396735, 4294967295};
    const ImU32 Cool[] = {4294967040, 4294960666, 4294954035, 4294947661, 4294941030, 4294934656, 4294928025, 4294921651, 4294915020, 4294908646, 4294902015};
    const ImU32 Pink[] = {4278190154, 4282532475, 4284308894, 4285690554, 4286879686, 4287870160, 4288794330, 4289651940, 4291685869, 4293392118, 4294967295};
    const ImU32 Jet[] = {4289331200, 4294901760, 4294923520, 4294945280, 4294967040, 4289396565, 4283826090, 4278255615, 4278233855, 4278212095, 4278190335};
    const ImU32 Twilight[] = {IM_RGB(226, 217, 226), IM_RGB(166, 191, 202), IM_RGB(109, 144, 192), IM_RGB(95, 88, 176), IM_RGB(83, 30, 124), IM_RGB(47, 20, 54), IM_RGB(100, 25, 75), IM_RGB(159, 60, 80), IM_RGB(192, 117, 94), IM_RGB(208, 179, 158), IM_RGB(226, 217, 226)};
    const ImU32 RdBu[] = {IM_RGB(103, 0, 31), IM_RGB(178, 24, 43), IM_RGB(214, 96, 77), IM_RGB(244, 165, 130), IM_RGB(253, 219, 199), IM_RGB(247, 247, 247), IM_RGB(209, 229, 240), IM_RGB(146, 197, 222), IM_RGB(67, 147, 195), IM_RGB(33, 102, 172), IM_RGB(5, 48, 97)};
    const ImU32 BrBG[] = {IM_RGB(84, 48, 5), IM_RGB(140, 81, 10), IM_RGB(191, 129, 45), IM_RGB(223, 194, 125), IM_RGB(246, 232, 195), IM_RGB(245, 245, 245), IM_RGB(199, 234, 229), IM_RGB(128, 205, 193), IM_RGB(53, 151, 143), IM_RGB(1, 102, 94), IM_RGB(0, 60, 48)};
    const ImU32 PiYG[] = {IM_RGB(142, 1, 82), IM_RGB(197, 27, 125), IM_RGB(222, 119, 174), IM_RGB(241, 182, 218), IM_RGB(253, 224, 239), IM_RGB(247, 247, 247), IM_RGB(230, 245, 208), IM_RGB(184, 225, 134), IM_RGB(127, 188, 65), IM_RGB(77, 146, 33), IM_RGB(39, 100, 25)};
    const ImU32 Spectral[] = {IM_RGB(158, 1, 66), IM_RGB(213, 62, 79), IM_RGB(244, 109, 67), IM_RGB(253, 174, 97), IM_RGB(254, 224, 139), IM_RGB(255, 255, 191), IM_RGB(230, 245, 152), IM_RGB(171, 221, 164), IM_RGB(102, 194, 165), IM_RGB(50, 136, 189), IM_RGB(94, 79, 162)};
    const ImU32 Greys[] = {IM_COL32_WHITE, IM_COL32_BLACK};

    IMPLOT3D_APPEND_CMAP(Deep, true);
    IMPLOT3D_APPEND_CMAP(Dark, true);
    IMPLOT3D_APPEND_CMAP(Pastel, true);
    IMPLOT3D_APPEND_CMAP(Paired, true);
    IMPLOT3D_APPEND_CMAP(Viridis, false);
    IMPLOT3D_APPEND_CMAP(Plasma, false);
    IMPLOT3D_APPEND_CMAP(Hot, false);
    IMPLOT3D_APPEND_CMAP(Cool, false);
    IMPLOT3D_APPEND_CMAP(Pink, false);
    IMPLOT3D_APPEND_CMAP(Jet, false);
    IMPLOT3D_APPEND_CMAP(Twilight, false);
    IMPLOT3D_APPEND_CMAP(RdBu, false);
    IMPLOT3D_APPEND_CMAP(BrBG, false);
    IMPLOT3D_APPEND_CMAP(PiYG, false);
    IMPLOT3D_APPEND_CMAP(Spectral, false);
    IMPLOT3D_APPEND_CMAP(Greys, false);
}

void ResetContext(ImPlot3DContext* ctx) {
    ctx->Plots.Clear();
    ctx->CurrentPlot = nullptr;
    ctx->CurrentItems = nullptr;
    ctx->NextItemData.Reset();
    ctx->Style = ImPlot3DStyle();
}

//-----------------------------------------------------------------------------
// [SECTION] Style Utils
//-----------------------------------------------------------------------------

bool IsColorAuto(const ImVec4& col) {
    return col.w == -1.0f;
}

bool IsColorAuto(ImPlot3DCol idx) {
    return IsColorAuto(GImPlot3D->Style.Colors[idx]);
}

ImVec4 GetAutoColor(ImPlot3DCol idx) {
    switch (idx) {
        case ImPlot3DCol_Line: return IMPLOT3D_AUTO_COL;          // Plot dependent
        case ImPlot3DCol_MarkerOutline: return IMPLOT3D_AUTO_COL; // Plot dependent
        case ImPlot3DCol_MarkerFill: return IMPLOT3D_AUTO_COL;    // Plot dependent
        case ImPlot3DCol_TitleText: return ImGui::GetStyleColorVec4(ImGuiCol_Text);
        case ImPlot3DCol_FrameBg: return ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        case ImPlot3DCol_PlotBg: return ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        case ImPlot3DCol_PlotBorder: return ImGui::GetStyleColorVec4(ImGuiCol_Border);
        case ImPlot3DCol_LegendBg: return ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
        case ImPlot3DCol_LegendBorder: return ImGui::GetStyleColorVec4(ImGuiCol_Border);
        case ImPlot3DCol_LegendText: return ImGui::GetStyleColorVec4(ImGuiCol_Text);
        case ImPlot3DCol_AxisText: return ImGui::GetStyleColorVec4(ImGuiCol_Text);
        case ImPlot3DCol_AxisGrid: return ImGui::GetStyleColorVec4(ImGuiCol_Text) * ImVec4(1, 1, 1, 0.25f);
        default: return IMPLOT3D_AUTO_COL;
    }
}

const char* GetStyleColorName(ImPlot3DCol idx) {
    static const char* color_names[ImPlot3DCol_COUNT] = {
        "Line",
        "MarkerOutline",
        "MarkerFill",
        "TitleText",
        "FrameBg",
        "PlotBg",
        "PlotBorder",
        "LegendBg",
        "LegendBorder",
        "LegendText",
        "AxisText",
        "AxisGrid",
    };
    return color_names[idx];
}

const ImPlot3DNextItemData& GetItemData() { return GImPlot3D->NextItemData; }

} // namespace ImPlot3D

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DPoint
//-----------------------------------------------------------------------------

ImPlot3DPoint ImPlot3DPoint::operator*(float rhs) const { return ImPlot3DPoint(x * rhs, y * rhs, z * rhs); }
ImPlot3DPoint ImPlot3DPoint::operator/(float rhs) const { return ImPlot3DPoint(x / rhs, y / rhs, z / rhs); }
ImPlot3DPoint ImPlot3DPoint::operator+(const ImPlot3DPoint& rhs) const { return ImPlot3DPoint(x + rhs.x, y + rhs.y, z + rhs.z); }
ImPlot3DPoint ImPlot3DPoint::operator-(const ImPlot3DPoint& rhs) const { return ImPlot3DPoint(x - rhs.x, y - rhs.y, z - rhs.z); }
ImPlot3DPoint ImPlot3DPoint::operator*(const ImPlot3DPoint& rhs) const { return ImPlot3DPoint(x * rhs.x, y * rhs.y, z * rhs.z); }
ImPlot3DPoint ImPlot3DPoint::operator/(const ImPlot3DPoint& rhs) const { return ImPlot3DPoint(x / rhs.x, y / rhs.y, z / rhs.z); }
ImPlot3DPoint ImPlot3DPoint::operator-() const { return ImPlot3DPoint(-x, -y, -z); }

ImPlot3DPoint& ImPlot3DPoint::operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
}
ImPlot3DPoint& ImPlot3DPoint::operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
}
ImPlot3DPoint& ImPlot3DPoint::operator+=(const ImPlot3DPoint& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}
ImPlot3DPoint& ImPlot3DPoint::operator-=(const ImPlot3DPoint& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}
ImPlot3DPoint& ImPlot3DPoint::operator*=(const ImPlot3DPoint& rhs) {
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    return *this;
}
ImPlot3DPoint& ImPlot3DPoint::operator/=(const ImPlot3DPoint& rhs) {
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    return *this;
}

bool ImPlot3DPoint::operator==(const ImPlot3DPoint& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
bool ImPlot3DPoint::operator!=(const ImPlot3DPoint& rhs) const { return !(*this == rhs); }

float ImPlot3DPoint::Dot(const ImPlot3DPoint& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }

ImPlot3DPoint ImPlot3DPoint::Cross(const ImPlot3DPoint& rhs) const {
    return ImPlot3DPoint(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}

float ImPlot3DPoint::Length() const { return std::sqrt(x * x + y * y + z * z); }

void ImPlot3DPoint::Normalize() {
    float l = Length();
    x /= l;
    y /= l;
    z /= l;
}

ImPlot3DPoint ImPlot3DPoint::Normalized() const {
    float l = Length();
    return ImPlot3DPoint(x / l, y / l, z / l);
}

ImPlot3DPoint operator*(float lhs, const ImPlot3DPoint& rhs) {
    return ImPlot3DPoint(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}
//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DBox
//-----------------------------------------------------------------------------

void ImPlot3DBox::Expand(const ImPlot3DPoint& point) {
    Min.x = ImMin(Min.x, point.x);
    Min.y = ImMin(Min.y, point.y);
    Min.z = ImMin(Min.z, point.z);
    Max.x = ImMax(Max.x, point.x);
    Max.y = ImMax(Max.y, point.y);
    Max.z = ImMax(Max.z, point.z);
}

bool ImPlot3DBox::Contains(const ImPlot3DPoint& point) const {
    return (point.x >= Min.x && point.x <= Max.x) &&
           (point.y >= Min.y && point.y <= Max.y) &&
           (point.z >= Min.z && point.z <= Max.z);
}

bool ImPlot3DBox::ClipLineSegment(const ImPlot3DPoint& p0, const ImPlot3DPoint& p1, ImPlot3DPoint& p0_clipped, ImPlot3DPoint& p1_clipped) const {
    // Check if the line segment is completely inside the box
    if (Contains(p0) && Contains(p1)) {
        p0_clipped = p0;
        p1_clipped = p1;
        return true;
    }

    // Perform Liang-Barsky 3D clipping
    double t0 = 0.0;
    double t1 = 1.0;
    ImPlot3DPoint d = p1 - p0;

    // Define the clipping boundaries
    const double xmin = Min.x, xmax = Max.x;
    const double ymin = Min.y, ymax = Max.y;
    const double zmin = Min.z, zmax = Max.z;

    // Lambda function to update t0 and t1
    auto update = [&](double p, double q) -> bool {
        if (p == 0.0) {
            if (q < 0.0)
                return false; // Line is parallel and outside the boundary
            else
                return true; // Line is parallel and inside or coincident with boundary
        }
        double r = q / p;
        if (p < 0.0) {
            if (r > t1)
                return false; // Line is outside
            if (r > t0)
                t0 = r; // Move up t0
        } else {
            if (r < t0)
                return false; // Line is outside
            if (r < t1)
                t1 = r; // Move down t1
        }
        return true;
    };

    // Clip against each boundary
    if (!update(-d.x, p0.x - xmin))
        return false; // Left
    if (!update(d.x, xmax - p0.x))
        return false; // Right
    if (!update(-d.y, p0.y - ymin))
        return false; // Bottom
    if (!update(d.y, ymax - p0.y))
        return false; // Top
    if (!update(-d.z, p0.z - zmin))
        return false; // Near
    if (!update(d.z, zmax - p0.z))
        return false; // Far

    // Compute clipped points
    p0_clipped = p0 + d * t0;
    p1_clipped = p0 + d * t1;

    return true;
}

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DRange
//-----------------------------------------------------------------------------

void ImPlot3DRange::Expand(float value) {
    Min = ImMin(Min, value);
    Max = ImMax(Max, value);
}

bool ImPlot3DRange::Contains(float value) const {
    return value >= Min && value <= Max;
}

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DQuat
//-----------------------------------------------------------------------------

ImPlot3DQuat::ImPlot3DQuat(float _angle, const ImPlot3DPoint& _axis) {
    float half_angle = _angle * 0.5f;
    float s = std::sin(half_angle);
    x = s * _axis.x;
    y = s * _axis.y;
    z = s * _axis.z;
    w = std::cos(half_angle);
}

float ImPlot3DQuat::Length() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

ImPlot3DQuat ImPlot3DQuat::Normalized() const {
    float l = Length();
    return ImPlot3DQuat(x / l, y / l, z / l, w / l);
}

ImPlot3DQuat ImPlot3DQuat::Conjugate() const {
    return ImPlot3DQuat(-x, -y, -z, w);
}

ImPlot3DQuat ImPlot3DQuat::Inverse() const {
    float l_squared = x * x + y * y + z * z + w * w;
    return ImPlot3DQuat(-x / l_squared, -y / l_squared, -z / l_squared, w / l_squared);
}

ImPlot3DQuat ImPlot3DQuat::operator*(const ImPlot3DQuat& rhs) const {
    return ImPlot3DQuat(
        w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
        w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
        w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
        w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
}

ImPlot3DQuat& ImPlot3DQuat::Normalize() {
    float l = Length();
    x /= l;
    y /= l;
    z /= l;
    w /= l;
    return *this;
}

ImPlot3DPoint ImPlot3DQuat::operator*(const ImPlot3DPoint& point) const {
    // Extract vector part of the quaternion
    ImPlot3DPoint qv(x, y, z);

    // Compute the cross products needed for rotation
    ImPlot3DPoint uv = qv.Cross(point); // uv = qv x point
    ImPlot3DPoint uuv = qv.Cross(uv);   // uuv = qv x uv

    // Compute the rotated vector
    return point + (uv * w * 2.0f) + (uuv * 2.0f);
}

bool ImPlot3DQuat::operator==(const ImPlot3DQuat& rhs) const {
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

bool ImPlot3DQuat::operator!=(const ImPlot3DQuat& rhs) const {
    return !(*this == rhs);
}

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DAxis
//-----------------------------------------------------------------------------

bool ImPlot3DAxis::HasLabel() const {
    return LabelOffset != -1 && !ImHasFlag(Flags, ImPlot3DAxisFlags_NoLabel);
}

void ImPlot3DAxis::ExtendFit(float value) {
    FitExtents.Min = ImMin(FitExtents.Min, value);
    FitExtents.Max = ImMax(FitExtents.Max, value);
}

void ImPlot3DAxis::ApplyFit() {
    if (!IsLockedMin() && !ImNanOrInf(FitExtents.Min))
        Range.Min = FitExtents.Min;
    if (!IsLockedMax() && !ImNanOrInf(FitExtents.Max))
        Range.Max = FitExtents.Max;
    if (ImAlmostEqual(Range.Min, Range.Max)) {
        Range.Max += 0.5;
        Range.Min -= 0.5;
    }
    FitExtents.Min = HUGE_VAL;
    FitExtents.Max = -HUGE_VAL;
}

float ImPlot3DAxis::PlotToNDC(float value) const {
    return (value - Range.Min) / (Range.Max - Range.Min) - 0.5f;
}

float ImPlot3DAxis::NDCToPlot(float value) const {
    return Range.Min + (value + 0.5f) * (Range.Max - Range.Min);
}

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DPlot
//-----------------------------------------------------------------------------

void ImPlot3DPlot::ExtendFit(const ImPlot3DPoint& point) {
    for (int i = 0; i < 3; i++) {
        if (!ImNanOrInf(point[i]) && Axes[i].FitThisFrame)
            Axes[i].ExtendFit(point[i]);
    }
}

ImPlot3DPoint ImPlot3DPlot::RangeMin() const {
    return ImPlot3DPoint(Axes[0].Range.Min, Axes[1].Range.Min, Axes[2].Range.Min);
}

ImPlot3DPoint ImPlot3DPlot::RangeMax() const {
    return ImPlot3DPoint(Axes[0].Range.Max, Axes[1].Range.Max, Axes[2].Range.Max);
}

ImPlot3DPoint ImPlot3DPlot::RangeCenter() const {
    return ImPlot3DPoint(
        (Axes[0].Range.Min + Axes[0].Range.Max) * 0.5f,
        (Axes[1].Range.Min + Axes[1].Range.Max) * 0.5f,
        (Axes[2].Range.Min + Axes[2].Range.Max) * 0.5f);
}

void ImPlot3DPlot::SetRange(const ImPlot3DPoint& min, const ImPlot3DPoint& max) {
    Axes[0].SetRange(min.x, max.x);
    Axes[1].SetRange(min.y, max.y);
    Axes[2].SetRange(min.z, max.z);
}

void ImPlot3DPlot::SetAxisLabel(ImPlot3DAxis& axis, const char* label) {
    if (label && ImGui::FindRenderedTextEnd(label, nullptr) != label) {
        axis.LabelOffset = TextBuffer.size();
        TextBuffer.append(label, label + strlen(label) + 1);
    } else {
        axis.LabelOffset = -1;
    }
}

const char* ImPlot3DPlot::GetAxisLabel(const ImPlot3DAxis& axis) const { return TextBuffer.Buf.Data + axis.LabelOffset; }

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DStyle
//-----------------------------------------------------------------------------

ImPlot3DStyle::ImPlot3DStyle() {
    // Item style
    LineWeight = 1.0f;
    Marker = ImPlot3DMarker_None;
    MarkerSize = 4.0f;
    MarkerWeight = 1.0f;
    FillAlpha = 1.0f;
    // Plot style
    PlotDefaultSize = ImVec2(400, 400);
    PlotMinSize = ImVec2(200, 200);
    PlotPadding = ImVec2(10, 10);
    LabelPadding = ImVec2(5, 5);
    // Legend style
    LegendPadding = ImVec2(10, 10);
    LegendInnerPadding = ImVec2(5, 5);
    LegendSpacing = ImVec2(5, 0);
    // Colors
    ImPlot3D::StyleColorsAuto(this);
    Colormap = ImPlot3DColormap_Deep;
};

#endif // #ifndef IMGUI_DISABLE
