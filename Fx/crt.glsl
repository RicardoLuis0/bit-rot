#version 450

out vec4 fragColor;

in vec2 texCoord;

layout (location = 0) uniform sampler2D frameBuffer;
layout (location = 1) uniform ivec2 windowResolution;
layout (location = 3) uniform ivec2 frameBufferResolution;
layout (location = 5) uniform float crtCurve = 1.0;
layout (location = 6) uniform bool crtScanlines = false;
layout (location = 7) uniform bool crtCA = false;
layout (location = 8) uniform bool crtVignette = false;

//#define CURVATURE 3.5
#define CURVATURE 3.5
//#define CURVATURE 1.0
#define SHRINK 0.01

#define BLUR .021

#define CA_AMT 1.005

void main()
{
    // based on the CRT shader from lalaoopybee ( https://www.shadertoy.com/view/DlfSz8 )
    //   This work is licensed under a Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
    
    //vec2 uv = texCoord * 1.2;
    vec2 uv = texCoord;
    
    //curving
    
    //center coords
    vec2 crtUV = uv * 2. - 1.;
    
    //curve
    vec2 offset = crtUV.yx / CURVATURE;
    crtUV += crtUV * offset * offset;
    
    //un-center coords
    crtUV = crtUV * .5 + .5;
    
    crtUV = mix(crtUV, uv, 1.0 - crtCurve);
    
    float shrink = crtCurve * SHRINK;
    
    // shrink
    vec2 crtUV2 = crtUV * (1.0 + shrink + shrink) - shrink;
    
    vec2 edge = smoothstep(0., BLUR, crtUV2)*(1.-smoothstep(1.-BLUR, 1., crtUV2));
    
    if(crtCA)
    {
        float r = texture(frameBuffer, (crtUV2-.5)*CA_AMT+.5).r;
        float g = texture(frameBuffer, crtUV2).g;
        float b = texture(frameBuffer, (crtUV2-.5)/CA_AMT+.5).b;
        
        //chromatic abberation
        fragColor.rgb=vec3(
            r,
            g,
            b
        );
    }
    else
    {
        fragColor.rgb = texture(frameBuffer, crtUV2).rgb;
    }
	
	if(crtVignette)
	{
		fragColor.rgb *= edge.x*edge.y;
	}
    
    if(crtScanlines)
    {
        float yfrac = mod(crtUV2.y * frameBufferResolution.y, 1.);
        
        fragColor.rgb*=0.4 + float(yfrac > 0.25 && yfrac < 0.75) * 0.8;
    }
}