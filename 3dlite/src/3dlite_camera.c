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

void lite3d_camera_apply(lite3d_camera *camera)
{
    SDL_assert(camera);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera->projection.mat);

    if(camera->cameraNode.recalc)
    {
        kmMat4 transMat;
        kmQuaternionNormalize(&camera->cameraNode.rotation,
            &camera->cameraNode.rotation);
        kmMat4RotationQuaternion(&camera->cameraNode.modelView, &camera->cameraNode.rotation);
        kmMat4Translation(&transMat, 
            camera->cameraNode.position.x, 
            camera->cameraNode.position.y, 
            camera->cameraNode.position.z);
        kmMat4Multiply(&camera->cameraNode.modelView, 
            &camera->cameraNode.modelView, &transMat);

        camera->cameraNode.recalc = LITE3D_FALSE;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera->cameraNode.modelView.mat);

    glPolygonMode(GL_FRONT_AND_BACK, camera->polygonMode);
    if(camera->cullBackFaces)
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

void lite3d_camera_init(lite3d_camera *camera)
{
    SDL_assert(camera);

    memset(camera, 0, sizeof(lite3d_camera));
    lite3d_scene_node_init(&camera->cameraNode);
    kmMat4Identity(&camera->projection);

    camera->cullBackFaces = LITE3D_TRUE;
    camera->polygonMode = LITE3D_POLYMODE_FILL;
}

void lite3d_camera_lookAt(lite3d_camera *camera, kmVec3 *pointTo)
{ 
    kmVec3 direction;
    kmVec3 up = {
        0.0f, 0.0f, 1.0f
    };

    SDL_assert(camera && pointTo);

    kmVec3Subtract(&direction, &camera->cameraNode.position, pointTo);
    kmQuaternionLookRotation(&camera->cameraNode.rotation, &direction, &up);
    camera->cameraNode.recalc = LITE3D_TRUE;
}