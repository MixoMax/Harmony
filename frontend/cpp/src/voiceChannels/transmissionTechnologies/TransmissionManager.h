//
// Created by tim on 15.08.26.
//

#ifndef HARMONY_TRANSMISSIONMANAGER_H
#define HARMONY_TRANSMISSIONMANAGER_H
#include <atomic>
#include <string>
#include <functional>


class TransmissionManager {
protected:
    std::function<void(const char *data, int length)> receiveCallback;

public:
    std::string roomName;
    std::string userName;

    std::atomic_bool isConnected{false};

    static TransmissionManager *instance;

    virtual ~TransmissionManager() = default;

    virtual void connect() = 0;

    virtual void disconnect() = 0;

    virtual void send(std::string payload) = 0;

    void setReceiveCallback(std::function<void(const char *data, int length)> callback);
};


#endif //HARMONY_TRANSMISSIONMANAGER_H
