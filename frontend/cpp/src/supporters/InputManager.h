//
// Created by tim on 17.07.26.
//

#ifndef HARMONY_INPUTMANAGER_H
#define HARMONY_INPUTMANAGER_H
#include <functional>
#include <unordered_map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define KEY_NOT_PRESSED = 0;
#define KEY_CLICKED = 1;
#define KEY_PRESSED = 2;
#define KEY_RELEASED = 3;

using KeyCallback = std::function<void(GLFWwindow *w, const int key, const int scancode, const int action,
                                       const int mods)>;

struct KeyCallbackEntry {
    size_t id;
    KeyCallback callback;
};

using MouseButtonCallback = std::function<void(GLFWwindow *window, const int button, const int action, const int mods)>;

struct MouseButtonCallbackEntry {
    size_t id;
    MouseButtonCallback callback;
};

class InputManager {
    static std::vector<KeyCallbackEntry> keyCallbacks;
    static size_t currentKeyCallbackIndex;
    static std::vector<MouseButtonCallbackEntry> mouseButtonCallbacks;
    static size_t currentMouseButtonCallbackIndex;

public:
    InputManager() = delete;

    static void initializeInputManager();

    static size_t addKeyCallback(KeyCallback callback);

    static void removeKeyCallback(size_t id);

    static size_t addMouseButtonCallback(MouseButtonCallback callback);

    static void removeMouseButtonCallback(size_t id);
};


#endif //HARMONY_INPUTMANAGER_H
