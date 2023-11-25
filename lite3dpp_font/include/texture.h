#ifndef NWE_TEXTURE_H
#define NWE_TEXTURE_H

#include <stdint.h>
#include <vector>

#include "rect.h"

namespace nw
{
    struct RGBA
    {
        uint8_t r, g, b, a;

        RGBA() : r(0), g(0), b(0), a(0) {};

        RGBA(uint8_t _r, uint8_t _g, uint8_t _b) :
            r(_r), g(_g), b(_b), a(255) {};

        RGBA(uint8_t _r, uint8_t _g, uint8_t _b, uint8_t _a) :
            r(_r), g(_g), b(_b), a(_a) {};

        bool operator==(const RGBA& _r) const
        {
            return (
                r == _r.r &&
                g == _r.g &&
                b == _r.b &&
                a == _r.a
            );
        }
    };

    class Texture
    {
        public:
            typedef std::vector<RGBA> Data;

        private:
            int m_width;
            int m_height;
            Data m_data;

        public:
            Texture();
            Texture(int _width, int _height);
            virtual ~Texture();

            int width() const;
            int height() const;

            Data& data();
            const Data& data() const;

            void resize(int _width, int _height);
            void fill(const RGBA& _color);
            void fill(const RGBA& _color, const Rect<int>& _rect);
            RGBA& pixel(int _x, int _y);

            void sub(const Texture& _texture, int _x, int _y);

            bool operator==(const Texture& _r);
            bool operator!=(const Texture& _r);
    };
}

#endif

