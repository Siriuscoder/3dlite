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
#include <SDL_log.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_buffers_manip.h>
#include <lite3d/lite3d_material.h>

static void lite3d_material_pass_purge(lite3d_material_pass *pass)
{
    if (!pass->passNo)
        return;
    
    lite3d_material_pass_remove_all_parameters(pass);
    pass->passNo = 0;
}

void lite3d_material_init(lite3d_material *material)
{
    SDL_assert(material);
    memset(material, 0, sizeof(lite3d_material));
    lite3d_array_init(&material->passes, sizeof (lite3d_material_pass), LITE3D_PASSNO_MAX);
    material->userdata = NULL;
}

void lite3d_material_purge(lite3d_material *material)
{
    SDL_assert(material);

    uint32_t i;
    for (i = 0; i < material->passes.size; ++i)
    {
        lite3d_material_pass_purge(lite3d_array_get(&material->passes, i));
    }

    lite3d_array_purge(&material->passes);
}

void lite3d_material_pass_init(lite3d_material_pass *pass, uint32_t no)
{
    SDL_assert(pass);
    lite3d_shader_parameters_init(&pass->parameters);
    pass->passNo = no;
    pass->program = NULL;
    pass->blending = LITE3D_FALSE;
    pass->blendingMode = LITE3D_BLENDING_MODE_RGB_LINEAR_SOURCE_ALPHA;
    pass->doubleSided = LITE3D_FALSE;
    pass->polygonMode = LITE3D_POLYMODE_FILL;
}

lite3d_material_pass* lite3d_material_add_pass(
    lite3d_material *material, uint32_t no)
{
    lite3d_material_pass *pass;
    size_t i;

    SDL_assert(material);

    if (!no || no > material->passes.capacity)
        return NULL;

    if (no > material->passes.size)
    {
        size_t addcount = no - material->passes.size;
        for(i = 0; i < addcount; ++i)
            memset(lite3d_array_add(&material->passes), 0, sizeof (lite3d_material_pass));
    }

    pass = lite3d_array_get(&material->passes, no - 1);
    lite3d_material_pass_init(pass, no);

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
    SDL_assert(pass);
    SDL_assert(param);
    lite3d_shader_parameters_add(&pass->parameters, param);
}

int lite3d_material_pass_remove_parameter(lite3d_material_pass *pass,
    const char *name)
{
    SDL_assert(pass);
    SDL_assert(name);
    return lite3d_shader_parameters_remove(&pass->parameters, name);
}

void lite3d_material_pass_remove_all_parameters(lite3d_material_pass *pass)
{
    SDL_assert(pass);
    lite3d_shader_parameters_remove_all(&pass->parameters);
}

lite3d_shader_parameter *lite3d_material_pass_get_parameter(
    lite3d_material_pass *pass, const char *name)
{
    SDL_assert(pass);
    return lite3d_shader_parameters_get(&pass->parameters, name);
}

lite3d_material_pass *lite3d_material_get_pass(
    const lite3d_material *material, uint32_t no)
{
    SDL_assert(material);

    if (material->passes.size < no || !no)
        return NULL;

    return lite3d_array_get(&((lite3d_material *)material)->passes, no - 1);
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

lite3d_material_pass *lite3d_material_apply(
    lite3d_material *material, uint16_t no)
{
    lite3d_material_pass *pass;
    SDL_assert(material);

    pass = lite3d_material_get_pass(material, no);

    SDL_assert(pass);
    /* ignode empty pass */
    if (pass->passNo == 0)
        return pass;

    /* bind current shander first */
    lite3d_shader_program_bind(pass->program);
    /* set up uniforms if shader changed */
    lite3d_shader_program_apply_parameters(pass->program, &pass->parameters, LITE3D_TRUE);
    lite3d_blending(pass->blending);
    lite3d_blending_mode_set(pass->blendingMode);
    lite3d_backface_culling(pass->doubleSided ? LITE3D_CULLFACE_NEVER : LITE3D_CULLFACE_BACK);
    lite3d_polygon_mode(pass->polygonMode);

    return pass;
}
