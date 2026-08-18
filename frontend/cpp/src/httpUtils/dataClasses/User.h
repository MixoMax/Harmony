//
// Created by tim on 18.08.26.
//

#ifndef HARMONY_USER_H
#define HARMONY_USER_H
#include <string>

#include "json.h"

using json = nlohmann::json;

struct User {
    std::string name;
    std::string ip;
    int port;

    explicit User(json user);
};


#endif //HARMONY_USER_H
