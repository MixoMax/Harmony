#version 460 core


layout (location = 0) in vec2 aVertexPosition;


uniform float uTime = 0;


out vec2 vUV;

void main()
{
    vec2 pos = aVertexPosition;
    vUV = pos;
    vec4 renderPos = vec4(pos, 0, 1);
    gl_Position = renderPos;
}