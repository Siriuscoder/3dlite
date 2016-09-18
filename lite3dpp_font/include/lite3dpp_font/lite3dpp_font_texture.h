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
#pragma once

#include <lite3dpp_font/lite3dpp_font_common.h>
#include <lite3dpp/lite3dpp_texture.h>

namespace lite3dpp
{
    namespace lite3dpp_font
    {
        class LITE3DPP_FONT_EXPORT FontTexture : public Texture
        {
        public:

            FontTexture(const String &name,
                const String &path, Main *main);

            ~FontTexture();
            
            void clean(const kmVec4 &color);
            void clean();
            void uploadChanges();

            void drawText(const String &text, const kmVec2 &pos, const kmVec4 &color);

        protected:

            virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
            virtual void reloadFromConfigImpl(const ConfigurationReader &helper) override;
            virtual void unloadImpl() override;

        private:

            static nw::FontLib gFontLib;
            std::unique_ptr<nw::Font> mFont;
            std::unique_ptr<nw::Text> mText;
            nw::Texture mTexBuf;
        };
    }
}
