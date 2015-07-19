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

#include <3dlite/3dlite_camera.h>

#include <3dlitepp/3dlitepp_common.h>
#include <3dlitepp/3dlitepp_scene_object.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Camera : public NoncopiableResource, public Manageable
    {
    public:

        Camera(const lite3dpp_string &name, Main *main);
        ~Camera();

        inline lite3d_camera *getPtr()
        { return &mCamera; }
        inline const lite3d_camera *getPtr() const
        { return &mCamera; }

        inline bool isOrtho()
        { return mCamera.isOrtho == LITE3D_TRUE; }

        inline lite3dpp_string getName()
        { return mName; }

        /* camera projection modes */
        void setupOrtho(float near, float far, float left, float right, 
            float bottom, float top);
        void setupPerspective(float znear, float zfar, float fovy, float aspect);

        void lookAt(const kmVec3 &pointTo);
        void setPosition(const kmVec3 &position);
        void setRotation(const kmQuaternion &orietation);
        void rotate(const kmQuaternion &orietation);
        void yaw(float angle);
        void pitch(float angle);
        void roll(float angle);
        void move(const kmVec3 &value);
        void moveRelative(const kmVec3 &value);
        void holdOnSceneObject(const SceneObject &sceneObj);
        void linkWithSceneObject(const SceneObject &sceneObj);

    private:

        lite3dpp_string mName; 
        Main *mMain;
        lite3d_camera mCamera;
    };
}

