//--------------------------------------------------
// ImPlot3D v0.1
// implot3d.cpp
// Date: 2024-11-16
// By brenocq
//--------------------------------------------------

// Table of Contents:
// [SECTION] Includes
// [SECTION] Macros
// [SECTION] Context
// [SECTION] Begin/End Plot
// [SECTION] Styles
// [SECTION] Context Utils
// [SECTION] Style Utils
// [SECTION] ImPlot3DVec3
// [SECTION] ImPlot3DQuat
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
    ImPlot3DPlot& plot = *gp.CurrentPlot;

    // Populate plot ID/flags
    plot.ID = ID;
    plot.Flags = flags;
    if (just_created) {
        plot.Rotation = init_rotation;
        plot.RangeMin = ImPlot3DVec3(0.0f, 0.0f, 0.0f);
        plot.RangeMax = ImPlot3DVec3(1.0f, 1.0f, 1.0f);
    }

    // Populate title
    if (title_id && ImGui::FindRenderedTextEnd(title_id, nullptr) != title_id && !(plot.Flags & ImPlot3DFlags_NoTitle))
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
        return false;
    }

    return true;
}

void AddTextCentered(ImDrawList* draw_list, ImVec2 top_center, ImU32 col, const char* text_begin) {
    const char* text_end = ImGui::FindRenderedTextEnd(text_begin);
    ImVec2 text_size = ImGui::CalcTextSize(text_begin, text_end, true);
    draw_list->AddText(ImVec2(top_center.x - text_size.x * 0.5f, top_center.y), col, text_begin, text_end);
}

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

    if (plot_clicked && ImGui::IsMouseDoubleClicked(0))
        plot.Rotation = init_rotation;
    if (plot.Held && ImGui::IsMouseDown(0)) {
        ImVec2 delta(IO.MouseDelta.x, IO.MouseDelta.y);

        // Map delta to rotation angles (in radians)
        float angle_x = delta.y * (3.1415f / 180.0f);  // Vertical movement -> rotation around X-axis
        float angle_y = -delta.x * (3.1415f / 180.0f); // Horizontal movement -> rotation around Y-axis

        // Create quaternions for the rotations
        ImPlot3DQuat quat_x(angle_x, ImPlot3DVec3(1.0f, 0.0f, 0.0f));
        ImPlot3DQuat quat_y(angle_y, ImPlot3DVec3(0.0f, 1.0f, 0.0f));

        // Combine the new rotations with the current rotation
        plot.Rotation = quat_x * quat_y * plot.Rotation;
        plot.Rotation.Normalize();
    }
}

