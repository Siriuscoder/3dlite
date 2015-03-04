/******************************************************************************
*	This file is part of 3dlite (Light-weight 3d engine).
*	Copyright (C) 2015  Sirius (Korolev Nikita)
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

#include <3dlite/3dlite_shader_params.h>

static lite3d_global_parameters globalParams;

void lite3d_shader_parameter_init(
    lite3d_shader_parameter *param)
{
    SDL_assert(param);

    memset(param, 0, sizeof(lite3d_shader_parameter));
    param->persist = LITE3D_FALSE;
}

void lite3d_shader_parameter_purge(
    lite3d_shader_parameter *param)
{
    SDL_assert(param);
}

void lite3d_shader_set_projection_matrix(kmMat4 *mat)
{
    globalParams.projectionMatrix.parameter.valmat4 = *mat;
}

void lite3d_shader_set_camera_matrix(kmMat4 *mat)
{
    globalParams.cameraMatrix.parameter.valmat4 = *mat;
}

void lite3d_shader_set_modelview_matrix(kmMat4 *mat)
{
    globalParams.modelviewMatrix.parameter.valmat4 = *mat;
}

void lite3d_shader_set_ambient_light(float val)
{
    globalParams.ambientLight.parameter.valfloat = val;
}

lite3d_global_parameters *lite3d_shader_global_parameters(void)
{
    return &globalParams;
}

void lite3d_shader_global_parameters_init(void)
{
    lite3d_shader_parameter_init(&globalParams.projectionMatrix);
    lite3d_shader_parameter_init(&globalParams.cameraMatrix);
    lite3d_shader_parameter_init(&globalParams.modelviewMatrix);
    lite3d_shader_parameter_init(&globalParams.ambientLight);

    strcpy(globalParams.projectionMatrix.name, "projectionMatrix");
    strcpy(globalParams.cameraMatrix.name, "cameraMatrix");
    strcpy(globalParams.modelviewMatrix.name, "modelviewMatrix");
    strcpy(globalParams.ambientLight.name, "ambientLight");

    globalParams.projectionMatrix.persist = 
        globalParams.cameraMatrix.persist = 
        globalParams.modelviewMatrix.persist = 
        globalParams.ambientLight.persist = LITE3D_TRUE;

    globalParams.projectionMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;
    globalParams.cameraMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;
    globalParams.modelviewMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;
    globalParams.ambientLight.type = LITE3D_SHADER_PARAMETER_FLOAT;

    kmMat4Identity(&globalParams.projectionMatrix.parameter.valmat4);
    kmMat4Identity(&globalParams.cameraMatrix.parameter.valmat4);
    kmMat4Identity(&globalParams.modelviewMatrix.parameter.valmat4);
    globalParams.ambientLight.parameter.valfloat = 0.8f;
}
