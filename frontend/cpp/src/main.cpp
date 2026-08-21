#include <atomic>
#include <filesystem>
#include <fstream>
#include <iostream>
#include "settings.h"
#include "graphics/GraphicsManager.h"
#include "voiceChannels/VoiceChannelManager.h"

int main(const int argc, const char *argv[]) {
    auto t1 = std::chrono::high_resolution_clock::now();
    std::cout << "> Starting Harmony..." << std::endl;
    std::cout << "> Working Directory: " << std::filesystem::current_path() << std::endl;

    running = true;

    {
        if (GraphicsManager *graphicsManager = GraphicsManager::init()) {
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
            std::cout << "Startup took " << duration << " ms" << std::endl;
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
