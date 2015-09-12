/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
 *
 *	Foobar is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	Foobar is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <SDL_assert.h>

#include <algorithm>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_render_target.h>

namespace lite3dpp
{
    RenderTarget::RenderTarget(const String &name, 
        const String &path, Main *main) : 
        JsonResource(name, path, main, AbstractResource::RENDER_TARGET),
        mRenderTarget(NULL)
    {}

    RenderTarget::~RenderTarget()
    {}

    bool RenderTarget::isEnabled()
    {
        SDL_assert_release(mRenderTarget);
        return mRenderTarget->enabled == LITE3D_TRUE;
    }

    void RenderTarget::enable()
    {
        SDL_assert_release(mRenderTarget);
        mRenderTarget->enabled = LITE3D_TRUE;
    }

    void RenderTarget::disable()
    {
        SDL_assert_release(mRenderTarget);
        mRenderTarget->enabled = LITE3D_FALSE;
    }

    int32_t RenderTarget::height()
    {
        SDL_assert_release(mRenderTarget);
        return mRenderTarget->height;
    }

    int32_t RenderTarget::width()
    {
        SDL_assert_release(mRenderTarget);
        return mRenderTarget->width;
    }

    void RenderTarget::setBlankColor(const kmVec4 &color)
    {
        SDL_assert_release(mRenderTarget);
        mRenderTarget->cleanColor = color;
    }

    void RenderTarget::setCleanMask(uint32_t mask)
    {
        SDL_assert_release(mRenderTarget);
        mRenderTarget->cleanMask = mask;
    }


    void RenderTarget::addCamera(Camera *camera, uint16_t pass, int priority)
    {
        SDL_assert_release(mRenderTarget);
        lite3d_render_target_attach_camera(mRenderTarget, camera->getPtr(), pass, priority);
    }

    void RenderTarget::removeCamera(Camera *camera, uint16_t pass, int priority)
    {
        SDL_assert_release(mRenderTarget);
        lite3d_render_target_dettach_camera(mRenderTarget, camera->getPtr(), pass, priority);
    }

    WindowRenderTarget::WindowRenderTarget(const String &name, 
        const String &path, Main *main) : 
        RenderTarget(name, path, main)
    {

    }

    WindowRenderTarget::~WindowRenderTarget()
    {}

    void WindowRenderTarget::loadFromJsonImpl(const JsonHelper &helper)
    {
        mRenderTarget = lite3d_render_target_screen_get();
    }

    void WindowRenderTarget::unloadImpl()
    {}

}

