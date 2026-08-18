//
// Created by tim on 18.08.26.
//

#ifndef HARMONY_USER_H
#define HARMONY_USER_H
#include <string>
#include <netinet/in.h>

#include "json.h"

using json = nlohmann::json;

struct User {
    std::string id;
    std::string name;
    std::string ip;
    int port;

    sockaddr_in sockaddr{};
    uint32_t receiveSequenceNumber{};

    explicit User(json user);
};


#endif //HARMONY_USER_H
