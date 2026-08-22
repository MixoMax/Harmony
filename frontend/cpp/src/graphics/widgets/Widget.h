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

    float width = -1, height = -1;

protected:
    float rotation = 0;

public:
    void setMinWidth(float newMinWidth);

    void setMaxWidth(float newMaxWidth);

    void setMinHeight(float newMinHeight);

    void setMaxHeight(float newMaxHeight);

    void setSize(float newWidth, float newHeight);

    void setWidth(float newWidth);

    void setHeight(float newHeight);

    void setRotation(float newRotation);

    [[nodiscard]] float getMinWidth() const;

    [[nodiscard]] float getMaxWidth() const;

    [[nodiscard]] float getMinHeight() const;

    [[nodiscard]] float getMaxHeight() const;

    [[nodiscard]] float getWidth() const;

    [[nodiscard]] float getHeight() const;

    [[nodiscard]] float getRotation() const;
};


#endif //HARMONY_WIDGET_H
