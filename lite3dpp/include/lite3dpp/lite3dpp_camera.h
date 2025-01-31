/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
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
    class LITE3DPP_EXPORT Camera : public SceneObjectBase
    {
    public:

        Camera(const String &name, Main *main);
        virtual ~Camera() = default;

        inline lite3d_camera *getPtr()
        { return &mCamera; }
        inline const lite3d_camera *getPtr() const
        { return &mCamera; }

        inline bool isOrtho()
        { return mCamera.isOrtho == LITE3D_TRUE; }

        /* camera projection modes */
        void setupOrtho(float znear, float zfar, float left, float right, 
            float bottom, float top);
        void setupPerspective(float znear, float zfar, float fovy, float aspect);
        void setAspect(float aspect);
        inline float getAspect()
        { return mCamera.projectionParams.aspect; }
        
        kmVec3 getDirection() const;
        kmVec3 getWorldDirection() const;
        void setDirection(const kmVec3 &direction);
        void lookAtLocal(const kmVec3 &pointTo);
        void lookAtWorld(const kmVec3 &pointTo);
        void lookAtWorld(const SceneObjectBase &obj);

        const kmMat4& getProjMatrix() const;
        inline const kmMat4& getViewMatrix() const { return mCamera.viewMatrix; }
        inline const kmMat4& getProjViewMatrix() const { return mCamera.viewProjectionMatrix; }
        const kmMat4& refreshViewMatrix();
        const kmMat4& refreshProjViewMatrix();

        void resetView();
        void yaw(float angleDelta);
        void pitch(float angleDelta);
        void roll(float angleDelta);
        // Установка углов свободной камеры в локальном пространстве камеры
        void setYawPitchRoll(float yaw, float pitch, float roll);
        // Установка углов свободной камеры в мировых осях ZW вправо влево, XW вниз вверх
        void setOrientationAngles(float ZW, float XW);
        float getYaw() const;
        float getPitch() const;
        float getRoll() const;
        float getZW() const;
        float getXW() const;
        void holdOnSceneObject(const SceneObjectBase &sceneObj);
        void linkWithSceneObject(const SceneObjectBase &sceneObj);
        void recalcFrustum();
        bool inFrustum(const LightSource &light) const;
        bool inFrustum(const lite3d_bounding_vol &vol) const;

        void computeCubeProjView(stl<kmMat4>::vector &matrices) const;
        void computeCubeProjView(const kmVec3 &position, stl<kmMat4>::vector &matrices) const;

        virtual void loadFromTemplate(const ConfigurationReader& conf);

    private:

        SceneNodeBase mNode;
        lite3d_camera mCamera;
    };
}

