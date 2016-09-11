#ifndef NWE_RECT_H
#define NWE_RECT_H

namespace nw
{
    template<typename T>
    struct Rect
    {
        T x0, y0, x1, y1;

        Rect() :
            x0(0), y0(0), x1(0), y1(0)
        {}

        Rect(T _x0, T _y0, T _x1, T _y1) :
            x0(_x0), y0(_y0), x1(_x1), y1(_y1)
        {}

        bool zero() const
        {
            return x0 == 0
                && y0 == 0
                && x1 == 0
                && y1 == 0;
        }

        bool inside(T _x, T _y) const
        {
            return _x >= x0 && _x <= x1
                && _y >= y0 && _y <= y1;
        }

        T width() const
        {
            return x1 - x0;
        }

        T height() const
        {
            return y1 - y0;
        }

        void set(T _x0, T _y0, T _x1, T _y1)
        {
            x0 = _x0;
            y0 = _y0;
            x1 = _x1;
            y1 = _y1;
        }

        void reset()
        {
            x0 = 0;
            y0 = 0;
            x1 = 0;
            y1 = 0;
        }

        void add(T _x, T _y)
        {
            x0 += _x;
            y0 += _y;
            x1 += _x;
            y1 += _y;
        }

        void sub(T _x, T _y)
        {
            return add(-_x, -_y);
        }

        void include(T _x, T _y)
        {
            x0 = x0 < _x ? x0 : _x;
            y0 = y0 < _y ? y0 : _y;
            x1 = x1 > _x ? x1 : _x;
            y1 = y1 > _y ? y1 : _y;
        }

        void include(T _x0, T _y0, T _x1, T _y1)
        {
            x0 = x0 < _x0 ? x0 : _x0;
            y0 = y0 < _y0 ? y0 : _y0;
            x1 = x1 > _x1 ? x1 : _x1;
            y1 = y1 > _y1 ? y1 : _y1;
        }
    };
}

#endif
