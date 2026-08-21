//
// Created by tim on 20.08.26.
//

#ifndef HARMONY_HOMEPAGE_H
#define HARMONY_HOMEPAGE_H
#include <vector>

#include "Page.h"
#include "../../../BasicCppLibrary/text/Font.h"
#include "../../../BasicCppLibrary/visuals/shader.h"
#include "../../httpUtils/Client.h"
#include "../../httpUtils/dataClasses/Room.h"
#include "../widgets/FilledButton.h"


class Homepage : public Page {
    Shader background{"backgroundShader"};

    std::vector<Room> rooms = Client::getRooms();
    std::vector<FilledButton> roomButtons{};

    FilledButton refreshButton{};
    FilledButton exitButton{};

    static constexpr float buttonHeight = 300;
    static constexpr float spaceBetweenButtons = buttonHeight * 2 + 20;
    float totalNeededHeight = (rooms.size() - 1) * spaceBetweenButtons;

    const Font defaultFont{
        .family = "Roboto", .scale = .2f, .rotation = 0, .color = {1, 1, 1, 1}, .alignment = Alignment::Center
    };

public:
    Homepage();

    ~Homepage() override;

    void draw() override;

    void reloadRooms();
};


#endif //HARMONY_HOMEPAGE_H
