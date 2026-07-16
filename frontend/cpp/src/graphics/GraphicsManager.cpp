#include "GraphicsManager.h"

#include <algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "../settings.h"

GraphicsManager *GraphicsManager::instance = nullptr;

bool GraphicsManager::initOpenGL() {
    if (instance == nullptr) {
        return false;
    }
    /* Initialize the library */
    if (!glfwInit()) {
        std::cerr << "Failed: GLFW init" << std::endl;
        return false;
    }

    /*Error Handling*/
    glfwSetErrorCallback([](const int error, const char *description) {
        std::cerr << "Unknown ERROR in GLFW (Code " << error << ") with description" << std::endl;
        std::cerr << description << std::endl;
    });

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    const GLFWvidmode videoMode = *glfwGetVideoMode(glfwGetPrimaryMonitor());
    screenWidth = videoMode.width;
    screenHeight = videoMode.height;

    instance->window = glfwCreateWindow(instance->screenWidth, instance->screenHeight, "Harmony", nullptr, nullptr);

    if (!instance->window) {
        std::cerr << "Failed: Create Window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwSetWindowSizeLimits(window, 200, 100, GLFW_DONT_CARE, GLFW_DONT_CARE);
    glfwMaximizeWindow(instance->window);
    glfwMakeContextCurrent(instance->window);
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cerr << "Failed: Initializing GLAD" << std::endl;
        return false;
    }

    // VSync
    glfwSwapInterval(1);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0., 0., 0., 1);

    return true;
}

void GraphicsManager::setScreenSize(const int width, const int height) {
    screenWidth = width;
    screenHeight = height;
    screenRatio = static_cast<double>(screenWidth) / screenHeight;
    glViewport(0, 0, screenWidth, screenHeight);
}

GraphicsManager::GraphicsManager() : screenWidth(100), screenHeight(100), screenRatio(1) {
}

GraphicsManager::~GraphicsManager() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

GraphicsManager &GraphicsManager::getInstance() {
    return *instance;
}

GraphicsManager *GraphicsManager::init() {
    if (instance != nullptr) {
        std::cerr << "Failed: Graphics Manager already initialized" << std::endl;
        return nullptr;
    }
    instance = new GraphicsManager();

    if (!instance->initOpenGL()) {
        return nullptr;
    }


    glfwSetFramebufferSizeCallback(instance->window, [](GLFWwindow *window, const int width, const int height) {
        if (instance != nullptr) {
            instance->setScreenSize(width, height);
        }
    });

    return instance;
}

void GraphicsManager::start() {
    glfwShowWindow(window);

    while (!glfwWindowShouldClose(window) && running) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        glfwSwapBuffers(window);

        glfwPollEvents();
    }
}

void GraphicsManager::cleanup() {
    delete instance;
}
