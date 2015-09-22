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
#pragma once

#include <lite3d/lite3d_render.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_camera.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT RenderTarget : public JsonResource, public NoncopiableResource
    {
    public:

        RenderTarget(const String &name, 
            const String &path, Main *main);
        ~RenderTarget();

        bool isEnabled();
        void enable();
        void disable();
        int32_t height();
        int32_t width();
        void setBackgroundColor(const kmVec4 &color);
        void setCleanMask(uint32_t mask);

        inline lite3d_render_target *getPtr()
        { return mRenderTargetPtr; }

        void addCamera(Camera *camera, uint16_t pass, int priority);
        void removeCamera(Camera *camera, uint16_t pass, int priority);

    protected:

        lite3d_render_target *mRenderTargetPtr;
    };

    class LITE3DPP_EXPORT WindowRenderTarget : public RenderTarget
    {
    public:

        WindowRenderTarget(const String &name, 
            const String &path, Main *main);
        ~WindowRenderTarget();

    protected:

        virtual void loadFromJsonImpl(const JsonHelper &helper) override final;
        virtual void unloadImpl() override final;
    };
}

