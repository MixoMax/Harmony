#ifndef HARMONY_GRAPHICSMANAGER_H
#define HARMONY_GRAPHICSMANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "pages/Page.h"

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
    Page *currentPage = nullptr;
    Page *nextPage = nullptr;

    static GraphicsManager &getInstance();

    static GraphicsManager *init();

    [[nodiscard]] static GLFWwindow *getWindow();

    [[nodiscard]] double getScreenRatio() const;

    [[nodiscard]] int getScreenWidth() const;

    [[nodiscard]] int getScreenHeight() const;

    void start();

    void setCurrentPage(Page *page);

    static void cleanup();
};


#endif //HARMONY_GRAPHICSMANAGER_H
