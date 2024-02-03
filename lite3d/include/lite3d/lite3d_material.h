/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2024 Sirius (Korolev Nikita)
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
#ifndef LITE3D_MATERIAL_H
#define	LITE3D_MATERIAL_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_shader_program.h>
#include <lite3d/lite3d_list.h>
#include <lite3d/lite3d_array.h>

#define LITE3D_PASSNO_MAX  16

typedef struct lite3d_material_pass
{
    uint32_t passNo;
    /* shader must be linked before set parameters */
    lite3d_shader_program *program;
    lite3d_shader_binding_context bindContext;
    /* list lite3d_shader_parameter_container */
    lite3d_list parameters;
    /* blending */
    uint8_t blending;
    uint8_t blendingMode;
} lite3d_material_pass;

typedef struct lite3d_material
{
    lite3d_array passes;
    void *userdata;
} lite3d_material;

LITE3D_CEXPORT void lite3d_material_init(
    lite3d_material *material);
LITE3D_CEXPORT void lite3d_material_purge(
    lite3d_material *material);
LITE3D_CEXPORT void lite3d_material_pass_init(
    lite3d_material_pass *pass, uint32_t no);
LITE3D_CEXPORT lite3d_material_pass* lite3d_material_add_pass(
    lite3d_material *material, uint32_t no);
LITE3D_CEXPORT int lite3d_material_remove_pass(
    lite3d_material *material, uint32_t no);
LITE3D_CEXPORT void lite3d_material_pass_add_parameter(lite3d_material_pass *pass,
    lite3d_shader_parameter *param);
LITE3D_CEXPORT int lite3d_material_pass_remove_parameter(lite3d_material_pass *pass,
    const char *name);
LITE3D_CEXPORT void lite3d_material_pass_remove_all_parameters(lite3d_material_pass *pass);
LITE3D_CEXPORT lite3d_shader_parameter *lite3d_material_pass_get_parameter(
    lite3d_material_pass *pass, const char *name);
LITE3D_CEXPORT lite3d_material_pass *lite3d_material_get_pass(
    const lite3d_material *material, uint32_t no);
LITE3D_CEXPORT int lite3d_material_pass_is_blend(
    const lite3d_material *material, uint32_t no);
LITE3D_CEXPORT int lite3d_material_pass_is_empty(
    const lite3d_material *material, uint32_t no);

LITE3D_CEXPORT lite3d_material_pass *lite3d_material_apply(lite3d_material *material, uint16_t no);
LITE3D_CEXPORT void lite3d_material_pass_set_params(lite3d_material *material,
    lite3d_material_pass *pass, uint8_t changed);

#endif	/* LITE3D_MATERIAL_H */

