//
// Created by tim on 21.08.26.
//

#include "TextButton.h"

#include "../../../BasicCppLibrary/text/CharacterManager.h"

void TextButton::setText(const std::string &newText) {
    text = newText;
    updateDimensions();
}

vec4 TextButton::getPadding() const {
    return padding;
}

void TextButton::setPadding(const vec4 &newPadding) {
    padding = newPadding;
    updateDimensions();
}

void TextButton::updateDimensions() {
    if (!text.empty()) {
        const vec2 textDimensions = CharacterManager::getSizeOfText(text, font);
        const float width = textDimensions.x / 2 + padding.x + padding.z;
        const float height = textDimensions.y / 2 + padding.y + padding.w;
        setMinWidth(width);
        setMinHeight(height);
    }
}

TextButton::TextButton() {
    updateDimensions();
}

std::string TextButton::getText() const {
    return text;
}

void TextButton::draw() {
    FilledButton::draw();
    CharacterManager::drawText(text, font, getX() + (padding.x - padding.z), getY() + (padding.w - padding.y));
}
