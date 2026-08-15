#pragma once

namespace GameData
{
    struct ImageData
    { // 32bpp image
        std::vector<uint32_t> pixels;
        uint32_t width;
        uint32_t height;
    };
    
    void Init();
    void Quit();
    
    bool Exists(std::string filename);
    
    std::string GetFontInfo();
    ImageData* GetFont(std::string fontName);
    
    std::string ReadFile(std::string filename);
    std::vector<std::byte> ReadFileBinary(std::string filename);
    
    std::vector<std::string> ListEmbeddedFiles();
}
