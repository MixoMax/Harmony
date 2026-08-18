//
// Created by tim on 18.08.26.
//

#include "User.h"

#include <iostream>
#include <arpa/inet.h>


User::User(json user) {
    id = user["id"];
    name = user["name"];
    ip = user["ip"];
    port = user["port"];

    sockaddr.sin_family = AF_INET;
    sockaddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &sockaddr.sin_addr) != 1) {
        std::cerr << "Invalid IP address: " << ip << std::endl;
    }
}
