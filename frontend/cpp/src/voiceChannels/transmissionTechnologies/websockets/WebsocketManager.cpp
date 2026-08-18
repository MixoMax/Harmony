//
// Created by tim on 15.08.26.
//

#include "WebsocketManager.h"

#include <iostream>

#include "../../../settings.h"
#include "ixwebsocket/IXProgressCallback.h"
#include "ixwebsocket/IXWebSocket.h"


WebsocketManager::~WebsocketManager() = default;

void WebsocketManager::connect() {
    TransmissionManager::connect();
    webSocket.setUrl(wsServerURL + "/ws/" + roomName + "/" + userName);
    webSocket.setOnMessageCallback(
        [this](const ix::WebSocketMessagePtr &msg) {
            switch (msg->type) {
                case ix::WebSocketMessageType::Open:
                    std::cout << "> Connected to Server" << std::endl;
                    isConnected = true;
                    break;
                case ix::WebSocketMessageType::Close:
                    std::cout << "> Disconnected from Server" << std::endl;
                    isConnected = false;
                    break;
                case ix::WebSocketMessageType::Error:
                    std::cout << "> !WS!: " << msg->errorInfo.reason << std::endl;
                    break;
                case ix::WebSocketMessageType::Message:
                    /*receiving audio*/
                    receiveCallback(msg->str.data(), msg->str.size());
                    break;
                default:
                    std::cout << "> Unknown message type: " << static_cast<int>(msg->type) << " (\"" << msg->str <<
                            "\")"
                            << std::endl;
                    break;
            }
        });
    webSocket.enableAutomaticReconnection();
    webSocket.start();
}

void WebsocketManager::disconnect() {
    TransmissionManager::disconnect();
    webSocket.close();
}

void WebsocketManager::send(char *data, const size_t length) {
    const std::string payload{
        data,
        length * sizeof(int8_t)
    };
    if (!webSocket.sendBinary(payload).success) {
        std::cerr << "!WS!: failed to send binaries" << std::endl;
    }
}
