//
// Created by tim on 20.08.26.
//

#ifndef HARMONY_PAGE_H
#define HARMONY_PAGE_H


class Page {
public:
    virtual ~Page() = default;

    [[nodiscard]] static double getScreenRatio();

    [[nodiscard]] static int getScreenWidth();

    [[nodiscard]] static int getScreenHeight();


    virtual void draw();
};


#endif //HARMONY_PAGE_H
