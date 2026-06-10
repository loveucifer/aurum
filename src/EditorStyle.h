#ifndef EDITORSTYLE_H
#define EDITORSTYLE_H

#include "imgui.h"
#include <algorithm> // for std::swap

// Draw a sunken or raised bevel border using the ImDrawList API
inline void DrawBevelRect(ImDrawList* dl, ImVec2 min, ImVec2 max, bool raised, float thickness = 2.0f) {
    ImU32 light = IM_COL32(255, 255, 255, 255);  // Highlight: White top-left
    ImU32 dark  = IM_COL32(128, 128, 128, 255);  // Shadow: Dark gray bottom-right

    if (!raised) {
        ImU32 temp = light;
        light = dark;
        dark = temp;
    }

    // Top edge
    dl->AddRectFilled(ImVec2(min.x, min.y), ImVec2(max.x, min.y + thickness), light);
    // Left edge
    dl->AddRectFilled(ImVec2(min.x, min.y), ImVec2(min.x + thickness, max.y), light);
    // Bottom edge
    dl->AddRectFilled(ImVec2(min.x, max.y - thickness), ImVec2(max.x, max.y), dark);
    // Right edge
    dl->AddRectFilled(ImVec2(max.x - thickness, min.y), ImVec2(max.x, max.y), dark);
}

