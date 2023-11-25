#include "texture.h"
#include "utils.h"

#include <algorithm>
#include <cstring>
#include <iostream>

namespace nw
{
    Texture::Texture() :
        m_width(0),
        m_height(0),
        m_data()
    {
    }

    Texture::Texture(int _width, int _height) :
        m_width(0),
        m_height(0),
        m_data()
    {
        resize(_width, _height);
    }

    Texture::~Texture()
    {
    }

    int Texture::width() const
    {
        return m_width;
    }

    int Texture::height() const
    {
        return m_height;
    }

    Texture::Data& Texture::data()
    {
        return m_data;
    }

    const Texture::Data& Texture::data() const
    {
        return m_data;
    }

    void Texture::resize(int _width, int _height)
    {
        m_width = _width;
        m_height = _height;
        m_data.resize(m_width * m_height);
    }

    void Texture::fill(const RGBA& _color)
    {
        std::fill(m_data.begin(), m_data.end(), _color);
    }

    void Texture::fill(const RGBA& _color, const Rect<int>& _rect)
    {
        int x = std::max(0, _rect.x0);
        int y = std::max(0, _rect.y0);
        int xm = std::min(m_width, _rect.x1 + 1);
        int ym = std::min(m_height, _rect.y1 + 1);
        for (; y < ym; ++y)
        {
            std::fill(
                m_data.begin() + y * m_width + x,
                m_data.begin() + y * m_width + xm,
                _color);
        }
    }

    RGBA& Texture::pixel(int _x, int _y)
    {
        return m_data[_y * width() + _x];
    }

    void Texture::sub(const Texture& _texture, int _x, int _y)
    {
        for (int i = _y, p = 0; i < _texture.height() + _y; ++i, ++p)
        {
            for (int j = _x, q = 0; j < _texture.width() + _x; ++j, ++q)
            {
                if (i < 0 || j < 0 || i >= height() || j >= width())
                {
                    continue;
                }

                RGBA& bg = data()[i * width() + j];
                const RGBA& fg = _texture.data()[p * _texture.width() + q];
                bg.r = (bg.r * (255 - fg.a) + fg.r * fg.a) / 255;
                bg.g = (bg.g * (255 - fg.a) + fg.g * fg.a) / 255;
                bg.b = (bg.b * (255 - fg.a) + fg.b * fg.a) / 255;
                bg.a = (bg.a * (255 - fg.a) + fg.a * fg.a) / 255;
            }
        }
    }


    bool Texture::operator==(const Texture& _r)
    {
        return m_data == _r.m_data;
    }

    bool Texture::operator!=(const Texture& _r)
    {
        return m_data != _r.m_data;
    }
}

