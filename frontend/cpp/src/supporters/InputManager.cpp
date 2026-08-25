//
// Created by tim on 17.07.26.
//

#include "InputManager.h"

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <utility>

#include "../graphics/GraphicsManager.h"

std::vector<KeyCallbackEntry> InputManager::keyCallbacks;
size_t InputManager::currentKeyCallbackIndex{0};
std::vector<MouseButtonCallbackEntry> InputManager::mouseButtonCallbacks;
size_t InputManager::currentMouseButtonCallbackIndex{0};

Widget *InputManager::selectedWidget{nullptr};
bool InputManager::shiftHeld{false};

void InputManager::initializeInputManager() {
    const auto window = GraphicsManager::getInstance().window;
    glfwSetKeyCallback(window,
                       [](GLFWwindow *w, const int key, const int scancode, const int action, const int mods) {
                           if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
                               shiftHeld = action == GLFW_PRESS;
                           }

                           for (auto &[id, callback]: keyCallbacks) {
                               callback(w, key, scancode, action, mods);
                           }
                       });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *w, const int button, const int action, const int mods) {
        if (action == GLFW_PRESS) {
            selectedWidget = nullptr;
        }

        for (auto &[id, callback]: mouseButtonCallbacks) {
            callback(w, button, action, mods);
        }
    });
}

void InputManager::requestFocus(Widget *widget) {
    selectedWidget = widget;
}

bool InputManager::hasFocus(const Widget *widget) {
    return selectedWidget == widget;
}

size_t InputManager::addKeyCallback(KeyCallback callback) {
    keyCallbacks.push_back({currentKeyCallbackIndex, std::move(callback)});
    return currentKeyCallbackIndex++;
}

void InputManager::removeKeyCallback(size_t id) {
    std::erase_if(keyCallbacks, [id](const KeyCallbackEntry &entry) { return entry.id == id; });
}

size_t InputManager::addMouseButtonCallback(MouseButtonCallback callback) {
    mouseButtonCallbacks.push_back({.id = currentMouseButtonCallbackIndex, .callback = std::move(callback)});
    return currentMouseButtonCallbackIndex++;
}

void InputManager::removeMouseButtonCallback(size_t id) {
    std::erase_if(mouseButtonCallbacks, [id](const MouseButtonCallbackEntry &entry) { return entry.id == id; });
}

bool InputManager::isMousePressed() {
    return glfwGetMouseButton(GraphicsManager::getInstance().window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS
           || glfwGetMouseButton(GraphicsManager::getInstance().window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS
           || glfwGetMouseButton(GraphicsManager::getInstance().window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
}
