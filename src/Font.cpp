#include "Font.h"
#include "Config.h"
#include "Json.h"
#include "Common.h"

#include <filesystem>
#include <algorithm>

constexpr const char * fontInfoFile = "Fonts/FontInfo.json";

struct font_info_t
{
    std::string filename;
    uint32_t char_width;
    uint32_t char_height;
    uint32_t cols;
    uint32_t rows;
};

uint32_t selectedFontIndex;
uint32_t selectedFontWidth;
uint32_t selectedFontHeight;
std::vector<uint32_t> selectedFont;
std::map<std::string, font_info_t> fontList;
std::vector<std::string> fontNameList;
std::string defaultFont;

void Font::Init() try
{
    JSON::Element fontInfo = JSON::Parse(Util::ReadFile(fontInfoFile));
    defaultFont = fontInfo["DefaultFont"].get_str();
    
    JSON::Element fonts = fontInfo["FontList"];
    for(auto &font : fonts.get_obj())
    {
        std::string name = font.first;
        JSON::Element obj = font.second;
        
        
        std::string file = "Fonts/"+obj["Path"].get_str();
        uint32_t char_width = obj["CharWidth"].get_uint();
        uint32_t char_height = obj["CharHeight"].get_uint();
        uint32_t cols = obj["Cols"].get_uint();
        uint32_t rows = obj["Rows"].get_uint();
        
        if((cols * rows) < 256)
        {
            throw FatalError("Font "+Util::QuoteString(name)+" missing characters (has "+std::to_string(cols * rows)+", needs 256)");
        }
        
        if(!std::filesystem::exists(file))
        {
            throw FatalError("Font "+Util::QuoteString(file)+" does not exist");
        }
        
        if(fontList.contains(name))
        {
            throw FatalError("Duplicate Font "+Util::QuoteString(name));
        }
        
        fontList[name] = {file, char_width, char_height, cols, rows};
        fontNameList.push_back(name);
    }
    
    if(fontList.size() == 0)
    {
        throw FatalError("No fonts defined in "+Util::QuoteString(fontInfoFile));
    }
    
    if(!fontList.contains(defaultFont))
    {
        throw FatalError("Invalid DefaultFont in "+Util::QuoteString(fontInfoFile));
    }
    
    setFont(Config::getStringOr("SelectedFont", defaultFont));
}

catch(JSON::JSON_Exception &e)
{
    throw FatalError("Malformed JSON in "+Util::QuoteString(fontInfoFile)+": "+e.msg_top);
}

void Font::setFont(std::string_view _fontName)
{
    std::string fontName (_fontName);
    if(!fontList.contains(fontName))
    {
        LogWarn("Invalid Font ("+Util::QuoteString(fontName)+") Selected, resetting to Default ("+Util::QuoteString(defaultFont)+")");
        fontName = defaultFont;
    }
    
    selectedFont = Util::ReadFileBitmap(fontList[fontName].filename, selectedFontWidth, selectedFontHeight);
    selectedFontIndex = std::ranges::find(fontNameList, fontName) - fontNameList.begin();
    
    LogDebug("Selecting Font "+Util::QuoteString(fontName)+" ("+std::to_string(fontList[fontName].char_width)+"x"+std::to_string(fontList[fontName].char_height)+")");
    
    Config::setString("SelectedFont", fontName);
}

void Font::setFont(int32_t index)
{
    while(index < 0) index += fontNameList.size();
    
    setFont(fontNameList[index % fontNameList.size()]);
}

uint32_t Font::numFonts()
{
    return fontNameList.size();
}

uint32_t Font::curFontIndex()
{
    return selectedFontIndex;
}

const std::vector<uint32_t>& Font::getSelectedFont(uint32_t &width, uint32_t &height, uint32_t &char_width, uint32_t &char_height, uint32_t &cols, uint32_t &rows)
{
    width = selectedFontWidth;
    height = selectedFontHeight;
    
    std::string &name = fontNameList[selectedFontIndex];
    
    char_width = fontList[name].char_width;
    char_height = fontList[name].char_height;
    cols = fontList[name].cols;
    rows = fontList[name].rows;
    
    return selectedFont;
}
std::string_view Font::curFontName()
{
    return fontNameList[selectedFontIndex];
}
