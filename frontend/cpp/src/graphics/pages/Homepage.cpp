//
// Created by tim on 20.08.26.
//

#include "Homepage.h"

#include "../GraphicsManager.h"
#include "../../../BasicCppLibrary/supporters/interpolation.h"
#include "../../../BasicCppLibrary/text/CharacterManager.h"
#include "../../httpUtils/Client.h"
#include "../meshes/RectangularMesh.h"
#include "../widgets/FilledButton.h"

Homepage::Homepage() {
    reloadRooms();

    exitButton.setBorderRadius(100);
    exitButton.setOnPressed([](GLFWwindow *window, const int button, const int action, const int mods) {
        // if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        Client::disconnectFromRoom();
        // }
    });
    exitButton.setText("exit");

    refreshButton.setBorderRadius(100);
    refreshButton.setOnPressed(
        [this](GLFWwindow *window, const int button, const int action,
               const int mods) {
            // if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
            reloadRooms();
            // }
        });
    refreshButton.setText("refresh");
}

Homepage::~Homepage() = default;

void Homepage::draw() {
    Page::draw();

    //Background
    background.useShader();
    glUniform1f(background.getUniform("uTime"), static_cast<float>(glfwGetTime()));
    glUniform2f(background.getUniform("uResolution"), static_cast<float>(getScreenWidth()),
                static_cast<float>(getScreenHeight()));
    RectangularMesh::getInstance()->draw();


    //Rooms
    for (auto &roomButton: roomButtons) {
        roomButton.draw();
    }


    //Exit
    const float exitButtonY = -static_cast<float>(getScreenHeight()) + 200 + 10 + 430;
    exitButton.setPosition(500.f - getScreenWidth(), exitButtonY);
    exitButton.draw();

    //Refresh
    const float refreshButtonY = -static_cast<float>(getScreenHeight()) + 200 + 10;
    refreshButton.setPosition(500.f - getScreenWidth(), refreshButtonY);
    refreshButton.draw();


    audioVisualizer.draw();
}


void Homepage::reloadRooms() {
    rooms = Client::getRooms();
    roomButtons.clear();
    for (int roomIndex = 0; roomIndex < rooms.size(); ++roomIndex) {
        const auto &room = rooms[roomIndex];
        const float buttonY = -roomIndex * spaceBetweenButtons + totalNeededHeight / 2;

        roomButtons.emplace_back();
        TextButton &roomButton = roomButtons.back();
        roomButton.setPosition(0, buttonY);
        roomButton.setBorderRadius(100);
        roomButton.setOnPressed([&room](GLFWwindow *window, const int button, const int action, const int mods) {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                std::cout << "Pressed on connect with room " << room.name << std::endl;
                Client::connectToRoom(room.name, "maxmustermann");
            }
        });


        std::string text = room.name + '(';
        for (int userIndex = 0; userIndex < room.users.size(); userIndex++) {
            const User &user = room.users[userIndex];
            text += user.name;
            if (userIndex < room.users.size() - 1)
                text += ", ";
        }
        text += ')';
        roomButton.setText(text);
    }
}
