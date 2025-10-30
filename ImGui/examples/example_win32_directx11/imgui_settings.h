#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui_internal.h"

#include <iostream>

#include <stdio.h>
#include <time.h>
#include <math.h>
#include <stdlib.h> 
#include <conio.h>
#include <Windows.h>
#include <random>

using namespace ImGui;

inline void rect_glow(ImDrawList* draw, ImVec2 start, ImVec2 end, ImColor col, float rounding, float intensity) {
    while (true) {
        if (col.Value.w < 0.0019f)
            break;

        draw->AddRectFilled(start, end, col, rounding);

        col.Value.w -= col.Value.w / intensity;
        start = ImVec2(start.x - 1, start.y - 1);
        end = ImVec2(end.x + 1, end.y + 1);
    }
}

inline int rotation_start_index;
inline void ImRotateStart()
{
    rotation_start_index = ImGui::GetWindowDrawList()->VtxBuffer.Size;
}

inline ImVec2 center_text(ImVec2 min, ImVec2 max, const char* text)
{
    return min + (max - min) / 2 - ImGui::CalcTextSize(text) / 2;
}

inline ImVec2 ImRotationCenter()
{
    ImVec2 l(FLT_MAX, FLT_MAX), u(-FLT_MAX, -FLT_MAX); // bounds

    const auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        l = ImMin(l, buf[i].pos), u = ImMax(u, buf[i].pos);

    return ImVec2((l.x + u.x) / 2, (l.y + u.y) / 2); // or use _ClipRectStack?
}

inline ImVec4 ImColorToImVec4(const ImColor& color)
{
    return ImVec4(color.Value.x, color.Value.y, color.Value.z, color.Value.w);
}

inline void ImRotateEnd(float rad, ImVec2 center = ImRotationCenter())
{
    float s = sin(rad), c = cos(rad);
    center = ImRotate(center, s, c) - center;

    auto& buf = ImGui::GetWindowDrawList()->VtxBuffer;
    for (int i = rotation_start_index; i < buf.Size; i++)
        buf[i].pos = ImRotate(buf[i].pos, s, c) - center;
}

inline int menu_state = 0;

inline ImColor main_color(80, 129, 201, 255);

inline ImColor main_color_shadow(80, 129, 201, 170);

inline ImFont *default_font;

inline ImFont *big_icon;

inline ImFont *big_font;

inline ImFont *icon_font;

inline ImFont *medium_icon_font;

inline ImFont *bold_font;

inline ImFont *bold_big_font;

inline ImFont* icon_small;

inline ImFont* dot_font;

inline ImColor text_color[2]{ ImColor(214, 214, 214, 255), ImColor(214, 214, 214, 65) };

inline ImColor main_color2(150, 125, 200, 120);

inline ImColor main_color_outline(150, 125, 200, 255);

inline ImColor background_color(22, 28, 41, 255);


inline ImColor second_color(20, 20, 20, 255);

inline ImColor winbg(16, 17, 23, 255);

inline ImVec4 inputtext_color[3] = { background_color, main_color, text_color[0] };

inline float anim_speed;

inline float random_float(float min, float max)
{
    return min + float(rand() / float(RAND_MAX)) * (max - min);
}


inline bool button_text(const char* first_text, const char* label)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImVec2 pos = window->DC.CursorPos;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    static ImVec4 color_text;
    static ImVec4 color_shadow;

    ImVec2 size = CalcTextSize(label);

    const ImRect bb(pos, pos + size);
    const ImRect bb_text(pos + ImVec2(CalcTextSize(first_text).x, 0), pos + ImVec2(CalcTextSize(first_text).x, 0) + size);
    ItemSize(size, style.FramePadding.y);
    ItemAdd(bb, id);

    bool hovered, held;
    bool pressed = ButtonBehavior(bb_text, id, &hovered, &held, 0);

    color_text = ImLerp(color_text, hovered ? main_color : text_color[0], anim_speed);

    color_shadow = ImLerp(color_shadow, hovered ? ImColor(main_color.Value.x, main_color.Value.y, main_color.Value.z, 0.1f) : ImColor(main_color.Value.x, main_color.Value.y, main_color.Value.z, 0.f), anim_speed);

    rect_glow(window->DrawList, pos + ImVec2(CalcTextSize(first_text).x, 0) + ImVec2(0, 2), pos + ImVec2(CalcTextSize(first_text).x, 0) + size - ImVec2(0, 2), color_shadow, 360.f, 5.f);

    window->DrawList->AddText(pos, text_color[1], first_text);

    window->DrawList->AddText(pos + ImVec2(CalcTextSize(first_text).x, 0), GetColorU32(color_text), label);

    return pressed;
}
