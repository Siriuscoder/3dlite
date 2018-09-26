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

#include <lite3d/lite3d_render.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_camera.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT RenderTarget : public ConfigurableResource, public Noncopiable
    {
    public:

        RenderTarget(const String &name, 
            const String &path, Main *main);
        virtual ~RenderTarget();

        bool isEnabled() const;
        void enable();
        void disable();
        int32_t height() const;
        int32_t width() const;
        void setBackgroundColor(const kmVec4 &color);
        void setCleanDepth(float value);
        /* clean buffers control, clean operation perform then 
           this render target begin updating */
        void setBuffersCleanBit(bool color, bool depth, bool stencil);
        /* output buffers write control */
        /* perform immediately */
        void depthOutput(bool flag);
        void colorOutput(bool flag);
        void stencilOutput(bool flag);
        /* Buffer testing control */
        void depthTest(bool flag);
        void depthTestFunc(uint32_t func);
        void stencilTest(bool flag);
        void stencilTestFunc(uint32_t func, int32_t value);
        /* clean buffers immediately */
        void clear(bool color, bool depth, bool stencil);
        void saveScreenshot(const String &filename);

        virtual void resize(int32_t width, int32_t height);

        inline lite3d_render_target *getPtr()
        { return mRenderTargetPtr; }

        void addCamera(Camera *camera, Scene *scene, uint16_t pass, int priority, uint32_t renderFlags);
        void removeCamera(Camera *camera, int priority);

    protected:

        lite3d_render_target *mRenderTargetPtr;
    };

    class LITE3DPP_EXPORT WindowRenderTarget : public RenderTarget
    {
    public:

        WindowRenderTarget(const String &name, 
            const String &path, Main *main);
        ~WindowRenderTarget();

        virtual void resize(int32_t width, int32_t height) override;
        void fullscreen(bool flag);
        float computeCameraAspect() const;

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override final;
        virtual void unloadImpl() override final;
    };
}

