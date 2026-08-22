#version 460 core


layout (location = 0) in vec2 aVertexPosition;


uniform float uTime = 0;

out VS_OUT {
    vec2 pos;
} vs_out;

void main()
{
    vec2 pos = aVertexPosition;
    vs_out.pos = pos;
    vec4 renderPos = vec4(pos, 0, 1);
    gl_Position = renderPos;
}