#ifndef HARMONY_GRAPHICSMANAGER_H
#define HARMONY_GRAPHICSMANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

class GraphicsManager {
    static GraphicsManager *instance;

    int screenWidth;
    int screenHeight;
    double screenRatio;


    [[nodiscard]] bool initOpenGL();

    void setScreenSize(int width, int height);

    GraphicsManager();

    ~GraphicsManager();

public:
    GLFWwindow *window = nullptr;
    double mouseX, mouseY;

    static GraphicsManager &getInstance();

    static GraphicsManager *init();

    [[nodiscard]] double getScreenRatio() const;

    void start();

    static void cleanup();
};


#endif //HARMONY_GRAPHICSMANAGER_H

