#ifndef NWE_POS2D_H
#define NWE_POS2D_H

namespace nw
{
    template<typename T>
    struct Pos2d
    {
        T x, y;

        Pos2d() :
            x(0), y(0)
        {}

        Pos2d(T _x, T _y) :
            x(_x), y(_y)
        {}

        void set(T _x=0, T _y=0)
        {
            x = _x;
            y = _y;
        }

        void add(T _x, T _y)
        {
            x += _x;
            y += _y;
        }
    };
}

#endif
