#pragma once

#include "Common.h"
#include <string>
#include <cstdint>
#include <span>

enum class ETextColor : uint8_t
{
    YELLOW,
    AMBER,
    RED,
    GREEN,
    WHITE,
    COUNT
};

#define CHAR_INVERT1 0x1
#define CHAR_INVERT2 0x2

#define CHAR_BLINK_INVERT 0x4

#define CHAR_UNDERSCORE 0x8


#define CHAR_BLINK1 0x10
#define CHAR_BLINK2 0x20
#define CHAR_BLINK3 0x30

namespace Renderer
{
    void Init();
    void Render();
    void Quit();
    void ToggleWireframe();
    void UpdateResolution(uint32_t width, uint32_t height);
    
    void LowRes();
    void HighRes();
    
    void CycleTextColor();
    void CycleTextColorDown();
    std::string_view GetTextColorName();
    
    void SetFont(uint32_t index);
    void SetFont(std::string_view name);
    
    void CycleFont();
    void CycleFontDown();
    
    void CycleVSync();
    void CycleVSyncDown();
    
    void SetTextColor(ETextColor color);
    void SetVSync(std::string_view VSync); // "Off", "On" or "Adaptive"
    
    void SetText(std::string_view newText);
    void SetText(std::string_view newText, std::span<const uint8_t> properties);
    
    void DrawClear();
    void DrawClear(uint8_t text_char, uint8_t prop);
    
    void DrawClear(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    
    void DrawClearText(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    void DrawClearProps(uint32_t x, uint32_t y, uint32_t width, uint32_t height);
    
    void DrawLineText(uint32_t x, uint32_t y, std::string_view newText, uint32_t width = 0);
    void DrawLineTextFillProp(uint32_t x, uint32_t y, std::string_view newText, uint8_t newProperty, uint32_t width = 0);
    
    void DrawLineTextCentered(uint32_t y, std::string_view newText);
    
    void DrawLineProp(uint32_t x, uint32_t y, std::span<const uint8_t> newProperties, uint32_t width = 0);
    
    void DrawFillLineProp(uint32_t x, uint32_t y, uint8_t newProperty, uint32_t width);
    void DrawFillLineText(uint32_t x, uint32_t y, char newText, uint32_t width);
    
    void DrawFillLineTextProp(uint32_t x, uint32_t y, char newText, uint8_t newProperty, uint32_t width);
    
    
    void DrawChar(uint32_t x, uint32_t y, char newChar, uint8_t newProperty = 0);
    
    template<Util::ContainerConvertibleTo<std::string_view> L>
    void DrawText(uint32_t x, uint32_t y, L &&lines, uint32_t width)
    {
        size_t n = std::size(lines);
        
        for(size_t i = 0; i < n; i++)
        {
            DrawLineText(x, y + i, lines[i], width);
        }
    }
    
    template<Util::ContainerConvertibleTo<std::string_view> L>
    void DrawText(uint32_t x, uint32_t y, L &&lines)
    {
        uint32_t width = Util::MaxAll(Util::Map(lines, std::size<std::remove_cvref_t<decltype(lines[0])>>));
        DrawText(x, y, lines, width);
    }
    
    template<Util::ContainerConvertibleTo<std::string_view> L>
    void DrawTextFillProp(uint32_t x, uint32_t y, L &&lines, uint8_t prop, uint32_t width)
    {
        size_t n = std::size(lines);
        
        for(size_t i = 0; i < n; i++)
        {
            DrawLineText(x, y + i, lines[i], width);
            DrawFillLineProp(x, y + i, prop, width);
        }
    }
    
    template<Util::ContainerConvertibleTo<std::string_view> L>
    void DrawTextFillProp(uint32_t x, uint32_t y, L &&lines, uint8_t prop)
    {
        uint32_t width = Util::MaxAll(Util::Map(lines, std::size<std::remove_cvref_t<decltype(lines[0])>>));
        DrawTextFillProp(x, y, lines, prop, width);
    }
    
    template
    <
        Util::ContainerConvertibleTo<std::string_view> L,
        Util::ContainerConvertibleTo<std::span<const uint8_t>> P
    >
    void DrawText(uint32_t x, uint32_t y, L &&lines, P &&props, uint32_t width)
    {
        size_t n1 = std::size(lines);
        size_t n2 = std::size(props);
        size_t n = std::max(n1, n2);
        
        for(size_t i = 0; i < n; i++)
        {
            if(i < n1) DrawLineText(x, y + i, lines[i], width);
            if(i < n2) DrawLineProp(x, y + i, props[i], width);
        }
    }
    template
    <
        Util::ContainerConvertibleTo<std::string_view> L,
        Util::ContainerConvertibleTo<std::span<const uint8_t>> P
    >
    void DrawText(uint32_t x, uint32_t y, L &&lines, P &&props)
    {
        uint32_t width = std::max<uint32_t>(Util::MaxAll(Util::Map(lines, std::size<std::remove_cvref_t<decltype(lines[0])>>)), Util::MaxAll(Util::Map(props, std::size<std::remove_cvref_t<decltype(props[0])>>)));
        DrawText(x, y, lines, props, width);
    }
}
