#include "Common.h"
#include "Bitmap.h"

#include <stdexcept>
#include <cassert>

//#include <windows.h>

#ifdef _MSC_VER
    #include <intrin.h>
    
    __forceinline uint8_t clz(uint32_t v)
    {
        unsigned long clz;
        return _BitScanReverse(&clz, v), 31 - clz;
    }
    
    __forceinline uint8_t ctz(uint32_t v)
    {
        unsigned long ctz;
        return _BitScanForward(&ctz, v), ctz;
    }
#else
    #define clz(v) __builtin_clz(v)
    #define ctz(v) __builtin_ctz(v)
#endif

namespace Util
{
    template<typename T>
    auto simpleHeader(T * header)
    {
        if constexpr(std::is_same_v<typename std::remove_cvref_t<T>, bitmap_header_v0_t>)
        {
            return header;
        }
        else
        {
            return (const bitmap_header_v1_t*) header;
        }
    }
    
    template<typename T>
    auto simpleHeaderPal(T * header)
    {
        if constexpr(std::is_same_v<typename std::remove_cvref_t<T>, bitmap_header_v0_t>)
        {
            return header;
        }
        else
        {
            return (const bitmap_header_v1_t*) header;
        }
    }
    
    //void putPalPixel(uint32_t x, uint32_t y
    
    template<typename T>
    std::vector<uint32_t> ReadPalettedBitmap(const std::string &filename, const std::vector<std::byte> &rawData, const bitmap_header_t * file_header, const T * header, uint32_t &width, uint32_t &height)
    {
        bool width_direction = header->width < 0;
        width = width_direction ? -header->width : header->width;
        
        bool height_direction = header->height < 0;
        height = height_direction ? -header->height : header->height;
        
        uint32_t palette_max = 1 << header->bpp;
        
        const std::byte * bitmap_start = rawData.data() + file_header->info.offset;
        const std::byte * bitmap_end = rawData.data() + file_header->info.size;
        
        //const std::byte * bitmap_end =  rawData.data() + file_header->info.offset + header->size_image;
        if constexpr(!std::is_same_v<typename std::remove_cvref_t<T>, bitmap_header_v0_t>)
        {
            if(header->size_image)
            {
                bitmap_end = rawData.data() + file_header->info.offset + header->size_image;
            }
            if(header->clr_used > 0 && header->clr_used < palette_max)
            {
                palette_max = header->clr_used;
            }
        }
        
        uint32_t rowSizeBits = header->bpp * width;
        
        uint32_t rowSizeBytes = (rowSizeBits / 8) + (rowSizeBits % 8 != 0);
        
        uint32_t calcSize = rowSizeBytes * height;
        uint32_t realSize = bitmap_end - bitmap_start;
        
        
        const std::byte * palette_start = rawData.data() + 14 + header->size;
        const std::byte * palette_end = palette_start + (palette_max * 3);
        
        assert(palette_end <= bitmap_start);
        
        if(calcSize > realSize || (file_header->info.offset + realSize) > rawData.size())
        {
            throw std::runtime_error("Failed to open bitmap "+Util::QuoteString(filename)+" : EOF");
        }
        
        std::vector<uint32_t> output;
        output.resize(width * height * 16); // width * height * sizeof(RGBA)
        
        uint32_t x = 0;
        uint32_t y = height - 1;
        
        if(height_direction)
        {
            y = 0;
        }
        
        for(uint32_t i = 0; i < height; i++)
        {
            if(width_direction)
            {
                x = width - 1;
            }
            else
            {
                x = 0;
            }
            
            uint32_t row = rowSizeBytes * i;
            for(uint32_t j = 0; j < width; j++)
            {
                uint32_t index;
                if(header->bpp == 8)
                {
                    index = uint8_t(bitmap_start[row + j]);
                }
                else
                {
                    uint32_t bit = (j * header->bpp);
                    uint32_t byte = bit / 8;
                    bit = (8 - header->bpp) - (bit % 8);
                    
                    index = (uint8_t(bitmap_start[row + byte]) >> bit) & ((1 << header->bpp) - 1);
                }
                if(index >= palette_max)
                {
                    throw std::runtime_error("Failed to open bitmap "+Util::QuoteString(filename)+" : Palette Index Out of Bounds");
                }
                
                output[x + (y * width)] = (((*reinterpret_cast<const uint32_t*>(palette_start + (index * 3))) & 0xFFFFFF) << 8) | 0x000000FF;
                
                if(width_direction)
                {
                    x--;
                }
                else
                {
                    x++;
                }
            }
            
            if(height_direction)
            {
                y++;
            }
            else
            {
                y--;
            }
        }
        
        return output;
    }
    
