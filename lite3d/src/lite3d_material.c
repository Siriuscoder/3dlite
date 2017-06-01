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
#include <string.h>

#include <SDL_assert.h>
#include <SDL_log.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_buffers_manip.h>
#include <lite3d/lite3d_material.h>

lite3d_shader_program *gActProg = NULL;

static void lite3d_material_pass_purge(lite3d_material_pass *pass)
{
    lite3d_list_node *parameterNode;

    while ((parameterNode = lite3d_list_remove_first_link(&pass->parameters)) != NULL)
    {
        lite3d_free_pooled(LITE3D_POOL_NO1,
            LITE3D_MEMBERCAST(lite3d_material_pass_parameter,
            parameterNode, parameterLink));
    }
}

static lite3d_material_pass_parameter *lite3d_material_pass_find_parameter(
    lite3d_material_pass *pass, const char *name)
{
    lite3d_list_node *parameterNode;
    lite3d_material_pass_parameter *parameter;
    SDL_assert(pass);

    for (parameterNode = pass->parameters.l.next;
        parameterNode != &pass->parameters.l;
        parameterNode = lite3d_list_next(parameterNode))
    {
        parameter = LITE3D_MEMBERCAST(lite3d_material_pass_parameter, parameterNode, parameterLink);
        if (strcmp(parameter->parameter->name, name) == 0)
            return parameter;
    }

    return NULL;
}

void lite3d_material_init(
    lite3d_material *material)
{
    SDL_assert(material);
    material->passes = NULL;
    material->passesSize = 0;
    material->textureUnitsBinded = 0;
    material->passesCapacity = 0;
}

void lite3d_material_purge(
    lite3d_material *material)
{
    uint32_t i;
    SDL_assert(material);

    for (i = 0; i < material->passesSize; ++i)
    {
        lite3d_material_pass_purge(&material->passes[i]);
    }

    lite3d_free(material->passes);
}

void lite3d_material_pass_init(
    lite3d_material_pass *pass)
{
    SDL_assert(pass);
    memset(pass, 0, sizeof (lite3d_material_pass));
    lite3d_list_init(&pass->parameters);
}

lite3d_material_pass* lite3d_material_add_pass(
    lite3d_material *material, uint32_t no)
{
    lite3d_material_pass *pass;

    SDL_assert(material);

    if (!no || no >= 0xff)
        return NULL;

    if (material->passesCapacity < no)
    {
        pass = (lite3d_material_pass *)
            lite3d_calloc(sizeof (lite3d_material_pass) * no * 2);
        SDL_assert_release(pass);

        if (material->passes)
            memcpy(pass, material->passes, sizeof (lite3d_material_pass) * material->passesSize);

        material->passes = pass;
        material->passesCapacity = no * 2;
        material->passesSize = no;
    }

    pass = &material->passes[no - 1];
    lite3d_material_pass_init(pass);

    pass->passNo = no;
    return pass;
}

int lite3d_material_remove_pass(
    lite3d_material *material, uint32_t no)
{
    lite3d_material_pass *pass;

    if ((pass = lite3d_material_get_pass(material, no)) != NULL)
    {
        lite3d_material_pass_purge(pass);
        return LITE3D_TRUE;
    }

    return LITE3D_FALSE;
}

void lite3d_material_pass_add_parameter(lite3d_material_pass *pass,
    lite3d_shader_parameter *param)
{
    lite3d_material_pass_parameter *parameter;
    SDL_assert(pass);

    parameter = (lite3d_material_pass_parameter *) lite3d_malloc_pooled(
        LITE3D_POOL_NO1,
        sizeof (lite3d_material_pass_parameter));
    SDL_assert_release(parameter);

    parameter->parameter = param;
    parameter->uniformLocation = -1; // unknown ??
    parameter->textureUnit = -1; // unknown ??
    lite3d_list_link_init(&parameter->parameterLink);
    lite3d_list_add_last_link(&parameter->parameterLink, &pass->parameters);
}

