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

#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_material.h>

static void lite3d_material_pass_purge(lite3d_material_pass *pass)
{
    lite3d_list_node *parameterNode;
    
    while ((parameterNode = lite3d_list_first_link(&pass->parameters)) != NULL)
    {
        lite3d_list_unlink_link(parameterNode);
        lite3d_free(MEMBERCAST(lite3d_material_pass_parameter, 
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
        parameter = MEMBERCAST(lite3d_material_pass_parameter, parameterNode, parameterLink);
        if (strcmp(parameter->parameter->name, name) == 0)
            return parameter;
    }

    return NULL;
}

void lite3d_material_init(
    lite3d_material *material)
{
    SDL_assert(material);
    lite3d_list_init(&material->passes);
}

void lite3d_material_purge(
    lite3d_material *material)
{
    lite3d_list_node *passNode;
    lite3d_material_pass *pass;
    SDL_assert(material);

    while ((passNode = lite3d_list_first_link(&material->passes)) != NULL)
    {
        pass = MEMBERCAST(lite3d_material_pass, passNode, passLink);
        lite3d_list_unlink_link(passNode);
        lite3d_material_pass_purge(pass);
        lite3d_free(pass);
    }
}

void lite3d_material_pass_init(
    lite3d_material_pass *pass)
{
    SDL_assert(pass);
    memset(pass, 0, sizeof (lite3d_material_pass));
    lite3d_list_init(&pass->parameters);
    lite3d_list_link_init(&pass->passLink);
}

lite3d_material_pass* lite3d_material_add_pass(
    lite3d_material *material)
{
    lite3d_material_pass *pass;

    SDL_assert(material);
    pass = (lite3d_material_pass *) lite3d_malloc(sizeof (lite3d_material_pass));
    SDL_assert_release(pass);

    lite3d_material_pass_init(pass);
    lite3d_list_add_last_link(&pass->passLink, &material->passes);

    pass->passNo = ++material->passesCounter;
    return pass;
}

int lite3d_material_remove_pass(
    lite3d_material *material, uint32_t no)
{
    lite3d_material_pass *pass;

    if ((pass = lite3d_material_get_pass(material, no)) != NULL)
    {
        lite3d_list_unlink_link(&pass->passLink);
        lite3d_material_pass_purge(pass);
        lite3d_free(pass);
        return LITE3D_TRUE;
    }
    
    return LITE3D_FALSE;
}

void lite3d_material_render(
    lite3d_material *material, lite3d_draw_batch_t render, void *data)
{

}

void lite3d_material_pass_add_parameter(lite3d_material_pass *pass,
    lite3d_shader_parameter *param)
{
    lite3d_material_pass_parameter *parameter;
    SDL_assert(pass);
    
    parameter = (lite3d_material_pass_parameter *)lite3d_malloc(
        sizeof(lite3d_material_pass_parameter));
    SDL_assert_release(parameter);
    
    parameter->parameter = param;
    parameter->shaderLocation = 0;
    lite3d_list_link_init(&parameter->parameterLink);
    lite3d_list_add_last_link(&parameter->parameterLink, &pass->parameters);
}

int lite3d_material_pass_remove_parameter(lite3d_material_pass *pass,
    const char *name)
{
    lite3d_material_pass_parameter *parameter;
    SDL_assert(pass);
    
    if((parameter = lite3d_material_pass_find_parameter(pass, name)) != NULL)
    {
        lite3d_list_unlink_link(&parameter->parameterLink);
        lite3d_free(parameter);
        
        return LITE3D_TRUE;
    }
    
    return LITE3D_FALSE;
}

lite3d_shader_parameter *lite3d_material_pass_get_parameter(
    lite3d_material_pass *pass, const char *name)
{
    lite3d_material_pass_parameter *parameter;
    parameter = lite3d_material_pass_find_parameter(pass, name);
    
    return parameter ? parameter->parameter : NULL;
}

lite3d_material_pass *lite3d_material_get_pass(
    lite3d_material *material, uint32_t no)
{
    lite3d_list_node *passNode;
    lite3d_material_pass *pass;
    SDL_assert(material);

    for (passNode = material->passes.l.next;
        passNode != &material->passes.l; passNode = lite3d_list_next(passNode))
    {
        pass = MEMBERCAST(lite3d_material_pass, passNode, passLink);
        if (pass->passNo == no)
            return pass;
    }

    return NULL;
}