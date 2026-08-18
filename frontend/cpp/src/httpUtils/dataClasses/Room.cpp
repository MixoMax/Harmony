//
// Created by tim on 18.08.26.
//

#include "Room.h"

Room::Room(json room) {
    name = room["name"];
    for (auto &user: room["users"]) {
        users.emplace_back(user);
    }
}
