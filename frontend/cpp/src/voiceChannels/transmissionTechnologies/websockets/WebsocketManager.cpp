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
    webSocket.setUrl("ws://" + serverURL + "/ws/" + roomName + "/" + userName);
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
                    std::cout << "> !WS ERROR!: " << msg->errorInfo.reason << std::endl;
                    break;
                case ix::WebSocketMessageType::Message:
                    /*receiving audio*/
                    receiveCallback(msg->str.data(), msg->str.size());
                    break;
                default:
                    std::cout << "> Unknown message type: " << static_cast<int>(msg->type) << std::endl;
                    break;
            }
        });
    webSocket.enableAutomaticReconnection();
    webSocket.start();
}

void WebsocketManager::disconnect() {
    webSocket.close();
}

void WebsocketManager::send(const std::string payload) {
    if (!webSocket.sendBinary(payload).success) {
        std::cerr << "failed to send binaries" << std::endl;
    }
}
