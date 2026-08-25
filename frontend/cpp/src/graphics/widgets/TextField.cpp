//
// Created by tim on 25.08.26.
//

#include "TextField.h"

#include "../GraphicsManager.h"
#include "../../../BasicCppLibrary/text/CharacterManager.h"
#include "../../supporters/InputManager.h"
#include "../basicShapes/Rectangle.h"
#include "../meshes/RectangularMesh.h"

Shader *TextField::shader = nullptr;

bool TextField::isHovered(const float mouseX, const float mouseY) const {
    return Rectangle::isHovered(
        mouseX, mouseY, position.x, position.y, getWidth(), getHeight(), borderRadius, rotation);
}


TextField::TextField() {
    clickCallbackId = InputManager::addMouseButtonCallback(
        [this](GLFWwindow *window, int button, const int action, int mods) {
            if (action == GLFW_PRESS && isHovered(GraphicsManager::getInstance().mouseX,
                                                  GraphicsManager::getInstance().mouseY)) {
                InputManager::requestFocus(this);
            }
        });

    textCallbackId = InputManager::addKeyCallback(
        [this](GLFWwindow *window, const int key, int scancode, const int action, int mods) {
            if (!InputManager::hasFocus(this)) {
                return;
            }
            if (action == GLFW_PRESS) {
                if (key == GLFW_KEY_BACKSPACE) {
                    if (!text.empty()) {
                        text.pop_back();
                    }
                    return;
                }

                if (key == GLFW_KEY_SPACE) {
                    text.emplace_back(' ');
                    return;
                }

                int character = key - GLFW_KEY_A + 97;

                if (character < 'a' || character > 'z') {
                    int number = key - GLFW_KEY_0 + 48;
                    if (number < '0' || number > '9') {
                        return;
                    }

                    if (InputManager::shiftHeld) {
                        char specialCharacters[] = {'=', '!', '"', ' ', '$', '%', '&', '/', '(', ')'};
                        text.emplace_back(specialCharacters[number - 48]);
                        return;
                    }

                    text.emplace_back(number);
                    return;
                }

                if (InputManager::shiftHeld) {
                    character -= 32;
                }

                text.emplace_back(static_cast<char>(character));
            }
        });
}

TextField::~TextField() {
    InputManager::removeMouseButtonCallback(clickCallbackId);
    InputManager::removeKeyCallback(textCallbackId);
}

void TextField::draw() {
    shader->useShader();
    const auto &graphicsManager = GraphicsManager::getInstance();
    if (isHovered(static_cast<float>(graphicsManager.mouseX), static_cast<float>(graphicsManager.mouseY))
        && InputManager::isMousePressed()) {
        lastClickTime = glfwGetTime();
        lastClickPosition = {
            static_cast<float>(graphicsManager.mouseX), static_cast<float>(graphicsManager.mouseY)
        };
    }

    glUniform2f(shader->getUniform("uResolution"), static_cast<float>(graphicsManager.getScreenWidth()),
                static_cast<float>(graphicsManager.getScreenHeight()));
    glUniform2f(shader->getUniform("uSize"), getWidth(), getHeight());
    glUniform1f(shader->getUniform("uRadius"), borderRadius);
    glUniform1f(shader->getUniform("uRotation"), rotation);
    glUniform2f(shader->getUniform("uPosition"), position.x, position.y);
    glUniform1f(shader->getUniform("uTime"), static_cast<float>(glfwGetTime()));
    glUniform2f(shader->getUniform("uMouse"), static_cast<float>(graphicsManager.mouseX),
                static_cast<float>(graphicsManager.mouseY));
    glUniform2f(shader->getUniform("uLastClickPosition"), lastClickPosition.x, lastClickPosition.y);
    glUniform1f(shader->getUniform("uTimeSinceClick"), static_cast<float>(glfwGetTime() - lastClickTime));


    RectangularMesh::getInstance()->draw();

    CharacterManager::drawText(std::string(text.begin(), text.end()), font, position.x, position.y);
}

void TextField::setPosition(const float x, const float y) {
    position.x = x;
    position.y = y;
}

std::string TextField::getText() {
    return std::string(text.begin(), text.end());
}

void TextField::initializeShaders() {
    shader = new Shader("textField");
    shader->dumpCompilationData();
}

void TextField::deleteShaders() {
    delete shader;
}
