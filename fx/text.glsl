#version 460

out vec4 fragColor;
in vec2 texCoord;

layout (location = 0) uniform sampler2D fontTexture;
layout (location = 1) uniform vec4 transparentKey; // 1,2,3,4
layout (location = 5) uniform uint time;
layout (location = 6) uniform vec4 textColor; // 6,7,8,9

#define CHAR_BLINK_BITS 0x30

#define CHAR_INVERT1 0x1
#define CHAR_INVERT2 0x2

#define CHAR_BLINK_INVERT 0x4

#define CHAR_UNDERSCORE 0x8

#define CHAR_BLINK1 0x10
#define CHAR_BLINK2 0x20
#define CHAR_BLINK3 0x30

layout (std140, binding = 0) uniform TextInfo
{
    int font_width;
    int font_height;
    int char_width;
    int char_height;
    int screen_width;
    int screen_height;
    ivec4 chars[200];
    ivec4 char_properties[200];
};

void main()
{
    vec2 baseCoord = (((texCoord * vec2(1.0, -1.0)) + vec2(0.0, 1.0)) * vec2(screen_width, screen_height));
    
    int i = int(baseCoord.x) + (int(baseCoord.y) * screen_width);
    
    int j = (i / 4) % 4;
    int s = ((i % 4) * 8);
    
    int c = (chars[i / 16][j] >> s) & 0xFF;
    int p = (char_properties[i / 16][j] >> s) & 0xFF;
    
    vec2 charCoord = mod(baseCoord, 1) + ivec2(c % font_width, c / font_width);
    
    vec4 tex = texture(fontTexture, charCoord / vec2(font_width, font_height));
    
    if((p & CHAR_UNDERSCORE) != 0 && (time % 1000) > 500)
    {
        charCoord = mod(baseCoord, 1) + ivec2('_' % font_width, '_' / font_width);
        vec4 tex2 = texture(fontTexture, charCoord / vec2(font_width, font_height));
        if(tex2 != transparentKey)
        {
            tex = tex2;
        }
    }
    
    bool blink = ((p & CHAR_BLINK_BITS) == CHAR_BLINK1 && (time % 200) > 100) || ((p & CHAR_BLINK_BITS) == CHAR_BLINK2 && (time % 1000) > 500) || ((p & CHAR_BLINK_BITS) == CHAR_BLINK3 && (time % 2000) > 1000);
    
    float pixel;
    if((p & CHAR_BLINK_INVERT) != 0)
    {
        pixel = float(((tex != transparentKey) != ((p & CHAR_INVERT1) != 0) == blink) != ((p & CHAR_INVERT2) != 0));
    }
    else
    {
        pixel = float(((tex != transparentKey) != ((p & CHAR_INVERT1) != 0) && !blink) != ((p & CHAR_INVERT2) != 0));
    }
    
    fragColor = vec4(pixel, pixel, pixel, 1.0) * textColor;
}
