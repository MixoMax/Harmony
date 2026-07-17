//
// Created by tim on 17.07.26.
//

#ifndef HARMONY_FILLEDBUTTON_H
#define HARMONY_FILLEDBUTTON_H
#include <functional>

#include "Widget.h"
#include "../../../BasicCppMathLibrary/vectors/vec3.h"
#include "../../../BasicCppMathLibrary/vectors/vec4.h"
#include "../../supporters/InputManager.h"
#include "../basicShapes/Rectangle.h"
#include "../shaders/shader.h"


class FilledButton : public Widget {
    Rectangle rectangle{};
    vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.f);
    size_t onPressedId = -1;

    static Shader *shader;

public:
    Widget child;

    FilledButton() = default;

    ~FilledButton();

    void setSize(float width, float height);

    void setColor(vec4 newColor);

    void setPosition(float x, float y);

    void setRotation(float rotation);

    void setRadius(float radius);

    void setOnPressed(const MouseButtonCallback &onPressed);

    void deleteOnPressed();


    void draw() const;

    static void initializeShaders();

    static void deleteShaders();
};


#endif //HARMONY_FILLEDBUTTON_H
