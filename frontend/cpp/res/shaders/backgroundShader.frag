#version 460 core


uniform float uTime;
uniform vec2 uResolution;


in vec2 vUV;//from -1 to 1

out vec4 fragColor;


/*Helper*/
float smin(float a, float b, float k)
{
    k *= 4.0;
    float h = max(k - abs(a - b), 0.0) / k;
    return min(a, b) - h * h * k * (1.0 / 4.0);
}


/*SDF*/
float sdSegment(in vec2 p, in vec2 a, in vec2 b)
{
    vec2 pa = p - a, ba = b - a;
    float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
    return length(pa - ba * h);
}
float sdCircle(vec2 p, float r)
{
    return length(p) - r;
}

float sdEllipse(in vec2 p, in vec2 ab)
{
    p = abs(p); if (p.x > p.y) {p = p.yx;ab = ab.yx;}
    float l = ab.y * ab.y - ab.x * ab.x;
    float m = ab.x * p.x / l;      float m2 = m * m;
    float n = ab.y * p.y / l;      float n2 = n * n;
    float c = (m2 + n2 - 1.0) / 3.0; float c3 = c * c * c;
    float q = c3 + m2 * n2 * 2.0;
    float d = c3 + m2 * n2;
    float g = m + m * n2;
    float co;
    if (d < 0.0)
    {
        float h = acos(q / c3) / 3.0;
        float s = cos(h);
        float t = sin(h) * sqrt(3.0);
        float rx = sqrt(-c * (s + t + 2.0) + m2);
        float ry = sqrt(-c * (s - t + 2.0) + m2);
        co = (ry + sign(l) * rx + abs(g) / (rx * ry) - m) / 2.0;
    }
    else
    {
        float h = 2.0 * m * n * sqrt(d);
        float s = sign(q + h) * pow(abs(q + h), 1.0 / 3.0);
        float u = sign(q - h) * pow(abs(q - h), 1.0 / 3.0);
        float rx = -s - u - c * 4.0 + 2.0 * m2;
        float ry = (s - u) * sqrt(3.0);
        float rm = sqrt(rx * rx + ry * ry);
        co = (ry / sqrt(rm - rx) + 2.0 * g / rm - m) / 2.0;
    }
    vec2 r = ab * vec2(co, sqrt(1.0 - co * co));
    return length(r - p) * sign(p.y - r.y);
}

float sdNote(in vec2 p, in vec2 position) {
    float width = .08;
    float height = .8;
    float headSize = .17;

    float dStroke = sdSegment(p, position - vec2(0, height / 2), position + vec2(0, height / 2)) - width;
    //    float dHead = sdCircle(p - position + vec2(headSize - width * 1 / 2, height / 2 - headSize / 2), headSize);
    float angle = 0.523599;
    mat2 rotationMatrix = mat2(cos(angle), -sin(angle), sin(angle), cos(angle));
    float dHead = sdEllipse(rotationMatrix * (p - position + vec2(headSize - width * 1 / 2, height / 2 - headSize / 2 + .12)), vec2(headSize * 1.3, headSize));

    return smin(dStroke, dHead, 0);
}

float sdNotes(in vec2 p, in vec2 position) {
    float distanceBetweenNotes = .6;
    float height = .8;



    vec2 leftPosition = position - vec2(distanceBetweenNotes / 2, 0) + vec2(0, sin(uTime * .5) * .1);
    vec2 rightPosition = position + vec2(distanceBetweenNotes / 2, 0) + vec2(0, sin(uTime * .5 + 1) * .2);

    float segement2YOffset = .25;

    float dSegment1 = sdSegment(p, leftPosition + vec2(0, height / 2), rightPosition + vec2(0, height / 2)) - .08;
    float dSegment2 = sdSegment(p, leftPosition + vec2(0, height / 2 - segement2YOffset), rightPosition + vec2(0, height / 2 - segement2YOffset)) - .08;
    float dLeftNote = sdNote(p, leftPosition);
    float dRightNote = sdNote(p, rightPosition);
    return min(min(dSegment1, dSegment2), min(dLeftNote, dRightNote));
}

void main()
{
    vec2 uv = vec2(vUV);
    uv.x *= uResolution.x / uResolution.y;

    float d = sdNotes(uv, vec2(0, 0));
    vec4 color = vec4(0, 0, 0, 1);
    if (d < 0.) {
        if (d > -.01) {
            color = vec4(vec3(1, 0, 1) * .2, 1);

        } else {
            color = vec4(.5, 0, .5, 1);
        }
    }
    float v = sin(d * 10 - uTime * .2);



    if (v > 0 && d > 0) {
        color = vec4(.3, .0, .3, 1);
    }

    const int totalCircles = 6;
/*pos, rad, minRad*/
    vec4 circles[totalCircles] = vec4[](
    vec4(.5, .7, .5, .0), // blue
    vec4(.1, .3, 1, 1),
    vec4(-.8, -.3, .4, .0), // red
    vec4(.7, 0, .3, 1),
    vec4(.8, -.6, .7, .0), // green
    vec4(.3, .7, .3, 1)
    );


    for (int circleIndex = 0; circleIndex < totalCircles; circleIndex += 2) {
        float c = 1 - min(1, sdCircle(uv - circles[circleIndex].xy, circles[circleIndex].w) * circles[circleIndex].z);
        c *= c;


        color += vec4(circles[circleIndex + 1].xyz * c, 0);
    }


    fragColor = color;
}