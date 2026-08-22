//
// Created by tim on 17.07.26.
//

#include "Rectangle.h"

#include "../GraphicsManager.h"
#include "../../../BasicCppLibrary/vectors/vec2.h"
#include "../../supporters/SDF.h"

Rectangle::Rectangle(const float x, const float y, const float width, const float height, const float borderRadius,
                     const float rotation) : x(x), y(y),
                                             width(width), height(height),
                                             borderRadius(borderRadius), rotation(rotation) {
}

bool Rectangle::isHovered(const float mouseX, const float mouseY) const {
    return isHovered(mouseX, mouseY, x, y, width, height, borderRadius, rotation);
}

bool Rectangle::isHovered(const float mouseX, const float mouseY, const float x, const float y, const float width,
                          const float height, const float borderRadius, const float rotation) {
    vec2 uv(mouseX, mouseY);
    uv.x *= static_cast<float>(GraphicsManager::getInstance().getScreenWidth());
    uv.y *= static_cast<float>(GraphicsManager::getInstance().getScreenHeight());
    uv.x -= x;
    uv.y -= y;

    uv = uv.rotate(rotation);


    if (sdRoundedBox(uv, width, height, borderRadius) <= 0) {
        return true;
    }

    return false;
}
