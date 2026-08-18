//
// Created by tim on 15.08.26.
//

#ifndef HARMONY_TRANSMISSIONMANAGER_H
#define HARMONY_TRANSMISSIONMANAGER_H
#include <atomic>
#include <string>
#include <functional>

#include "../../httpUtils/dataClasses/User.h"


class TransmissionManager {
protected:
    std::function<void(const char *data, int length)> receiveCallback;

public:
    std::string roomName;
    std::string userName;
    static std::vector<User> otherUsers;

    static long totalSendedPackages;

    std::atomic_bool isConnected{false};

    // Here you can select the transmission technology (UDP or Websockets)
    static TransmissionManager *instance;

    virtual ~TransmissionManager() = default;

    virtual void connect();

    virtual void disconnect();

    virtual void send(char *data, size_t length) = 0;

    void setReceiveCallback(std::function<void(const char *data, int length)> callback);


    static void cleanUp();
};


#endif //HARMONY_TRANSMISSIONMANAGER_H
