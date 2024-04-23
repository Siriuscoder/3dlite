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

    void Camera::resetView()
    {
        mCamera.cameraNode.rotation = KM_QUATERNION_IDENTITY;
        mCamera.cameraNode.recalc = LITE3D_TRUE;
    }

    void Camera::lookAt(const kmVec3 &pointTo)
    {
        lite3d_camera_lookAt(&mCamera, &pointTo);
    }

    void Camera::lookAt(const SceneObjectBase &obj)
    {
        lookAt(obj.getPosition());
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
        lite3d_camera_set_direction(&mCamera, &direction);
    }

    void Camera::rotate(const kmQuaternion &orietation)
    {
        lite3d_camera_set_rotation(&mCamera, &orietation);
    }

    void Camera::yaw(float angleDelta)
    {
        lite3d_camera_yaw(&mCamera, angleDelta);
    }

    void Camera::pitch(float angleDelta)
    {
        lite3d_camera_pitch(&mCamera, angleDelta);
    }

    void Camera::roll(float angleDelta)
    {
        lite3d_camera_roll(&mCamera, angleDelta);
    }

    void Camera::setYawPitchRoll(float yaw, float pitch, float roll)
    {
        lite3d_camera_set_yaw_pitch_roll(&mCamera, yaw, pitch, roll);
    }

    void Camera::setOrientationAngles(float ZW, float XW)
    {
        resetView();
        roll(ZW);
        pitch(XW);
    }

    float Camera::getZW() const
    {
        return getRoll();
    }

    float Camera::getXW() const
    {
        return getPitch();
    }

    void Camera::move(const kmVec3 &value)
    {
        lite3d_camera_move(&mCamera, &value);
    }
    
    void Camera::rotateY(float angleDelta)
    {
        lite3d_camera_rotate_y(&mCamera, angleDelta);
    }
    
    void Camera::rotateX(float angleDelta)
    {
        lite3d_camera_rotate_x(&mCamera, angleDelta);
    }
    
    void Camera::rotateZ(float angleDelta)
    {
        lite3d_camera_rotate_z(&mCamera, angleDelta);
    }

    void Camera::moveRelative(const kmVec3 &value)
    {
        lite3d_camera_move_relative(&mCamera, &value);
    }

    void Camera::holdOnSceneObject(const SceneObjectBase &sceneObj)
    {
        lite3d_camera_tracking(&mCamera, sceneObj.getRoot()->getPtr());
    }

    void Camera::linkWithSceneObject(const SceneObjectBase &sceneObj)
    {
        lite3d_camera_link_to(&mCamera, sceneObj.getRoot()->getPtr(), LITE3D_CAMERA_LINK_POSITION);
    }
    
    kmVec3 Camera::getDirection() const
    {
        kmVec3 direction;
        lite3d_camera_direction(&mCamera, &direction);
        return direction;
    }

    kmVec3 Camera::getWorldDirection() const
    {
        kmVec3 direction;
        lite3d_camera_world_direction(&mCamera, &direction);
        return direction;
    }

    kmVec3 Camera::getWorldPosition() const
    {
        kmVec3 position;
        lite3d_camera_world_position(&mCamera, &position);
        return position;
    }

    kmQuaternion Camera::getWorldRotation() const
    {
        kmQuaternion q;
        lite3d_camera_world_rotation(&mCamera, &q);
        return q;
    }

    const kmMat4& Camera::refreshViewMatrix()
    {
        lite3d_scene_node_update(&mCamera.cameraNode);
        lite3d_camera_compute_view(&mCamera);
        return mCamera.view;
    }

    const kmMat4& Camera::getProjMatrix() const
    {
        return mCamera.projection;
    }

    const kmMat4& Camera::refreshProjViewMatrix()
    {
        kmMat4Multiply(&mCamera.screen, &getProjMatrix(), &refreshViewMatrix());
        return mCamera.screen;
    }

    void Camera::recalcFrustum()
    {
        lite3d_frustum_compute(&mCamera.frustum, &refreshProjViewMatrix());
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

