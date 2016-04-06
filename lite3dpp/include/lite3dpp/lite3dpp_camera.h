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

#include <lite3d/lite3d_camera.h>

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_scene_object.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT Camera : public Noncopiable, public Manageable
    {
    public:

        Camera(const String &name, Main *main);
        ~Camera();

        inline lite3d_camera *getPtr()
        { return &mCamera; }
        inline const lite3d_camera *getPtr() const
        { return &mCamera; }

        inline bool isOrtho()
        { return mCamera.isOrtho == LITE3D_TRUE; }

        inline String getName()
        { return mName; }
        inline void showWireframe(bool flag)
        { mCamera.polygonMode = flag ? LITE3D_POLYMODE_LINE : LITE3D_POLYMODE_FILL; }
        inline void cullBackFaces(bool flag)
        { mCamera.cullBackFaces = flag ? LITE3D_TRUE : LITE3D_FALSE; }

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
        void rotateY(float angle);
        void rotateX(float angle);
        void rotateZ(float angle);
        void move(const kmVec3 &value);
        void moveRelative(const kmVec3 &value);
        void holdOnSceneObject(const SceneObject &sceneObj);
        void linkWithSceneObject(const SceneObject &sceneObj);

    private:

        String mName; 
        Main *mMain;
        lite3d_camera mCamera;
    };
}

