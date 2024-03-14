#pragma once

#include <cstdint>

constexpr uint16_t BITMAP_MAGIC = 0x4D42;

#pragma pack(push,2)

struct bitmap_file_header_t
{
    uint16_t magic;
    uint32_t size;
    uint16_t reserved_1;
    uint16_t reserved_2;
    uint32_t offset;
};

static_assert(sizeof(bitmap_file_header_t) == 14);

struct bitmap_header_v0_t
{
    uint32_t size;
    uint16_t width;
    uint16_t height;
    uint16_t planes;
    uint16_t bpp;
};

static_assert(sizeof(bitmap_header_v0_t) == 12);

enum class CompressionType : uint32_t
{
    RGB = 0, // always rgb555 on 16bpp
    //RLE8 = 1,
    //RLE4 = 2,
    BITFIELDS = 3, // check masks, only for v2+
    //JPEG = 4,
    //PNG = 5,
    BI_ALPHABITFIELDS = 6, // check masks, only for v3+
};

struct bitmap_header_v1_t
{
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bpp;
    CompressionType compression;
    uint32_t size_image;
    int32_t  x_ppm;
    int32_t  y_ppm;
    uint32_t clr_used;
    uint32_t clr_important;
};

static_assert(sizeof(bitmap_header_v1_t) == 40);

struct bitmap_header_v2_t
{
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bpp;
    CompressionType compression;
    uint32_t size_image;
    int32_t  x_ppm;
    int32_t  y_ppm;
    uint32_t clr_used;
    uint32_t clr_important;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
};

static_assert(sizeof(bitmap_header_v2_t) == 52);

struct bitmap_header_v3_t
{
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bpp;
    CompressionType compression;
    uint32_t size_image;
    int32_t  x_ppm;
    int32_t  y_ppm;
    uint32_t clr_used;
    uint32_t clr_important;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t alphaMask;
};

static_assert(sizeof(bitmap_header_v3_t) == 56);

struct bitmap_header_v4_t
{
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bpp;
    CompressionType compression;
    uint32_t size_image;
    int32_t  x_ppm;
    int32_t  y_ppm;
    uint32_t clr_used;
    uint32_t clr_important;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t alphaMask;
    uint32_t cstype;        // COLOR SPACE
    struct
    {   // fixed point 2.30
        struct { uint32_t x, y, z; } red;
        struct { uint32_t x, y, z; } green;
        struct { uint32_t x, y, z; } blue;
    } cie;
    struct
    {   // fixed point, 16.16
        uint32_t red;
        uint32_t green;
        uint32_t blue;
    } gamma;
};

static_assert(sizeof(bitmap_header_v4_t) == 108);

struct bitmap_header_v5_t
{
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bpp;
    CompressionType compression;
    uint32_t size_image;
    int32_t  x_ppm;
    int32_t  y_ppm;
    uint32_t clr_used;
    uint32_t clr_important;
    uint32_t redMask;
    uint32_t greenMask;
    uint32_t blueMask;
    uint32_t alphaMask;
    uint32_t cstype;        // COLOR SPACE
    struct
    {   // fixed point 2.30
        struct { uint32_t x, y, z; } red;
        struct { uint32_t x, y, z; } green;
        struct { uint32_t x, y, z; } blue;
    } cie;
    struct
    {   // fixed point, 16.16
        uint32_t red;
        uint32_t green;
        uint32_t blue;
    } gamma;
    uint32_t intent;
    uint32_t profile_data;
    uint32_t profile_size;
    uint32_t reserved;
};

static_assert(sizeof(bitmap_header_v5_t) == 124);

struct bitmap_header_t
{
    bitmap_file_header_t info;
    
    union
    {
        uint32_t header_size;
        bitmap_header_v0_t v0;
        bitmap_header_v1_t v1;
        bitmap_header_v2_t v2;
        bitmap_header_v3_t v3;
        bitmap_header_v4_t v4;
        bitmap_header_v5_t v5;
    };
};

#pragma pack(pop)
