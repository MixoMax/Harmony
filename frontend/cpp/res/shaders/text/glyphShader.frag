#version 460 core



uniform vec4 uColor = vec4(1., 0., 0., 1.);
uniform sampler2D uText;
uniform float uTime;

in vec2 vUV;
in vec2 vPos;
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

    vec4 sampled = vec4(1., 1., 1., texture(uText, uv).r);
    vec4 color = vec4(hsv2rgb(vec3(fract(uTime / 5 + vPos.x + vPos.y / 5), 1, 1)), 1);
    fragColor = color * sampled;
    //    fragColor = vec4(1, 1, 1, 1);
}