#include "GraphicsManager.h"

#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../settings.h"
#include "../supporters/InputManager.h"
#include "../supporters/interpolation.h"
#include "meshes/MeshInitializer.h"
#include "shaders/ShaderInitializer.h"
#include "text/CharacterManager.h"

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

GraphicsManager::GraphicsManager() : screenWidth(100), screenHeight(100), screenRatio(1), mouseX(0), mouseY(0) {
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

double GraphicsManager::getScreenRatio() const {
    return screenRatio;
}

int GraphicsManager::getScreenWidth() const {
    return screenWidth;
}

int GraphicsManager::getScreenHeight() const {
    return screenHeight;
}

void GraphicsManager::start() {
    glfwShowWindow(window);

    InputManager::initializeInputManager();
    initializeMeshes();
    initializeShaders();
    CharacterManager::initializeCharacterManager();


    FilledButton button1{};
    button1.setSize(.7, .25);
    button1.setColor(vec4(45 / 255.f, 120 / 255.f, 72 / 255.f, 1));
    button1.setRadius(.1);
    button1.setRotation(.4);
    button1.setOnPressed([](GLFWwindow *window, const int button, const int action, const int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            std::cout << "Hey" << std::endl;
        }
    });

    while (!glfwWindowShouldClose(window) && running) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseX /= screenWidth;
        mouseY /= screenHeight;
        mouseX = mouseX * 2 - 1;
        mouseY = -mouseY * 2 + 1;

        const float rotation = Interpolation::easeInElastic(std::sin(glfwGetTime() * 2.f) * .5 + .5) * .3f;
        button1.setRotation(rotation);
        button1.draw();

        CharacterManager::drawText("Start Harmony Client", 0.0f, 0.0f, .24, rotation, vec4(1, 1, 1, 1),
                                   Alignment::Center);


        glfwSwapBuffers(window);

        glfwPollEvents();
    }
}

void GraphicsManager::cleanup() {
    deleteShaders();
    deleteMeshes();
    CharacterManager::deleteCharacterManager();
    delete instance;
}
