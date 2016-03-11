/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
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

#include <lite3d/lite3d_shader_params.h>

static lite3d_global_parameters globalParams;

void lite3d_shader_parameter_init(
    lite3d_shader_parameter *param)
{
    SDL_assert(param);

    memset(param, 0, sizeof(lite3d_shader_parameter));
    param->changed = LITE3D_TRUE;
}

void lite3d_shader_parameter_purge(
    lite3d_shader_parameter *param)
{
    SDL_assert(param);
}

void lite3d_shader_set_projection_matrix(kmMat4 *mat)
{
    globalParams.projectionMatrix.parameter.valmat4 = *mat;
    globalParams.projectionMatrix.changed = LITE3D_TRUE;
}

void lite3d_shader_set_modelview_matrix(kmMat4 *mat)
{
    globalParams.modelviewMatrix.parameter.valmat4 = *mat;
    globalParams.modelviewMatrix.changed = LITE3D_TRUE;
}

void lite3d_shader_set_model_matrix(kmMat4 *mat)
{
    globalParams.modelMatrix.parameter.valmat4 = *mat;
    globalParams.modelMatrix.changed = LITE3D_TRUE;
}

void lite3d_shader_set_view_matrix(kmMat4 *mat)
{
    globalParams.viewMatrix.parameter.valmat4 = *mat;
    globalParams.viewMatrix.changed = LITE3D_TRUE;
}

void lite3d_shader_set_ambient_light(float val)
{
    globalParams.ambientLight.parameter.valfloat = val;
    globalParams.ambientLight.changed = LITE3D_TRUE;
}

lite3d_global_parameters *lite3d_shader_global_parameters(void)
{
    return &globalParams;
}

void lite3d_shader_global_parameters_init(void)
{
    lite3d_shader_parameter_init(&globalParams.projectionMatrix);
    lite3d_shader_parameter_init(&globalParams.viewMatrix);
    lite3d_shader_parameter_init(&globalParams.modelMatrix);
    lite3d_shader_parameter_init(&globalParams.modelviewMatrix);
    lite3d_shader_parameter_init(&globalParams.ambientLight);

    strcpy(globalParams.projectionMatrix.name, "projectionMatrix");
    strcpy(globalParams.viewMatrix.name, "viewMatrix");
    strcpy(globalParams.modelMatrix.name, "modelMatrix");
    strcpy(globalParams.modelviewMatrix.name, "modelviewMatrix");
    strcpy(globalParams.ambientLight.name, "ambientLight");

    globalParams.projectionMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;
    globalParams.viewMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;
    globalParams.modelMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;
    globalParams.modelviewMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;
    globalParams.ambientLight.type = LITE3D_SHADER_PARAMETER_FLOAT;

    kmMat4Identity(&globalParams.projectionMatrix.parameter.valmat4);
    kmMat4Identity(&globalParams.viewMatrix.parameter.valmat4);
    kmMat4Identity(&globalParams.modelMatrix.parameter.valmat4);
    kmMat4Identity(&globalParams.modelviewMatrix.parameter.valmat4);
    globalParams.ambientLight.parameter.valfloat = 0.8f;
}
