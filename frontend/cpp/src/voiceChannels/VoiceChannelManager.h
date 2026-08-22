#ifndef HARMONY_VOICECHANNELMANAGER_H
#define HARMONY_VOICECHANNELMANAGER_H
#include <atomic>
#include <condition_variable>
#include <string>
#include <thread>


class VoiceChannelManager {
    std::atomic_bool shouldConnect{false};
    std::atomic_bool stopThread{false};

    std::thread thread;

    std::mutex mutex;
    std::string roomName{};
    std::string userName{};
    std::string micDeviceName{};
    std::string speakerDeviceName{};

    VoiceChannelManager();

    static VoiceChannelManager &getInstance();

public:
    static void connect(std::string roomName, std::string userName);

    static void disconnect();

    static void setDeviceNames(std::string micDeviceName, std::string speakerDeviceName);

    static void join();
};


#endif //HARMONY_VOICECHANNELMANAGER_H
