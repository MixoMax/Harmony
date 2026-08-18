//
// Created by tim on 18.08.26.
//

#include "Client.h"
#include "httplib.h"
#include "../settings.h"
#include "../voiceChannels/VoiceChannelManager.h"
#include "dataClasses/Room.h"

ix::WebSocket Client::roomWebsocket{};

std::vector<Room> Client::getRooms() {
    httplib::Client cli(serverURL);
    httplib::Result res = cli.Get("/api/v1/rooms");
    if (res) {
        json j = json::parse(res->body);
        std::vector<Room> rooms;
        for (auto &room: j) {
            rooms.emplace_back(room);
        }
        return rooms;
    }
    return {};
}

void Client::connectToRoom(const std::string &roomName, const std::string &username) {
    std::cout << "> Client tries to connect to room: \"" << roomName << "\"..." << std::endl;
    roomWebsocket.setUrl(wsServerURL + "/ws/" + roomName + "/" + username);
    roomWebsocket.setOnMessageCallback(
        [roomName, username](const ix::WebSocketMessagePtr &msg) {
            switch (msg->type) {
                case ix::WebSocketMessageType::Open:
                    std::cout << "> roomWebsocket opened" << std::endl;
                    std::cout << "> ";
                    VoiceChannelManager::connect(roomName, username);
                    break;
                case ix::WebSocketMessageType::Close:
                    std::cout << "> Closing roomWebsocket" << std::endl;
                    std::cout << "> ";
                    VoiceChannelManager::disconnect();
                    break;
                case ix::WebSocketMessageType::Error:
                    std::cout << "> !WS!: " << msg->errorInfo.reason << std::endl;
                    std::cout << "> ";
                    break;
                case ix::WebSocketMessageType::Message:
                    /*receiving connects & disconnects*/
                    std::cout << msg->str << std::endl;
                    std::cout << "> ";
                    break;
                case ix::WebSocketMessageType::Ping:
                    std::cout << "> Ping received" << std::endl;
                    break;
                case ix::WebSocketMessageType::Pong:
                    std::cout << "> Pong received" << std::endl;
                    break;
                default:
                    std::cout << "> Unknown message type: " << static_cast<int>(msg->type) << " (\"" << msg->str <<
                            "\")"
                            << std::endl;
                    std::cout << "> ";
                    break;
            }
        });
    roomWebsocket.disableAutomaticReconnection();
    roomWebsocket.start();
}

void Client::disconnectFromRoom() {
    std::cout << "> Client tries to disconnect from room..." << std::endl;
    roomWebsocket.stop();
}
