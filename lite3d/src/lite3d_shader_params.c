/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2025 Sirius (Korolev Nikita)
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

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_shader_params.h>

static lite3d_global_parameters globalParams;

void lite3d_shader_parameter_init(
    lite3d_shader_parameter *param)
{
    SDL_assert(param);

    memset(param, 0, sizeof(lite3d_shader_parameter));
    param->changed = LITE3D_TRUE;
    param->direction = LITE3D_SHADER_PARAMETER_DIRECTION_INOUT;
    param->imageLayer = -1;
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

void lite3d_shader_set_normal_matrix(kmMat3 *mat)
{
    globalParams.normalMatrix.parameter.valmat3 = *mat;
    globalParams.normalMatrix.changed = LITE3D_TRUE;
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

void lite3d_shader_set_screen_matrix(kmMat4 *mat)
{
    globalParams.screenMatrix.parameter.valmat4 = *mat;
    globalParams.screenMatrix.changed = LITE3D_TRUE;
}

void lite3d_shader_set_projview_matrix(kmMat4 *mat)
{
    globalParams.projViewMatrix.parameter.valmat4 = *mat;
    globalParams.projViewMatrix.changed = LITE3D_TRUE;
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
    lite3d_shader_parameter_init(&globalParams.normalMatrix);
    lite3d_shader_parameter_init(&globalParams.screenMatrix);
    lite3d_shader_parameter_init(&globalParams.projViewMatrix);

    strcpy(globalParams.projectionMatrix.name, "projectionMatrix");
    strcpy(globalParams.viewMatrix.name, "viewMatrix");
    strcpy(globalParams.modelMatrix.name, "modelMatrix");
    strcpy(globalParams.normalMatrix.name, "normalMatrix");
    strcpy(globalParams.screenMatrix.name, "screenMatrix");
    strcpy(globalParams.projViewMatrix.name, "projViewMatrix");

    globalParams.projectionMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;
    globalParams.viewMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;
    globalParams.modelMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;
    globalParams.normalMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM3;
    globalParams.screenMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;
    globalParams.projViewMatrix.type = LITE3D_SHADER_PARAMETER_FLOATM4;

    kmMat4Identity(&globalParams.projectionMatrix.parameter.valmat4);
    kmMat4Identity(&globalParams.viewMatrix.parameter.valmat4);
    kmMat4Identity(&globalParams.modelMatrix.parameter.valmat4);
    kmMat3Identity(&globalParams.normalMatrix.parameter.valmat3);
    kmMat4Identity(&globalParams.screenMatrix.parameter.valmat4);
    kmMat4Identity(&globalParams.projViewMatrix.parameter.valmat4);
}

static lite3d_shader_parameter_container *lite3d_shader_parameters_find(
    lite3d_shader_parameters *params, const char *name)
{
    lite3d_list_node *parameterNode;
    lite3d_shader_parameter_container *parameter;
    SDL_assert(params);

    for (parameterNode = params->parameters.l.next;
        parameterNode != &params->parameters.l;
        parameterNode = lite3d_list_next(parameterNode))
    {
        parameter = LITE3D_MEMBERCAST(
            lite3d_shader_parameter_container,
            parameterNode,
            parameterLink);

        if (strcmp(parameter->parameter->name, name) == 0)
        {
            return parameter;
        }
    }

    return NULL;
}

void lite3d_shader_parameters_init(lite3d_shader_parameters *params)
{
    SDL_assert(params);
    memset(params, 0, sizeof(lite3d_shader_parameters));
    lite3d_list_init(&params->parameters);
}

void lite3d_shader_parameters_add(lite3d_shader_parameters *params, lite3d_shader_parameter *param)
{
    lite3d_shader_parameter_container *parameter;
    SDL_assert(params);

    parameter = (lite3d_shader_parameter_container *) lite3d_malloc_pooled(
        LITE3D_POOL_NO1,
        sizeof (lite3d_shader_parameter_container));
    SDL_assert_release(parameter);

    parameter->parameter = param;
    parameter->location = -1; // unknown ??
    parameter->binding = -1; // unknown ??
    parameter->bindContext = &params->bindContext;
    lite3d_list_link_init(&parameter->parameterLink);
    lite3d_list_add_last_link(&parameter->parameterLink, &params->parameters);
}

int lite3d_shader_parameters_remove(lite3d_shader_parameters *params, const char *name)
{
    lite3d_shader_parameter_container *parameter;
    SDL_assert(params);

    if ((parameter = lite3d_shader_parameters_find(params, name)) != NULL)
    {
        lite3d_list_unlink_link(&parameter->parameterLink);
        lite3d_free_pooled(LITE3D_POOL_NO1, parameter);

        return LITE3D_TRUE;
    }

    return LITE3D_FALSE;
}

void lite3d_shader_parameters_remove_all(lite3d_shader_parameters *params)
{
    lite3d_list_node *parameterNode;
    while ((parameterNode = lite3d_list_remove_first_link(&params->parameters)) != NULL)
    {
        lite3d_free_pooled(LITE3D_POOL_NO1,
            LITE3D_MEMBERCAST(lite3d_shader_parameter_container,
            parameterNode, parameterLink));
    }
}

lite3d_shader_parameter *lite3d_shader_parameters_get(lite3d_shader_parameters *params, const char *name)
{
    lite3d_shader_parameter_container *parameter;
    parameter = lite3d_shader_parameters_find(params, name);

    return parameter ? parameter->parameter : NULL;
}
