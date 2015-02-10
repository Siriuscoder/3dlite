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
#include <3dlite/3dlite_shader_params.h>
#include <3dlite/3dlite_list.h>

typedef void (*lite3d_draw_batch_t)(void *data);

typedef struct lite3d_material_pass
{
    lite3d_list_node textureBy;
    kmVec4 ambient;
    kmVec4 diffuse;
    kmVec4 specular;
    kmVec4 emission;
    kmVec4 shininess;
    lite3d_shader_parameter *params[20];
    
} lite3d_material_pass;

typedef struct lite3d_material
{
    lite3d_list passes;
    
} lite3d_material;

LITE3D_CEXPORT void lite3d_material_init(
    lite3d_material *store);
LITE3D_CEXPORT void lite3d_material_purge(
    lite3d_material *store);
LITE3D_CEXPORT void lite3d_material_pass_init(
    lite3d_material_pass *pass);
LITE3D_CEXPORT lite3d_material_pass* lite3d_material_add_pass(
    lite3d_material *material);
LITE3D_CEXPORT void lite3d_material_render(
    lite3d_material *material, lite3d_draw_batch_t render, void *data);

#endif	/* LITE3D_MATERIAL_H */

