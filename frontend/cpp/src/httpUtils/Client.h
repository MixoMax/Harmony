//
// Created by tim on 18.08.26.
//

#ifndef HARMONY_CLIENT_H
#define HARMONY_CLIENT_H

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

    static std::vector<Room> getRooms();


    static void connectToRoom(const std::string &roomName, const std::string &username);

    static void disconnectFromRoom();
};


#endif //HARMONY_CLIENT_H
