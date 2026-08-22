//
// Created by tim on 21.08.26.
//

#ifndef HARMONY_TEXTBUTTON_H
#define HARMONY_TEXTBUTTON_H
#include "FilledButton.h"
#include "../../../BasicCppLibrary/text/Font.h"


class TextButton : public FilledButton {
    std::string text{};

    vec4 padding{50, 20, 50, 20};

    void updateDimensions();

public:
    TextButton();

    Font font{.family = "Cause", .size = 2, .rotation = 0, .color = {1, 1, 1, 1}, .alignment = Alignment::Center};

    [[nodiscard]] std::string getText() const;

    void setText(const std::string &newText);

    [[nodiscard]] vec4 getPadding() const;

    void setPadding(const vec4 &newPadding);


    void draw();
};


#endif //HARMONY_TEXTBUTTON_H
