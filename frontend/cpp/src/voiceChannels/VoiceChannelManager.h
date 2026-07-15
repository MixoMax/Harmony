#ifndef HARMONY_VOICECHANNELMANAGER_H
#define HARMONY_VOICECHANNELMANAGER_H
#include <atomic>
#include <condition_variable>
#include <string>
#include <thread>


class VoiceChannelManager {
    std::atomic_bool shouldConnect{false};
    std::atomic_bool isConnected{false};
    std::atomic_bool stopThread{false};

    std::thread thread;

    std::mutex mutex;
    std::string roomName;
    std::string userName;

    VoiceChannelManager();

public:
    static VoiceChannelManager &getInstance();

    static void connect(std::string roomName, std::string userName);

    static void disconnect();

    static void join();
};


#endif //HARMONY_VOICECHANNELMANAGER_H
