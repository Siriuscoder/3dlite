/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
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

#include <3dlite/3dlite_render.h>

#include <3dlitepp/3dlitepp_common.h>
#include <3dlitepp/3dlitepp_resource.h>
#include <3dlitepp/3dlitepp_camera.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT RenderTarget : public JsonResource, public NoncopiableResource
    {
    public:

        RenderTarget(const lite3dpp_string &name, 
            const lite3dpp_string &path, Main *main);
        ~RenderTarget();

        bool isEnabled();
        void enable();
        void disable();
        int32_t height();
        int32_t width();
        void setBlankColor(const kmVec4 &color);
        void setCleanMask(uint32_t mask);

        void addCamera(Camera *camera, uint16_t pass);
        void removeCamera(Camera *camera, uint16_t pass);

    protected:

        lite3d_render_target *mRenderTarget;
    };

    class LITE3DPP_EXPORT WindowRenderTarget : public RenderTarget
    {
    public:

        WindowRenderTarget(const lite3dpp_string &name, 
            const lite3dpp_string &path, Main *main);
        ~WindowRenderTarget();

    protected:

        virtual void loadFromJsonImpl(const JsonHelper &helper) override final;
        virtual void unloadImpl() override final;
    };
}

