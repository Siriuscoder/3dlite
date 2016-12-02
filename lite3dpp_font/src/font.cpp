#include "font.h"
#include <iostream>
#include <algorithm>
#include "math.h"

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wshadow"
#include "utf8.h"
#pragma GCC diagnostic pop

namespace
{
    FT_Pos ftFloor(FT_Pos _x)
    {
        return _x & -64;
    }

    struct User {
        nw::Pos2d<int> pos;
        nw::RGBA color;
        nw::Texture* pTexture;
        const nw::Rect<int>* pRect;
        User() : pos(0, 0), color(), pTexture(NULL), pRect(NULL) {}
    };

    void RasterCallback(
        const int _y,
        const int _count,
        const FT_Span* _spans,
        void * const _user)
    {
        User* pUser = static_cast<User*>(_user);
        const FT_Span* pSpan;

        for (int i = 0; i < _count; ++i)
        {
            pSpan = &_spans[i];
            int py = -_y + pUser->pos.y;

            if (!pUser->pTexture || py < 0 || py >= pUser->pTexture->height())
                continue;

            for (int x = 0; x < pSpan->len; ++x)
            {
                int px = pSpan->x + x + pUser->pos.x;
                if (pUser->pRect && !pUser->pRect->inside(px, py)) continue;
                if (px < 0 || px >= pUser->pTexture->width()) continue;

                nw::RGBA fg = pUser->color;
                fg.a = pSpan->coverage;

                nw::RGBA& bg = pUser->pTexture->pixel(px, py);
                bg.r = (bg.r * (255 - fg.a) + fg.r * fg.a) / 255;
                bg.g = (bg.g * (255 - fg.a) + fg.g * fg.a) / 255;
                bg.b = (bg.b * (255 - fg.a) + fg.b * fg.a) / 255;
                bg.a = (bg.a * (255 - fg.a) + fg.a * fg.a) / 255;
            }
        }
    }

}

namespace nw
{
    // FontLib

    FontLib::FontLib(size_t _hdpi, size_t _vdpi) :
        Loggable(),
        m_library(),
        m_manager(),
        m_imageCache(),
        m_stroker(),
        m_faceIds(),
        m_hdpi(_hdpi),
        m_vdpi(_vdpi)
    {
        init();
    }

    FontLib::FontLib(const FontLib& _copy) :
        Loggable(),
        m_library(),
        m_manager(),
        m_imageCache(),
        m_stroker(),
        m_faceIds(),
        m_hdpi(_copy.m_hdpi),
        m_vdpi(_copy.m_vdpi)
    {
        init();
    }

    FontLib::~FontLib()
    {
        release();
    }

    FontLib& FontLib::operator=(const FontLib& _copy)
    {
        m_hdpi = _copy.m_hdpi;
        m_vdpi = _copy.m_vdpi;
        return *this;
    }

    void FontLib::init()
    {
        release();

        int error = FT_Init_FreeType(&m_library);
        if (error)
        {
            NW_ERROR("FreeType2 init failed.")
        }

        error = FTC_Manager_New(
            m_library, 0, 0, 0, &Font::FaceRequester, NULL, &m_manager);
        if (error)
        {
            NW_ERROR("FreeType2 manager init failed.")
        }

        error = FTC_ImageCache_New(m_manager, &m_imageCache);
        if (error)
        {
            NW_ERROR("FreeType2 cache init failed.")
        }

        error = FT_Stroker_New(m_library, &m_stroker);
        if (error)
        {
            NW_ERROR("FreeType2 stroker init failed.")
        }
    }

    void FontLib::release()
    {
        FT_Stroker_Done(m_stroker);
        FTC_Manager_Done(m_manager);
        FT_Done_FreeType(m_library);
    }

    const FT_Library& FontLib::library() const
    {
        return m_library;
    }

    const FTC_Manager& FontLib::manager() const
    {
        return m_manager;
    }

    const FTC_ImageCache& FontLib::imageCache() const
    {
        return m_imageCache;
    }

    const FT_Stroker& FontLib::stroker(int _size) const
    {
        if (_size > 0)
        {
            FT_Stroker_Set(
                m_stroker,
                _size * 64,
                FT_STROKER_LINECAP_ROUND,
                FT_STROKER_LINEJOIN_ROUND,
                0);
        }
        return m_stroker;
    }

    size_t FontLib::hDPI() const
    {
        return m_hdpi;
    }

    size_t FontLib::vDPI() const
    {
        return m_vdpi;
    }

