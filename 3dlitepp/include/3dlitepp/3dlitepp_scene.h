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
#include <3dlitepp/3dlitepp_scene_object.h>
#include <3dlitepp/3dlitepp_camera.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Scene : public JsonResource, public NoncopiableResource
    {
    public:

        typedef stl<lite3dpp_string, Camera *>::map Cameras;
        typedef stl<lite3dpp_string, SceneObject *>::map Objects;

        Scene(const lite3dpp_string &name, 
            const lite3dpp_string &path, Main *main);
        ~Scene();

        inline lite3d_scene *getPtr()
        { return &mScene; }

        Camera *addCamera(const lite3dpp_string &name);
        SceneObject *addObject(const lite3dpp_string &name,
            const lite3dpp_string &templatePath, SceneObject *parent);
        SceneObject *getObject(const lite3dpp_string &name);

        void removeAllCameras();
        void removeCamera(const lite3dpp_string &name);
        void removeAllObjects();
        void removeObject(const lite3dpp_string &name);

    protected:

        virtual void loadFromJsonImpl(const JsonHelper &helper);
        virtual void unloadImpl();

    private:

        void setupObjects(const stl<JsonHelper>::vector &objects, SceneObject *base);

        lite3d_scene mScene;
        Cameras mCameras;
        Objects mObjects;
    };
}

