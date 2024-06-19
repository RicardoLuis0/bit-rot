#version 450

layout (location = 0) in vec2 pos2D;
layout (location = 1) in vec2 coord2D;

out vec2 texCoord;

void main()
{
    gl_Position = vec4(pos2D.x, pos2D.y, 0.0, 1.0);
    texCoord = coord2D;
}
