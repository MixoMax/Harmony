//
// Created by tim on 17.07.26.
//

#include "Widget.h"

void Widget::setMinWidth(const float newMinWidth) { this->minWidth = std::max(this->minWidth, newMinWidth); }

void Widget::setMaxWidth(const float newMaxWidth) { this->maxWidth = std::min(this->maxWidth, newMaxWidth); }

void Widget::setMinHeight(const float newMinHeight) { this->minHeight = std::max(this->minHeight, newMinHeight); }

void Widget::setMaxHeight(const float newMaxHeight) { this->maxHeight = std::min(this->maxHeight, newMaxHeight); }

float Widget::getMinWidth() const { return this->minWidth; }

float Widget::getMaxWidth() const { return this->maxWidth; }

float Widget::getMinHeight() const { return this->minHeight; }

float Widget::getMaxHeight() const { return this->maxHeight; }
