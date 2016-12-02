/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
 *
 *	Lite3D is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Lite3D is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <SDL_log.h>

#include <font.h>
#include <texture.h>
#include <log/logger.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp_font/lite3dpp_font_texture.h>

nw::FontLib lite3dpp::lite3dpp_font::FontTexture::gFontLib;

namespace lite3dpp
{
    namespace lite3dpp_font
    {
        class FontLibLogger : public nw::Logger
        {
        public:

            virtual void log(
                Level _level,
                const char* _msg,
                const char* _function,
                const char* _file,
                int _line) override
            {
                lite3dpp::Stringstream ss;
                ss << _file << ":" << _function << ": " << _msg;

                switch(_level)
                {
                case nw::Logger::Info:
                    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, ss.str().c_str());
                    break;
                case nw::Logger::Error:
                    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, ss.str().c_str());
                    break;
                case nw::Logger::Warning:
                    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ss.str().c_str());
                    break;
                case nw::Logger::Debug:
                    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, ss.str().c_str());
                    break;
                }
            }
        } gFontLibLogger;

        FontTexture::FontTexture(const String &name,
            const String &path, Main *main) : 
            Texture(name, path, main),
            mTexBuf(new nw::Texture())
        {
            gFontLib.setLogger(&gFontLibLogger);
        }
        
        FontTexture::~FontTexture()
        {
            if(mText)
                mText->release();
        }
        
        void FontTexture::loadFromConfigImpl(const ConfigurationReader &helper)
        {
            Texture::loadFromConfigImpl(helper);
            
            if(getPtr()->texFormat != LITE3D_TEXTURE_FORMAT_RGBA)
                LITE3D_THROW("Font texture must be in RGBA format");
            if(!helper.has(L"BlankColor"))
                LITE3D_THROW("BlankColor parameter not found");
            if(!helper.has(L"Font"))
                LITE3D_THROW("Font parameter not found");
            if(!helper.has(L"FontSize"))
                LITE3D_THROW("FontSize parameter not found");
                
            if(!mFont)
            {
                const lite3d_file *fontFile = mMain->getResourceManager()->loadFileToMemory(helper.getString(L"Font"));
                mFont.reset(new nw::Font(gFontLib, helper.getString(L"Font"), (const nw::FaceId::Byte *)fontFile->fileBuff, 
                    fontFile->fileSize, helper.getInt(L"FontSize")));
                mFont->setLogger(&gFontLibLogger);

                mText.reset(new nw::Text(*mFont, ""));
                mText->setLogger(&gFontLibLogger);
                mTexBuf->resize(getWidth(), getHeight());
            }
        }
        
        void FontTexture::reloadFromConfigImpl(const ConfigurationReader &helper)
        {
            Texture::reloadFromConfigImpl(helper);
        }
        
        void FontTexture::unloadImpl()
        {
            Texture::unloadImpl();
        }
        
        void FontTexture::clean(const kmVec4 &color)
        {
            mTexBuf->fill(nw::RGBA(color.x * 255,
                color.y * 255, 
                color.z * 255, 
                color.w * 255));
        }
            
        void FontTexture::clean()
        {
            clean(getJson().getVec4(L"BlankColor"));
        }
        
        void FontTexture::uploadChanges()
        {
            setPixels(0, &mTexBuf->data()[0]);
            generateMipmaps();
        }
        
        void FontTexture::drawText(const String &text, const kmVec2 &pos, const kmVec4 &color)
        {
            if(!mText)
                LITE3D_THROW("Resource is not initialized");

            mText->setPos(pos.x, pos.y);
            mText->setWidth(getWidth() - pos.x);
            mText->setColor(nw::RGBA(color.x * 255,
                color.y * 255, 
                color.z * 255, 
                color.w * 255));
            mText->setText(text);
            mText->render(*mTexBuf);
        }
    }
}

