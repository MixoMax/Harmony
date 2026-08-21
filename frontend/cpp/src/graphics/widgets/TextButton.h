//
// Created by tim on 21.08.26.
//

#ifndef HARMONY_TEXTBUTTON_H
#define HARMONY_TEXTBUTTON_H
#include "FilledButton.h"
#include "../../../BasicCppLibrary/text/Font.h"


class TextButton : public FilledButton {
public:
    Font font{.family = "Roboto", .size = 2, .rotation = 0, .color = {1, 1, 1, 1}, .alignment = Alignment::Center};
    std::string text{};

    void draw();
};


#endif //HARMONY_TEXTBUTTON_H
