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
#include <SDL_assert.h>
#include <lite3dpp/lite3dpp_camera.h>

namespace lite3dpp
{
    Camera::Camera(const String &name) : 
        mName(name)
    {
        lite3d_camera_init(&mCamera);
        mCamera.userdata = this;
    }

    Camera::~Camera()
    {}

    void Camera::setupOrtho(float znear, float zfar, float left, float right, 
        float bottom, float top)
    {
        lite3d_camera_ortho(&mCamera, znear, zfar, left, right, bottom, top);
    }

    void Camera::setupPerspective(float znear, float zfar, float fovy, float aspect)
    {
        lite3d_camera_perspective(&mCamera, znear, zfar, fovy, aspect);
    }

    void Camera::setAspect(float aspect)
    {
        lite3d_camera_perspective(&mCamera, mCamera.projectionParams.znear,
            mCamera.projectionParams.zfar,
            mCamera.projectionParams.fovy,
            aspect);
    }

    Scene *Camera::getScene()
    {
        lite3d_scene *scene = static_cast<lite3d_scene *>(mCamera.cameraNode.scene);
        if (!scene)
        {
            return nullptr;
        }

        return reinterpret_cast<Scene *>(scene->userdata);
    }

    void Camera::lookAt(const kmVec3 &pointTo)
    {
        lite3d_camera_lookAt(&mCamera, &pointTo);
    }

    void Camera::lookAt(const SceneObject &obj)
    {
        lookAt(obj.getRoot()->getPosition());
    }

    void Camera::setPosition(const kmVec3 &position)
    {
        lite3d_camera_set_position(&mCamera, &position);
    }

    void Camera::setRotation(const kmQuaternion &orietation)
    {
        lite3d_camera_set_rotation(&mCamera, &orietation);
    }

    void Camera::setDirection(const kmVec3 &direction)
    {
        kmQuaternion rot;
        kmVec3 up = {
            0.0f, 0.0f, 1.0f
        };

        kmQuaternionLookRotation(&rot, &direction, &up);
        setRotation(rot);
    }

    void Camera::rotate(const kmQuaternion &orietation)
    {
        lite3d_camera_set_rotation(&mCamera, &orietation);
    }

    void Camera::yaw(float angle)
    {
        lite3d_camera_yaw(&mCamera, angle);
    }

    void Camera::pitch(float angle)
    {
        lite3d_camera_pitch(&mCamera, angle);
    }

    void Camera::roll(float angle)
    {
        lite3d_camera_roll(&mCamera, angle);
    }

    void Camera::move(const kmVec3 &value)
    {
        lite3d_camera_move(&mCamera, &value);
    }
    
    void Camera::rotateY(float angle)
    {
        lite3d_camera_rotate_y(&mCamera, angle);
    }
    
    void Camera::rotateX(float angle)
    {
        lite3d_camera_rotate_x(&mCamera, angle);
    }
    
    void Camera::rotateZ(float angle)
    {
        lite3d_camera_rotate_z(&mCamera, angle);
    }

    void Camera::moveRelative(const kmVec3 &value)
    {
        lite3d_camera_move_relative(&mCamera, &value);
    }

    void Camera::holdOnSceneObject(const SceneObject &sceneObj)
    {
        lite3d_camera_tracking(&mCamera, sceneObj.getRoot()->getPtr());
    }

    void Camera::linkWithSceneObject(const SceneObject &sceneObj)
    {
        lite3d_camera_link_to(&mCamera, sceneObj.getRoot()->getPtr(), LITE3D_CAMERA_LINK_POSITION);
    }
    
    kmVec3 Camera::getDirection() const
    {
        kmVec3 direction;
        lite3d_camera_direction(&mCamera, &direction);
        return direction;
    }
    
    kmQuaternion Camera::getRotation() const 
    {
        kmQuaternion inverseRot;
        kmQuaternionInverse(&inverseRot, &mCamera.cameraNode.rotation);
        return inverseRot;
    }

    const kmMat4& Camera::getTransformMatrix()
    {
        lite3d_scene_node_update(&mCamera.cameraNode);
        return mCamera.cameraNode.worldView;
    }

    const kmMat4& Camera::getProjMatrix() const
    {
        return mCamera.projection;
    }

    const kmMat4& Camera::getProjTransformMatrix()
    {
        kmMat4Multiply(&mCamera.screen, &getProjMatrix(), &getTransformMatrix());
        return mCamera.screen;
    }

    void Camera::recalcFrustum()
    {
        lite3d_frustum_compute(&mCamera.frustum, &getProjTransformMatrix());
    }

    bool Camera::inFrustum(const LightSource &light) const
    {
        auto volToCheck = light.getBoundingVolumeWorld();
        return lite3d_frustum_test_sphere(&mCamera.frustum, &volToCheck) == LITE3D_TRUE;
    }

    bool Camera::inFrustum(const lite3d_bounding_vol &vol) const
    {
        return lite3d_frustum_test(&mCamera.frustum, &vol) == LITE3D_TRUE;
    }

    float Camera::getYaw() const
    {
        return kmQuaternionGetYaw(&mCamera.cameraNode.rotation);
    }

    float Camera::getPitch() const
    {
        return kmQuaternionGetPitch(&mCamera.cameraNode.rotation);
    }

    float Camera::getRoll() const
    {
        return kmQuaternionGetRoll(&mCamera.cameraNode.rotation);
    }
}

