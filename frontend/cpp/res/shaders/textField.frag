#version 460 core



uniform vec4 uColor = vec4(1., 0., 0., 1.);
uniform vec2 uSize = vec2(1, 1);
uniform vec2 uPosition = vec2(0, 0);
uniform float uRadius = 0;


uniform float uTime = 0;

uniform vec2 uResolution;
uniform vec2 uMouse;
uniform bool uClick;
uniform float uTimeSinceClick;
uniform vec2 uLastClickPosition;

in float vSizeMultiplier;
in vec2 vAnimationOffset;
in vec2 vPos;
out vec4 fragColor;


float sdBox(in vec2 p, in vec2 b)
{
    vec2 d = abs(p) - b;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0);
}

float sdRoundedBox(vec2 p, in vec2 b, in float radius) {
    return sdBox(p, b - radius) - radius;
}

float sdCircle(vec2 p, float r)
{
    return length(p) - r;
}


void main()
{
    const vec2 mouse = uMouse * uResolution;
    const vec2 lastClickPosition = uLastClickPosition * uResolution;

    float mouseDistance = sdRoundedBox(mouse - uPosition, uSize * vSizeMultiplier, uRadius);
    float d = sdRoundedBox(vPos - uPosition + vAnimationOffset, uSize * vSizeMultiplier, uRadius);
    if (d < 0) {

        vec2 relativePosition = (vPos - uPosition + uSize) / (uSize * 2);
        if (d > -10) {
            //Outline
            relativePosition.x -= .2;
        }

        //Gradient
        vec3 color = mix(vec3(0.9804, 0.1373, 0.4941),
                         vec3(0.9922, 0.549, 0.0039),
                         relativePosition.x - relativePosition.y * .2);

        if (d > -10) {
            //Outline
            color *= .7;
        } else {
            //Click-Wave-Effect
            if (sdCircle(vPos - lastClickPosition, uTimeSinceClick * 2000) > 0) {
                fragColor = vec4(.2, 0, .3, 1);

                return;
            }
        }

        //Hover
        if (mouseDistance < 0) {
            color *= .7;
        }


        fragColor = vec4(color, 1);
        return;
    }

    discard;
}