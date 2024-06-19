#version 450

out vec4 fragColor;
in vec2 texCoord;

#define NUM_BUFFERS 10

layout (location = 0) uniform sampler2D frameBuffer;
layout (location = 1) uniform float phosphorStrength = 0.5;
layout (location = 2) uniform sampler2D phopsphorBuffers[NUM_BUFFERS];

vec4 toSRGB(vec4 linearRGB)
{
    bvec3 cutoff = lessThan(linearRGB.rgb, vec3(0.0031308));
    vec3 higher = vec3(1.055)*pow(linearRGB.rgb, vec3(1.0/2.4)) - vec3(0.055);
    vec3 lower = linearRGB.rgb * vec3(12.92);

    return vec4(mix(higher, lower, cutoff), linearRGB.a);
}

// Converts a color from sRGB gamma to linear light gamma
vec4 toLinear(vec4 sRGB)
{
    bvec3 cutoff = lessThan(sRGB.rgb, vec3(0.04045));
    vec3 higher = pow((sRGB.rgb + vec3(0.055))/vec3(1.055), vec3(2.4));
    vec3 lower = sRGB.rgb/vec3(12.92);

    return vec4(mix(higher, lower, cutoff), sRGB.a);
}

void main()
{
    vec3 color = toLinear(texture(frameBuffer, texCoord)).rgb;// * (2.0 - phosphorStrength);
    float mult = phosphorStrength * 0.7;
    for(int i = 0; i < NUM_BUFFERS; i++)
    {
        color += toLinear(texture(phopsphorBuffers[i], texCoord)).rgb * mult;
        mult /= 2.0;
    }
    fragColor = toSRGB(vec4(color / 2, 1.0));
    
    /*
    fragColor = texture(frameBuffer, texCoord) * 0.5
            + texture(phopsphorBuffer1, texCoord) * 0.25
            + texture(phopsphorBuffer2, texCoord) * 0.125
            + texture(phopsphorBuffer3, texCoord) * 0.0625
            + texture(phopsphorBuffer4, texCoord) * 0.03125
            + texture(phopsphorBuffer5, texCoord) * 0.015625;
    */
}
