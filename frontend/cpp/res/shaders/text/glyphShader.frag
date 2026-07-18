#version 460 core



uniform vec4 uColor = vec4(1., 0., 0., 1.);
uniform sampler2D uText;

in vec2 vUV;
out vec4 fragColor;

void main()
{
    vec2 uv = vUV;

    vec4 sampled = vec4(1., 1., 1., texture(uText, uv).r);
    fragColor = uColor * sampled;
    //    fragColor = vec4(1, 1, 1, 1);
}