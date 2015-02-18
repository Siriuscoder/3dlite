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
#ifndef LITE3D_SHADER_PARAMS_H
#define	LITE3D_SHADER_PARAMS_H

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_texture_unit.h>
#include <3dlite/3dlite_rb_tree.h>
#include <3dlite/kazmath/vec3.h>
#include <3dlite/kazmath/vec4.h>
#include <3dlite/kazmath/mat3.h>
#include <3dlite/kazmath/mat4.h>

#define LITE3D_SHADER_PARAMETER_MAX_NAME    30
#define LITE3D_SHADER_PARAMETER_FLOAT       0x1
#define LITE3D_SHADER_PARAMETER_FLOATV3     0x2
#define LITE3D_SHADER_PARAMETER_FLOATV4     0x3
#define LITE3D_SHADER_PARAMETER_FLOATM3     0x4
#define LITE3D_SHADER_PARAMETER_FLOATM4     0x5
#define LITE3D_SHADER_PARAMETER_SAMPLER     0x6

typedef struct lite3d_sampler
{
    lite3d_texture_unit *texture;
    uint32_t textureUnit;
} lite3d_sampler;

typedef struct lite3d_shader_parameter
{
    lite3d_rb_node storeBy;
    char name[LITE3D_SHADER_PARAMETER_MAX_NAME];
    uint8_t type;
    
    union
    {
        float valfloat;
        lite3d_sampler valsampler;
        kmVec3 valvec3;
        kmVec4 valvec4;
        kmMat3 valmat3;
        kmMat4 valmat4;
    } parameter;
} lite3d_shader_parameter;

typedef struct lite3d_shader_parameters_store
{
    lite3d_rb_tree *parameters;
} lite3d_shader_parameters_store;

LITE3D_CEXPORT void lite3d_shader_parameters_store_init(
    lite3d_shader_parameters_store *store);
LITE3D_CEXPORT void lite3d_shader_parameters_store_purge(
    lite3d_shader_parameters_store *store);
LITE3D_CEXPORT void lite3d_shader_parameter_init(
    lite3d_shader_parameter *param);
LITE3D_CEXPORT void lite3d_shader_parameter_purge(
    lite3d_shader_parameter *param);
LITE3D_CEXPORT int lite3d_shader_parameters_store_add(
    lite3d_shader_parameters_store *store, 
    lite3d_shader_parameter *param);
LITE3D_CEXPORT int lite3d_shader_parameters_store_remove(
    lite3d_shader_parameters_store *store, 
    const char *name);
LITE3D_CEXPORT lite3d_shader_parameter *lite3d_shader_parameters_store_lookup(
    lite3d_shader_parameters_store *store, 
    const char *name);


#endif	/* LITE3D_SHADER_PARAMS_H */

