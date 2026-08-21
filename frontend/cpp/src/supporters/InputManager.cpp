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

void InputManager::initializeInputManager() {
    const auto window = GraphicsManager::getInstance().window;
    glfwSetKeyCallback(window,
                       [](GLFWwindow *w, const int key, const int scancode, const int action, const int mods) {
                           for (auto &[id, callback]: keyCallbacks) {
                               callback(w, key, scancode, action, mods);
                           }
                       });

    glfwSetMouseButtonCallback(window, [](GLFWwindow *w, const int button, const int action, const int mods) {
        for (auto &[id, callback]: mouseButtonCallbacks) {
            callback(w, button, action, mods);
        }
    });
}

size_t InputManager::addKeyCallback(KeyCallback callback) {
    keyCallbacks.push_back({currentKeyCallbackIndex, std::move(callback)});
    return currentKeyCallbackIndex++;
}

void InputManager::removeKeyCallback(size_t id) {
    std::erase_if(keyCallbacks, [id](const KeyCallbackEntry &entry) { return entry.id == id; });
}

size_t InputManager::addMouseButtonCallback(MouseButtonCallback callback) {
    mouseButtonCallbacks.push_back({currentMouseButtonCallbackIndex, std::move(callback)});
    return currentMouseButtonCallbackIndex++;
}

void InputManager::removeMouseButtonCallback(size_t id) {
    std::erase_if(mouseButtonCallbacks, [id](const MouseButtonCallbackEntry &entry) { return entry.id == id; });
}
