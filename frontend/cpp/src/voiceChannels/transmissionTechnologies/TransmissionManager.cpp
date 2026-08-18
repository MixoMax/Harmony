//
// Created by tim on 15.08.26.
//

#include "TransmissionManager.h"

#include <iostream>
#include <utility>

#include "udpSockets/UDPSocketManager.h"
#include "websockets/WebsocketManager.h"


/*Queue zum empfangen, server websocket für teilnehmer*/

TransmissionManager *TransmissionManager::instance = new UDPSocketManager();
long TransmissionManager::totalSendedPackages = 0;


void TransmissionManager::connect() {
    std::cout << "> Starting TransmissionManager" << std::endl;
}

void TransmissionManager::setReceiveCallback(std::function<void(const char *data, int length)> callback) {
    this->receiveCallback = std::move(callback);
}

void TransmissionManager::cleanUp() {
    delete instance;
    instance = nullptr;
    std::cout << "> TransmissionManager closed" << std::endl;
}