int lite3d_material_pass_remove_parameter(lite3d_material_pass *pass,
    const char *name)
{
    lite3d_material_pass_parameter *parameter;
    SDL_assert(pass);

    if ((parameter = lite3d_material_pass_find_parameter(pass, name)) != NULL)
    {
        lite3d_list_unlink_link(&parameter->parameterLink);
        lite3d_free_pooled(LITE3D_POOL_NO1, parameter);

        return LITE3D_TRUE;
    }

    return LITE3D_FALSE;
}

void lite3d_material_pass_remove_all_parameters(lite3d_material_pass *pass)
{
    lite3d_material_pass_purge(pass);
}

lite3d_shader_parameter *lite3d_material_pass_get_parameter(
    lite3d_material_pass *pass, const char *name)
{
    lite3d_material_pass_parameter *parameter;
    parameter = lite3d_material_pass_find_parameter(pass, name);

    return parameter ? parameter->parameter : NULL;
}

lite3d_material_pass *lite3d_material_get_pass(
    const lite3d_material *material, uint32_t no)
{
    SDL_assert(material);

    if (material->passesSize < no || !no)
        return NULL;

    return &material->passes[no - 1];
}

int lite3d_material_pass_is_blend(
    const lite3d_material *material, uint32_t no)
{
    lite3d_material_pass *pass = lite3d_material_get_pass(material, no);
    SDL_assert(pass);

    return pass->blending && pass->passNo > 0;
}

int lite3d_material_pass_is_empty(
    const lite3d_material *material, uint32_t no)
{
    lite3d_material_pass *pass = lite3d_material_get_pass(material, no);
    SDL_assert(pass);

    return pass->passNo == 0;
}

lite3d_material_pass *lite3d_material_apply(lite3d_material *material, uint16_t no)
{
    lite3d_material_pass *pass;

    material->textureUnitsBinded = 0;
    pass = lite3d_material_get_pass(material, no);

    SDL_assert(pass);
    /* ignode empty pass */
    if (pass->passNo == 0)
        return pass;

    /* bind current shander first */
    if (gActProg != pass->program)
    {
        lite3d_shader_program_bind(pass->program);
        gActProg = pass->program;
    }

    /* set up uniforms if shader changed */
    lite3d_material_pass_set_params(material, pass, LITE3D_TRUE);
    /* validate shader program */
    if (!lite3d_shader_program_validate(gActProg))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "%s: validate shader %p: %s", LITE3D_CURRENT_FUNCTION, 
            (void *)gActProg, gActProg->statusString);
    }

    lite3d_blending(pass->blending);
    lite3d_blending_mode_set(pass->blendingMode);
    return pass;
}

void lite3d_material_pass_set_params(lite3d_material *material,
    lite3d_material_pass *pass, uint8_t changed)
{
    lite3d_list_node *parameterNode;
    lite3d_material_pass_parameter *parameter;
    uint16_t texUnitCounter = 0;

    /* check parameters and set it if changed */
    for (parameterNode = pass->parameters.l.next;
        parameterNode != &pass->parameters.l;
        parameterNode = lite3d_list_next(parameterNode))
    {
        parameter = LITE3D_MEMBERCAST(lite3d_material_pass_parameter,
            parameterNode, parameterLink);

        if (changed || parameter->parameter->changed)
        {
            /* sampler case */
            if (parameter->parameter->type == LITE3D_SHADER_PARAMETER_SAMPLER)
            {
                material->textureUnitsBinded++;
                if (parameter->textureUnit < 0)
                    parameter->textureUnit = texUnitCounter++;

                parameter->uniformLocation =
                    lite3d_shader_program_sampler_set(pass->program,
                    parameter->parameter,
                    parameter->uniformLocation,
                    parameter->textureUnit);
            }
                /* others */
            else
            {
                parameter->uniformLocation =
                    lite3d_shader_program_uniform_set(pass->program,
                    parameter->parameter, parameter->uniformLocation);
            }

            parameter->parameter->changed = LITE3D_FALSE;
        }
    }
}