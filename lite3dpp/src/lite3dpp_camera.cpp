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
#include <SDL_assert.h>
#include <lite3dpp/lite3dpp_camera.h>

namespace lite3dpp
{
    Camera::Camera(const String &name, Main *main) : 
        mName(name),
        mMain(main)
    {
        lite3d_camera_init(&mCamera);
        mCamera.userdata = this;
    }

    Camera::~Camera()
    {}

    void Camera::setupOrtho(float near, float far, float left, float right, 
        float bottom, float top)
    {
        lite3d_camera_ortho(&mCamera, near, far, left, right, bottom, top);
    }

    void Camera::setupPerspective(float znear, float zfar, float fovy, float aspect)
    {
        lite3d_camera_perspective(&mCamera, znear, zfar, fovy, aspect);
    }

    void Camera::setAspect(float aspect)
    {
        lite3d_camera_perspective(&mCamera, mCamera.projectionParams.perspective.znear,
            mCamera.projectionParams.perspective.zfar,
            mCamera.projectionParams.perspective.fovy,
            aspect);
    }

    Scene &Camera::getScene()
    {
        SDL_assert(mCamera.cameraNode.scene);
        lite3d_scene *scene = (lite3d_scene *)mCamera.cameraNode.scene;
        SDL_assert(scene->userdata);
        return *reinterpret_cast<Scene *>(scene->userdata);
    }

    void Camera::lookAt(const kmVec3 &pointTo)
    {
        lite3d_camera_lookAt(&mCamera, &pointTo);
    }

    void Camera::setPosition(const kmVec3 &position)
    {
        lite3d_camera_set_position(&mCamera, &position);
    }

    void Camera::setRotation(const kmQuaternion &orietation)
    {
        lite3d_camera_set_rotation(&mCamera, &orietation);
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
}

