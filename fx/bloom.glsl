#version 450

out vec4 fragColor;

in vec2 texCoord;

layout (location = 0) uniform sampler2D frameBuffer;
layout (location = 1) uniform ivec2 windowResolution;

/*
float bloomPower;
float bloomStrenght;
float bloomContrast;
*/
#define bloomPower 0.0025
#define bloomStrenght 0.5
#define bloomContrast 0.5

float lum(vec4 color)
{
    return dot(color.rgb, vec3(0.299, 0.587, 0.114));// * color.a;
}

void main()
{
    vec2 offsets[8] = vec2[](
        //bottom
        vec2(0, -bloomPower),
        //bottom-left
        vec2(-bloomPower, -bloomPower),
        //left
        vec2(-bloomPower, 0),
        //top-left
        vec2(-bloomPower, +bloomPower),
        //top
        vec2(0, +bloomPower),
        //top-right
        vec2(+bloomPower, +bloomPower),
        //right
        vec2(+bloomPower, 0),
        //bottom-right
        vec2(+bloomPower, -bloomPower)
    );
    vec4 orig = texture(frameBuffer, texCoord);
    vec4 acc = orig;
    
    for(int i = 0; i < 8; i++)
    {
        acc += texture(frameBuffer, texCoord + offsets[i]);
    }
    acc /= 9.0;
    
    
	if(lum(acc) > lum(orig))
	{
		fragColor = vec4((orig.rgb * (1.0f - bloomStrenght) + acc.rgb * bloomStrenght) * (1.0 + bloomContrast), 1.0);
	}
	else
	{
        fragColor = vec4(orig.rgb *(1.0 + bloomContrast), 1.0);
	}
}