    FaceId& FontLib::getFaceId(const std::string& _name)
    {
        FontLib::FaceIdIterator pos = m_faceIds.find(_name);
        if (m_faceIds.end() == pos)
        {
            FaceId faceId;
            faceId.name = _name;
            faceId.isFile = true;
            m_faceIds[_name] = faceId;
            return m_faceIds[_name];
        }
        return pos->second;
    }

    FaceId& FontLib::getFaceId(
        const std::string& _name,
        const FaceId::Byte* _pData,
        FaceId::Size _dataSize
        )
    {
        FontLib::FaceIdIterator pos = m_faceIds.find(_name);
        if (m_faceIds.end() == pos)
        {
            FaceId faceId;
            faceId.name = _name;
            faceId.isFile = false;
            faceId.pData = new FaceId::Byte[_dataSize];
            memcpy(faceId.pData, _pData, _dataSize);
            faceId.dataSize = _dataSize;
            m_faceIds[_name] = faceId;
            return m_faceIds[_name];
        }
        return pos->second;
    }

    // Font

    Font::Font(FontLib& _fontLib, const std::string& _fileName, size_t _size) :
        Loggable(),
        m_fontLib(_fontLib),
        m_faceId(_fontLib.getFaceId(_fileName)),
        m_face(),
        m_scaler()
    {
        init(_size);
    }

    Font::Font(
            FontLib& _fontLib,
            const std::string& _name,
            const FaceId::Byte* _pData,
            FaceId::Size _dataSize,
            size_t _size) :
        Loggable(),
        m_fontLib(_fontLib),
        m_faceId(_fontLib.getFaceId(_name, _pData, _dataSize)),
        m_face(),
        m_scaler()
    {
        init(_size);
    }

    void Font::init(size_t _size)
    {
        int error = FTC_Manager_LookupFace(
            m_fontLib.manager(), &m_faceId, &m_face);
        if (error == FT_Err_Unknown_File_Format)
        {
            NW_ERROR(
                "FreeType2  font '" << m_faceId.name << "' unknown format.")
        }
        else if (error)
        {
            NW_ERROR("FreeType2  font '" << m_faceId.name << "' init failed.")
        }

        m_scaler.face_id = static_cast<FTC_FaceID>(&m_faceId);
        m_scaler.width = _size * 64;
        m_scaler.height = _size * 64;
        m_scaler.pixel = 0;
        m_scaler.x_res = m_fontLib.hDPI();
        m_scaler.y_res = m_fontLib.vDPI();
    }

    Font::Font(const Font& _copy) :
        Loggable(_copy.logger()),
        m_fontLib(_copy.m_fontLib),
        m_faceId(_copy.m_faceId),
        m_face(_copy.m_face),
        m_scaler(_copy.m_scaler)
    {
    }

    Font::~Font()
    {
    }

    Font& Font::operator=(const Font& _copy)
    {
        m_fontLib = _copy.m_fontLib;
        m_faceId = _copy.m_faceId;
        m_face = _copy.m_face;
        m_scaler = _copy.m_scaler;
        return *this;
    }

    FT_Error Font::FaceRequester(
        FTC_FaceID _faceId,
        FT_Library _library,
        FT_Pointer /*_requestData*/,
        FT_Face* _aFace)
    {
        FaceId* pFace = static_cast<FaceId*>(_faceId);

        if (pFace->isFile) {
            return FT_New_Face(
                _library, pFace->name.c_str(), pFace->index, _aFace);
        }

        return FT_New_Memory_Face(
            _library, pFace->pData, pFace->dataSize, pFace->index, _aFace);
    }

    const FontLib& Font::fontLib() const
    {
        return m_fontLib;
    }

    const FaceId& Font::faceId() const
    {
        return m_faceId;
    }

    const FT_Face& Font::face() const
    {
        return m_face;
    }

    const FTC_ScalerRec& Font::scaler() const
    {
        return m_scaler;
    }

    size_t Font::size() const
    {
        return m_scaler.width / 64;
    }

    void Font::setSize(size_t _size)
    {
        m_scaler.width = _size * 64;
        m_scaler.height = _size * 64;
    }

    bool Font::hasKerning() const
    {
        return FT_HAS_KERNING(m_face) != 0;
    }

    size_t Font::toNextOrigin() const
    {
        float height = static_cast<float>(m_face->height);
        height /= m_face->units_per_EM;
        height *= size();
        return static_cast<size_t>(height);
    }

