#version 460 core


layout (location = 0) in vec2 aVertexPosition;

uniform vec2 uSize = vec2(1, 1);
uniform vec2 uPosition = vec2(0);
uniform float uRatio = 1;
uniform float uRotation = 0;
uniform float uTime = 0;


out vec2 vUV;

void main()
{
    mat2 rotationMatrix = mat2(cos(uRotation), -sin(uRotation), sin(uRotation), cos(uRotation));

    vec2 pos = aVertexPosition;
    pos *= uSize;
    vUV = pos;

    pos = rotationMatrix * pos;
    pos += uPosition;
    pos.x /= uRatio;

    vec4 renderPos = vec4(pos, 0, 1);
    gl_Position = renderPos;
}