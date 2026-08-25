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
#include "../../voiceChannels/graphics/AudioVisualizer.h"
#include "../widgets/TextButton.h"
#include "../widgets/TextField.h"


class Homepage : public Page {
    Shader background{"backgroundShader"};

    std::vector<Room> rooms = Client::getRooms();

    TextButton bRefreshRooms{};
    std::vector<TextButton> bRooms{};
    TextButton bCreateRoom{};
    TextField tfUserName{};
    TextField tfRoomName{};

    static constexpr float buttonHeight = 300;
    static constexpr float spaceBetweenButtons = buttonHeight * 2 + 20;


    const Font defaultFont{
        .family = "Cause", .size = 2, .rotation = 0, .color = {1, 1, 1, 1}, .alignment = Alignment::Center
    };

public:
    Homepage();

    ~Homepage() override;

    void draw() override;

    void reloadRooms();
};


#endif //HARMONY_HOMEPAGE_H
