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
// [SECTION] ImVec3
// [SECTION] ImQuat

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

// Global ImPlot3D context
#ifndef GImPlot3D
ImPlot3DContext* GImPlot3D = nullptr;
#endif

ImPlot3DContext* ImPlot3D::CreateContext() {
    ImPlot3DContext* ctx = IM_NEW(ImPlot3DContext)();
    if (GImPlot3D == nullptr)
        SetCurrentContext(ctx);
    InitializeContext(ctx);
    return ctx;
}

void ImPlot3D::DestroyContext(ImPlot3DContext* ctx) {
    if (ctx == nullptr)
        ctx = GImPlot3D;
    if (GImPlot3D == ctx)
        SetCurrentContext(nullptr);
    IM_DELETE(ctx);
}

ImPlot3DContext* ImPlot3D::GetCurrentContext() { return GImPlot3D; }

void ImPlot3D::SetCurrentContext(ImPlot3DContext* ctx) { GImPlot3D = ctx; }

//-----------------------------------------------------------------------------
// [SECTION] Begin/End Plot
//-----------------------------------------------------------------------------
bool ImPlot3D::BeginPlot(const char* title_id, const ImVec2& size, ImPlot3DFlags flags) {
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
    gp.CurrentPlot = gp.Plots.GetOrAddByKey(ID);
    ImPlot3DPlot& plot = *gp.CurrentPlot;

    // Populate plot ID/flags
    plot.ID = ID;
    plot.Flags = flags;

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

void DrawAxes(ImDrawList* draw_list, ImRect plot_area) {
    float zoom = std::min(plot_area.GetWidth(), plot_area.GetHeight());
    ImVec2 center = plot_area.GetCenter();
    // AddQuad(const ImVec2& p1, const ImVec2& p2, const ImVec2& p3, const ImVec2& p4, ImU32 col, float thickness = 1.0f)
}

void ImPlot3D::EndPlot() {
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

    // Plot title
    if (!plot.TextBuffer.empty()) {
        ImU32 col = GetStyleColorU32(ImPlot3DCol_TitleText);
        ImVec2 top_center = ImVec2(plot.FrameRect.GetCenter().x, plot.CanvasRect.Min.y);
        AddTextCentered(draw_list, top_center, col, plot.TextBuffer.c_str());
        plot.PlotRect.Min.y += ImGui::GetTextLineHeight() + gp.Style.LabelPadding.y;
    }

    // Draw plot background
    ImU32 p_bg_color = GetStyleColorU32(ImPlot3DCol_PlotBg);
    ImU32 p_b_color = GetStyleColorU32(ImPlot3DCol_PlotBorder);
    draw_list->AddRectFilled(plot.PlotRect.Min, plot.PlotRect.Max, p_bg_color);
    draw_list->AddRect(plot.PlotRect.Min, plot.PlotRect.Max, p_b_color);

    ImGui::PopClipRect();

    // Reset current plot
    gp.CurrentPlot = nullptr;
}

//-----------------------------------------------------------------------------
// [SECTION] Styles
//-----------------------------------------------------------------------------

ImPlot3DStyle& ImPlot3D::GetStyle() { return GImPlot3D->Style; }

void ImPlot3D::StyleColorsAuto(ImPlot3DStyle* dst) {
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

void ImPlot3D::StyleColorsClassic(ImPlot3DStyle* dst) {
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

void ImPlot3D::StyleColorsDark(ImPlot3DStyle* dst) {
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

void ImPlot3D::StyleColorsLight(ImPlot3DStyle* dst) {
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

ImVec4 ImPlot3D::GetStyleColorVec4(ImPlot3DCol idx) {
    return IsColorAuto(idx) ? GetAutoColor(idx) : GImPlot3D->Style.Colors[idx];
}

ImU32 ImPlot3D::GetStyleColorU32(ImPlot3DCol idx) {
    return ImGui::ColorConvertFloat4ToU32(GetStyleColorVec4(idx));
}

ImPlot3DStyle::ImPlot3DStyle() {
    PlotDefaultSize = ImVec2(400, 400);
    PlotMinSize = ImVec2(200, 200);
    PlotPadding = ImVec2(10, 10);
    LabelPadding = ImVec2(5, 5);
    ImPlot3D::StyleColorsAuto(this);
};

//-----------------------------------------------------------------------------
// [SECTION] Context Utils
//-----------------------------------------------------------------------------

void ImPlot3D::InitializeContext(ImPlot3DContext* ctx) { ResetContext(ctx); }

void ImPlot3D::ResetContext(ImPlot3DContext* ctx) { ctx->CurrentPlot = nullptr; }

//-----------------------------------------------------------------------------
// [SECTION] Style Utils
//-----------------------------------------------------------------------------

bool ImPlot3D::IsColorAuto(const ImVec4& col) {
    return col.w == -1.0f;
}

bool ImPlot3D::IsColorAuto(ImPlot3DCol idx) {
    return IsColorAuto(GImPlot3D->Style.Colors[idx]);
}

ImVec4 ImPlot3D::GetAutoColor(ImPlot3DCol idx) {
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

const char* ImPlot3D::GetStyleColorName(ImPlot3DCol idx) {
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

//-----------------------------------------------------------------------------
// [SECTION] ImVec3
//-----------------------------------------------------------------------------

namespace ImPlot3D {

ImVec3 ImVec3::operator*(float rhs) const { return ImVec3(x * rhs, y * rhs, z * rhs); }
ImVec3 ImVec3::operator/(float rhs) const { return ImVec3(x / rhs, y / rhs, z / rhs); }
ImVec3 ImVec3::operator+(const ImVec3& rhs) const { return ImVec3(x + rhs.x, y + rhs.y, z + rhs.z); }
ImVec3 ImVec3::operator-(const ImVec3& rhs) const { return ImVec3(x - rhs.x, y - rhs.y, z - rhs.z); }
ImVec3 ImVec3::operator*(const ImVec3& rhs) const { return ImVec3(x * rhs.x, y * rhs.y, z * rhs.z); }
ImVec3 ImVec3::operator/(const ImVec3& rhs) const { return ImVec3(x / rhs.x, y / rhs.y, z / rhs.z); }
ImVec3 ImVec3::operator-() const { return ImVec3(-x, -y, -z); }

ImVec3& ImVec3::operator*=(float rhs) {
    x *= rhs;
    y *= rhs;
    z *= rhs;
    return *this;
}
ImVec3& ImVec3::operator/=(float rhs) {
    x /= rhs;
    y /= rhs;
    z /= rhs;
    return *this;
}
ImVec3& ImVec3::operator+=(const ImVec3& rhs) {
    x += rhs.x;
    y += rhs.y;
    z += rhs.z;
    return *this;
}
ImVec3& ImVec3::operator-=(const ImVec3& rhs) {
    x -= rhs.x;
    y -= rhs.y;
    z -= rhs.z;
    return *this;
}
ImVec3& ImVec3::operator*=(const ImVec3& rhs) {
    x *= rhs.x;
    y *= rhs.y;
    z *= rhs.z;
    return *this;
}
ImVec3& ImVec3::operator/=(const ImVec3& rhs) {
    x /= rhs.x;
    y /= rhs.y;
    z /= rhs.z;
    return *this;
}

bool ImVec3::operator==(const ImVec3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
bool ImVec3::operator!=(const ImVec3& rhs) const { return !(*this == rhs); }

float ImVec3::Dot(const ImVec3& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }

ImVec3 ImVec3::Cross(const ImVec3& rhs) const {
    return ImVec3(y * rhs.z - z * rhs.y, z * rhs.x - x * rhs.z, x * rhs.y - y * rhs.x);
}

float ImVec3::Magnitude() const { return std::sqrt(x * x + y * y + z * z); }

ImVec3 operator*(float lhs, const ImVec3& rhs) {
    return ImVec3(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}

//-----------------------------------------------------------------------------
// [SECTION] ImQuat
//-----------------------------------------------------------------------------

constexpr ImQuat::ImQuat() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
constexpr ImQuat::ImQuat(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}

ImQuat::ImQuat(float _angle, const ImVec3& _axis) {
    float half_angle = _angle * 0.5f;
    float s = std::sin(half_angle);
    x = s * _axis.x;
    y = s * _axis.y;
    z = s * _axis.z;
    w = std::cos(half_angle);
}

float ImQuat::Magnitude() const {
    return std::sqrt(x * x + y * y + z * z + w * w);
}

ImQuat ImQuat::Normalized() const {
    float mag = Magnitude();
    return ImQuat(x / mag, y / mag, z / mag, w / mag);
}

ImQuat ImQuat::Conjugate() const {
    return ImQuat(-x, -y, -z, w);
}

ImQuat ImQuat::Inverse() const {
    float mag_squared = x * x + y * y + z * z + w * w;
    return ImQuat(-x / mag_squared, -y / mag_squared, -z / mag_squared, w / mag_squared);
}

ImQuat ImQuat::operator*(const ImQuat& rhs) const {
    return ImQuat(
        w * rhs.x + x * rhs.w + y * rhs.z - z * rhs.y,
        w * rhs.y - x * rhs.z + y * rhs.w + z * rhs.x,
        w * rhs.z + x * rhs.y - y * rhs.x + z * rhs.w,
        w * rhs.w - x * rhs.x - y * rhs.y - z * rhs.z);
}

ImQuat& ImQuat::Normalize() {
    float mag = Magnitude();
    x /= mag;
    y /= mag;
    z /= mag;
    w /= mag;
    return *this;
}

ImVec3 ImQuat::operator*(const ImVec3& point) const {
    // Extract vector part of the quaternion
    ImVec3 qv(x, y, z);

    // Compute the cross products needed for rotation
    ImVec3 uv = qv.Cross(point); // uv = qv x point
    ImVec3 uuv = qv.Cross(uv);   // uuv = qv x uv

    // Compute the rotated vector
    return point + (uv * w * 2.0f) + (uuv * 2.0f);
}

bool ImQuat::operator==(const ImQuat& rhs) const {
    return x == rhs.x && y == rhs.y && z == rhs.z && w == rhs.w;
}

bool ImQuat::operator!=(const ImQuat& rhs) const {
    return !(*this == rhs);
}

ImQuat operator*(float lhs, const ImQuat& rhs) {
    return ImQuat(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z, lhs * rhs.w);
}

} // namespace ImPlot3D

#endif // #ifndef IMGUI_DISABLE
