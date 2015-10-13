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
#include <lite3dpp/lite3dpp_render_target.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT TextureRenderTarget : public RenderTarget
    {
    public:

        TextureRenderTarget(const String &name, 
            const String &path, Main *main);
        ~TextureRenderTarget();

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;

    private:

        lite3d_render_target mRenderTarget;
    };
}
