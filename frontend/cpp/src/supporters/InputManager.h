//
// Created by tim on 17.07.26.
//

#ifndef HARMONY_INPUTMANAGER_H
#define HARMONY_INPUTMANAGER_H
#include <functional>
#include <unordered_map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../graphics/widgets/Widget.h"

#define KEY_NOT_PRESSED = 0;
#define KEY_CLICKED = 1;
#define KEY_PRESSED = 2;
#define KEY_RELEASED = 3;

using KeyCallback = std::function<void(GLFWwindow *w, int key, int scancode, int action, int mods)>;

struct KeyCallbackEntry {
    size_t id;
    KeyCallback callback;
};

using MouseButtonCallback = std::function<void(GLFWwindow *window, int button, int action, int mods)>;

struct MouseButtonCallbackEntry {
    size_t id;
    MouseButtonCallback callback;
};

class InputManager {
    static std::vector<KeyCallbackEntry> keyCallbacks;
    static size_t currentKeyCallbackIndex;
    static std::vector<MouseButtonCallbackEntry> mouseButtonCallbacks;
    static size_t currentMouseButtonCallbackIndex;


    static Widget *selectedWidget;

public:
    static bool shiftHeld;

    InputManager() = delete;

    static void initializeInputManager();


    static void requestFocus(Widget *widget);

    static bool hasFocus(const Widget *widget);

    static size_t addKeyCallback(KeyCallback callback);

    static void removeKeyCallback(size_t id);

    static size_t addMouseButtonCallback(MouseButtonCallback callback);

    static void removeMouseButtonCallback(size_t id);

    static bool isMousePressed();
};


#endif //HARMONY_INPUTMANAGER_H
