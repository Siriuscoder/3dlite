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
#ifndef LITE3D_MATERIAL_H
#define	LITE3D_MATERIAL_H

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_shader_program.h>
#include <3dlite/3dlite_list.h>

typedef void (*lite3d_draw_batch_t)(void *data);

typedef struct lite3d_material_pass_parameter
{
    lite3d_list_node parameterLink;
    lite3d_shader_parameter *parameter;
    /* uniform location in shader program attached to this pass */
    uint32_t shaderLocation;
} lite3d_material_pass_parameter;

typedef struct lite3d_material_pass
{
    lite3d_list_node passLink;
    uint32_t passNo;
    /* shader must be linked before set parameters */
    lite3d_shader_program *program;
    /* list lite3d_material_pass_parameter */
    lite3d_list parameters;
} lite3d_material_pass;

typedef struct lite3d_material
{
    lite3d_list passes;
    uint32_t passesCounter;
} lite3d_material;

LITE3D_CEXPORT void lite3d_material_init(
    lite3d_material *material);
LITE3D_CEXPORT void lite3d_material_purge(
    lite3d_material *material);
LITE3D_CEXPORT void lite3d_material_pass_init(
    lite3d_material_pass *pass);
LITE3D_CEXPORT lite3d_material_pass* lite3d_material_add_pass(
    lite3d_material *material);
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
    lite3d_material *material, uint32_t no);

#endif	/* LITE3D_MATERIAL_H */

