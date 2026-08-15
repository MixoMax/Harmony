#pragma once
#include <cmath>


class Interpolation {
public:
    static float easeInSine(float x) {
        return 1.f - std::cos((x * 3.1415926535f) / 2.f);
    }


    static float easeOutSine(float x) {
        return std::sin((x * 3.1415926535f) / 2.f);
    }


    static float easeInOutSine(float x) {
        return -(std::cos(3.1415926535f * x) - 1.f) / 2.f;
    }


    static float easeInQuad(float x) {
        return x * x;
    }


    static float easeOutQuad(float x) {
        return 1.f - (1.f - x) * (1.f - x);
    }


    static float easeInOutQuad(float x) {
        return x < 0.5f ? 2.f * x * x : 1.f - std::pow(-2.f * x + 2.f, 2.f) / 2.f;
    }


    static float easeInCubic(float x) {
        return x * x * x;
    }


    static float easeOutCubic(float x) {
        return 1.f - std::pow(1.f - x, 3.f);
    }


    static float easeInOutCubic(float x) {
        return x < 0.5f ? 4.f * x * x * x : 1.f - std::pow(-2.f * x + 2.f, 3.f) / 2.f;
    }


    static float easeInQuart(float x) {
        return x * x * x * x;
    }


    static float easeOutQuart(float x) {
        return 1.f - std::pow(1.f - x, 4.f);
    }


    static float easeInOutQuart(float x) {
        return x < 0.5f ? 8.f * x * x * x * x : 1.f - std::pow(-2.f * x + 2.f, 4.f) / 2.f;
    }


    static float easeInQuint(float x) {
        return x * x * x * x * x;
    }


    static float easeOutQuint(float x) {
        return 1.f - std::pow(1.f - x, 5.f);
    }


    static float easeInOutQuint(float x) {
        return x < 0.5f ? 16.f * x * x * x * x * x : 1.f - std::pow(-2.f * x + 2.f, 5.f) / 2.f;
    }


    static float easeInExpo(float x) {
        return x == 0.f ? 0.f : std::pow(2.f, 10.f * x - 10.f);
    }


    static float easeOutExpo(float x) {
        return x == 1.f ? 1.f : 1.f - std::pow(2.f, -10.f * x);
    }


    static float easeInOutExpo(float x) {
        return x == 0.f
                   ? 0.f
                   : x == 1.f
                         ? 1.f
                         : x < 0.5f
                               ? std::pow(2.f, 20.f * x - 10.f) / 2.f
                               : (2.f - std::pow(2.f, -20.f * x + 10.f)) / 2.f;
    }


    static float easeInCirc(float x) {
        return 1.f - std::sqrt(1.f - std::pow(x, 2.f));
    }


    static float easeOutCirc(float x) {
        return std::sqrt(1.f - std::pow(x - 1.f, 2.f));
    }


    static float easeInOutCirc(float x) {
        return x < 0.5f
                   ? (1.f - std::sqrt(1.f - std::pow(2.f * x, 2.f))) / 2.f
                   : (std::sqrt(1.f - std::pow(-2.f * x + 2.f, 2.f)) + 1.f) / 2.f;
    }


    static float easeInBack(float x) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.f;
        return c3 * x * x * x - c1 * x * x;
    }


    static float easeOutBack(float x) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.f;
        return 1.f + c3 * std::pow(x - 1.f, 3.f) + c1 * std::pow(x - 1.f, 2.f);
    }


    static float easeInOutBack(float x) {
        const float c1 = 1.70158f;
        const float c2 = c1 * 1.525f;
        return x < 0.5f
                   ? (std::pow(2.f * x, 2.f) * ((c2 + 1.f) * 2.f * x - c2)) / 2.f
                   : (std::pow(2.f * x - 2.f, 2.f) * ((c2 + 1.f) * (x * 2.f - 2.f) + c2) + 2.f) / 2.f;
    }


    static float easeInElastic(float x) {
        const float c4 = (2.f * 3.1415926535f) / 3.f;
        return x == 0.f
                   ? 0.f
                   : x == 1.f
                         ? 1.f
                         : -std::pow(2.f, 10.f * x - 10.f) * std::sin((x * 10.f - 10.75f) * c4);
    }


    static float easeOutElastic(float x) {
        const float c4 = (2.f * 3.1415926535f) / 3.f;
        return x == 0.f
                   ? 0.f
                   : x == 1.f
                         ? 1.f
                         : std::pow(2.f, -10.f * x) * std::sin((x * 10.f - 0.75f) * c4) + 1.f;
    }


    static float easeInOutElastic(float x) {
        const float c5 = (2.f * 3.1415926535f) / 4.5f;
        return x == 0.f
                   ? 0.f
                   : x == 1.f
                         ? 1.f
                         : x < 0.5f
                               ? -(std::pow(2.f, 20.f * x - 10.f) * std::sin((20.f * x - 11.125f) * c5)) / 2.f
                               : (std::pow(2.f, -20.f * x + 10.f) * std::sin((20.f * x - 11.125f) * c5)) / 2.f + 1.f;
    }


    static float easeInBounce(float x) {
        return 1.f - easeOutBounce(1.f - x);
    }


    static float easeOutBounce(float x) {
        const float n1 = 7.5625f;
        const float d1 = 2.75f;
        if (x < 1.f / d1) {
            return n1 * x * x;
        } else if (x < 2.f / d1) {
            return n1 * (x -= 1.5f / d1) * x + 0.75f;
        } else if (x < 2.5f / d1) {
            return n1 * (x -= 2.25f / d1) * x + 0.9375f;
        } else {
            return n1 * (x -= 2.625f / d1) * x + 0.984375f;
        }
    }


    static float easeInOutBounce(float x) {
        return x < 0.5f
                   ? (1.f - easeOutBounce(1.f - 2.f * x)) / 2.f
                   : (1.f + easeOutBounce(2.f * x - 1.f)) / 2.f;
    }
};
