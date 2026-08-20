//
// Created by tim on 18.08.26.
//

#include "Client.h"
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "httplib.h"
#include "../settings.h"
#include "../voiceChannels/VoiceChannelManager.h"
#include "../voiceChannels/transmissionTechnologies/TransmissionManager.h"
#include "../voiceChannels/transmissionTechnologies/udpSockets/UDPSocketManager.h"
#include "dataClasses/Room.h"

ix::WebSocket Client::roomWebsocket{};

void Client::bindPort() {
    udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (udpSocket < 0) {
        std::cerr << "Error creating socket" << std::endl;
        return;
    }

    ownAddress.sin_family = AF_INET;
    ownAddress.sin_port = htons(0);
    ownAddress.sin_addr.s_addr = INADDR_ANY;
    if (bind(udpSocket, reinterpret_cast<sockaddr *>(&ownAddress), sizeof(ownAddress)) < 0) {
        std::cerr << "Error binding socket" << strerror(errno) << std::endl;
        disconnectFromRoom();
        return;
    }

    socklen_t ownAddressLength = sizeof(ownAddress);
    if (getsockname(udpSocket, reinterpret_cast<sockaddr *>(&ownAddress), &ownAddressLength) < 0) {
        std::cerr << "Error getting own address" << strerror(errno) << std::endl;
        disconnectFromRoom();
        return;
    }
    std::cout << "> bound to port " << ntohs(ownAddress.sin_port) << std::endl;
    port = ntohs(ownAddress.sin_port);
}

int Client::port{0};
int Client::udpSocket{0};
sockaddr_in Client::ownAddress{};

std::vector<Room> Client::getRooms() {
    httplib::Client cli(serverURL);
    httplib::Result res = cli.Get("/api/v1/rooms");
    if (res) {
        if (res->status != 200) {
            std::cerr << "Error getting rooms: " << res->status << std::endl;
            return {};
        }
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
    bindPort();
    roomWebsocket.setUrl(
        wsServerURL + "/ws/" + roomName + "/" + username + "/" + std::to_string(port));
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
                    // std::cout << "> Ping received" << std::endl;
                    break;
                }
                case ix::WebSocketMessageType::Pong: {
                    // std::cout << "> Pong received" << std::endl;
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
