//
// Created by tim on 20.08.26.
//

#include "Homepage.h"

#include "../GraphicsManager.h"
#include "../../../BasicCppLibrary/supporters/interpolation.h"
#include "../../httpUtils/Client.h"
#include "../meshes/RectangularMesh.h"
#include "../text/CharacterManager.h"
#include "../widgets/FilledButton.h"

Homepage::Homepage() {
    reloadRooms();


    exitButton.setSize(500, 200);
    exitButton.setColor(vec4(0, 0, 0, 1));
    exitButton.setRadius(100);
    exitButton.setRotation(0);
    exitButton.setOnPressed([](GLFWwindow *window, const int button, const int action, const int mods) {
        // if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        Client::disconnectFromRoom();
        // }
    });


    refreshButton.setSize(500, 200);
    refreshButton.setColor(vec4(0, 0, 0, 1));
    refreshButton.setRadius(100);
    refreshButton.setRotation(0);
    refreshButton.setOnPressed(
        [this](GLFWwindow *window, const int button, const int action,
               const int mods) {
            // if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            reloadRooms();
            // }
        });
}

Homepage::~Homepage() {
}

void Homepage::draw() {
    Page::draw();

    //Background
    background.useShader();
    glUniform1f(background.getUniform("uTime"), glfwGetTime());
    glUniform2f(background.getUniform("uResolution"), static_cast<float>(getScreenWidth()),
                static_cast<float>(getScreenHeight()));
    RectangularMesh::getInstance()->draw();


    //Rooms
    for (int roomIndex = 0; roomIndex < roomButtons.size(); roomIndex++) {
        const Room &room = rooms[roomIndex];
        std::string text = room.name + '(';
        for (int userIndex = 0; userIndex < room.users.size(); userIndex++) {
            const User &user = room.users[userIndex];
            text += user.name;
            if (userIndex < room.users.size() - 1)
                text += ", ";
        }
        text += ')';

        auto &roomButton = roomButtons[roomIndex];

        roomButton.draw(GraphicsManager::getWindow());

        CharacterManager::drawText(text, 0, roomButton.getY(), .2, 0, vec4(1, 1, 1, 1),
                                   Alignment::Center);
    }


    //Exit
    const float exitButtonY = -static_cast<float>(getScreenHeight()) + 200 + 10 + 430;
    exitButton.setPosition(-getScreenWidth() + 500, exitButtonY);
    exitButton.draw(GraphicsManager::getWindow());
    CharacterManager::drawText("exit", -getScreenWidth() + 500, exitButtonY, .2, 0, vec4(1, 1, 1, 1),
                               Alignment::Center);


    //Refresh
    const float refreshButtonY = -static_cast<float>(getScreenHeight()) + 200 + 10;
    refreshButton.setPosition(-getScreenWidth() + 500, refreshButtonY);
    refreshButton.draw(GraphicsManager::getWindow());
    CharacterManager::drawText("refresh", -getScreenWidth() + 500, refreshButtonY, .2, 0, vec4(1, 1, 1, 1),
                               Alignment::Center);
}


void Homepage::reloadRooms() {
    rooms = Client::getRooms();
    roomButtons.clear();
    for (int roomIndex = 0; roomIndex < rooms.size(); ++roomIndex) {
        const auto &room = rooms[roomIndex];
        const float buttonY = -roomIndex * spaceBetweenButtons + totalNeededHeight / 2;

        roomButtons.emplace_back();
        FilledButton &roomButton = roomButtons.back();
        roomButton.setSize(1400, buttonHeight);
        roomButton.setPosition(0, buttonY);
        roomButton.setColor(vec4(0.8353, 0.0784, 0.7882, 1));
        roomButton.setRadius(100);
        roomButton.setRotation(0);
        roomButton.setOnPressed([&room](GLFWwindow *window, const int button, const int action, const int mods) {
            // if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            std::cout << "Test" << std::endl;
            Client::connectToRoom(room.name, "maxmustermann");
            // }
        });
    }
}
