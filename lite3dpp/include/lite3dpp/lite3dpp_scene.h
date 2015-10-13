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

#include <lite3d/lite3d_scene.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_resource.h>
#include <lite3dpp/lite3dpp_scene_object.h>
#include <lite3dpp/lite3dpp_camera.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Scene : public ConfigurableResource, public NoncopiableResource
    {
    public:

        typedef stl<String, Camera *>::map Cameras;
        typedef stl<String, SceneObject *>::map Objects;

        Scene(const String &name, 
            const String &path, Main *main);
        ~Scene();

        inline lite3d_scene *getPtr()
        { return &mScene; }

        Camera *addCamera(const String &name);
        SceneObject *addObject(const String &name,
            const String &templatePath, SceneObject *parent);
        SceneObject *getObject(const String &name);
        Camera *getCamera(const String &name);

        void removeAllCameras();
        void removeCamera(const String &name);
        void removeAllObjects();
        void removeObject(const String &name);

    protected:

        virtual void loadFromConfigImpl(const ConfigurationReader &helper) override;
        virtual void unloadImpl() override;

    private:

        void setupObjects(const stl<ConfigurationReader>::vector &objects, SceneObject *base);
        void setupCameras(const stl<ConfigurationReader>::vector &cameras);

        lite3d_scene mScene;
        Cameras mCameras;
        Objects mObjects;
    };
}