    // Text
    Text::Text(const Font& _font, const std::string& _text) :
        Loggable(),
        m_font(_font),
        m_text(),
        m_pos(0, 0),
        m_width(-1),
        m_transform(),
        m_color(0, 0, 0),
        m_bgColor(255, 255, 255),
        m_useBgColor(false),
        m_oColor(0, 0, 0),
        m_oSize(0),
        m_selBegin(0),
        m_selEnd(0),
        m_useSelection(false),
        m_dirty(false),
        m_glyphs(),
        m_rect()
    {
        rotate(0.0);
        setText(_text);
    }

    Text::~Text()
    {
    }

    void Text::release()
    {
        for (GlyphIterator it = m_glyphs.begin(); it != m_glyphs.end(); ++it)
        {
            FT_Done_Glyph(*it);
        }
        m_glyphs.resize(0);
        m_rect.reset();
    }

    const Font& Text::font() const
    {
        return m_font;
    }

    void Text::setFont(const Font& _font)
    {
        m_font = _font;
        m_dirty = true;
    }

    const Text::UString& Text::text() const
    {
        return m_text;
    }

    void Text::setText(const std::string& _text)
    {
        m_text.resize(0);
        utf8::unchecked::utf8to32(
            _text.begin(), _text.end(), back_inserter(m_text));
        if (m_useSelection)
        {
            select(m_selBegin, m_selEnd);
        }
        m_dirty = true;
    }

    void Text::setPos(int _x, int _y)
    {
        m_pos.x = _x;
        m_pos.y = _y;
    }

    void Text::setWidth(int _width)
    {
        if (_width != m_width)
        {
            m_width = _width;
            m_dirty = true;
        }
    }

    void Text::rotate(FPType _degree)
    {
        FPType angle = -_degree * static_cast<FPType>(M_PI) / 180;
        m_transform.xx = (FT_Fixed)( cos(angle) * 0x10000L);
        m_transform.xy = (FT_Fixed)(-sin(angle) * 0x10000L);
        m_transform.yx = (FT_Fixed)( sin(angle) * 0x10000L);
        m_transform.yy = (FT_Fixed)( cos(angle) * 0x10000L);
        m_dirty = true;
    }

    void Text::setColor(const RGBA& _color)
    {
        m_color = _color;
    }

    void Text::setBgColor(const RGBA& _color)
    {
        m_bgColor = _color;
    }

    void Text::useBgColor(bool _use)
    {
        m_useBgColor = _use;
    }

    void Text::setOutlineColor(const RGBA& _color)
    {
        m_oColor = _color;
    }

    void Text::setOutlineSize(int _size)
    {
        if (_size != m_oSize)
        {
            m_oSize = _size;
            m_dirty = true;
        }
    }

    void Text::select(size_t _begin, size_t _end)
    {
        if (m_text.size())
        {
            m_useSelection = true;
            size_t last = m_text.size() - 1;
            m_selBegin = std::max<size_t>(0, std::min(_begin, last));
            m_selEnd = std::max<size_t>(0, std::min(_end, last));
        }
        else
        {
            deselect();
        }
    }

    void Text::deselect()
    {
        m_useSelection = false;
        m_selBegin = 0;
        m_selEnd = 0;
    }

    Rect<int> Text::rect()
    {
        validate();
        Rect<int> ret(m_rect);
        ret.add(m_pos.x - m_rect.x0, m_pos.y - m_rect.y0);
        return ret;
    }

    void Text::render(Texture& _texture)
    {
        validate();

        Pos2d<int> adjustedPos(m_pos);
        adjustedPos.add(-m_rect.x0, -m_rect.y0);

        const FT_Library& library = m_font.fontLib().library();
        const FT_Stroker& stroker = m_font.fontLib().stroker(m_oSize);

        User user;
        user.pos = adjustedPos;
        user.pTexture = &_texture;
        user.pRect = NULL;

        FT_Raster_Params params;
        memset(&params, 0, sizeof(params));
        params.flags = FT_RASTER_FLAG_AA | FT_RASTER_FLAG_DIRECT;
        params.gray_spans = RasterCallback;
        params.user = &user;

        GlyphIterator begin = m_glyphs.begin();
        GlyphIterator end = m_glyphs.end();
        if (m_useSelection)
        {
            begin += m_selBegin;
            end = m_glyphs.begin() + m_selEnd + 1;
        }

        if (m_oSize)
        {
            user.color = m_oColor;
            for (GlyphIterator it = begin; it != end; ++it)
            {
                FT_Glyph glyph = *it;
                if (!glyph) continue;
                if (glyph->format != FT_GLYPH_FORMAT_OUTLINE) continue;
                FT_Glyph_StrokeBorder(&glyph, stroker, false, false);
                FT_Outline_Render(
                    library,
                    &reinterpret_cast<FT_OutlineGlyph>(glyph)->outline,
                    &params);
                FT_Done_Glyph(glyph);
            }
        }

        user.color = m_color;
        for (GlyphIterator it = begin; it != end; ++it)
        {
            if (!(*it)) continue;
            if ((*it)->format != FT_GLYPH_FORMAT_OUTLINE) continue;
            FT_Outline_Render(
                library,
                &reinterpret_cast<FT_OutlineGlyph>(*it)->outline,
                &params);
        }
    }

