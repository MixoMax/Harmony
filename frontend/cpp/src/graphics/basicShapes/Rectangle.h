//
// Created by tim on 17.07.26.
//

#ifndef HARMONY_RECTANGLE_H
#define HARMONY_RECTANGLE_H


class Rectangle {
public:
    float x, y;
    float width, height;
    float borderRadius;
    float rotation;

    Rectangle() : x(0), y(0), width(1), height(1), borderRadius(0), rotation(0) {
    }

    Rectangle(float x, float y, float width, float height, float borderRadius, float rotation);

    [[nodiscard]] bool isHovered(float mouseX, float mouseY) const;

    [[nodiscard]] static bool isHovered(float mouseX, float mouseY, float x, float y, float width, float height,
                                        float borderRadius, float rotation);
};


#endif //HARMONY_RECTANGLE_H
