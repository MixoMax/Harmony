//
// Created by tim on 17.07.26.
//

#include "FilledButton.h"

#include <utility>

#include "../GraphicsManager.h"
#include "../../supporters/InputManager.h"
#include "../meshes/RectangularMesh.h"

Shader *FilledButton::shader = nullptr;

FilledButton::~FilledButton() {
    deleteOnPressed();
}

void FilledButton::setSize(const float width, const float height) {
    rectangle.width = width;
    rectangle.height = height;
}

void FilledButton::setColor(const vec4 newColor) {
    this->color = newColor;
}

void FilledButton::setPosition(const float x, const float y) {
    rectangle.x = x;
    rectangle.y = y;
}

vec2 FilledButton::getPosition() const {
    return {rectangle.x, rectangle.y};
}

float FilledButton::getX() const {
    return rectangle.x;
}

float FilledButton::getY() const {
    return rectangle.y;
}


void FilledButton::setRotation(const float rotation) {
    rectangle.rotation = rotation;
}

void FilledButton::setRadius(const float radius) {
    rectangle.radius = radius;
}

void FilledButton::setOnPressed(const MouseButtonCallback &onPressed) {
    deleteOnPressed();
    onPressedId = InputManager::addMouseButtonCallback(
        [this, onPressed](GLFWwindow *window, const int button, const int action, const int mods) {
            if (const auto &graphicsManager = GraphicsManager::getInstance();
                rectangle.isHovered(
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
        if (rectangle.isHovered(
                static_cast<float>(graphicsManager.mouseX),
                static_cast<float>(graphicsManager.mouseY)) && glfwGetMouseButton(
                GraphicsManager::getWindow(), GLFW_MOUSE_BUTTON_LEFT) ==
            GLFW_PRESS) {
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
        glUniform2f(shader->getUniform("uSize"), rectangle.width, rectangle.height);
        glUniform1f(shader->getUniform("uRadius"), rectangle.radius);
        glUniform1f(shader->getUniform("uRotation"), rectangle.rotation);
        glUniform2f(shader->getUniform("uPosition"), rectangle.x, rectangle.y);
        glUniform4f(shader->getUniform("uColor"), color.x, color.y, color.z, color.w);
        glUniform1f(shader->getUniform("uTime"), glfwGetTime());
        glUniform2f(shader->getUniform("uMouse"), static_cast<float>(graphicsManager.mouseX),
                    static_cast<float>(graphicsManager.mouseY));
        glUniform2f(shader->getUniform("uLastClickPosition"), lastClickPosition.x, lastClickPosition.y);
        glUniform1f(shader->getUniform("uTimeSinceClick"), glfwGetTime() - lastClickTime);
        // std::cout << glfwGetTime() - lastClickTime << std::endl;

        mesh->draw();
    }
}

void FilledButton::initializeShaders() {
    shader = new Shader("filledButtonShader");
}

void FilledButton::deleteShaders() {
    delete shader;
}