        /*
    template<typename T>
    std::vector<uint32_t> ReadMaskedBitmap(const std::string &filename, const std::vector<std::byte> &rawData, const bitmap_header_t * file_header, const T * header, uint32_t &width, uint32_t &height, uint32_t maskR, uint32_t maskG, uint32_t maskB, uint32_t maskA)
    {
        if((header->bpp % 8) != 0 || header->bpp > 32)
        {
            throw std::runtime_error("Failed to open bitmap "+Util::QuoteString(filename)+" : Invalid bpp for non-Paletted BMP");
        }
        uint8_t shiftR = maskR ? ctz(maskR) : 0;
        uint8_t shiftG = maskG ? ctz(maskG) : 0;
        uint8_t shiftB = maskB ? ctz(maskB) : 0;
        uint8_t shiftA = maskA ? ctz(maskA) : 0;
        
        uint8_t preR = maskR ? clz(maskR) : 0;
        uint8_t preG = maskG ? clz(maskG) : 0;
        uint8_t preB = maskB ? clz(maskB) : 0;
        uint8_t preA = maskA ? clz(maskA) : 0;
        
        uint8_t lenR = maskR ? std::max(uint32_t(preR), 32u - shiftR) - std::min(uint32_t(preR), 32u - shiftR) : 0;
        uint8_t lenG = maskG ? std::max(uint32_t(preG), 32u - shiftG) - std::min(uint32_t(preG), 32u - shiftG) : 0;
        uint8_t lenB = maskB ? std::max(uint32_t(preB), 32u - shiftB) - std::min(uint32_t(preB), 32u - shiftB) : 0;
        uint8_t lenA = maskA ? std::max(uint32_t(preA), 32u - shiftA) - std::min(uint32_t(preA), 32u - shiftA) : 0;
        
        uint8_t divR = 1 << lenR;
        uint8_t divG = 1 << lenG;
        uint8_t divB = 1 << lenB;
        uint8_t divA = 1 << lenA;
        
        uint32_t maxMask = std::max(std::max(shiftR + lenR, shiftG + lenG), std::max(shiftB + lenB, shiftA + lenA));
        uint32_t maxLen = std::max(std::max(lenR, lenG), std::max(lenB, lenA));
        
        if(maxMask > header->bpp)
        {
            throw std::runtime_error("Failed to open bitmap "+Util::QuoteString(filename)+" : Mask doesn't fit for bpp");
        }
        
        if(maxLen > 8)
        {
            throw std::runtime_error("Failed to open bitmap "+Util::QuoteString(filename)+" : Mask doesn't fit in 8 bits");
        }
        
        bool width_direction = header->width < 0;
        width = width_direction ? -header->width : header->width;
        
        bool height_direction = header->height < 0;
        height = height_direction ? -header->height : header->height;
        
        
        uint32_t bytesPerPixel = header->bpp / 8;
        
        const std::byte * bitmap_start = rawData.data() + file_header->info.offset;
        const std::byte * bitmap_end = rawData.data() + file_header->info.size;
        
        if constexpr(!std::is_same_v<typename std::remove_cvref_t<T>, bitmap_header_v0_t>)
        {
            if(header->size_image)
            {
                bitmap_end = rawData.data() + file_header->info.offset + header->size_image;
            }
        }
        
        uint32_t calcSize = bytesPerPixel * width * height;
        uint32_t realSize = bitmap_end - bitmap_start;
        
        if(calcSize > realSize || (file_header->info.offset + realSize) > rawData.size())
        {
            throw std::runtime_error("Failed to open bitmap "+Util::QuoteString(filename)+" : EOF");
        }
        
        std::vector<uint32_t> output;
        output.resize(width * height * 16); // width * height * sizeof(RGBA)
        
        uint32_t x = 0;
        uint32_t y = height - 1;
        
        if(height_direction)
        {
            y = 0;
        }
        
        //might technically dereference one/two bytes past the end of bitmap_start, but it doesn't actually _read_ them, so it's fine
        for(uint32_t i = 0; i < height; i++)
        {
            if(width_direction)
            {
                x = width - 1;
            }
            else
            {
                x = 0;
            }
            
            uint32_t row = rowSizeBytes * i;
            for(uint32_t j = 0; j < width; j++)
            {
                //TODO
                uint8_t R;
                uint8_t G;
                uint8_t B;
                
                output[x + (y * width)] = (((*reinterpret_cast<const uint32_t*>(palette_start + (index * 3))) & 0xFFFFFF) << 8) | 0x000000FF;
                
                if(width_direction)
                {
                    x--;
                }
                else
                {
                    x++;
                }
            }
            
            if(height_direction)
            {
                y++;
            }
            else
            {
                y--;
            }
        }
        
        return output;
        throw std::runtime_error("ReadMaskedBitmap unimplemented");
    }
        */
    
