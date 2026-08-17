#version 410

out vec4 fragColor;
in vec2 texCoord;

#define NUM_BUFFERS 10

uniform sampler2D frameBuffer;
uniform float phosphorStrength;
uniform sampler2D phopsphorBuffer;
uniform float deltaTime;

vec3 toSRGB(vec3 linearRGB)
{
    bvec3 cutoff = lessThan(linearRGB, vec3(0.0031308));
    vec3 higher = vec3(1.055)*pow(linearRGB, vec3(1.0/2.4)) - vec3(0.055);
    vec3 lower = linearRGB * vec3(12.92);

    return mix(higher, lower, cutoff);
}

// Converts a color from sRGB gamma to linear light gamma
vec3 toLinear(vec3 sRGB)
{
    bvec3 cutoff = lessThan(sRGB, vec3(0.04045));
    vec3 higher = pow((sRGB + vec3(0.055))/vec3(1.055), vec3(2.4));
    vec3 lower = sRGB/vec3(12.92);

    return mix(higher, lower, cutoff);
}

void main()
{
    if(phosphorStrength > 0.0)
    {
        vec3 colorNew = texture(frameBuffer, texCoord).rgb;
        vec3 colorOld = texture(phopsphorBuffer, texCoord).rgb;
        
        if(length(colorOld) < 0.1) colorOld = vec3(0,0,0); // prevent ghosting on high fps
        
        float str = phosphorStrength * 0.015;
        float mult = min(str / deltaTime, 0.95);
        float imult = 1.0 - mult;
        
        fragColor = vec4(toSRGB((toLinear(colorOld) * mult) + (toLinear(colorNew) * imult)), 1.0);
    }
    else
    {
        fragColor = vec4(toSRGB(toLinear(texture(frameBuffer, texCoord).rgb)), 1.0);
    }
}
