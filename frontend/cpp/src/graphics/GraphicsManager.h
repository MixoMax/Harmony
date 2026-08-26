#ifndef HARMONY_GRAPHICSMANAGER_H
#define HARMONY_GRAPHICSMANAGER_H

#include <string>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "../../BasicCppLibrary/text/Font.h"
#include "widgets/TextButton.h"


class GraphicsManager {
    static GraphicsManager *instance;

    int screenWidth;
    int screenHeight;
    double screenRatio;


    Font fpsFont{.family = "Roboto", .alignment = Alignment::TopLeft};

    std::string errorMessage{};
    Font errorFont{.family = "Cause", .color = vec4(0.6314, 0.1216, 0.1216, 1)};
    TextButton okButton{};


    [[nodiscard]] bool initOpenGL();

    void setScreenSize(int width, int height);

    GraphicsManager();

    ~GraphicsManager();

public:
    GLFWwindow *window = nullptr;
    double mouseX, mouseY;

    static GraphicsManager &getInstance();

    static GraphicsManager *init();

    [[nodiscard]] static GLFWwindow *getWindow();

    [[nodiscard]] double getScreenRatio() const;

    [[nodiscard]] int getScreenWidth() const;

    [[nodiscard]] int getScreenHeight() const;

    static void setErrorMessage(const std::string &message);

    void start();

    static void cleanup();
};


#endif //HARMONY_GRAPHICSMANAGER_H
