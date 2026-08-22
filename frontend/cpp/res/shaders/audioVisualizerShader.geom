#version 460 core
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in VS_OUT {
    vec2 pos;
} gs_in[];

out vec2 vUV;

uniform float uWidth = .01;


void main() {
    vec2 p = gs_in[0].pos;
    gl_Position = vec4(p + vec2(-uWidth / 2, 0), 0, 1);
    EmitVertex();

    gl_Position = vec4(p + vec2(uWidth / 2, 0), 0, 1);
    EmitVertex();

    gl_Position = vec4(vec2(-uWidth / 2 + p.x, 0), 0, 1);
    EmitVertex();

    gl_Position = vec4(vec2(uWidth / 2 + p.x, 0), 0, 1);
    EmitVertex();
    EndPrimitive();
}