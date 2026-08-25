//
// Created by tim on 17.07.26.
//

#include "Widget.h"

void Widget::setMinWidth(const float newMinWidth) { this->minWidth = std::max(this->minWidth, newMinWidth); }

void Widget::setMaxWidth(const float newMaxWidth) { this->maxWidth = std::min(this->maxWidth, newMaxWidth); }

void Widget::setMinHeight(const float newMinHeight) { this->minHeight = std::max(this->minHeight, newMinHeight); }

void Widget::setMaxHeight(const float newMaxHeight) { this->maxHeight = std::min(this->maxHeight, newMaxHeight); }

void Widget::setMinSize(const float newMinWidth, const float newMinHeight) {
    minWidth = newMinWidth;
    minHeight = newMinHeight;
}

void Widget::setSize(const float newWidth, const float newHeight) {
    width = newWidth;
    height = newHeight;
}

void Widget::setWidth(const float newWidth) {
    this->width = newWidth;
}

void Widget::setHeight(const float newHeight) {
    this->height = newHeight;
}

void Widget::setRotation(const float newRotation) {
    this->rotation = newRotation;
}

float Widget::getMinWidth() const { return this->minWidth; }

float Widget::getMaxWidth() const { return this->maxWidth; }

float Widget::getMinHeight() const { return this->minHeight; }

float Widget::getMaxHeight() const { return this->maxHeight; }

float Widget::getWidth() const { return this->width == -1 ? this->minWidth : this->width; }

float Widget::getHeight() const { return this->height == -1 ? this->minHeight : this->height; }

float Widget::getRotation() const { return this->rotation; }
