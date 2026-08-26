//
// Created by tim on 25.08.26.
//

#include "CallPage.h"

#include "Homepage.h"
#include "../GraphicsManager.h"
#include "../../httpUtils/Client.h"
#include "../../supporters/Navigator.h"
#include "../meshes/RectangularMesh.h"

void CallPage::reloadUsers() {
    bUsers.clear();
    const float totalNeededHeight = (users.size() - 1) * 20;
    for (int userIndex = 0; userIndex < users.size(); ++userIndex) {
        const auto &room = users[userIndex];
        const float buttonY = -userIndex * 20 + totalNeededHeight / 2;

        TextButton &userButton = bUsers.emplace_back();
        userButton.setPosition(0, buttonY);
        userButton.setBorderRadius(100);

        userButton.setText(users[userIndex].name);
    }
}


CallPage::CallPage() {
    reloadUsers();

    bExit.setOnPressed([](GLFWwindow *window, const int button, const int action, const int mods) {
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            Client::disconnectFromRoom();
            Navigator::pop();
            return true;
        }
        return false;
    });
    bExit.setText("exit");

    Client::joinCallback = [this](const User &user) {
        users.push_back(user);
        reloadUsers();
    };
    Client::leaveCallback = [this](const User &user) {
        std::erase_if(users, [&user](const User &u) {
            if (u.id == user.id)
                return true;
            return false;
        });
        reloadUsers();
    };
}


CallPage::~CallPage() {
    Client::joinCallback = [](const User &) {
    };
    Client::leaveCallback = [](const User &) {
    };
}

void CallPage::draw() {
    Page::draw();
    //Background
    background.useShader();
    glUniform1f(background.getUniform("uTime"), static_cast<float>(glfwGetTime()));
    glUniform2f(background.getUniform("uResolution"), static_cast<float>(getScreenWidth()),
                static_cast<float>(getScreenHeight()));
    RectangularMesh::getInstance()->draw();


    for (int userIndex = 0; userIndex < bUsers.size(); userIndex++) {
        auto &userButton = bUsers[userIndex];
        const float yOffset = userIndex * (bUsers[userIndex].getHeight() * 2 + 20);
        userButton.setPosition(userButton.getWidth() - getScreenWidth(),
                               getScreenHeight() - userButton.getHeight() - yOffset);
        userButton.draw();
    }

    bExit.setPosition(getScreenWidth() - bExit.getWidth() - 20, getScreenHeight() - bExit.getHeight() - 20);
    bExit.draw();

    audioVisualizer.draw();
}
