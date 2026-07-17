//
// Created by tim on 17.07.26.
//

#ifndef HARMONY_WIDGET_H
#define HARMONY_WIDGET_H
#include <algorithm>
#include <limits>


class Widget {
    float minWidth = 0, minHeight = 0,
            maxWidth = std::numeric_limits<float>::infinity(), maxHeight = std::numeric_limits<float>::infinity();

public:
    void setMinWidth(float newMinWidth);

    void setMaxWidth(float newMaxWidth);

    void setMinHeight(float newMinHeight);

    void setMaxHeight(float newMaxHeight);

    [[nodiscard]] float getMinWidth() const;

    [[nodiscard]] float getMaxWidth() const;

    [[nodiscard]] float getMinHeight() const;

    [[nodiscard]] float getMaxHeight() const;
};


#endif //HARMONY_WIDGET_H