void DrawAxes(ImDrawList* draw_list, const ImRect& plot_area, const ImPlot3DQuat& rotation, const ImPlot3DVec3& range_min, const ImPlot3DVec3& range_max) {
    float zoom = std::min(plot_area.GetWidth(), -plot_area.GetHeight()) / 1.8f;
    ImVec2 center = plot_area.GetCenter();
    ImPlot3DVec3 plane_normal[3] = {
        rotation * ImPlot3DVec3(1.0f, 0.0f, 0.0f),
        rotation * ImPlot3DVec3(0.0f, 1.0f, 0.0f),
        rotation * ImPlot3DVec3(0.0f, 0.0f, 1.0f)};
    ImPlot3DVec3 plane[3][4] = {
        {ImPlot3DVec3(0.5f, -0.5f, -0.5f), ImPlot3DVec3(0.5f, -0.5f, 0.5f), ImPlot3DVec3(0.5f, 0.5f, 0.5f), ImPlot3DVec3(0.5f, 0.5f, -0.5f)},
        {ImPlot3DVec3(-0.5f, 0.5f, -0.5f), ImPlot3DVec3(-0.5f, 0.5f, 0.5f), ImPlot3DVec3(0.5f, 0.5f, 0.5f), ImPlot3DVec3(0.5f, 0.5f, -0.5f)},
        {ImPlot3DVec3(-0.5f, -0.5f, 0.5f), ImPlot3DVec3(-0.5f, 0.5f, 0.5f), ImPlot3DVec3(0.5f, 0.5f, 0.5f), ImPlot3DVec3(0.5f, -0.5f, 0.5f)}};

    // Transform planes
    for (int c = 0; c < 3; c++) {
        const float sign = -plane_normal[c][2] > 0.0f ? 1.0f : -1.0f; // Dot product between plane normal and view vector
        for (int i = 0; i < 4; i++)
            plane[c][i] = zoom * (rotation * (sign * plane[c][i])) + ImPlot3DVec3(center.x, center.y, 0.0f);
    }

    // Draw background
    const ImU32 colBg = GetStyleColorU32(ImPlot3DCol_PlotBg);
    for (int c = 0; c < 3; c++) {
        // const ImU32 colBg = ImGui::ColorConvertFloat4ToU32(ImVec4(c == 0, c == 1, c == 2, 0.5f)); // XXX
        draw_list->AddQuadFilled(ImVec2(plane[c][0].x, plane[c][0].y), ImVec2(plane[c][1].x, plane[c][1].y), ImVec2(plane[c][2].x, plane[c][2].y), ImVec2(plane[c][3].x, plane[c][3].y), colBg);
    }
    // Draw border
    const ImU32 colBorder = GetStyleColorU32(ImPlot3DCol_PlotBorder);
    for (int c = 0; c < 3; c++)
        for (int i = 0; i < 4; i++)
            draw_list->AddLine(ImVec2(plane[c][i].x, plane[c][i].y), ImVec2(plane[c][(i + 1) % 4].x, plane[c][(i + 1) % 4].y), colBorder);

    // Draw ticks
    const float target_lines = 10.0f; // Target number of tick lines
    const ImU32 colTicks = GetStyleColorU32(ImPlot3DCol_PlotBorder);
    for (int c = 0; c < 3; c++) {
        float range = range_max[c] - range_min[c];

        // Estimate initial spacing (powers of 10 for zoom-level adaptation)
        float spacing = std::pow(10.0f, std::floor(std::log10(range / target_lines)));

        float start = std::floor(range_min[c] / spacing) * spacing;
        float end = std::ceil(range_max[c] / spacing) * spacing;
        for (float t = start; t <= end; t += spacing) {
            if (t < range_min[c] || t > range_max[c])
                continue;
            float tNorm = (t - range_min[c]) / range;
            // Draw ticks for the other 2 planes
            for (size_t i = 0; i < 2; i++) {
                size_t planeIdx = (c + i + 1) % 3;
                ImPlot3DVec3 p0, p1;
                if (c == 0 || (c == 1 && i == 1)) {
                    p0 = plane[planeIdx][0] + (plane[planeIdx][3] - plane[planeIdx][0]) * tNorm;
                    p1 = plane[planeIdx][1] + (plane[planeIdx][2] - plane[planeIdx][1]) * tNorm;
                } else if (c == 2 || (c == 1 && i == 0)) {
                    p0 = plane[planeIdx][0] + (plane[planeIdx][1] - plane[planeIdx][0]) * tNorm;
                    p1 = plane[planeIdx][3] + (plane[planeIdx][2] - plane[planeIdx][3]) * tNorm;
                }
                // const ImU32 colTicks = ImGui::ColorConvertFloat4ToU32(ImVec4(c == 0, c == 1, c == 2, 1.0f)); // XXX
                draw_list->AddLine(ImVec2(p0.x, p0.y), ImVec2(p1.x, p1.y), colTicks);
            }
        }
    }
}

void EndPlot() {
    IMPLOT3D_CHECK_CTX();
    ImPlot3DContext& gp = *GImPlot3D;
    IM_ASSERT_USER_ERROR(gp.CurrentPlot != nullptr, "Mismatched BeginPlot()/EndPlot()!");

    ImGuiContext& g = *GImGui;
    ImPlot3DPlot& plot = *gp.CurrentPlot;
    ImGuiWindow* window = g.CurrentWindow;
    ImDrawList* draw_list = window->DrawList;

    ImGui::PushClipRect(plot.FrameRect.Min, plot.FrameRect.Max, true);

    // Draw frame background
    ImU32 f_bg_color = GetStyleColorU32(ImPlot3DCol_FrameBg);
    draw_list->AddRectFilled(plot.FrameRect.Min, plot.FrameRect.Max, f_bg_color);

    // Compute canvas/canvas rectangle
    plot.CanvasRect = ImRect(plot.FrameRect.Min + gp.Style.PlotPadding, plot.FrameRect.Max - gp.Style.PlotPadding);
    plot.PlotRect = plot.CanvasRect;

    // Handle user input
    HandleInput(plot);

    // Plot axes
    DrawAxes(draw_list, plot.PlotRect, plot.Rotation, plot.RangeMin, plot.RangeMax);

    // Plot title
    if (!plot.TextBuffer.empty()) {
        ImU32 col = GetStyleColorU32(ImPlot3DCol_TitleText);
        ImVec2 top_center = ImVec2(plot.FrameRect.GetCenter().x, plot.CanvasRect.Min.y);
        AddTextCentered(draw_list, top_center, col, plot.TextBuffer.c_str());
        plot.PlotRect.Min.y += ImGui::GetTextLineHeight() + gp.Style.LabelPadding.y;
    }

    ImGui::PopClipRect();

    // Reset current plot
    gp.CurrentPlot = nullptr;
}

//-----------------------------------------------------------------------------
// [SECTION] Styles
//-----------------------------------------------------------------------------

ImPlot3DStyle& GetStyle() { return GImPlot3D->Style; }

