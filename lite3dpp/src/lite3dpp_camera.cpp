/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2026 Sirius (Korolev Nikita)
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
#include <lite3dpp/lite3dpp_camera.h>

#include <SDL_assert.h>
#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{
    Camera::Camera(const String &name, Main *main) : 
        SceneObjectBase(name, nullptr, main, nullptr, KM_VEC3_ZERO, KM_QUATERNION_IDENTITY, KM_VEC3_ONE),
        mNode(&mCamera.cameraNode)
    {
        lite3d_camera_init(&mCamera);
        mCamera.userdata = this;

        setRoot(&mNode);
    }

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

    void Camera::resetView()
    {
        mCamera.cameraNode.rotation = KM_QUATERNION_IDENTITY;
        mCamera.cameraNode.recalc = LITE3D_TRUE;
        LITE3D_EXT_OBSERVER_NOTIFY_1(getRoot(), updateRotation, getRoot());
    }

    void Camera::lookAtLocal(const kmVec3 &pointTo)
    {
        lite3d_camera_lookAt(&mCamera, &pointTo);
        LITE3D_EXT_OBSERVER_NOTIFY_1(getRoot(), updateRotation, getRoot());
    }

    void Camera::lookAtWorld(const SceneObjectBase &obj)
    {
        lookAtWorld(obj.getWorldPosition());
        LITE3D_EXT_OBSERVER_NOTIFY_1(getRoot(), updateRotation, getRoot());
    }

    void Camera::lookAtWorld(const kmVec3 &pointTo)
    {
        lite3d_camera_lookAt_world(&mCamera, &pointTo);
        LITE3D_EXT_OBSERVER_NOTIFY_1(getRoot(), updateRotation, getRoot());
    }

    void Camera::setDirection(const kmVec3 &direction)
    {
        lite3d_camera_set_direction(&mCamera, &direction);
        LITE3D_EXT_OBSERVER_NOTIFY_1(getRoot(), updateRotation, getRoot());
    }

    void Camera::yaw(float angleDelta)
    {
        rotateY(angleDelta);
    }

    void Camera::pitch(float angleDelta)
    {
        rotateX(angleDelta);
    }

    void Camera::roll(float angleDelta)
    {
        rotateZ(angleDelta);
    }

    void Camera::setYawPitchRoll(float yaw, float pitch, float roll)
    {
        lite3d_camera_set_yaw_pitch_roll(&mCamera, yaw, pitch, roll);
        LITE3D_EXT_OBSERVER_NOTIFY_1(getRoot(), updateRotation, getRoot());
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

    void Camera::trackToSceneObject(const SceneObjectBase &sceneObj)
    {
        lite3d_camera_tracking(&mCamera, sceneObj.getRoot()->getPtr());
        LITE3D_EXT_OBSERVER_NOTIFY_1(getRoot(), updateRotation, getRoot());
    }

    void Camera::followToSceneObject(const SceneObjectBase &sceneObj)
    {
        lite3d_camera_follow_to(&mCamera, sceneObj.getRoot()->getPtr(), LITE3D_CAMERA_LINK_POSITION);
        LITE3D_EXT_OBSERVER_NOTIFY_1(getRoot(), updateRotation, getRoot());
        LITE3D_EXT_OBSERVER_NOTIFY_1(getRoot(), updatePosition, getRoot());
    }
    
    kmVec3 Camera::getDirection() const
    {
        kmVec3 direction;
        lite3d_camera_direction(&mCamera, &direction);
        return direction;
    }

    kmVec3 Camera::getRight() const
    {
        kmVec3 right;
        lite3d_camera_right(&mCamera, &right);
        return right;
    }

    kmVec3 Camera::getUp() const
    {
        kmVec3 up;
        lite3d_camera_up(&mCamera, &up);
        return up;
    }

    kmVec3 Camera::getWorldDirection() const
    {
        return mCamera.forward;
    }

    kmVec3 Camera::getWorldRight() const
    {
        return mCamera.right;
    }

    kmVec3 Camera::getWorldUp() const
    {
        return mCamera.up;
    }

    const kmMat4& Camera::refreshViewMatrix()
    {
        lite3d_scene_node_update(&mCamera.cameraNode);
        lite3d_camera_compute_view(&mCamera);
        return mCamera.viewMatrix;
    }

    const kmMat4& Camera::getProjMatrix() const
    {
        return mCamera.projectionMatrix;
    }

    const kmMat4& Camera::refreshProjViewMatrix()
    {
        auto view = refreshViewMatrix();
        kmMat4Multiply(&mCamera.viewProjectionMatrix, &getProjMatrix(), &view);
        lite3d_frustum_compute(&mCamera.frustum, &mCamera.viewProjectionMatrix);
        return mCamera.viewProjectionMatrix;
    }

    const kmMat4& Camera::refreshProjViewMatrix(const kmMat4 &view)
    {
        kmMat4Multiply(&mCamera.viewProjectionMatrix, &getProjMatrix(), &view);
        lite3d_frustum_compute(&mCamera.frustum, &mCamera.viewProjectionMatrix);
        return mCamera.viewProjectionMatrix;
    }

    bool Camera::intersectFrustum(const LightSource &light) const
    {
        if (light.getType() == LightSourceFlags::TypeDirectional)
            return true;
            
        auto aabb = light.getBoundingVolumeWorld();
        return lite3d_frustum_test_sphere(&mCamera.frustum, &aabb) == LITE3D_TRUE;
    }

    bool Camera::intersectFrustum(const lite3d_bounding_vol &vol) const
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

    void Camera::loadFromTemplate(const ConfigurationReader& conf)
    {
        ConfigurationReader perspectiveOptionsJson = conf.getObject(L"Perspective");
        ConfigurationReader orthoOptionsJson = conf.getObject(L"Ortho");
        if (!perspectiveOptionsJson.isEmpty())
        {
            auto aspect = static_cast<float>(getMain().window()->width()) / getMain().window()->height();
            setupPerspective(perspectiveOptionsJson.getDouble(L"Znear"),
                perspectiveOptionsJson.getDouble(L"Zfar"),
                perspectiveOptionsJson.getDouble(L"Fov"),
                perspectiveOptionsJson.getDouble(L"Aspect", aspect));
        }
        else if (!orthoOptionsJson.isEmpty())
        {
            setupOrtho(orthoOptionsJson.getDouble(L"Near"),
                orthoOptionsJson.getDouble(L"Far"),
                orthoOptionsJson.getDouble(L"Left"),
                orthoOptionsJson.getDouble(L"Right"),
                orthoOptionsJson.getDouble(L"Bottom"),
                orthoOptionsJson.getDouble(L"Top"));
        }

        if (conf.has(L"Position"))
            setPosition(conf.getVec3(L"Position"));
        if (conf.has(L"LookAt"))
            lookAtLocal(conf.getVec3(L"LookAt"));
    }

    void Camera::computeCubeProjView(stl<kmMat4>::vector &matrices) const
    {
        computeCubeProjView(mCamera.cameraNode.position, matrices);
    }

    void Camera::computeCubeProjView(const kmVec3 &position, stl<kmMat4>::vector &matrices) const
    {
        matrices.resize(6);

        kmMat4 projection;
        kmMat4PerspectiveProjection(&projection, 90.0, 1.0, mCamera.projectionParams.znear, 
            mCamera.projectionParams.zfar);

        kmMat4Multiply(&matrices[0], &projection, kmMat4LookDirection(&matrices[0], &position, &KM_VEC3_POS_X, &KM_VEC3_NEG_Y));
        kmMat4Multiply(&matrices[1], &projection, kmMat4LookDirection(&matrices[1], &position, &KM_VEC3_NEG_X, &KM_VEC3_NEG_Y));
        kmMat4Multiply(&matrices[2], &projection, kmMat4LookDirection(&matrices[2], &position, &KM_VEC3_POS_Y, &KM_VEC3_POS_Z));
        kmMat4Multiply(&matrices[3], &projection, kmMat4LookDirection(&matrices[3], &position, &KM_VEC3_NEG_Y, &KM_VEC3_NEG_Z));
        kmMat4Multiply(&matrices[4], &projection, kmMat4LookDirection(&matrices[4], &position, &KM_VEC3_POS_Z, &KM_VEC3_NEG_Y));
        kmMat4Multiply(&matrices[5], &projection, kmMat4LookDirection(&matrices[5], &position, &KM_VEC3_NEG_Z, &KM_VEC3_NEG_Y));
    }

    float Camera::getDistance(const kmVec3 &point)
    {
        return lite3d_camera_distance(&mCamera, &point);
    }
}

