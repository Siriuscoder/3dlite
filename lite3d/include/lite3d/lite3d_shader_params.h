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
#ifndef LITE3D_SHADER_PARAMS_H
#define	LITE3D_SHADER_PARAMS_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_texture_unit.h>
#include <lite3d/lite3d_rb_tree.h>
#include <lite3d/lite3d_kazmath.h>

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
    char name[LITE3D_SHADER_PARAMETER_MAX_NAME];
    uint8_t type;
    uint8_t changed;
    
    union
    {
        float valfloat;
        lite3d_sampler valsampler;
        kmVec3 valvec3;
        kmVec4 valvec4;
        kmMat3 valmat3;
        kmMat4 valmat4;
    } parameter;
    /* userdata */
    void *userdata;
} lite3d_shader_parameter;

typedef struct lite3d_global_parameters
{
    lite3d_shader_parameter projectionMatrix;
    lite3d_shader_parameter viewMatrix;
    lite3d_shader_parameter modelMatrix;
    lite3d_shader_parameter modelviewMatrix;
    lite3d_shader_parameter ambientLight;
} lite3d_global_parameters;

LITE3D_CEXPORT void lite3d_shader_parameter_init(
    lite3d_shader_parameter *param);
LITE3D_CEXPORT void lite3d_shader_parameter_purge(
    lite3d_shader_parameter *param);

LITE3D_CEXPORT void lite3d_shader_global_parameters_init(void);
LITE3D_CEXPORT lite3d_global_parameters *lite3d_shader_global_parameters(void);
LITE3D_CEXPORT void lite3d_shader_set_projection_matrix(kmMat4 *mat);
LITE3D_CEXPORT void lite3d_shader_set_view_matrix(kmMat4 *mat);
LITE3D_CEXPORT void lite3d_shader_set_model_matrix(kmMat4 *mat);
LITE3D_CEXPORT void lite3d_shader_set_modelview_matrix(kmMat4 *mat);
LITE3D_CEXPORT void lite3d_shader_set_ambient_light(float val);

#endif	/* LITE3D_SHADER_PARAMS_H */

