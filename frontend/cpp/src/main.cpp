#include <atomic>
#include <fstream>
#include <pulse/simple.h>
#include <pulse/error.h>
#include <ixwebsocket/IXWebSocket.h>
#include <iostream>
#include <thread>
#include <vector>
#include <sys/stat.h>

#include "settings.h"
#include "voiceChannels/VoiceChannelManager.h"

int main(const int argc, const char *argv[]) {
    std::cout << "> Starting Harmony..." << std::endl;

    int state = 0;
    std::string roomName = "test";
    std::string username = "test";
    std::cout <<
            "Welcome to \x1b[31;1mH\x1b[32;1ma\x1b[33;1mr\x1b[34;1mm\x1b[35;1mo\x1b[36;1mn\x1b[37;1my\x1b[0m!"
            <<
            std::endl;
    std::cout << "PLEASE DO \x1b[31;1mNOT\x1b[0m KILL THE PROGRAM WITH \x1b[31;1mCTRL + C\x1b[0m" <<
            std::endl;
    while (running) {
        std::cout << std::endl;
        switch (state) {
            case 0: {
                std::cout << "Please choose a option" << std::endl;
                std::cout << "\x1b[33;1m1\x1b[0m \t\t- set room name" << std::endl;
                std::cout << "\x1b[33;1m2\x1b[0m \t\t- set username" << std::endl;
                std::cout << "\x1b[33;1m3\x1b[0m \t\t- connect to room" << std::endl;
                std::cout << "\x1b[33;1mexit\x1b[0m \t- stop program" << std::endl;
                std::cout << "> ";


                std::string input;
                std::cin >> input;
                if (input == "exit") {
                    running = false;
                    break;
                }

                if (input == "1") {
                    state = 1;
                }
                if (input == "2") {
                    state = 2;
                }
                if (input == "3") {
                    state = 3;
                    VoiceChannelManager::connect(roomName, username);
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                }
                break;
            }
            case 1:
                std::cout << "Please enter a room name" << std::endl;
                std::cout << "> ";

                std::cin >> roomName;
                std::cout << "Room Name \"" << roomName << "\" saved" << std::endl;
                state = 0;
                break;
            case 2:
                std::cout << "Please enter a username" << std::endl;
                std::cout << "> ";

                std::cin >> username;
                std::cout << "Username \"" << username << "\" saved" << std::endl;
                state = 0;
                break;
            case 3: {
                std::cout << "Please choose a option" << std::endl;
                std::cout << "\x1b[33;1m1\x1b[0m \t\t- disconnect" << std::endl;
                std::cout << "\x1b[33;1mexit\x1b[0m \t- stop program" << std::endl;
                std::cout << "> ";
                std::string input;
                std::cin >> input;
                if (input == "exit") {
                    running = false;
                    break;
                }

                if (input == "1") {
                    VoiceChannelManager::disconnect();
                    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
                    state = 0;
                }
                break;
            }
            default: ;
        }
    }
    std::cout << "> Stopping Harmony..." << std::endl;
    running = false;
    VoiceChannelManager::join();
    std::cout << "> Websocket was stopped" << std::endl;


    std::cout << "> stopped successfully" << std::endl;

    return 0;
}
