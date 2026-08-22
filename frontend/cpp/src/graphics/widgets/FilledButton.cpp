//
// Created by tim on 17.07.26.
//

#include "FilledButton.h"

#include "../GraphicsManager.h"
#include "../../supporters/InputManager.h"
#include "../meshes/RectangularMesh.h"

Shader *FilledButton::shader = nullptr;

bool FilledButton::isHovered(const float mouseX, const float mouseY) const {
    return Rectangle::isHovered(
        mouseX, mouseY, position.x, position.y, getWidth(), getHeight(), borderRadius, rotation);
}

FilledButton::~FilledButton() {
    deleteOnPressed();
}


void FilledButton::setColor(const vec4 newColor) {
    this->color = newColor;
}

void FilledButton::setPosition(const float x, const float y) {
    position.x = x;
    position.y = y;
}

vec2 FilledButton::getPosition() const {
    return position;
}

float FilledButton::getX() const {
    return position.x;
}

float FilledButton::getY() const {
    return position.y;
}


void FilledButton::setBorderRadius(const float newBorderRadius) {
    this->borderRadius = newBorderRadius;
}

void FilledButton::setOnPressed(const MouseButtonCallback &onPressed) {
    deleteOnPressed();
    onPressedId = InputManager::addMouseButtonCallback(
        [this, onPressed](GLFWwindow *window, const int button, const int action, const int mods) {
            if (const auto &graphicsManager = GraphicsManager::getInstance();
                isHovered(
                    static_cast<float>(graphicsManager.mouseX),
                    static_cast<float>(graphicsManager.mouseY))) {
                onPressed(window, button, action, mods);
            }
        });
}

void FilledButton::deleteOnPressed() {
    if (onPressedId != -1) {
        InputManager::removeMouseButtonCallback(onPressedId);
        onPressedId = -1;
    }
}

void FilledButton::draw() {
    if (const RectangularMesh *mesh = RectangularMesh::getInstance(); mesh != nullptr) {
        shader->useShader();
        const auto &graphicsManager = GraphicsManager::getInstance();
        if (isHovered(static_cast<float>(graphicsManager.mouseX), static_cast<float>(graphicsManager.mouseY))
            && InputManager::isMousePressed()) {
            glUniform1i(shader->getUniform("uClick"), 1);
            lastClickTime = glfwGetTime();
            lastClickPosition = {
                static_cast<float>(graphicsManager.mouseX), static_cast<float>(graphicsManager.mouseY)
            };
        } else {
            glUniform1i(shader->getUniform("uClick"), 0);
        }
        glUniform2f(shader->getUniform("uResolution"), static_cast<float>(graphicsManager.getScreenWidth()),
                    static_cast<float>(graphicsManager.getScreenHeight()));
        glUniform2f(shader->getUniform("uSize"), getWidth(), getHeight());
        glUniform1f(shader->getUniform("uRadius"), borderRadius);
        glUniform1f(shader->getUniform("uRotation"), rotation);
        glUniform2f(shader->getUniform("uPosition"), position.x, position.y);
        glUniform4f(shader->getUniform("uColor"), color.x, color.y, color.z, color.w);
        glUniform1f(shader->getUniform("uTime"), static_cast<float>(glfwGetTime()));
        glUniform2f(shader->getUniform("uMouse"), static_cast<float>(graphicsManager.mouseX),
                    static_cast<float>(graphicsManager.mouseY));
        glUniform2f(shader->getUniform("uLastClickPosition"), lastClickPosition.x, lastClickPosition.y);
        glUniform1f(shader->getUniform("uTimeSinceClick"), static_cast<float>(glfwGetTime() - lastClickTime));

        mesh->draw();
    }
}

void FilledButton::initializeShaders() {
    shader = new Shader("filledButtonShader");
}

void FilledButton::deleteShaders() {
    delete shader;
}
