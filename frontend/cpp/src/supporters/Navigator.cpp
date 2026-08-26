//
// Created by tim on 26.08.26.
//

#include "Navigator.h"

#include <mutex>


Navigator *Navigator::instance = nullptr;

Navigator::Navigator() = default;

Navigator::~Navigator() = default;

void Navigator::initialize() {
    instance = new Navigator();
}

void Navigator::deinitialize() {
    delete instance;
}

void Navigator::draw() {
    Navigator &nav = *instance;
    if (const std::unique_lock lock(nav.mutex, std::try_to_lock); lock.owns_lock()) {
        // not blocking, to prevent draw-thread to stutter
        //Push onto stack
        while (!nav.pagesToPush.empty()) {
            nav.pages.push_back(std::move(nav.pagesToPush.front()));
            nav.pagesToPush.pop();
        }

        //Pop from stack
        for (int i = 0; i < nav.popCounter; ++i) {
            if (nav.pages.empty()) {
                break;
            }
            nav.pages.pop_back();
        }
        nav.popCounter = 0;
    }

    if (nav.pages.empty()) {
        return;
    }
    nav.pages.back()->draw();
}

void Navigator::pop() {
    if (!instance) {
        return;
    }
    Navigator &nav = *instance;
    std::lock_guard lock(nav.mutex);
    ++nav.popCounter;
}
