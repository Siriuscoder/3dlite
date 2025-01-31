/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025  Sirius (Korolev Nikita)
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

    /* camera link to node */
    lite3d_camera_link_to(camera, camera->linkNode, camera->linkType);
    /* camera track object */
    lite3d_camera_tracking(camera, camera->trackNode);
    /* compute world matrix */
    lite3d_scene_node_update(&camera->cameraNode);
    /* compute view matrix */
    lite3d_camera_compute_view(camera);
    /* update global shader projection matrix */
    lite3d_shader_set_projection_matrix(&camera->projectionMatrix);
    /* update global camera view matrix */
    lite3d_shader_set_view_matrix(&camera->viewMatrix);
    /* compute frustum planes */
    if (camera->cameraNode.invalidated)
    {
        /* compute viewProjection martix */
        kmMat4Multiply(&camera->viewProjectionMatrix, &camera->projectionMatrix, &camera->viewMatrix);
        /* compute frustum */
        lite3d_frustum_compute(&camera->frustum, &camera->viewProjectionMatrix);
    }

    /* update global projection view matrix */
    lite3d_shader_set_projview_matrix(&camera->viewProjectionMatrix);
}

void lite3d_camera_compute_view(lite3d_camera *camera)
{
    kmVec3 forward, up, right, worldPosition;
    kmMat3 worldRotation;
    kmMat4 translate;
    kmMat4ExtractRotation(&worldRotation, &camera->cameraNode.worldMatrix);
    kmMat4ExtractPosition(&worldPosition, &camera->cameraNode.worldMatrix);
    kmVec3MultiplyMat3(&forward, &KM_VEC3_NEG_Z, &worldRotation);
    kmVec3MultiplyMat3(&up, &KM_VEC3_POS_Y, &worldRotation);
    kmVec3MultiplyMat3(&right, &KM_VEC3_POS_X, &worldRotation);
    kmVec3Normalize(&forward, &forward);
    kmVec3Normalize(&up, &up);
    kmVec3Normalize(&right, &right);

    kmMat4Identity(&camera->viewMatrix);
    camera->viewMatrix.mat[0] = right.x;
    camera->viewMatrix.mat[4] = right.y;
    camera->viewMatrix.mat[8] = right.z;

    camera->viewMatrix.mat[1] = up.x;
    camera->viewMatrix.mat[5] = up.y;
    camera->viewMatrix.mat[9] = up.z;

    camera->viewMatrix.mat[2] = -forward.x;
    camera->viewMatrix.mat[6] = -forward.y;
    camera->viewMatrix.mat[10] = -forward.z;

    kmMat4Translation(&translate, -worldPosition.x, -worldPosition.y, -worldPosition.z);
    kmMat4Multiply(&camera->viewMatrix, &camera->viewMatrix, &translate);
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
    kmMat4OrthographicProjection(&camera->projectionMatrix, left, right, bottom, top, znear, zfar);
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
    kmMat4PerspectiveProjection(&camera->projectionMatrix, fovy, aspect, znear, zfar);
    camera->projectionParams.bottom = znear * (camera->projectionMatrix.mat[9] - 1) / camera->projectionMatrix.mat[5];
    camera->projectionParams.top = znear * (camera->projectionMatrix.mat[9] + 1) / camera->projectionMatrix.mat[5];
    camera->projectionParams.left = znear * (camera->projectionMatrix.mat[8] - 1) / camera->projectionMatrix.mat[0];
    camera->projectionParams.right = znear * (camera->projectionMatrix.mat[8] + 1) / camera->projectionMatrix.mat[0];

    camera->cameraNode.invalidated = LITE3D_TRUE;
}

void lite3d_camera_init(lite3d_camera *camera)
{
    SDL_assert(camera);

    memset(camera, 0, sizeof (lite3d_camera));
    lite3d_scene_node_init(&camera->cameraNode);
    camera->cameraNode.rotationCentered = LITE3D_TRUE;
    camera->cameraNode.renderable = LITE3D_FALSE;
    camera->cameraNode.isCamera = LITE3D_TRUE;
    kmMat4Identity(&camera->viewMatrix);
    kmMat4Identity(&camera->projectionMatrix);
    kmMat4Identity(&camera->viewProjectionMatrix);
}

