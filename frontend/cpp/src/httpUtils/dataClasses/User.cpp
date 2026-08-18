//
// Created by tim on 18.08.26.
//

#include "User.h"


User::User(json user) {
    name = user["name"];
    ip = user["ip"];
    port = user["port"];
}
