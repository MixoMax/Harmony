//
// Created by tim on 17.07.26.
// thanks to iquilezles (https://iquilezles.org/articles/distfunctions2d/)
//

#ifndef HARMONY_SDF_H
#define HARMONY_SDF_H
#include <algorithm>

#include "../../BasicCppLibrary/vectors/vec2.h"

[[nodiscard]] inline float sdBox(const vec2 &uv, const float width, const float height) {
    const vec2 d = uv.abs() - vec2(width, height);

    return d.max(vec2(0.f)).length() + std::min(d.max(), 0.f);
}

[[nodiscard]] inline float sdRoundedBox(const vec2 &uv, const float width, const float height, const float radius) {
    return sdBox(uv, width - radius, height - radius) - radius;
}

#endif //HARMONY_SDF_H
