#ifndef HARMONY_GRAPHICSMANAGER_H
#define HARMONY_GRAPHICSMANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class GraphicsManager {
    static GraphicsManager *instance;

    int screenWidth;
    int screenHeight;
    double screenRatio;
    GLFWwindow *window = nullptr;

    [[nodiscard]] bool initOpenGL();

    void setScreenSize(int width, int height);

    GraphicsManager();

    ~GraphicsManager();

public:
    static GraphicsManager &getInstance();

    static GraphicsManager *init();

    void start();

    static void cleanup();
};


#endif //HARMONY_GRAPHICSMANAGER_H

