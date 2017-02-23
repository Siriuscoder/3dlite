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
#include <SDL_assert.h>

#include <algorithm>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_texture_render_target.h>

namespace lite3dpp
{
    TextureRenderTarget::TextureRenderTarget(const String &name, 
        const String &path, Main *main) : 
        RenderTarget(name, path, main)
    {
        mRenderTargetPtr = &mRenderTarget;
        mRenderTargetPtr->userdata = this;
    }

    TextureRenderTarget::~TextureRenderTarget()
    {}

    void TextureRenderTarget::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        int32_t width = helper.getInt(L"Width", 0), 
            height = helper.getInt(L"Height", 0);
        int8_t attachColorRenderBuffer = LITE3D_FALSE;
        int8_t attachDepthRenderBuffer = LITE3D_FALSE;
        int8_t attachStencilRenderBuffer = LITE3D_FALSE;
        stl<lite3d_texture_unit *>::vector colorAttachments;
        lite3d_texture_unit *depthAttachment = NULL;

        /* use screen size if not specified */
        if(width == 0 && height == 0)
        {
            width = mMain->window()->width();
            height = mMain->window()->height();
        }

        lite3d_render_target_init(mRenderTargetPtr, width, height);
        setBackgroundColor(helper.getVec4(L"BackgroundColor"));
        setBuffersCleanBit(helper.getBool(L"CleanColorBuf", true),
            helper.getBool(L"CleanDepthBuf", true), helper.getBool(L"CleanStencilBuf", true));

        {
            ConfigurationReader attachmentJson = helper.getObject(L"ColorAttachments");
            attachColorRenderBuffer = attachmentJson.getBool(L"Renderbuffer", false) ? LITE3D_TRUE : LITE3D_FALSE;
            for(const ConfigurationReader &targetJson : attachmentJson.getObjects(L"Attachments"))
            {
                colorAttachments.push_back(mMain->getResourceManager()->queryResource<TextureImage>(
                    targetJson.getString(L"TextureName"), targetJson.getString(L"TexturePath"))->getPtr());
            }
        }

        {
            ConfigurationReader attachmentJson = helper.getObject(L"DepthAttachments");
            attachDepthRenderBuffer = attachmentJson.getBool(L"Renderbuffer", false) ? LITE3D_TRUE : LITE3D_FALSE;
            if(!attachDepthRenderBuffer && !attachmentJson.isEmpty())
                depthAttachment = mMain->getResourceManager()->queryResource<TextureImage>(
                    attachmentJson.getString(L"TextureName"), attachmentJson.getString(L"TexturePath"))->getPtr();
        }

        {
            ConfigurationReader attachmentJson = helper.getObject(L"StencilAttachments");
            attachStencilRenderBuffer = attachmentJson.getBool(L"Renderbuffer", false) ? LITE3D_TRUE : LITE3D_FALSE;
        }

        /* setup render target framebuffer */
        if (!lite3d_framebuffer_setup(&mRenderTargetPtr->fb, 
            colorAttachments.size() > 0 ? &colorAttachments[0] : NULL, colorAttachments.size(), attachColorRenderBuffer,
            depthAttachment, attachDepthRenderBuffer, attachStencilRenderBuffer))
            LITE3D_THROW(getName() << " framebuffer setup failed.. ");
            
        lite3d_render_target_add(mRenderTargetPtr, helper.getInt(L"Priority"));
    }

    void TextureRenderTarget::unloadImpl()
    {
        lite3d_render_target_purge(mRenderTargetPtr);
    }

}