    template<typename T>
    std::vector<uint32_t> ReadBitmap(const std::string &filename, const std::vector<std::byte> &rawData, const bitmap_header_t * file_header, const T * header, uint32_t &width, uint32_t &height)
    {
        if constexpr(!std::is_same_v<T, bitmap_header_v0_t>)
        {
            if(header->compression == CompressionType::RGB)
            {
            }
        }
        
        switch(header->bpp)
        {
        case 1: // 2-color palette
        case 2: // 4-color palette
        case 4: // 16-color palette
        case 8: // 256-color palette
            return ReadPalettedBitmap(filename, rawData, file_header, simpleHeader(header), width, height);
            /*
        case 16: // 5.5.5.0.1
            //R = 0b0111110000000000
            //G = 0b0000001111100000
            //B = 0b0000000000011111
            return ReadMaskedBitmap(filename, rawData, file_header,
                            simpleHeader(header), width, height,
                            0b0111110000000000u,
                            0b0000001111100000u,
                            0b0000000000011111u,
                            0b0000000000000000u);
        case 24: // 8.8.8.0.0
            return ReadMaskedBitmap(filename, rawData, file_header,
                            simpleHeader(header), width, height,
                            0x00FF0000u,
                            0x0000FF00u,
                            0x000000FFu,
                            0x00000000u);
        case 32: // 8.8.8.8.0
            return ReadMaskedBitmap(filename, rawData, file_header,
                            simpleHeader(header), width, height,
                            0x00FF0000u,
                            0x0000FF00u,
                            0x000000FFu,
                            0xFF000000u);*/
        default:
            throw std::runtime_error("Unsupported bpp for bitmap");
        }
    }
    
    #define CHECK_BITMAP_HEADER_SIZE(ver)\
            if(rawData.size() < (sizeof(bitmap_file_header_t) + sizeof(bitmap_header_v##ver##_t)))\
            {\
                throw std::runtime_error("Failed to open bitmap "+Util::QuoteString(filename)+" : EOF");\
            }\
            return ReadBitmap(filename, rawData, hdr, &hdr->v##ver, width, height);
    
    std::vector<uint32_t> ReadFileBitmap(const std::string &filename, uint32_t &width, uint32_t &height)
    {
        std::vector<std::byte> rawData = ReadFileBinary(filename);
        if(rawData.size() < (sizeof(bitmap_file_header_t) + 4))
        {
            throw std::runtime_error("Failed to open bitmap "+Util::QuoteString(filename)+" : EOF");
        }
        
        const bitmap_header_t * hdr = reinterpret_cast<const bitmap_header_t*>(rawData.data());
        
        if(hdr->info.magic != BITMAP_MAGIC)
        {
            throw std::runtime_error("Failed to open bitmap "+Util::QuoteString(filename)+" : Not a Bitmap file");
        }
        
        switch(hdr->header_size)
        {
        case sizeof(bitmap_header_v0_t): CHECK_BITMAP_HEADER_SIZE(0);
        case sizeof(bitmap_header_v1_t): CHECK_BITMAP_HEADER_SIZE(1);
        case sizeof(bitmap_header_v2_t): CHECK_BITMAP_HEADER_SIZE(2);
        case sizeof(bitmap_header_v3_t): CHECK_BITMAP_HEADER_SIZE(3);
        case sizeof(bitmap_header_v4_t):
        case sizeof(bitmap_header_v5_t):
        default:
            throw std::runtime_error("Failed to open bitmap "+Util::QuoteString(filename)+" : Unsupported Bitmap file");
        }
    }
}
