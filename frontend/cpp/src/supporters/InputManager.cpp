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
std::vector<CharCallbackEntry> InputManager::charCallbacks;
size_t InputManager::currentCharCallbackIndex{0};

std::mutex InputManager::keyCallbackActionMutex{};
std::queue<KeyCallbackAction> InputManager::keyCallbackActionQueue{};
std::mutex InputManager::charCallbackActionMutex{};
std::queue<CharCallbackAction> InputManager::charCallbackActionQueue{};
std::mutex InputManager::mouseButtonCallbackActionMutex{};
std::queue<MouseButtonCallbackAction> InputManager::mouseButtonCallbackActionQueue{};

Widget *InputManager::selectedWidget{nullptr};
bool InputManager::shiftHeld{false};

void InputManager::initializeInputManager() {
    const auto window = GraphicsManager::getInstance().window;
    glfwSetKeyCallback(window,
                       [](GLFWwindow *w, const int key, const int scancode, const int action, const int mods) {
                           if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT) {
                               shiftHeld = action == GLFW_PRESS;
                           }

                           std::lock_guard lock(keyCallbackActionMutex);
                           keyCallbackActionQueue.emplace(w, key, scancode, action, mods);
                       });

    glfwSetCharCallback(window, [](GLFWwindow *w, const unsigned int codepoint) {
        std::lock_guard lock(charCallbackActionMutex);
        charCallbackActionQueue.emplace(w, codepoint);
    });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *w, const int button, const int action, const int mods) {
        if (action == GLFW_PRESS) {
            selectedWidget = nullptr;
        }

        std::lock_guard lock(mouseButtonCallbackActionMutex);
        mouseButtonCallbackActionQueue.emplace(w, button, action, mods);
    });
}

void InputManager::update() {
    if (const std::unique_lock lock(keyCallbackActionMutex, std::try_to_lock); lock.owns_lock()) {
        while (!keyCallbackActionQueue.empty()) {
            auto [window, key, scancode, action, mods] = keyCallbackActionQueue.front();
            keyCallbackActionQueue.pop();
            for (auto &[id, callback]: keyCallbacks) {
                if (callback(window, key, scancode, action, mods)) {
                    break;
                }
            }
        }
    }

    if (const std::unique_lock lock(charCallbackActionMutex, std::try_to_lock); lock.owns_lock()) {
        while (!charCallbackActionQueue.empty()) {
            auto [window, codepoint] = charCallbackActionQueue.front();
            charCallbackActionQueue.pop();
            for (auto &[id, callback]: charCallbacks) {
                if (callback(window, codepoint)) {
                    break;
                }
            }
        }
    }

    if (const std::unique_lock lock(mouseButtonCallbackActionMutex, std::try_to_lock); lock.owns_lock()) {
        while (!mouseButtonCallbackActionQueue.empty()) {
            auto [window, button, action, mods] = mouseButtonCallbackActionQueue.front();
            mouseButtonCallbackActionQueue.pop();
            for (auto &[id, callback]: mouseButtonCallbacks) {
                if (callback(window, button, action, mods)) {
                    break;
                }
            }
        }
    }
}

void InputManager::requestFocus(Widget *widget) {
    selectedWidget = widget;
}

void InputManager::removeFocus(const Widget *widget) {
    if (selectedWidget == widget) {
        selectedWidget = nullptr;
    }
}

void InputManager::removeFocus() {
    selectedWidget = nullptr;
}

bool InputManager::hasFocus(const Widget *widget) {
    return selectedWidget == widget;
}

//Key Callback
size_t InputManager::addKeyCallback(KeyCallback callback) {
    keyCallbacks.push_back({currentKeyCallbackIndex, std::move(callback)});
    return currentKeyCallbackIndex++;
}

void InputManager::removeKeyCallback(size_t id) {
    std::erase_if(keyCallbacks, [id](const KeyCallbackEntry &entry) { return entry.id == id; });
}

//Char Callback
size_t InputManager::addCharCallback(CharCallback callback) {
    charCallbacks.push_back({currentCharCallbackIndex, std::move(callback)});
    return currentCharCallbackIndex++;
}

void InputManager::removeCharCallback(size_t id) {
    std::erase_if(charCallbacks, [id](const CharCallbackEntry &entry) { return entry.id == id; });
}

//Mouse Callback
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
