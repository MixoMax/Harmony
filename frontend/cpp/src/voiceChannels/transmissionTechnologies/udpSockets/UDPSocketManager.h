//
// Created by tim on 15.08.26.
//

#ifndef HARMONY_UDPSOCKETMANAGER_H
#define HARMONY_UDPSOCKETMANAGER_H
#include <thread>
#include <netinet/in.h>

#include "../TransmissionManager.h"


class UDPSocketManager : public TransmissionManager {
    int udpSocket{0};
    sockaddr_in ownAddress{};
    sockaddr_in senderAddress{};
    socklen_t senderAddressLength{0};

    std::unique_ptr<std::thread> receiveThread{nullptr};

public:
    void connect() override;

    void disconnect() override;

    void send(std::string payload) override;
};


#endif //HARMONY_UDPSOCKETMANAGER_H
