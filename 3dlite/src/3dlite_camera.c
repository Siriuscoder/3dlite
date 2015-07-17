/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
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
#include <string.h>
#include <SDL_assert.h>

#include <3dlite/GL/glew.h>
#include <3dlite/3dlite_camera.h>
#include <3dlite/3dlite_shader_params.h>

void lite3d_camera_update_view(lite3d_camera *camera)
{
    SDL_assert(camera);

    glPolygonMode(GL_FRONT_AND_BACK, camera->polygonMode);
    if (camera->cullBackFaces)
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
    }
    else
    {
        glDisable(GL_CULL_FACE);
    }

    /* update global shader proj matrix */
    lite3d_shader_set_projection_matrix(&camera->projection);

    /* camera link to node */
    lite3d_camera_link_to(camera, camera->linkNode, camera->linkType);
    /* camera track object */
    lite3d_camera_tracking(camera, camera->trackNode);
    /* compute local camera matrix */
    lite3d_scene_node_update(&camera->cameraNode);
    /* update global camera view matrix */
    lite3d_shader_set_camera_matrix(&camera->cameraNode.localView);
}

void lite3d_camera_ortho(lite3d_camera *camera, float near,
    float far, float left, float right, float bottom, float top)
{
    SDL_assert(camera);
    camera->isOrtho = LITE3D_FALSE;
    camera->projectionParams.ortho.near = near;
    camera->projectionParams.ortho.far = far;
    camera->projectionParams.ortho.left = left;
    camera->projectionParams.ortho.right = right;
    camera->projectionParams.ortho.bottom = bottom;
    camera->projectionParams.ortho.top = top;
    kmMat4OrthographicProjection(&camera->projection, left, right, bottom, top, near, far);
}

void lite3d_camera_perspective(lite3d_camera *camera, float znear,
    float zfar, float fovy, float aspect)
{
    SDL_assert(camera);
    camera->isOrtho = LITE3D_FALSE;
    camera->projectionParams.perspective.znear = znear;
    camera->projectionParams.perspective.zfar = zfar;
    camera->projectionParams.perspective.fovy = fovy;
    camera->projectionParams.perspective.aspect = aspect;
    kmMat4PerspectiveProjection(&camera->projection, fovy, aspect, znear, zfar);
}

void lite3d_camera_init(lite3d_camera *camera)
{
    SDL_assert(camera);

    memset(camera, 0, sizeof (lite3d_camera));
    lite3d_scene_node_init(&camera->cameraNode);
    camera->cameraNode.rotationCentered = LITE3D_FALSE;
    camera->cameraNode.renderable = LITE3D_FALSE;
    kmMat4Identity(&camera->projection);
    /* pass no 1 by default */
    camera->materialPass = 1;

    camera->cullBackFaces = LITE3D_TRUE;
    camera->polygonMode = LITE3D_POLYMODE_FILL;
}

void lite3d_camera_lookAt(lite3d_camera *camera, const kmVec3 *pointTo)
{
    kmVec3 direction, scaled;
    kmVec3 up = {
        0.0f, 0.0f, 1.0f
    };

    SDL_assert(camera && pointTo);

    kmVec3Scale(&scaled, pointTo, -1);
    kmVec3Subtract(&direction, &camera->cameraNode.position, &scaled);
    kmQuaternionLookRotation(&camera->cameraNode.rotation, &direction, &up);
    camera->cameraNode.recalc = LITE3D_TRUE;
}

void lite3d_camera_link_to(lite3d_camera *camera,
    const lite3d_scene_node *target, uint8_t linkType)
{
    SDL_assert(camera);

    camera->linkNode = target;
    camera->linkType = linkType;

    if (camera->linkNode)
    {
        if (linkType & LITE3D_CAMERA_LINK_POSITION)
            lite3d_camera_set_position(camera, &camera->linkNode->position);

        if (linkType & LITE3D_CAMERA_LINK_ORIENTATION)
            lite3d_camera_set_rotation(camera, &camera->linkNode->rotation);
    }
}

void lite3d_camera_tracking(lite3d_camera *camera,
    const lite3d_scene_node *target)
{
    SDL_assert(camera);

    camera->trackNode = target;
    if (camera->trackNode)
    {
        lite3d_camera_lookAt(camera, &camera->trackNode->position);
    }
}

void lite3d_camera_set_position(lite3d_camera *camera,
    const kmVec3 *position)
{
    kmVec3 postmp;
    kmVec3Scale(&postmp, position, -1.0f);
    lite3d_scene_node_set_position(&camera->cameraNode, &postmp);
}

void lite3d_camera_set_rotation(lite3d_camera *camera,
    const kmQuaternion *orientation)
{
    SDL_assert(camera);
    lite3d_scene_node_set_rotation(&camera->cameraNode, orientation);
}

void lite3d_camera_rotate(lite3d_camera *camera,
    const kmQuaternion *orietation)
{
    SDL_assert(camera);
    lite3d_scene_node_rotate(&camera->cameraNode, orietation);
}

void lite3d_camera_yaw(lite3d_camera *camera, float angle)
{
    kmQuaternion quat;

    SDL_assert(camera);
    kmQuaternionRotationAxisAngle(&quat, &KM_VEC3_POS_Y, angle);  
    lite3d_scene_node_rotate_by(&camera->cameraNode, &quat);
}

void lite3d_camera_pitch(lite3d_camera *camera, float angle)
{
    kmQuaternion quat;

    SDL_assert(camera);
    kmQuaternionRotationAxisAngle(&quat, &KM_VEC3_POS_X, angle);
    lite3d_scene_node_rotate_by(&camera->cameraNode, &quat);
}

void lite3d_camera_roll(lite3d_camera *camera, float angle)
{
    kmQuaternion quat;

    SDL_assert(camera);
    kmQuaternionRotationAxisAngle(&quat, &KM_VEC3_POS_Z, angle);  
    lite3d_scene_node_rotate_by(&camera->cameraNode, &quat);
}

void lite3d_camera_move(lite3d_camera *camera, const kmVec3 *value)
{
    kmVec3 postmp;

    SDL_assert(camera);
    kmVec3Scale(&postmp, value, -1.0f);
    lite3d_scene_node_move(&camera->cameraNode, &postmp);
}

void lite3d_camera_move_relative(lite3d_camera *camera,
    const kmVec3 *vec)
{
    kmVec3 vecLocalCamera;
    kmQuaternion inverseRot;

    SDL_assert(camera);
    kmQuaternionInverse(&inverseRot, &camera->cameraNode.rotation);
    kmQuaternionMultiplyVec3(&vecLocalCamera, &inverseRot, vec);
    lite3d_scene_node_move(&camera->cameraNode, &vecLocalCamera);
}