void StyleColorsAuto(ImPlot3DStyle* dst) {
    ImPlot3DStyle* style = dst ? dst : &ImPlot3D::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImPlot3DCol_FrameBg] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_PlotBg] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_PlotBorder] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_LegendBg] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_LegendBorder] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_LegendText] = IMPLOT3D_AUTO_COL;
    colors[ImPlot3DCol_TitleText] = IMPLOT3D_AUTO_COL;
}

void StyleColorsClassic(ImPlot3DStyle* dst) {
    ImPlot3DStyle* style = dst ? dst : &ImPlot3D::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImPlot3DCol_FrameBg] = ImVec4(0.43f, 0.43f, 0.43f, 0.39f);
    colors[ImPlot3DCol_PlotBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
    colors[ImPlot3DCol_PlotBorder] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImPlot3DCol_LegendBg] = ImVec4(0.11f, 0.11f, 0.14f, 0.92f);
    colors[ImPlot3DCol_LegendBorder] = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    colors[ImPlot3DCol_LegendText] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
    colors[ImPlot3DCol_TitleText] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
}

void StyleColorsDark(ImPlot3DStyle* dst) {
    ImPlot3DStyle* style = dst ? dst : &ImPlot3D::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImPlot3DCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.07f);
    colors[ImPlot3DCol_PlotBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    colors[ImPlot3DCol_PlotBorder] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImPlot3DCol_LegendBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
    colors[ImPlot3DCol_LegendBorder] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
    colors[ImPlot3DCol_LegendText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlot3DCol_TitleText] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
}

void StyleColorsLight(ImPlot3DStyle* dst) {
    ImPlot3DStyle* style = dst ? dst : &ImPlot3D::GetStyle();
    ImVec4* colors = style->Colors;

    colors[ImPlot3DCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    colors[ImPlot3DCol_PlotBg] = ImVec4(0.42f, 0.57f, 1.00f, 0.13f);
    colors[ImPlot3DCol_PlotBorder] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
    colors[ImPlot3DCol_LegendBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.98f);
    colors[ImPlot3DCol_LegendBorder] = ImVec4(0.82f, 0.82f, 0.82f, 0.80f);
    colors[ImPlot3DCol_LegendText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImPlot3DCol_TitleText] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
}

ImVec4 GetStyleColorVec4(ImPlot3DCol idx) {
    return IsColorAuto(idx) ? GetAutoColor(idx) : GImPlot3D->Style.Colors[idx];
}

ImU32 GetStyleColorU32(ImPlot3DCol idx) {
    return ImGui::ColorConvertFloat4ToU32(ImPlot3D::GetStyleColorVec4(idx));
}

//-----------------------------------------------------------------------------
// [SECTION] Context Utils
//-----------------------------------------------------------------------------

void InitializeContext(ImPlot3DContext* ctx) { ResetContext(ctx); }

void ResetContext(ImPlot3DContext* ctx) { ctx->CurrentPlot = nullptr; }

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
    ImVec4 col(0, 0, 0, 1);
    switch (idx) {
        // case ImPlot3DCol_Line:          return col; // Plot dependent
        // case ImPlot3DCol_Fill:          return col; // Plot dependent
        // case ImPlot3DCol_MarkerOutline: return col; // Plot dependent
        // case ImPlot3DCol_MarkerFill:    return col; // Plot dependent
        case ImPlot3DCol_FrameBg: return ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        case ImPlot3DCol_PlotBg: return ImGui::GetStyleColorVec4(ImGuiCol_WindowBg);
        case ImPlot3DCol_PlotBorder: return ImGui::GetStyleColorVec4(ImGuiCol_Border);
        case ImPlot3DCol_LegendBg: return ImGui::GetStyleColorVec4(ImGuiCol_PopupBg);
        case ImPlot3DCol_LegendBorder: return GetStyleColorVec4(ImPlot3DCol_PlotBorder);
        case ImPlot3DCol_LegendText: return GetStyleColorVec4(ImPlot3DCol_TitleText); // TODO Change to inlay text
        case ImPlot3DCol_TitleText: return ImGui::GetStyleColorVec4(ImGuiCol_Text);
        default: return col;
    }
}

const char* GetStyleColorName(ImPlot3DCol idx) {
    static const char* color_names[ImPlot3DCol_COUNT] = {
        "TitleText",
        "FrameBg",
        "PlotBg",
        "PlotBorder",
        "LegendBg",
        "LegendBorder",
        "LegendText",
    };
    return color_names[idx];
}

} // namespace ImPlot3D

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DVec3
//-----------------------------------------------------------------------------

