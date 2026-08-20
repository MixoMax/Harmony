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
    Rectangle rectangle{};
    vec4 color = vec4(1.0f, 1.0f, 1.0f, 1.f);
    size_t onPressedId = -1;


    double lastClickTime = -99999.0;
    vec2 lastClickPosition{};

    static Shader *shader;

public:
    Widget child;

    FilledButton() = default;

    ~FilledButton();

    void setSize(float width, float height);

    void setColor(vec4 newColor);

    void setPosition(float x, float y);

    [[nodiscard]] vec2 getPosition() const;

    [[nodiscard]] float getY() const;

    void setRotation(float rotation);

    void setRadius(float radius);

    void setOnPressed(const MouseButtonCallback &onPressed);

    void deleteOnPressed();


    void draw(GLFWwindow *window);

    static void initializeShaders();

    static void deleteShaders();
};


#endif //HARMONY_FILLEDBUTTON_H
