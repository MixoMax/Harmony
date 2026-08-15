//
// Created by tim on 15.08.26.
//

#include "UDPSocketManager.h"

#include <cstring>
#include <iostream>
#include <arpa/inet.h>

#include "../../../settings.h"

void UDPSocketManager::connect() {
    isConnected = true;
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    ownAddress.sin_family = AF_INET;
    ownAddress.sin_port = htons(ownPort);
    ownAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(udpSocket, reinterpret_cast<struct sockaddr *>(&ownAddress), sizeof(ownAddress)) < 0) {
        std::cerr << "Error binding socket" << strerror(errno) << std::endl;
        disconnect();
        return;
    }

    senderAddress.sin_family = AF_INET;
    senderAddress.sin_port = htons(serverPort);
    senderAddress.sin_addr.s_addr = inet_addr(serverURL.c_str());

    senderAddressLength = sizeof(senderAddress);


    receiveThread = std::make_unique<std::thread>([this]() {
        char buffer[packageSize];
        while (isConnected) {
            int bytesRead = recvfrom(udpSocket, buffer, packageSize, 0,
                                     reinterpret_cast<struct sockaddr *>(&senderAddress), &senderAddressLength);
            if (bytesRead < 0) {
                if (isConnected) {
                    std::cerr << "Error receiving data" << strerror(errno) << std::endl;
                    isConnected = false;
                    break;
                }
            } else {
                receiveCallback(buffer, bytesRead);
            }
        }
    });
}

void UDPSocketManager::disconnect() {
    isConnected = false;

    if (udpSocket >= 0) {
        shutdown(udpSocket, SHUT_RDWR);
        close(udpSocket);
        udpSocket = -1;
    }
    senderAddressLength = 0;

    if (receiveThread && receiveThread->joinable()) {
        receiveThread->join();
    }
    receiveThread.reset();
}

void UDPSocketManager::send(std::string payload) {
    ssize_t sendResult = sendto(udpSocket, payload.c_str(), payload.length(), 0, (struct sockaddr *) &senderAddress,
                                senderAddressLength);
    if (sendResult < 0) {
        std::cerr << "Error sending data" << strerror(errno) << std::endl;
    }
}