void lite3d_camera_lookAt(lite3d_camera *camera, const kmVec3 *pointTo)
{
    kmVec3 direction;
    SDL_assert(camera && pointTo);

    kmVec3Subtract(&direction, pointTo, &camera->cameraNode.position);
    lite3d_camera_set_direction(camera, &direction);
}

void lite3d_camera_lookAt_world(lite3d_camera *camera, const kmVec3 *pointTo)
{
    kmVec3 direction;
    kmVec3 worldPos;
    SDL_assert(camera && pointTo);

    lite3d_camera_world_position(camera, &worldPos);
    kmVec3Subtract(&direction, pointTo, &worldPos);
    lite3d_camera_set_direction(camera, &direction);
}

void lite3d_camera_set_direction(lite3d_camera *camera, const kmVec3 *direction)
{
    kmQuaternion q;
    kmQuaternionLookRotation(&q, direction, &KM_VEC3_POS_Z);
    kmQuaternionInverse(&q, &q);
    lite3d_camera_set_rotation(camera, &q);
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
    SDL_assert(camera);
    lite3d_scene_node_rotate_y(&camera->cameraNode, angle);
}

void lite3d_camera_pitch(lite3d_camera *camera, float angle)
{
    SDL_assert(camera);
    lite3d_scene_node_rotate_x(&camera->cameraNode, angle);
}

void lite3d_camera_roll(lite3d_camera *camera, float angle)
{
    SDL_assert(camera);
    lite3d_scene_node_rotate_z(&camera->cameraNode, angle);
}

void lite3d_camera_set_yaw_pitch_roll(lite3d_camera *camera, float yaw, float pitch, float roll)
{
    kmQuaternion rotation;

    SDL_assert(camera);
    kmQuaternionRotationPitchYawRoll(&rotation, pitch, yaw, roll);
    lite3d_scene_node_set_rotation(&camera->cameraNode, &rotation);
}

void lite3d_camera_move(lite3d_camera *camera, const kmVec3 *value)
{
    SDL_assert(camera);
    lite3d_scene_node_move(&camera->cameraNode, value);
}

void lite3d_camera_move_relative(lite3d_camera *camera,
    const kmVec3 *vec)
{
    lite3d_scene_node_move_relative(&camera->cameraNode, vec);
}

void lite3d_camera_direction(const lite3d_camera *camera,
    kmVec3 *vec)
{
    SDL_assert(camera && vec);
    kmQuaternionMultiplyVec3(vec, &camera->cameraNode.rotation, &KM_VEC3_NEG_Z);
    kmVec3Normalize(vec, vec);
}

float lite3d_camera_distance(const lite3d_camera *camera, 
    const kmVec3 *point)
{
    kmVec3 pointDir;
    kmVec3 worldCameraPos;
    SDL_assert(camera);
    SDL_assert(point);

    lite3d_camera_world_position(camera, &worldCameraPos);
    kmVec3Subtract(&pointDir, point, &worldCameraPos);
    return kmVec3Length(&pointDir);
}

void lite3d_camera_world_direction(const lite3d_camera *camera, kmVec3 *vec)
{
    kmMat3 worldRotation;
    kmMat4ExtractRotation(&worldRotation, &camera->cameraNode.worldMatrix);
    kmVec3MultiplyMat3(vec, &KM_VEC3_NEG_Z, &worldRotation);
    kmVec3Normalize(vec, vec);
}

void lite3d_camera_world_position(const lite3d_camera *camera, kmVec3 *pos)
{
    lite3d_scene_node_get_world_position(&camera->cameraNode, pos);
}

void lite3d_camera_world_rotation(const lite3d_camera *camera, kmQuaternion *q)
{
    lite3d_scene_node_get_world_rotation(&camera->cameraNode, q);
}
