//
// Created by tim on 15.08.26.
//

#include "UDPSocketManager.h"

#include <complex>
#include <cstring>
#include <iostream>
#include <arpa/inet.h>

#include "../../../settings.h"
#include "../../../httpUtils/Client.h"
#include "../../graphics/AudioVisualizer.h"

void UDPSocketManager::connect() {
    if (isConnected) {
        std::cerr << "Already connected" << std::endl;
        return;
    }

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
            // std::cout << "> received " << bytesRead << " bytes" << std::endl;

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
                // std::cout << "> total receiving progress: " <<
                //         static_cast<double>(sequenceNumber) / 0xFFFFFFFF * 100 << "%" << std::endl;
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


using Complex = std::complex<float>; // float is faster than double for live audio
const float PI = std::acos(-1.0f);

// 1. High-speed bit reversal helper to reorder data in-place
void bit_reverse_permutation(std::vector<Complex> &x) {
    size_t n = x.size();
    size_t j = 0;
    for (size_t i = 0; i < n; ++i) {
        if (i < j) {
            std::swap(x[i], x[j]);
        }
        size_t bit = n >> 1;
        while (j & bit) {
            j ^= bit;
            bit >>= 1;
        }
        j ^= bit;
    }
}

// 2. High-performance, iterative In-Place FFT (No allocations)
void fft_inplace(std::vector<Complex> &x, bool inverse = false) {
    size_t n = x.size();

    // First, scramble the array indices into bit-reversed order
    bit_reverse_permutation(x);

    // Bottom-up butterfly merge
    for (size_t len = 2; len <= n; len <<= 1) {
        float angle = 2 * PI / len * (inverse ? 1 : -1);
        Complex wlen(std::cos(angle), std::sin(angle));

        for (size_t i = 0; i < n; i += len) {
            Complex w(1);
            for (size_t j = 0; j < len / 2; ++j) {
                Complex u = x[i + j];
                Complex v = x[i + j + len / 2] * w;

                x[i + j] = u + v;
                x[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }

    // Scaling factor for Inverse FFT (IFFT)
    if (inverse) {
        for (auto &val: x) {
            val /= n;
        }
    }
}

void UDPSocketManager::send(int16_t *data, const size_t length) {
    ++totalSendedPackages;
    ++sendSequenceNumber;

    const size_t sampleCount = length / 2;

    std::vector<Complex> x;
    x.resize(sampleCount);
    for (size_t i = 0; i < sampleCount; ++i) {
        x[i] = Complex(data[i]);
    }

    fft_inplace(x, false);

    for (size_t i = 0; i < sampleCount; ++i) {
        if (std::abs(x[i]) < 1000.0f) {
            // Adjust this threshold for your noise floor
            x[i] = 0.0f;
        }
    }

    fft_inplace(x, true);


    std::vector<int16_t> outputBuffer{};
    for (size_t i = 0; i < sampleCount; ++i) {
        // Grab the real part, clamp it to protect against clipping overflows
        float realVal = x[i].real();

        if (realVal > 32767.0f) realVal = 32767.0f;
        if (realVal < -32768.0f) realVal = -32768.0f;

        outputBuffer.emplace_back(static_cast<int16_t>(std::round(realVal)));
    }


    AudioVisualizer::audioData.store(std::make_shared<std::vector<int16_t> >(outputBuffer));


    char sendBuffer[4 + packageSize];
    const uint32_t sequenceNumberNetworkRepresentation = htonl(sendSequenceNumber);
    memcpy(sendBuffer, &sequenceNumberNetworkRepresentation, 4);
    memcpy(sendBuffer + 4, outputBuffer.data(), length);

    /*ifft*/


    // auto t1 = std::chrono::high_resolution_clock::now();
    for (const auto &user: otherUsers) {
        // std::cout << "> sending to " << user.name << "(" << inet_ntoa(user.sockaddr.sin_addr) << ":" <<
        //         ntohs(user.sockaddr.sin_port) << ")" << std::endl;
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
