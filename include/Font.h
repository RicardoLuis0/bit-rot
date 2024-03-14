#pragma once
#include <cstdint>
#include <vector>
#include <string_view>

namespace Font
{
    void Init();
    inline void Quit() {}
    
    void setFont(std::string_view fontName);
    
    void setFont(int32_t index);
    uint32_t numFonts();
    uint32_t curFontIndex();
    std::string_view curFontName();
    
    const std::vector<uint32_t>& getSelectedFont(uint32_t &width, uint32_t &height, uint32_t &char_width, uint32_t &char_height, uint32_t &cols, uint32_t &rows);
}
