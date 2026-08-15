#version 460 core



uniform vec4 uColor = vec4(1., 0., 0., 1.);
uniform vec2 uSize = vec2(1, 1);
uniform float uRadius = 0;
uniform bool uIsSelected = false;
uniform float uTime = 0;

in vec2 vUV;
in vec2 vPos;
out vec4 fragColor;

vec3 hsv2rgb(in vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
float sdBox(in vec2 p, in vec2 b)
{
    vec2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

float roundedBox(vec2 p, in vec2 b, in float radius) {
    return sdBox(p, b - radius) - radius;
}

void main()
{
    vec2 uv = vUV;

    float d = roundedBox(uv, uSize, uRadius);

    if (d <= 0) {
        fragColor = uColor;

        fragColor.xyz = hsv2rgb(vec3(fract(-uTime / 5 + vPos.x * 3 / 2 + vPos.y / 2 / 5), 1, 1));

        if (uIsSelected) {
            fragColor.xyz *= .5;
        }
        return;
    }
    discard;
}