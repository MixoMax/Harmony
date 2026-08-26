#include "GraphicsManager.h"

#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <ft2build.h>
#include FT_FREETYPE_H

#include "../settings.h"
#include "../../BasicCppLibrary/text/CharacterManager.h"
#include "../httpUtils/Client.h"
#include "../supporters/InputManager.h"
#include "../../BasicCppLibrary/supporters/interpolation.h"
#include "../supporters/Navigator.h"
#include "meshes/MeshInitializer.h"
#include "pages/Homepage.h"
#include "shaders/ShaderInitializer.h"

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

    CharacterManager::updateScreenSize(static_cast<float>(screenWidth), static_cast<float>(screenHeight));
}

GraphicsManager::GraphicsManager() : screenWidth(100), screenHeight(100), screenRatio(1), mouseX(0), mouseY(0) {
}

GraphicsManager::~GraphicsManager() {
    Navigator::deinitialize();
    glfwDestroyWindow(window);
    glfwTerminate();
}

GraphicsManager &GraphicsManager::getInstance() {
    return *instance;
}

GraphicsManager *GraphicsManager::init() {
    std::cout << "> initializing GraphicsManager" << std::endl;
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

GLFWwindow *GraphicsManager::getWindow() {
    return instance->window;
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

void GraphicsManager::setErrorMessage(const std::string &message) {
    instance->errorMessage = message;
}

void GraphicsManager::start() {
    glfwShowWindow(window);

    Navigator::initialize();

    InputManager::initializeInputManager();
    initializeMeshes();
    initializeShaders();
    CharacterManager::initializeCharacterManager(fontQualityInPixel, fontFamilies);


    okButton.setPosition(0, -200);
    okButton.setText("OK");
    okButton.setOnPressed([this](GLFWwindow *w, int button, const int action, int mods) {
        if (action == GLFW_PRESS) {
            errorMessage = "";
            return true;
        }
        return false;
    });


    Navigator::push<Homepage>();

    double lastUpdate = glfwGetTime();

    while (!glfwWindowShouldClose(window) && running) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);


        glfwGetCursorPos(window, &mouseX, &mouseY);
        mouseX /= screenWidth;
        mouseY /= screenHeight;
        mouseX = mouseX * 2 - 1;
        mouseY = -mouseY * 2 + 1;


        InputManager::update();
        Client::update();
        /*Draw current Page*/
        Navigator::draw();

        if (!errorMessage.empty()) {
            CharacterManager::drawText(errorMessage, errorFont, 0, 0);
            okButton.draw();
        }

        /*FPS counter*/
        const double now = glfwGetTime();
        CharacterManager::drawText(std::to_string(static_cast<int>(std::round(1 / (now - lastUpdate)))), fpsFont,
                                   -static_cast<float>(getScreenWidth()), static_cast<float>(getScreenHeight()));
        lastUpdate = now;

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
