/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024 Sirius (Korolev Nikita)
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
#include <lite3dpp/lite3dpp_observer.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT RenderTarget : public Observable<RenderTargetObserver>, 
        public ConfigurableResource, public Noncopiable
    {
    public:

        using RenderLayers = stl<lite3d_framebuffer_layer>::vector;

        enum TestFunc : uint32_t
        {
            TestFuncNever = LITE3D_TEST_NEVER,
            TestFuncLess = LITE3D_TEST_LESS,
            TestFuncEqual = LITE3D_TEST_EQUAL,
            TestFuncLEqual = LITE3D_TEST_LEQUAL,
            TestFuncGreater = LITE3D_TEST_GREATER,
            TestFuncNotEqual = LITE3D_TEST_NOTEQUAL,
            TestFuncGEqual = LITE3D_TEST_GEQUAL,
            TestFuncAlways = LITE3D_TEST_ALWAYS
        };
        
    public:

        /* output buffers write control */
        /* perform immediately */
        static void depthOutput(bool flag);
        static void colorOutput(bool flag);
        static void stencilOutput(bool flag);
        /* Buffer testing control */
        static void depthTest(bool flag);
        static void depthTestFunc(TestFunc func);
        static void stencilTest(bool flag);
        static void stencilTestFunc(TestFunc func, int32_t value);

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
        /* clean buffers immediately */
        void clear(bool color, bool depth, bool stencil);
        void saveScreenshot(const String &filename);
        void resize(int32_t width, int32_t height);

        inline lite3d_render_target *getPtr()
        { return mRenderTargetPtr; }

        void addCamera(Camera *camera, Scene *scene, uint16_t pass, const RenderLayers &layers,
            int priority, uint32_t renderFlags);
        void removeCamera(Camera *camera, int priority);
        void setActive();

    protected:

        static int beginUpdate(lite3d_render_target *target);
        static void postUpdate(lite3d_render_target *target);

        lite3d_render_target *mRenderTargetPtr;
    };

    class LITE3DPP_EXPORT WindowRenderTarget : public RenderTarget
    {
    public:

        WindowRenderTarget(const String &name, 
            const String &path, Main *main);
        ~WindowRenderTarget();

        void fullscreen(bool flag);
        float computeCameraAspect() const;

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override final;
        virtual void unloadImpl() override final;
    };
}

