#version 460 core


uniform float uTime;
uniform vec2 uResolution;


in vec2 vUV;//from -1 to 1

out vec4 fragColor;



void main()
{
    vec2 uv = vec2(vUV);
    uv.x *= uResolution.x / uResolution.y;

    fragColor = vec4(0, 1, 1, 1);
}