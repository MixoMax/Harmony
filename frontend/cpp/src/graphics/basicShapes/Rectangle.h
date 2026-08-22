//
// Created by tim on 17.07.26.
//

#ifndef HARMONY_RECTANGLE_H
#define HARMONY_RECTANGLE_H


class Rectangle {
public:
    float x = 0, y = 0;
    float width = 1, height = 1;
    float borderRadius = 0;
    float rotation = 0;

    [[nodiscard]] bool isHovered(float mouseX, float mouseY) const;

    [[nodiscard]] static bool isHovered(float mouseX, float mouseY, float x, float y, float width, float height,
                                        float borderRadius, float rotation);
};


#endif //HARMONY_RECTANGLE_H
