//
// Created by tim on 20.08.26.
//

#include "Page.h"

#include "../GraphicsManager.h"

double Page::getScreenRatio() {
    return GraphicsManager::getInstance().getScreenRatio();
}

int Page::getScreenWidth() {
    return GraphicsManager::getInstance().getScreenWidth();
}

int Page::getScreenHeight() {
    return GraphicsManager::getInstance().getScreenHeight();
}

void Page::draw() {
}