ImPlot3DVec3 ImPlot3DVec3::operator*(float rhs) const { return ImPlot3DVec3(x * rhs, y * rhs, z * rhs); }
ImPlot3DVec3 ImPlot3DVec3::operator/(float rhs) const { return ImPlot3DVec3(x / rhs, y / rhs, z / rhs); }
ImPlot3DVec3 ImPlot3DVec3::operator+(const ImPlot3DVec3& rhs) const { return ImPlot3DVec3(x + rhs.x, y + rhs.y, z + rhs.z); }
ImPlot3DVec3 ImPlot3DVec3::operator-(const ImPlot3DVec3& rhs) const { return ImPlot3DVec3(x - rhs.x, y - rhs.y, z - rhs.z); }
ImPlot3DVec3 ImPlot3DVec3::operator*(const ImPlot3DVec3& rhs) const { return ImPlot3DVec3(x * rhs.x, y * rhs.y, z * rhs.z); }
ImPlot3DVec3 ImPlot3DVec3::operator/(const ImPlot3DVec3& rhs) const { return ImPlot3DVec3(x / rhs.x, y / rhs.y, z / rhs.z); }
ImPlot3DVec3 ImPlot3DVec3::operator-() const { return ImPlot3DVec3(-x, -y, -z); }

ImPlot3DVec3& ImPlot3DVec3::operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
}
ImPlot3DVec3& ImPlot3DVec3::operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
}
ImPlot3DVec3& ImPlot3DVec3::operator+=(const ImPlot3DVec3& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}
ImPlot3DVec3& ImPlot3DVec3::operator-=(const ImPlot3DVec3& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}
ImPlot3DVec3& ImPlot3DVec3::operator*=(const ImPlot3DVec3& rhs) {
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    return *this;
}
ImPlot3DVec3& ImPlot3DVec3::operator/=(const ImPlot3DVec3& rhs) {
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    return *this;
}

bool ImPlot3DVec3::operator==(const ImPlot3DVec3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
bool ImPlot3DVec3::operator!=(const ImPlot3DVec3& rhs) const { return !(*this == rhs); }

float ImPlot3DVec3::Dot(const ImPlot3DVec3& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }

ImPlot3DVec3 ImPlot3DVec3::Cross(const ImPlot3DVec3& rhs) const {
    return ImPlot3DVec3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}

float ImPlot3DVec3::Magnitude() const { return std::sqrt(x * x + y * y + z * z); }

ImPlot3DVec3 operator*(float lhs, const ImPlot3DVec3& rhs) {
    return ImPlot3DVec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

//-----------------------------------------------------------------------------
// [SECTION] ImPlot3DQuat
//-----------------------------------------------------------------------------

constexpr ImPlot3DQuat::ImPlot3DQuat() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
constexpr ImPlot3DQuat::ImPlot3DQuat(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

ImPlot3DQuat::ImPlot3DQuat(float _angle, const ImPlot3DVec3& _axis) {
    float half_angle = _angle * 0.5f;
    float s = std::sin(half_angle);
    x = s * _axis.x;
    y = s * _axis.y;
    z = s * _axis.z;
    w = std::cos(half_angle);
}

float ImPlot3DQuat::Magnitude() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

ImPlot3DQuat ImPlot3DQuat::Normalized() const {
    float mag = Magnitude();
    return ImPlot3DQuat(x / mag, y / mag, z / mag, w / mag);
}

ImPlot3DQuat ImPlot3DQuat::Conjugate() const {
    return ImPlot3DQuat(-x, -y, -z, w);
}

ImPlot3DQuat ImPlot3DQuat::Inverse() const {
    float mag_squared = x * x + y * y + z * z + w * w;
    return ImPlot3DQuat(-x / mag_squared, -y / mag_squared, -z / mag_squared, w / mag_squared);
}

ImPlot3DQuat ImPlot3DQuat::operator*(const ImPlot3DQuat& rhs) const {
    return ImPlot3DQuat(
        w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
        w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
        w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
        w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
}

ImPlot3DQuat& ImPlot3DQuat::Normalize() {
    float mag = Magnitude();
    x /= mag;
    y /= mag;
    z /= mag;
    w /= mag;
    return *this;
}

ImPlot3DVec3 ImPlot3DQuat::operator*(const ImPlot3DVec3& point) const {
    // Extract vector part of the quaternion
    ImPlot3DVec3 qv(x, y, z);

    // Compute the cross products needed for rotation
    ImPlot3DVec3 uv = qv.Cross(point); // uv = qv x point
    ImPlot3DVec3 uuv = qv.Cross(uv);   // uuv = qv x uv

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
// [SECTION] ImPlot3DStyle
//-----------------------------------------------------------------------------

ImPlot3DStyle::ImPlot3DStyle() {
    PlotDefaultSize = ImVec2(400, 400);
    PlotMinSize = ImVec2(200, 200);
    PlotPadding = ImVec2(10, 10);
    LabelPadding = ImVec2(5, 5);
    ImPlot3D::StyleColorsAuto(this);
};

#endif // #ifndef IMGUI_DISABLE
