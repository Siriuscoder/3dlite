#ifndef NWE_FONT_H
#define NWE_FONT_H

#include <string>
#include <vector>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_LCD_FILTER_H
#include FT_GLYPH_H
#include FT_CACHE_H
#include FT_STROKER_H
#include FT_BBOX_H

#include "rect.h"
#include "pos2d.h"
#include "texture.h"
#include "log/loggable.h"

namespace nw
{
    struct FaceId {
        typedef unsigned char Byte;
        typedef signed long Size;

        std::string name;
        int index;
        bool isFile;
        const Byte* pData;
        Size dataSize;

        FaceId() :
            name(),
            index(0),
            isFile(true),
            pData(NULL),
            dataSize(0)
        {
        }
    };

    class FontLib : public Loggable
    {
        public:
            typedef std::map<std::string, FaceId>::iterator FaceIdIterator;

        private:
            FT_Library m_library;
            FTC_Manager m_manager;
            FTC_ImageCache m_imageCache;
            FT_Stroker m_stroker;

            std::map<std::string, FaceId> m_faceIds;

            size_t m_hdpi;
            size_t m_vdpi;

        public:
            FontLib(size_t _hdpi=120, size_t _vdpi=120);
            FontLib(const FontLib& _copy);
            virtual ~FontLib();
            FontLib& operator=(const FontLib& _copy);

            const FT_Library& library() const;
            const FTC_Manager& manager() const;
            const FTC_ImageCache& imageCache() const;
            const FT_Stroker& stroker(int _size=0) const;
            size_t hDPI() const;
            size_t vDPI() const;

            FaceId& getFaceId(const std::string& _name);
            FaceId& getFaceId(
                const std::string& _name,
                const FaceId::Byte* _pData,
                FaceId::Size _dataSize);

        private:
            void init();
            void release();
    };

    class Font : public Loggable
    {
        private:
            FontLib& m_fontLib;
            FaceId& m_faceId;
            FT_Face m_face;
            FTC_ScalerRec m_scaler;

        public:
            Font(
                FontLib& _fontLib,
                const std::string& _fileName,
                size_t _size);

            Font(
                FontLib& _fontLib,
                const std::string& _name,
                const FaceId::Byte* _pData,
                FaceId::Size _dataSize,
                size_t _size);

            Font(const Font& _copy);

            void init(size_t _size);

            virtual ~Font();
            Font& operator=(const Font& _copy);

            static FT_Error FaceRequester(
                FTC_FaceID _faceId,
                FT_Library _library,
                FT_Pointer _requestData,
                FT_Face* _aFace);

            const FontLib& fontLib() const;
            const FaceId& faceId() const;
            const FT_Face& face() const;
            const FTC_ScalerRec& scaler() const;
            size_t size() const;
            void setSize(size_t _size);
            bool hasKerning() const;
            size_t toNextOrigin() const;
    };

    class Text : public Loggable
    {
        public:
            typedef std::basic_string<uint32_t> UString;
            typedef std::vector<FT_Glyph>::iterator GlyphIterator;

        private:
            Font m_font;
            UString m_text;

            Pos2d<int> m_pos;
            int m_width;
            FT_Matrix m_transform;

            RGBA m_color;

            RGBA m_bgColor;
            bool m_useBgColor;

            RGBA m_oColor;
            int m_oSize;

            size_t m_selBegin;
            size_t m_selEnd;
            bool m_useSelection;

            // CACHE
            bool m_dirty;
            std::vector<FT_Glyph> m_glyphs;
            Rect<int> m_rect;

        public:
            Text(const Font& _font, const std::string& _text);
            virtual ~Text();
            void release();

            const Font& font() const;
            void setFont(const Font& _font);

            const UString& text() const;
            void setText(const std::string& _text);

            const Pos2d<int>& pos() const;
            void setPos(int _x, int _y);

            void setWidth(int _width);

            void rotate(FPType _degree);

            void setColor(const RGBA& _color);

            void setBgColor(const RGBA& _color);
            void useBgColor(bool _use=true);

            void setOutlineColor(const RGBA& _color);
            void setOutlineSize(int _size);

            void select(size_t _begin, size_t _end);
            void deselect();

            Rect<int> rect();
            void render(Texture& _texture);

        private:
            void validate();
            void genGlyphs();
            void calcRect();
    };
}

#endif
