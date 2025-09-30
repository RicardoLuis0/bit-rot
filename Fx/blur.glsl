#version 450

out vec4 fragColor;

in vec2 texCoord;

layout (location = 0) uniform sampler2D frameBuffer;
layout (location = 1) uniform ivec2 windowResolution;

#define blurPower 0.0025
#define blurStrenght1 0.5
#define blurStrength2 0.1

float lum(vec4 color)
{
    return dot(color.rgb, vec3(0.299, 0.587, 0.114));// * color.a;
}

void main()
{
    vec2 offsets[8] = vec2[](
        //bottom
        vec2(0, -blurPower),
        //bottom-left
        vec2(-blurPower, -blurPower),
        //left
        vec2(-blurPower, 0),
        //top-left
        vec2(-blurPower, +blurPower),
        //top
        vec2(0, +blurPower),
        //top-right
        vec2(+blurPower, +blurPower),
        //right
        vec2(+blurPower, 0),
        //bottom-right
        vec2(+blurPower, -blurPower)
    );
    vec4 orig = texture(frameBuffer, texCoord);
    vec4 acc = orig;
    
    for(int i = 0; i < 8; i++)
    {
        acc += texture(frameBuffer, texCoord + offsets[i]);
    }
    acc /= 9.0;
    
    fragColor = vec4((acc.rgb * blurStrenght1) + orig.rgb * blurStrength2, 1.0);
}