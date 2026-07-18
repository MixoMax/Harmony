#version 460 core


layout (location = 0) in vec4 aVertexPosition;

uniform vec2 uScreenSize = vec2(1, 1);
uniform float uRotation = 0;

out vec2 vUV;

void main()
{
    mat2 rotationMatrix = mat2(cos(uRotation), -sin(uRotation), sin(uRotation), cos(uRotation));

    vUV = aVertexPosition.zw;
    vec2 pos = aVertexPosition.xy;
    pos = rotationMatrix * pos;
    pos /= uScreenSize;


    vec4 renderPos = vec4(pos, 0, 1);
    gl_Position = renderPos;
}