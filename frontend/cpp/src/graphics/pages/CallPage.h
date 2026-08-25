//
// Created by tim on 25.08.26.
//

#ifndef HARMONY_CALLPAGE_H
#define HARMONY_CALLPAGE_H
#include "Page.h"
#include "../../../BasicCppLibrary/text/Font.h"
#include "../../../BasicCppLibrary/visuals/shader.h"
#include "../../httpUtils/dataClasses/User.h"
#include "../../voiceChannels/graphics/AudioVisualizer.h"
#include "../widgets/TextButton.h"


class CallPage : public Page {
    Shader background{"backgroundShader"};

    std::vector<User> users{};

    std::vector<TextButton> bUsers{};
    TextButton bExit{};

    AudioVisualizer audioVisualizer{};

    const Font defaultFont{
        .family = "Cause", .size = 2, .rotation = 0, .color = {1, 1, 1, 1}, .alignment = Alignment::Center
    };

    void reloadUsers();

public:
    CallPage();

    ~CallPage() override;

    void draw() override;
};


#endif //HARMONY_CALLPAGE_H
