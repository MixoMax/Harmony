//
// Created by tim on 18.08.26.
//

#ifndef HARMONY_ROOM_H
#define HARMONY_ROOM_H
#include <string>
#include <vector>

#include "User.h"


struct Room {
    std::string name;

    std::vector<User> users;

    explicit Room(json room);
};


#endif //HARMONY_ROOM_H
