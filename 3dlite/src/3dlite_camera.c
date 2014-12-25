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
#include <SDL_assert.h>

#include <3dlite/GL/glew.h>
#include <3dlite/3dlite_camera.h>

void lite3d_apply_camera(lite3d_camera *camera)
{
    SDL_assert(camera);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(camera->projection.mat);

    if(camera->sceneNode.recalc)
    {
        lite3d_scene_node_matrix_recalc(&camera->sceneNode);
        camera->sceneNode.recalc = LITE3D_FALSE;
    }

    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(camera->sceneNode.modelView.mat);
}

void lite3d_ortho_camera(lite3d_camera *camera, float near,
    float far, float left, float right, float bottom, float top)
{
    SDL_assert(camera);
}

void lite3d_projection_camera(lite3d_camera *camera, float znear,
    float zfar, float fovy, float aspect)
{
    SDL_assert(camera);
}