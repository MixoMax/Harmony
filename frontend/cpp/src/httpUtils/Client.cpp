//
// Created by tim on 18.08.26.
//

#include "Client.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "../settings.h"
#include "../voiceChannels/VoiceChannelManager.h"
#include "../voiceChannels/transmissionTechnologies/TransmissionManager.h"
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
    roomWebsocket.setUrl(wsServerURL + "/ws/" + roomName + "/" + username + "/" + std::to_string(ownPort));
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
                case ix::WebSocketMessageType::Message: {
                    /*receiving connects & disconnects*/
                    json j = json::parse(msg->str);
                    if (j["type"] == "user_joined") {
                        TransmissionManager::otherUsers.emplace_back(j["user"]);
                        std::cout << "> User joined: " << j["user"]["name"] << std::endl;
                    } else if (j["type"] == "user_left") {
                        const User user{j["user"]};
                        std::erase_if(TransmissionManager::otherUsers,
                                      [&](const User &other) {
                                          return user.id == other.id;
                                      });
                        std::cout << "> User left: " << user.name << std::endl;
                    } else if (j["type"] == "room_joined") {
                        const Room room{j["room"]};
                        TransmissionManager::otherUsers = room.users;
                        std::cout << "> Room joined: " << room.name << std::endl;
                        if (room.users.empty()) {
                            std::cout << "> No users in room" << std::endl;
                        } else {
                            std::cout << "> Say hello to ";
                            for (int userIndex = 0; userIndex < room.users.size(); ++userIndex) {
                                std::cout << "\x1b[35;1m" << room.users[userIndex].name << "\x1b[0m";
                                if (userIndex < room.users.size() - 1) {
                                    std::cout << ", ";
                                }
                            }
                            std::cout << std::endl;
                        }
                    } else {
                        std::cout << "> Unknown message type: " << j["type"] << " (\"" << msg->str << "\")" <<
                                std::endl;
                    }
                    break;
                }
                case ix::WebSocketMessageType::Ping: {
                    std::cout << "> Ping received" << std::endl;
                    break;
                }
                case ix::WebSocketMessageType::Pong: {
                    std::cout << "> Pong received" << std::endl;
                    break;
                }
                default: {
                    std::cout << "> Unknown message type: " << static_cast<int>(msg->type) << " (\"" << msg->str <<
                            "\")"
                            << std::endl;
                    std::cout << "> ";
                    break;
                }
            }
        });
    roomWebsocket.disableAutomaticReconnection();
    roomWebsocket.start();
}

void Client::disconnectFromRoom() {
    std::cout << "> Client tries to disconnect from room..." << std::endl;
    roomWebsocket.stop();
}
