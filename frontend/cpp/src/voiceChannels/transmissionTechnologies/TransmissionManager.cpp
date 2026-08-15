//
// Created by tim on 15.08.26.
//

#include "TransmissionManager.h"

#include <utility>

#include "udpSockets/UDPSocketManager.h"
#include "websockets/WebsocketManager.h"


TransmissionManager *TransmissionManager::instance = new UDPSocketManager();

void TransmissionManager::setReceiveCallback(std::function<void(const char *data, int length)> callback) {
    this->receiveCallback = std::move(callback);
}

void TransmissionManager::cleanUp() {
    delete instance;
    instance = nullptr;
}