// Retro style setup based on classic Windows 95/98 light gray look
inline void ApplyRetroStyle() {
    ImGuiStyle& style = ImGui::GetStyle();

    // GEOMETRY - everything is hard rectangles, tight spacing
    style.WindowRounding    = 0.0f;
    style.ChildRounding     = 0.0f;
    style.FrameRounding     = 0.0f;
    style.PopupRounding     = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding      = 0.0f;
    style.TabRounding       = 0.0f;

    style.WindowBorderSize  = 2.0f;   // thick outer border for the bevel effect
    style.ChildBorderSize   = 1.0f;
    style.FrameBorderSize   = 1.0f;   // input fields, buttons get a border
    style.PopupBorderSize   = 1.0f;
    style.TabBorderSize     = 1.0f;

    // Dense packing
    style.WindowPadding     = ImVec2(4.0f, 4.0f);
    style.FramePadding      = ImVec2(4.0f, 2.0f);
    style.CellPadding       = ImVec2(4.0f, 2.0f);
    style.ItemSpacing       = ImVec2(4.0f, 2.0f);
    style.ItemInnerSpacing  = ImVec2(4.0f, 2.0f);
    style.IndentSpacing     = 12.0f;

    // Scrollbar - chunky
    style.ScrollbarSize     = 14.0f;
    style.GrabMinSize       = 10.0f;

    // Window title bar
    style.WindowTitleAlign  = ImVec2(0.0f, 0.5f);  // left-aligned title

    // Separator
    style.SeparatorTextBorderSize = 1.0f;

    // COLORS - classic light gray background, white input fields, dark blue titles
    ImVec4* c = style.Colors;

    // Background colors
    c[ImGuiCol_WindowBg]             = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);  // #C0C0C0 light gray
    c[ImGuiCol_ChildBg]              = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);  // #C0C0C0
    c[ImGuiCol_PopupBg]              = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);  // #C0C0C0

    // Borders
    c[ImGuiCol_Border]               = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);  // #808080 dark gray
    c[ImGuiCol_BorderShadow]         = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);  // #000000 black shadow

    // Title bar (classic Windows 95 dark blue active title bar)
    c[ImGuiCol_TitleBg]              = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);  // inactive gray
    c[ImGuiCol_TitleBgActive]        = ImVec4(0.00f, 0.00f, 0.50f, 1.00f);  // #000080 active dark blue
    c[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Menu bar
    c[ImGuiCol_MenuBarBg]            = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);  // #C0C0C0

    // Scrollbar
    c[ImGuiCol_ScrollbarBg]          = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);  // #D8D8D8 very light gray
    c[ImGuiCol_ScrollbarGrab]        = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);  // #C0C0C0
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    c[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);

    // Frame backgrounds (input fields, checkboxes - sunken white)
    c[ImGuiCol_FrameBg]              = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);  // #FFFFFF white
    c[ImGuiCol_FrameBgHovered]       = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c[ImGuiCol_FrameBgActive]        = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);

    // Buttons (raised light gray)
    c[ImGuiCol_Button]               = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);  // #C0C0C0
    c[ImGuiCol_ButtonHovered]        = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    c[ImGuiCol_ButtonActive]         = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);  // pressed

    // Headers (active selections in trees)
    c[ImGuiCol_Header]               = ImVec4(0.00f, 0.00f, 0.50f, 1.00f);  // active dark blue
    c[ImGuiCol_HeaderHovered]        = ImVec4(0.10f, 0.10f, 0.60f, 1.00f);
    c[ImGuiCol_HeaderActive]         = ImVec4(0.00f, 0.00f, 0.40f, 1.00f);

    // Tabs
    c[ImGuiCol_Tab]                  = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);  // #C0C0C0
    c[ImGuiCol_TabHovered]           = ImVec4(0.80f, 0.80f, 0.80f, 1.00f);
    c[ImGuiCol_TabActive]            = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);
    c[ImGuiCol_TabUnfocused]         = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    c[ImGuiCol_TabUnfocusedActive]   = ImVec4(0.75f, 0.75f, 0.75f, 1.00f);

    // Docking
    c[ImGuiCol_DockingPreview]       = ImVec4(0.00f, 0.00f, 0.50f, 0.30f);  // blue preview
    c[ImGuiCol_DockingEmptyBg]       = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

    // Table
    c[ImGuiCol_TableHeaderBg]        = ImVec4(0.70f, 0.70f, 0.70f, 1.00f);
    c[ImGuiCol_TableBorderStrong]    = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    c[ImGuiCol_TableBorderLight]     = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    c[ImGuiCol_TableRowBg]           = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
    c[ImGuiCol_TableRowBgAlt]        = ImVec4(0.95f, 0.95f, 0.95f, 1.00f);

    // Slider, CheckMark, Separator
    c[ImGuiCol_SliderGrab]           = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    c[ImGuiCol_SliderGrabActive]     = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
    c[ImGuiCol_CheckMark]            = ImVec4(0.00f, 0.00f, 0.50f, 1.00f);  // blue check
    c[ImGuiCol_Separator]            = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
    c[ImGuiCol_SeparatorHovered]     = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);
    c[ImGuiCol_SeparatorActive]      = ImVec4(0.00f, 0.00f, 0.50f, 1.00f);

    // Resize grip
    c[ImGuiCol_ResizeGrip]           = ImVec4(0.50f, 0.50f, 0.50f, 0.20f);
    c[ImGuiCol_ResizeGripHovered]    = ImVec4(0.50f, 0.50f, 0.50f, 0.50f);
    c[ImGuiCol_ResizeGripActive]     = ImVec4(0.00f, 0.00f, 0.50f, 0.80f);

    // Text (black text on light gray)
    c[ImGuiCol_Text]                 = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);  // #000000 black
    c[ImGuiCol_TextDisabled]         = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);  // #808080 gray

    // Selection
    c[ImGuiCol_TextSelectedBg]       = ImVec4(0.00f, 0.00f, 0.50f, 0.40f);

    // Drag/Drop
    c[ImGuiCol_DragDropTarget]       = ImVec4(0.00f, 0.00f, 0.50f, 0.90f);

    // Nav
    c[ImGuiCol_NavHighlight]         = ImVec4(0.00f, 0.00f, 0.50f, 1.00f);
    c[ImGuiCol_NavWindowingHighlight]= ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
    c[ImGuiCol_NavWindowingDimBg]    = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

    // Modal dim
    c[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);

    // Plot
    c[ImGuiCol_PlotLines]            = ImVec4(0.00f, 0.00f, 0.50f, 1.00f);
    c[ImGuiCol_PlotLinesHovered]     = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
    c[ImGuiCol_PlotHistogram]        = ImVec4(0.00f, 0.00f, 0.50f, 1.00f);
    c[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
}

#endif
