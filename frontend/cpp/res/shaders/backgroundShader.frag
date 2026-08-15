#version 460 core



uniform float uTime = 0;

in vec2 vUV;
out vec4 fragColor;

vec3 hsv2rgb(in vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec2 uv = vUV;
    uv += 1;
    uv /= 2;

    fragColor = vec4(hsv2rgb(vec3(fract(-uTime / 5 + uv.x * 3 + uv.y / 5), 1, 1)), 1);
}