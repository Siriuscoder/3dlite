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

void lite3d_camera_update_node(lite3d_camera *camera, lite3d_scene_node *node)
{
    SDL_assert(camera && node);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera->projection.mat);

    lite3d_scene_node_update(node);
    /* camera tracking enabled */
    if (camera->trackNode)
        lite3d_camera_track(camera, camera->trackNode, camera->trackType);

    lite3d_scene_node_update(&camera->cameraNode);

    /* world to camera */
    kmMat4Multiply(&camera->cameraNode.worldView, &camera->cameraNode.localView, &node->worldView);

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera->cameraNode.worldView.mat);

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

void lite3d_camera_init(lite3d_camera *camera, lite3d_scene_node *baseNode)
{
    SDL_assert(camera);

    memset(camera, 0, sizeof (lite3d_camera));
    lite3d_scene_node_init(&camera->cameraNode, baseNode);
    camera->cameraNode.rotationCentered = LITE3D_FALSE;
    kmMat4Identity(&camera->projection);

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

void lite3d_camera_track(lite3d_camera *camera, lite3d_scene_node *target,
    uint8_t trackType)
{
    SDL_assert(camera && target);

    camera->trackNode = target;
    camera->trackType = trackType;
    if(lite3d_scene_node_update(camera->trackNode))
        camera->cameraNode.recalc = LITE3D_TRUE;

    if (trackType & LITE3D_CAMERA_TRACK_POSITION)
        kmVec3Scale(&camera->cameraNode.position,
        &camera->trackNode->position, -1);

    if (trackType & LITE3D_CAMERA_TRACK_ORIENTATION)
        camera->cameraNode.rotation = camera->trackNode->rotation;
}