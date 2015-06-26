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

#include <3dlite/3dlite_scene.h>

#include <3dlitepp/3dlitepp_common.h>
#include <3dlitepp/3dlitepp_resource.h>
#include <3dlitepp/3dlitepp_json_helper.h>
#include <3dlitepp/3dlitepp_mesh.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT SceneObject : public AbstractResource
    {
    public:

        typedef stl<lite3dpp_string, lite3d_scene_node>::map Nodes;

        SceneObject(const lite3dpp_string &name, 
            const lite3dpp_string &path, Main *main);
        ~SceneObject();

    protected:

        virtual void loadImpl(const void *buffer, size_t size);
        virtual void unloadImpl();
        virtual void reloadImpl();

    private:

        JsonHelper *mOptions;
        Nodes mNodes;
    };
}

