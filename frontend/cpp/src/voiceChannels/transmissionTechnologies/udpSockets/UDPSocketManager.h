//
// Created by tim on 15.08.26.
//

#ifndef HARMONY_UDPSOCKETMANAGER_H
#define HARMONY_UDPSOCKETMANAGER_H
#include <thread>
#include <netinet/in.h>

#include "../TransmissionManager.h"


class UDPSocketManager : public TransmissionManager {
    std::unique_ptr<std::thread> receiveThread{nullptr};

    u_int32_t sendSequenceNumber{0};
    u_int32_t receiveSequenceNumber{0};

    std::atomic_bool running{false};

public:
    void connect() override;

    void disconnect() override;

    void send(int16_t *data, size_t length) override;
};


#endif //HARMONY_UDPSOCKETMANAGER_H
