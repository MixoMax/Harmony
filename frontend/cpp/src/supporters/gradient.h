#pragma once
#ifndef GRADIENT_H
#define GRADIENT_H

#include <vector>

#include "../BasicCppLibrary/vectors/vec4.h"

class Gradient{
public:
    struct Value{
        vec4 color;
        float t;
    };
    std::vector<Value> values;

    vec4 getColor(float t);
};

#endif // !GRADIENT_H