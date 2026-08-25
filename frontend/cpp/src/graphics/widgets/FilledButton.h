//
// Created by tim on 17.07.26.
//

#ifndef HARMONY_FILLEDBUTTON_H
#define HARMONY_FILLEDBUTTON_H
#include <functional>

#include "Widget.h"
#include "../../../BasicCppLibrary/vectors/vec3.h"
#include "../../../BasicCppLibrary/vectors/vec4.h"
#include "../../../BasicCppLibrary/visuals/shader.h"
#include "../../supporters/InputManager.h"
#include "../basicShapes/Rectangle.h"


class FilledButton : public Widget {
    float borderRadius = 100;
    vec2 position{};
    vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.f);
    size_t onPressedId = -1;


    double lastClickTime = -99999.0;
    vec2 lastClickPosition{};

    static Shader *shader;


    [[nodiscard]] bool isHovered(float mouseX, float mouseY) const;

public:
    Widget child;

    FilledButton() = default;

    ~FilledButton();

    void setColor(vec4 newColor);

    void setPosition(float x, float y);

    [[nodiscard]] vec2 getPosition() const;

    [[nodiscard]] float getX() const;

    [[nodiscard]] float getY() const;

    void setBorderRadius(float newBorderRadius);

    void setOnPressed(const MouseButtonCallback &onPressed);

    void deleteOnPressed();


    void draw();

    static void initializeShaders();

    static void deleteShaders();
};


#endif //HARMONY_FILLEDBUTTON_H
