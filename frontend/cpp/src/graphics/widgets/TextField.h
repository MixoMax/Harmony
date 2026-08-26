//
// Created by tim on 25.08.26.
//

#ifndef HARMONY_TEXTFIELD_H
#define HARMONY_TEXTFIELD_H
#include "Widget.h"
#include "../../../BasicCppLibrary/text/Font.h"
#include "../../../BasicCppLibrary/vectors/vec2.h"
#include "../../../BasicCppLibrary/visuals/shader.h"
#include "../../supporters/Alignment.h"


class TextField : public Widget {
    float borderRadius = 0;
    vec2 position{};
    vec4 padding{50, 20, 50, 20};

    double lastClickTime = -99999.0;
    vec2 lastClickPosition{};

    std::vector<unsigned int> text{};

    static Shader *shader;

    [[nodiscard]] bool isHovered(float mouseX, float mouseY) const;

    size_t clickCallbackId = 0;
    size_t charCallbackId = 0;
    size_t keyCallbackId = 0;

public:
    Font font{.family = "Cause", .size = 1.5, .rotation = 0, .color = {1, 1, 1, 1}, .alignment = Alignment::Center};

    TextField();

    ~TextField();

    void draw();

    void setPosition(float x, float y);

    std::string getText();

    static void initializeShaders();

    static void deleteShaders();
};


#endif //HARMONY_TEXTFIELD_H
