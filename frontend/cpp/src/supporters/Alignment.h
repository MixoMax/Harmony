//
// Created by tim on 18.07.26.
//

#ifndef HARMONY_ALIGNMENT_H
#define HARMONY_ALIGNMENT_H

class Alignment {
public:
    float x, y;

    Alignment(const float x, const float y)
        : x(x),
          y(y) {
    }


    static Alignment TopLeft;
    static Alignment TopCenter;
    static Alignment TopRight;
    static Alignment CenterLeft;
    static Alignment Center;
    static Alignment CenterRight;
    static Alignment BottomLeft;
    static Alignment BottomCenter;
    static Alignment BottomRight;

    bool operator==(const Alignment &other) const {
        return x == other.x && y == other.y;
    }
};


#endif //HARMONY_ALIGNMENT_H
