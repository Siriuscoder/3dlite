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
        RenderTarget(name, path, main),
        mRenderTargetBlitTo(nullptr)
    {
        mRenderTargetPtr = &mRenderTarget;
        addObserver(this);
    }

    TextureRenderTarget::~TextureRenderTarget()
    {
        removeObserver(this);
    }

    void TextureRenderTarget::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        int32_t width = helper.getInt(L"Width", 0), 
            height = helper.getInt(L"Height", 0);
        int32_t scale = helper.getInt(L"Scale", 1);
        uint32_t flags = 0;
        stl<lite3d_texture_unit *>::vector colorAttachments;
        lite3d_texture_unit *depthAttachment = NULL;

        /* use screen size if not specified */
        if(width == 0 && height == 0)
        {
            width = mMain->window()->width() / scale;
            height = mMain->window()->height() / scale;
        }

        lite3d_render_target_init(mRenderTargetPtr, width, height);
        mRenderTargetPtr->userdata = this;
        mRenderTargetPtr->preUpdate = RenderTarget::beginUpdate;
        mRenderTargetPtr->postUpdate = RenderTarget::postUpdate;

        setBackgroundColor(helper.getVec4(L"BackgroundColor"));
        setBuffersCleanBit(helper.getBool(L"CleanColorBuf", true),
            helper.getBool(L"CleanDepthBuf", true), helper.getBool(L"CleanStencilBuf", true));

        {
            ConfigurationReader attachmentJson = helper.getObject(L"ColorAttachments");
            for(const ConfigurationReader &targetJson : attachmentJson.getObjects(L"Attachments"))
            {
                colorAttachments.push_back(mMain->getResourceManager()->queryResource<TextureImage>(
                    targetJson.getString(L"TextureName"), targetJson.getString(L"TexturePath"))->getPtr());
            }

            if (attachmentJson.getBool(L"Renderbuffer", false))
            {
                flags |= LITE3D_FRAMEBUFFER_USE_COLOR_BUFFER;
                if (helper.has(L"RenderbufferInternalFormat"))
                {
                    mRenderTargetPtr->fb.rbIntFormat = helper.getInt(L"RenderbufferInternalFormat");
                }
            }

            if (colorAttachments.size() > 0)
            {
                flags |= LITE3D_FRAMEBUFFER_USE_COLOR_BUFFER;
            }
        }

        {
            ConfigurationReader attachmentJson = helper.getObject(L"DepthAttachments");
            if(attachmentJson.has(L"TextureName"))
                depthAttachment = mMain->getResourceManager()->queryResource<TextureImage>(
                    attachmentJson.getString(L"TextureName"), attachmentJson.getString(L"TexturePath"))->getPtr();

            if (attachmentJson.getBool(L"Renderbuffer", false) || depthAttachment)
            {
                flags |= LITE3D_FRAMEBUFFER_USE_DEPTH_BUFFER;
            }
        }

        {
            ConfigurationReader attachmentJson = helper.getObject(L"StencilAttachments");
            if (attachmentJson.getBool(L"Renderbuffer", false))
            {
                flags |= LITE3D_FRAMEBUFFER_USE_STENCIL_BUFFER;
            }
        }

        switch (helper.getInt(L"MSAA", 1))
        {
        case 2:
            flags |= LITE3D_FRAMEBUFFER_USE_MSAA_X2;
            break;
        case 4:
            flags |= LITE3D_FRAMEBUFFER_USE_MSAA_X4;
            break;
        case 8:
            flags |= LITE3D_FRAMEBUFFER_USE_MSAA_X8;
            break;
        case 16:
            flags |= LITE3D_FRAMEBUFFER_USE_MSAA_X16;
            break;
        }

        /* setup render target framebuffer */
        if (!lite3d_framebuffer_setup(&mRenderTargetPtr->fb,
            colorAttachments.size() > 0 ? &colorAttachments[0] : NULL,
            static_cast<int8_t>(colorAttachments.size()),
            depthAttachment, flags))
        {
            LITE3D_THROW(getName() << " framebuffer setup failed.. ");
        }

        if (helper.has(L"BlitResultTo"))
        {
            ConfigurationReader rtConf = helper.getObject(L"BlitResultTo");
            if (rtConf.getString(L"Name") == "Window")
                mRenderTargetBlitTo = mMain->window();
            else
            {
                mRenderTargetBlitTo = mMain->getResourceManager()->queryResource<TextureRenderTarget>(
                    rtConf.getString(L"Name"),
                    rtConf.getString(L"Path"));
            }
        }

        // -1 is detached render target
        int32_t priority = helper.getInt(L"Priority", -1);
        if (priority >= 0)
            lite3d_render_target_add(mRenderTargetPtr, priority);
    }

    void TextureRenderTarget::postUpdate(RenderTarget *rt)
    {
        if (mRenderTargetBlitTo && mRenderTargetBlitTo->getPtr())
        {
            if (getPtr())
            {
                lite3d_framebuffer_blit(&getPtr()->fb, &mRenderTargetBlitTo->getPtr()->fb);
            }
        }
    }

    void TextureRenderTarget::unloadImpl()
    {
        lite3d_render_target_purge(mRenderTargetPtr);
    }
}