    void Text::validate()
    {
        if (m_dirty)
        {
            genGlyphs();
            m_dirty = false;
        }
    }

    void Text::genGlyphs()
    {
        release();

        const FTC_ImageCache& imageCache = m_font.fontLib().imageCache();
        FTC_Scaler scaler = const_cast<FTC_Scaler>(&m_font.scaler());

        FT_Vector pen = {0, 0};
        FT_UInt prevIndex = 0;

        m_glyphs.resize(m_text.size());
        for (size_t i = 0; i < m_text.size(); ++i)
        {
            FT_UInt index = FT_Get_Char_Index(m_font.face(), m_text[i] == '\n' ? ' ' : m_text[i]);
            int error = FTC_ImageCache_LookupScaler(
                imageCache,
                scaler,
                FT_LOAD_DEFAULT,
                index,
                &m_glyphs[i],
                NULL);
            if (error)
            {
                NW_WARNING(
                    "FreeType2 get glyph failed for char '"
                    << m_text[i] << "'")
                continue;
            }
            FT_Glyph_Copy(m_glyphs[i], &m_glyphs[i]);

            if (m_glyphs[i]->format != FT_GLYPH_FORMAT_OUTLINE) continue;

            // Kerning
            if (m_font.hasKerning() && prevIndex && index)
            {
                FT_Vector delta;
                FT_Get_Kerning(
                    m_font.face(),
                    prevIndex,
                    index,
                    FT_KERNING_DEFAULT,
                    &delta);
                pen.x += delta.x;
                pen.y += delta.y;
            }
            prevIndex = index;

            FT_Vector vec = pen;
            pen.x += m_glyphs[i]->advance.x >> 10;
            pen.y += m_glyphs[i]->advance.y >> 10;

            if (m_text[i] == '\n')
            {
                pen.x = 0;
                pen.y -= m_font.toNextOrigin() * 64 * 1.75;
                prevIndex = 0;
            }
            else if ((pen.x >> 6) > m_width)
            {
                pen.x = 0;
                pen.y -= m_font.toNextOrigin() * 64;
                prevIndex = 0;
            }

            FT_Vector_Transform(&vec, &m_transform);
            FT_Glyph_Transform(m_glyphs[i], &m_transform, &vec);
        }

        calcRect();
    }

    void Text::calcRect()
    {
        FT_BBox bbox, gBBox;
        bbox.xMin = 32000;
        bbox.yMin = 32000;
        bbox.xMax = -32000;
        bbox.yMax = -32000;

        for (GlyphIterator it = m_glyphs.begin(); it != m_glyphs.end(); ++it)
        {
            if (!(*it)) continue;

            FT_Outline_Get_BBox(
                &reinterpret_cast<FT_OutlineGlyph>(*it)->outline,
                &gBBox);

            if (gBBox.xMin < bbox.xMin) bbox.xMin = gBBox.xMin;
            if (gBBox.yMin < bbox.yMin) bbox.yMin = gBBox.yMin;
            if (gBBox.xMax > bbox.xMax) bbox.xMax = gBBox.xMax;
            if (gBBox.yMax > bbox.yMax) bbox.yMax = gBBox.yMax;
        }

        m_rect.set(
             (ftFloor(bbox.xMin) >> 6),
            -(ftFloor(bbox.yMax) >> 6),
             (ftFloor(bbox.xMax) >> 6),
            -(ftFloor(bbox.yMin) >> 6)
            );

        if (m_oSize)
        {
            m_rect.x0 -= m_oSize;
            m_rect.y0 -= m_oSize;
            m_rect.x1 += m_oSize;
            m_rect.y1 += m_oSize;
        }
    }
}

