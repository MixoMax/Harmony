#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "settings.h"
#include "graphics/GraphicsManager.h"
#include "voiceChannels/VoiceChannelManager.h"

int main(const int argc, const char *argv[]) {
    std::cout << "> Starting Harmony..." << std::endl;
    std::cout << "> Working Directory: " << std::filesystem::current_path() << std::endl;

    running = true;

    {
        if (GraphicsManager *graphicsManager = GraphicsManager::init()) {
            graphicsManager->start();
        }

        GraphicsManager::cleanup();


        std::cout
                <<
                "> Stopping Harmony..." << std::endl;
        running = false;
        VoiceChannelManager::join();
    }


    std::cout << "> stopped successfully" << std::endl;

    return 0;
}
