//
// Created by tim on 18.08.26.
//

#ifndef HARMONY_CLIENT_H
#define HARMONY_CLIENT_H

#include <queue>

#include "json.h"
#include "dataClasses/Room.h"
#include "ixwebsocket/IXWebSocket.h"

using json = nlohmann::json;

class Client {
    static ix::WebSocket roomWebsocket;

    static void bindPort();

public:
    static int port;
    static int udpSocket;
    static sockaddr_in ownAddress;

    static std::mutex usersJoinedMutex;
    static std::queue<User> usersJoined;
    static std::function<void(User &)> joinCallback;
    static std::mutex usersLeftMutex;
    static std::queue<User> usersLeft;
    static std::function<void(User &)> leaveCallback;


    static void update();


    static std::vector<Room> getRooms();

    static void connectToRoom(const std::string &roomName, const std::string &username);

    static void disconnectFromRoom();
};


#endif //HARMONY_CLIENT_H
