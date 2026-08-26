//
// Created by tim on 17.07.26.
//

#ifndef HARMONY_INPUTMANAGER_H
#define HARMONY_INPUTMANAGER_H
#include <functional>
#include <mutex>
#include <queue>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../graphics/widgets/Widget.h"

#define KEY_NOT_PRESSED = 0;
#define KEY_CLICKED = 1;
#define KEY_PRESSED = 2;
#define KEY_RELEASED = 3;

using KeyCallback = std::function<bool(GLFWwindow *window, int key, int scancode, int action, int mods)>;

struct KeyCallbackEntry {
    size_t id;
    KeyCallback callback;
};

struct KeyCallbackAction {
    GLFWwindow *window;
    int key;
    int scancode;
    int action;
    int mods;
};

using CharCallback = std::function<bool(GLFWwindow *window, unsigned int codepoint)>;

struct CharCallbackEntry {
    size_t id;
    CharCallback callback;
};

struct CharCallbackAction {
    GLFWwindow *window;
    unsigned int codepoint;
};

using MouseButtonCallback = std::function<bool(GLFWwindow *window, int button, int action, int mods)>;

struct MouseButtonCallbackEntry {
    size_t id;
    MouseButtonCallback callback;
};

struct MouseButtonCallbackAction {
    GLFWwindow *window;
    int button;
    int action;
    int mods;
};

class InputManager {
    //keyboard & mouse input
    static std::vector<KeyCallbackEntry> keyCallbacks;
    static size_t currentKeyCallbackIndex;
    static std::vector<CharCallbackEntry> charCallbacks;
    static size_t currentCharCallbackIndex;
    static std::vector<MouseButtonCallbackEntry> mouseButtonCallbacks;
    static size_t currentMouseButtonCallbackIndex;

    static std::mutex keyCallbackActionMutex;
    static std::queue<KeyCallbackAction> keyCallbackActionQueue;
    static std::mutex charCallbackActionMutex;
    static std::queue<CharCallbackAction> charCallbackActionQueue;
    static std::mutex mouseButtonCallbackActionMutex;
    static std::queue<MouseButtonCallbackAction> mouseButtonCallbackActionQueue;

    //custom callbacks


    //other
    static Widget *selectedWidget;

public:
    static bool shiftHeld;

    InputManager() = delete;

    static void initializeInputManager();

    static void update();


    static void requestFocus(Widget *widget);

    static void removeFocus(const Widget *widget);

    static void removeFocus();

    static bool hasFocus(const Widget *widget);

    static size_t addKeyCallback(KeyCallback callback);

    static void removeKeyCallback(size_t id);

    static size_t addCharCallback(CharCallback callback);

    static void removeCharCallback(size_t id);

    static size_t addMouseButtonCallback(MouseButtonCallback callback);

    static void removeMouseButtonCallback(size_t id);

    static bool isMousePressed();
};


#endif //HARMONY_INPUTMANAGER_H
