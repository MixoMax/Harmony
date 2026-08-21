//
// Created by tim on 21.08.26.
//

#include "TextButton.h"

#include "../../../BasicCppLibrary/text/CharacterManager.h"

void TextButton::draw() {
    FilledButton::draw();
    CharacterManager::drawText(text, font, getX(), getY());
}
