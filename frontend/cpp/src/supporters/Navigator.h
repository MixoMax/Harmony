//
// Created by tim on 26.08.26.

#ifndef HARMONY_NAVIGATOR_H
#define HARMONY_NAVIGATOR_H
#include <memory>
#include <queue>

#include "../graphics/pages/Page.h"


/**
 * Contains a stack of Pages, where the last Page is the currently displayed one.
 * To change the currently displayed Page, use push() and pop().
 */
class Navigator {
    std::vector<std::unique_ptr<Page> > pages{};
    std::mutex mutex;
    std::queue<std::unique_ptr<Page> > pagesToPush{};
    int popCounter = 0;


    static Navigator *instance;

public:
    Navigator();

    ~Navigator();

    static void initialize();

    static void deinitialize();

    static void draw();


    /**
     * Pushes a Page onto the stack
     * @tparam PageToPush Classtype of new Page (ex. Homepage)
     * @tparam Args Constructor Arguments types
     * @param args Constructor Arguments of new Page
     */
    template<typename PageToPush, typename... Args>
    static void push(Args &&... args) {
        if (!instance) {
            return;
        }
        Navigator &nav = *instance;
        std::lock_guard lock(nav.mutex);
        nav.pagesToPush.emplace(std::make_unique<PageToPush>(std::forward<Args>(args)...));
    }

    /**
     * Pops a Page from the stack
     */
    static void pop();
};


#endif //HARMONY_NAVIGATOR_H
