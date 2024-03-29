/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024  Sirius (Korolev Nikita)
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
#include <string.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_camera.h>
#include <lite3d/lite3d_shader_params.h>
#include <lite3d/lite3d_buffers_manip.h>

void lite3d_camera_update_view(lite3d_camera *camera)
{
    SDL_assert(camera);

    lite3d_polygon_mode(camera->polygonMode);
    lite3d_backface_culling(camera->cullFaceMode);

    /* camera link to node */
    lite3d_camera_link_to(camera, camera->linkNode, camera->linkType);
    /* camera track object */
    lite3d_camera_tracking(camera, camera->trackNode);
    /* compute local camera matrix */
    lite3d_scene_node_update(&camera->cameraNode);
    /* update global shader projection matrix */
    lite3d_shader_set_projection_matrix(&camera->projection);
    /* update global camera view matrix */
    lite3d_shader_set_view_matrix(&camera->cameraNode.worldView);
    /* compute frustum planes */
    if (camera->cameraNode.invalidated)
    {
        kmMat4Multiply(&camera->screen, &camera->projection, &camera->cameraNode.worldView);
        lite3d_frustum_compute(&camera->frustum, &camera->screen);
        /* compute screen martix */
    }

    /* update global projection view matrix */
    lite3d_shader_set_projview_matrix(&camera->screen);
}

void lite3d_camera_ortho(lite3d_camera *camera, float znear,
    float zfar, float left, float right, float bottom, float top)
{
    SDL_assert(camera);
    memset(&camera->frustum, 0, sizeof(camera->frustum));
    camera->isOrtho = LITE3D_TRUE;
    camera->projectionParams.znear = znear;
    camera->projectionParams.zfar = zfar;
    camera->projectionParams.left = left;
    camera->projectionParams.right = right;
    camera->projectionParams.bottom = bottom;
    camera->projectionParams.top = top;
    kmMat4OrthographicProjection(&camera->projection, left, right, bottom, top, znear, zfar);
    camera->cameraNode.invalidated = LITE3D_TRUE;
}

void lite3d_camera_perspective(lite3d_camera *camera, float znear,
    float zfar, float fovy, float aspect)
{
    SDL_assert(camera);
    camera->isOrtho = LITE3D_FALSE;
    camera->projectionParams.znear = znear;
    camera->projectionParams.zfar = zfar;
    camera->projectionParams.fovy = fovy;
    camera->projectionParams.aspect = aspect;
    kmMat4PerspectiveProjection(&camera->projection, fovy, aspect, znear, zfar);
    camera->projectionParams.bottom = znear * (camera->projection.mat[9] - 1) / camera->projection.mat[5];
    camera->projectionParams.top = znear * (camera->projection.mat[9] + 1) / camera->projection.mat[5];
    camera->projectionParams.left = znear * (camera->projection.mat[8] - 1) / camera->projection.mat[0];
    camera->projectionParams.right = znear * (camera->projection.mat[8] + 1) / camera->projection.mat[0];

    camera->cameraNode.invalidated = LITE3D_TRUE;
}

void lite3d_camera_init(lite3d_camera *camera)
{
    SDL_assert(camera);

    memset(camera, 0, sizeof (lite3d_camera));
    lite3d_scene_node_init(&camera->cameraNode);
    camera->cameraNode.rotationCentered = LITE3D_FALSE;
    camera->cameraNode.renderable = LITE3D_FALSE;
    camera->cameraNode.isCamera = LITE3D_TRUE;
    kmMat4Identity(&camera->projection);
    kmMat4Identity(&camera->screen);

    camera->cullFaceMode = LITE3D_CULLFACE_BACK;
    camera->polygonMode = LITE3D_POLYMODE_FILL;
}

void lite3d_camera_lookAt(lite3d_camera *camera, const kmVec3 *pointTo)
{
    kmVec3 direction;
    kmVec3 up = {
        0.0f, 0.0f, 1.0f
    };

    SDL_assert(camera && pointTo);

    kmVec3Subtract(&direction, pointTo, &camera->cameraNode.position);
    kmQuaternionLookRotation(&camera->cameraNode.rotation, &direction, &up);
    camera->cameraNode.recalc = LITE3D_TRUE;
    camera->cameraNode.invalidated = LITE3D_TRUE;
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
    lite3d_scene_node_set_position(&camera->cameraNode, position);
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

void lite3d_camera_rotate_y(lite3d_camera *camera, float angle)
{
    SDL_assert(camera);
    lite3d_scene_node_rotate_angle(&camera->cameraNode, &KM_VEC3_POS_Y, angle);
}

void lite3d_camera_rotate_x(lite3d_camera *camera, float angle)
{
    SDL_assert(camera);
    lite3d_scene_node_rotate_angle(&camera->cameraNode, &KM_VEC3_POS_X, angle);
}

void lite3d_camera_rotate_z(lite3d_camera *camera, float angle)
{
    SDL_assert(camera);
    lite3d_scene_node_rotate_angle(&camera->cameraNode, &KM_VEC3_POS_Z, angle);
}

void lite3d_camera_move(lite3d_camera *camera, const kmVec3 *value)
{
    SDL_assert(camera);
    lite3d_scene_node_move(&camera->cameraNode, value);
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

void lite3d_camera_direction(const lite3d_camera *camera,
    kmVec3 *vec)
{
    kmQuaternion inverseRot;

    SDL_assert(camera && vec);
    kmQuaternionInverse(&inverseRot, &camera->cameraNode.rotation);
    kmQuaternionMultiplyVec3(vec, &inverseRot, &KM_VEC3_NEG_Z);
}

float lite3d_camera_distance(lite3d_camera *camera, 
    const kmVec3 *point)
{
    kmVec3 pointDir;
    SDL_assert(camera);
    SDL_assert(point);

    kmVec3Subtract(&pointDir, point, &camera->cameraNode.position);
    return kmVec3Length(&pointDir);
}

