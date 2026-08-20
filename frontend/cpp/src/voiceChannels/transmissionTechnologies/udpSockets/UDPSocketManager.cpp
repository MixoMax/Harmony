//
// Created by tim on 15.08.26.
//

#include "UDPSocketManager.h"

#include <cstring>
#include <iostream>
#include <arpa/inet.h>

#include "../../../settings.h"
#include "../../../httpUtils/Client.h"

void UDPSocketManager::connect() {
    TransmissionManager::connect();
    isConnected = true;


    receiveThread = std::make_unique<std::thread>([this]() {
        char buffer[4 + packageSize];
        while (isConnected) {
            sockaddr_in receiveAddress{};
            socklen_t receiveAddressLength = sizeof(receiveAddress);
            const ssize_t bytesRead = recvfrom(Client::udpSocket, buffer, 4 + packageSize, 0,
                                               reinterpret_cast<sockaddr *>(&receiveAddress),
                                               &receiveAddressLength);
            std::cout << "> received " << bytesRead << " bytes" << std::endl;

            if (bytesRead < 4) {
                if (isConnected) {
                    std::cerr << "Error receiving data" << strerror(errno) << std::endl;
                    isConnected = false;
                    break;
                }
            } else {
                /*getting sender*/
                char senderIp[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &receiveAddress.sin_addr, senderIp, INET_ADDRSTRLEN);
                int senderPort = ntohs(receiveAddress.sin_port);
                User &sender = *std::ranges::find_if(otherUsers,
                                                     [senderIp, senderPort](const User &user) {
                                                         return user.ip == senderIp && user.port == senderPort;
                                                     });

                /*receive data*/
                uint32_t sequenceNumberNetworkRepresentation;
                memcpy(&sequenceNumberNetworkRepresentation, buffer, 4);
                const uint32_t sequenceNumber = ntohl(sequenceNumberNetworkRepresentation);
                if (sequenceNumber < sender.receiveSequenceNumber) {
                    continue;
                }
                sender.receiveSequenceNumber = sequenceNumber;

                receiveCallback(buffer + 4, bytesRead - 4);
                std::cout << "> total receiving progress: " <<
                        static_cast<double>(sequenceNumber) / 0xFFFFFFFF * 100 << "%" << std::endl;
            }
        }
    });
}

void UDPSocketManager::disconnect() {
    TransmissionManager::disconnect();
    isConnected = false;

    if (Client::udpSocket >= 0) {
        shutdown(Client::udpSocket, SHUT_RDWR);
        close(Client::udpSocket);
        Client::udpSocket = -1;
    }

    if (receiveThread && receiveThread->joinable()) {
        receiveThread->join();
    }
    receiveThread.reset();
}

void UDPSocketManager::send(char *data, const size_t length) {
    ++totalSendedPackages;
    ++sendSequenceNumber;

    char sendBuffer[4 + packageSize];
    const uint32_t sequenceNumberNetworkRepresentation = htonl(sendSequenceNumber);
    memcpy(sendBuffer, &sequenceNumberNetworkRepresentation, 4);
    memcpy(sendBuffer + 4, data, length);

    /*ifft*/


    // auto t1 = std::chrono::high_resolution_clock::now();
    for (const auto &user: otherUsers) {
        // std::cout << "> sending to " << user.name << std::endl;
        const ssize_t sendResult = sendto(Client::udpSocket, sendBuffer, 4 + packageSize, 0,
                                          (struct sockaddr *) &user.sockaddr,
                                          sizeof(user.sockaddr));
        if (sendResult < 0) {
            std::cerr << "Error sending data" << strerror(errno) << std::endl;
        }
    }
    // auto t2 = std::chrono::high_resolution_clock::now();
    // std::cout << "Sending took: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms" <<
    //         std::endl;
}
