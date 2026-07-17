#version 460 core



uniform vec4 uColor = vec4(1., 0., 0., 1.);
uniform vec2 uSize = vec2(1, 1);
uniform float uRadius = 0;
uniform bool uIsSelected = false;

in vec2 vUV;
out vec4 fragColor;

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
        if (uIsSelected) {
            fragColor.xyz *= .5;
        }
        return;
    }
    discard;
}