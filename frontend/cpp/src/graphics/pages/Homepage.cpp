//
// Created by tim on 20.08.26.
//

#include "Homepage.h"

#include "CallPage.h"
#include "../GraphicsManager.h"
#include "../../../BasicCppLibrary/text/CharacterManager.h"
#include "../../httpUtils/Client.h"
#include "../../supporters/Navigator.h"
#include "../meshes/RectangularMesh.h"
#include "../widgets/FilledButton.h"

Homepage::Homepage() {
    reloadRooms();


    bRefreshRooms.setOnPressed(
        [this](GLFWwindow *window, const int button, const int action,
               const int mods) {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                reloadRooms();
                return true;
            }
            return false;
        });
    bRefreshRooms.setText("refresh");

    bCreateRoom.setOnPressed([this](GLFWwindow *window, const int button, const int action,
                                    const int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            const std::string roomName = tfRoomName.getText();
            const std::string userName = tfUserName.getText();
            if (userName.empty() || roomName.empty()) {
                std::cerr << "> Username or room name is empty!" << std::endl;
                GraphicsManager::setErrorMessage("Username or room name is empty!");
                return true;
            }

            std::cout << "Pressed on connect with room " << roomName << std::endl;
            Client::connectToRoom(roomName, userName);
            Navigator::push<CallPage>();
            return true;
        }
        return false;
    });
    bCreateRoom.setText("create room");

    tfUserName.setMinSize(400, 100);
    tfRoomName.setMinSize(400, 100);
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
    for (int roomIndex = 0; roomIndex < bRooms.size(); roomIndex++) {
        auto &roomButton = bRooms[roomIndex];
        const float yOffset = roomIndex * (bRooms[roomIndex].getHeight() * 2 + 20);
        roomButton.setPosition(roomButton.getWidth() - getScreenWidth(),
                               getScreenHeight() - roomButton.getHeight() - bRefreshRooms.getHeight() * 2 - 40 -
                               yOffset);
        roomButton.draw();
    }

    //Refresh
    bRefreshRooms.setPosition(500.f - getScreenWidth(), getScreenHeight() - bRefreshRooms.getHeight());
    bRefreshRooms.draw();


    tfRoomName.setPosition(tfRoomName.getWidth() - getScreenWidth(),
                           tfRoomName.getHeight() - getScreenHeight() + bCreateRoom.getHeight() * 2);
    tfRoomName.draw();
    bCreateRoom.setPosition(bCreateRoom.getWidth() - getScreenWidth(), bCreateRoom.getHeight() - getScreenHeight());
    bCreateRoom.draw();

    tfUserName.setPosition(getScreenWidth() - tfUserName.getWidth(), tfUserName.getHeight() - getScreenHeight());
    tfUserName.draw();
}


void Homepage::reloadRooms() {
    rooms = Client::getRooms();
    bRooms.clear();
    const float totalNeededHeight = (rooms.size() - 1) * spaceBetweenButtons;
    for (int roomIndex = 0; roomIndex < rooms.size(); ++roomIndex) {
        const auto &room = rooms[roomIndex];
        const float buttonY = -roomIndex * spaceBetweenButtons + totalNeededHeight / 2;

        TextButton &roomButton = bRooms.emplace_back();
        roomButton.setPosition(0, buttonY);
        roomButton.setOnPressed([&room, this](GLFWwindow *window, const int button, const int action, const int mods) {
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                if (tfUserName.getText().empty()) {
                    std::cerr << "> Username is empty!" << std::endl;
                    GraphicsManager::setErrorMessage("Username is empty!");
                    return true;
                }

                std::cout << "Pressed on connect with room " << room.name << std::endl;
                Client::connectToRoom(room.name, tfUserName.getText());
                Navigator::push<CallPage>();
                return true;
            }
            return false;
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
