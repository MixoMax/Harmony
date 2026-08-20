#include "gradient.h"

#include "../BasicCppLibrary/customMath.h"

vec4 Gradient::getColor(float t) {
    for (size_t i = 0; i < values.size(); i++)
    {
        Value* v = &values[i];
        if(v->t >= t){
            if(i==0){
                return v->color;
            }else{
                Value* prev = &values[i-1];
                float dT = v->t - prev->t;
                float relativProgress = (t-prev->t) / dT;

                return math::mix(prev->color,v->color, relativProgress);
            }
        }

    }
    
    return values.back().color;
}