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
#include <SDL_log.h>

#include <algorithm>

#include <lite3d/lite3d_buffers_manip.h>

#include <lite3dpp/lite3dpp_main.h>
#include <lite3dpp/lite3dpp_render_target.h>

namespace lite3dpp
{
    RenderTarget::RenderTarget(const String &name, 
        const String &path, Main *main) : 
        ConfigurableResource(name, path, main, AbstractResource::RENDER_TARGET),
        mRenderTargetPtr(nullptr)
    {}

    RenderTarget::~RenderTarget()
    {}

    bool RenderTarget::isEnabled() const
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

    int32_t RenderTarget::height() const
    {
        SDL_assert_release(mRenderTargetPtr);
        return mRenderTargetPtr->height;
    }

    int32_t RenderTarget::width() const
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

    void RenderTarget::colorOutput(bool flag)
    {
        lite3d_color_output(flag ? LITE3D_TRUE : LITE3D_FALSE);
    }

    void RenderTarget::stencilOutput(bool flag)
    {
        lite3d_stencil_output(flag ? LITE3D_TRUE : LITE3D_FALSE);
    }

    void RenderTarget::depthTest(bool flag)
    {
        lite3d_depth_test(flag ? LITE3D_TRUE : LITE3D_FALSE);
    }

    void RenderTarget::depthTestFunc(TestFunc func)
    {
        lite3d_depth_test_func(func);
    }

    void RenderTarget::stencilTest(bool flag)
    {
        lite3d_stencil_test(flag ? LITE3D_TRUE : LITE3D_FALSE);
    }

    void RenderTarget::stencilTestFunc(TestFunc func, int32_t value)
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

    void RenderTarget::addCamera(Camera *camera, Scene *scene, uint16_t pass, int priority, uint32_t renderFlags)
    {
        SDL_assert_release(mRenderTargetPtr);
        if (!lite3d_render_target_attach_camera(mRenderTargetPtr, camera->getPtr(), scene->getPtr(), pass, NULL, 0, priority, renderFlags))
            LITE3D_THROW("Failed to add camera to render target '" << getName() << "', probably camera with priority " << 
                priority << " already exist");
    }

    void RenderTarget::removeCamera(Camera *camera, int priority)
    {
        SDL_assert_release(mRenderTargetPtr);
        if (!lite3d_render_target_dettach_camera(mRenderTargetPtr, camera->getPtr(), priority))
            LITE3D_THROW("Failed to detach camera from render target '" << getName() << "', camera with priority " <<
                priority << " not found");
    }
    
    void RenderTarget::saveScreenshot(const String &filename)
    {
        SDL_assert_release(mRenderTargetPtr);
        lite3d_render_target_screenshot(mRenderTargetPtr, filename.c_str());
    }

    void RenderTarget::resize(int32_t width, int32_t height)
    {}

    int RenderTarget::beginUpdate(lite3d_render_target *target)
    {
        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_CHECK_1(reinterpret_cast<RenderTarget *>(target->userdata), beginUpdate,
                reinterpret_cast<RenderTarget *>(target->userdata));
            LITE3D_EXT_OBSERVER_RETURN
        }
        catch (std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }

        return LITE3D_FALSE;
    }

    void RenderTarget::postUpdate(lite3d_render_target *target)
    {
        try
        {
            LITE3D_EXT_OBSERVER_NOTIFY_1(reinterpret_cast<RenderTarget *>(target->userdata), postUpdate,
                reinterpret_cast<RenderTarget *>(target->userdata));
        }
        catch (std::exception &ex)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, ex.what());
        }
    }

    WindowRenderTarget::WindowRenderTarget(const String &name, 
        const String &path, Main *main) : 
        RenderTarget(name, path, main)
    {}

    WindowRenderTarget::~WindowRenderTarget()
    {}
    
    void WindowRenderTarget::loadFromConfigImpl(const ConfigurationReader &helper)
    {
        mRenderTargetPtr = lite3d_render_target_screen_get();
        mRenderTargetPtr->userdata = this;
        mRenderTargetPtr->preUpdate = beginUpdate;
        mRenderTargetPtr->postUpdate = postUpdate;
    }
    
    void WindowRenderTarget::unloadImpl()
    {}

    void WindowRenderTarget::resize(int32_t width, int32_t height)
    {
        lite3d_render_target_resize(mRenderTargetPtr, width, height);
    }

    void WindowRenderTarget::fullscreen(bool flag)
    {
        lite3d_render_target_fullscreen(mRenderTargetPtr, flag);
    }

    float WindowRenderTarget::computeCameraAspect() const
    {
        return (float)width() / (float)height();
    }
}

