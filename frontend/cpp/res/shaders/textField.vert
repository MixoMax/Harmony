#version 460 core


layout (location = 0) in vec2 aVertexPosition;

uniform vec2 uSize = vec2(1, 1);
uniform vec2 uPosition = vec2(0);
uniform vec2 uResolution = vec2(1, 1);
uniform float uRotation = 0;
uniform float uTime = 0;
uniform float uTimeSinceClick;
uniform vec2 uLastClickPosition;
uniform vec2 uMouse;

out float vSizeMultiplier;
out vec2 vAnimationOffset;
out vec2 vPos;


float easeOutElastic(const float x) {
    const float c4 = (2.f * 3.1415926535f) / 3.f;
    return x == 0.f
    ? 0.f
    : x == 1.f
    ? 1.f
    : pow(2.f, -10.f * x) * sin((x * 10.f - 0.75f) * c4) + 1.f;
}

void main()
{

    mat2 rotationMatrix = mat2(cos(uRotation), -sin(uRotation), sin(uRotation), cos(uRotation));
    vec2 pos = aVertexPosition;

    // Click Animation
    vec2 lastClickPosition = uLastClickPosition * uResolution;
    vec2 mouse = uMouse * uResolution;
    const float sizeStrength = .7;
    const float animationSpeed = 1;
    const float animationProgress = clamp(uTimeSinceClick * animationSpeed, 0, 1);
    const float size = 1;//easeOutElastic(animationProgress) * (1 - sizeStrength) + sizeStrength;

    const vec2 dMouse = uPosition - lastClickPosition;
    vAnimationOffset = vec2(0);//sign(dMouse) * min(vec2(uSize - uSize * (sizeStrength)), abs(dMouse)) * (1 - easeOutElastic(animationProgress));


    vSizeMultiplier = size;
    pos *= uSize * size;

    pos = rotationMatrix * pos;

    pos += uPosition - vAnimationOffset;

    vPos = pos;
    pos /= uResolution;


    vec4 renderPos = vec4(pos, 0, 1);
    gl_Position = renderPos;
}