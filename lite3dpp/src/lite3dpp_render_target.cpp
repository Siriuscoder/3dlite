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

#include <lite3d/lite3d_buffers_manip.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_render_target.h>

namespace lite3dpp
{
    RenderTarget::RenderTarget(const String &name, 
        const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::RENDER_TARGET),
        mRenderTargetPtr(NULL)
    {}

    RenderTarget::~RenderTarget()
    {}

    bool RenderTarget::isEnabled()
    {
        SDL_assert_release(mRenderTargetPtr);
        return mRenderTargetPtr->enabled == LITE3D_TRUE;
    }

    void RenderTarget::enable()
    {
        SDL_assert_release(mRenderTargetPtr);
        mRenderTargetPtr->enabled = LITE3D_TRUE;
    }

    void RenderTarget::disable()
    {
        SDL_assert_release(mRenderTargetPtr);
        mRenderTargetPtr->enabled = LITE3D_FALSE;
    }

    int32_t RenderTarget::height()
    {
        SDL_assert_release(mRenderTargetPtr);
        return mRenderTargetPtr->height;
    }

    int32_t RenderTarget::width()
    {
        SDL_assert_release(mRenderTargetPtr);
        return mRenderTargetPtr->width;
    }

    void RenderTarget::setBackgroundColor(const kmVec4 &color)
    {
        SDL_assert_release(mRenderTargetPtr);
        mRenderTargetPtr->cleanColor = color;
    }

    void RenderTarget::setCleanDepth(float value)
    {
        SDL_assert_release(mRenderTargetPtr);
        mRenderTargetPtr->cleanDepth = value;
    }

    void RenderTarget::setBuffersCleanBit(bool color, bool depth, bool stencil)
    {
        SDL_assert_release(mRenderTargetPtr);
        mRenderTargetPtr->clearColorBuffer = color ? LITE3D_TRUE : LITE3D_FALSE;
        mRenderTargetPtr->clearDepthBuffer = depth ? LITE3D_TRUE : LITE3D_FALSE;
        mRenderTargetPtr->clearStencilBuffer = stencil ? LITE3D_TRUE : LITE3D_FALSE;
    }

    void RenderTarget::depthOutput(bool flag)
    {
        lite3d_depth_output(flag ? LITE3D_TRUE : LITE3D_FALSE);
    }

    void RenderTarget::colorOutput(bool r, bool g, bool b, bool a)
    {
        lite3d_color_output(r ? LITE3D_TRUE : LITE3D_FALSE,
            g ? LITE3D_TRUE : LITE3D_FALSE, 
            b ? LITE3D_TRUE : LITE3D_FALSE, 
            a ? LITE3D_TRUE : LITE3D_FALSE);
    }

    void RenderTarget::stencilOutput(bool flag)
    {
        lite3d_stencil_output(flag ? LITE3D_TRUE : LITE3D_FALSE);
    }

    void RenderTarget::depthTest(bool flag)
    {
        lite3d_depth_test(flag ? LITE3D_TRUE : LITE3D_FALSE);
    }

    void RenderTarget::depthTestFunc(uint32_t func)
    {
        lite3d_depth_test_func(func);
    }

    void RenderTarget::stencilTest(bool flag)
    {
        lite3d_stencil_test(flag ? LITE3D_TRUE : LITE3D_FALSE);
    }

    void RenderTarget::stencilTestFunc(uint32_t func, int32_t value)
    {
        lite3d_stencil_test_func(func, value);
    }

    void RenderTarget::clear(bool color, bool depth, bool stencil)
    {
        SDL_assert_release(mRenderTargetPtr);
        lite3d_buffers_clear_values(&mRenderTargetPtr->cleanColor,
            mRenderTargetPtr->cleanDepth, mRenderTargetPtr->cleanStencil);
        lite3d_buffers_clear(color ? LITE3D_TRUE : LITE3D_FALSE,
            depth ? LITE3D_TRUE : LITE3D_FALSE,
            stencil ? LITE3D_TRUE : LITE3D_FALSE);
    }

    void RenderTarget::addCamera(Camera *camera, uint16_t pass, int priority)
    {
        SDL_assert_release(mRenderTargetPtr);
        lite3d_render_target_attach_camera(mRenderTargetPtr, camera->getPtr(), pass, priority);
    }

    void RenderTarget::removeCamera(Camera *camera, uint16_t pass, int priority)
    {
        SDL_assert_release(mRenderTargetPtr);
        lite3d_render_target_dettach_camera(mRenderTargetPtr, camera->getPtr(), pass, priority);
    }

    WindowRenderTarget::WindowRenderTarget(const String &name, 
        const String &path, Main *main) : 
        RenderTarget(name, path, main)
    {

    }

    WindowRenderTarget::~WindowRenderTarget()
    {}
    
    void WindowRenderTarget::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        mRenderTargetPtr = lite3d_render_target_screen_get();
    }
    
    void WindowRenderTarget::unloadImpl()
    {}

}

