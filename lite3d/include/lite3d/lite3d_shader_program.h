/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2024  Sirius (Korolev Nikita)
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
#ifndef LITE3D_SHADER_PROGRAM_H
#define	LITE3D_SHADER_PROGRAM_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_shader.h>
#include <lite3d/lite3d_shader_params.h>

typedef struct lite3d_shader_program
{
    uint32_t programID;
    char *statusString;
    uint8_t success;
    uint8_t validated;
    /* userdata */
    void *userdata;
} lite3d_shader_program;

typedef struct lite3d_shader_binding_context
{
    int16_t textureBindingsCount;
    int16_t blockBindingsCount;  
} lite3d_shader_binding_context;

typedef struct lite3d_shader_parameter_container
{
    lite3d_list_node parameterLink;
    lite3d_shader_parameter *parameter;
    lite3d_shader_binding_context *bindContext;
    /* uniform location in shader program attached to this pass */
    int32_t location;
    int16_t binding;
} lite3d_shader_parameter_container;

LITE3D_CEXPORT int lite3d_shader_program_technique_init(void);
LITE3D_CEXPORT int lite3d_shader_program_init(lite3d_shader_program *program);
LITE3D_CEXPORT int lite3d_shader_program_link(
    lite3d_shader_program *program, lite3d_shader *shaders, size_t count);
LITE3D_CEXPORT void lite3d_shader_program_purge(
    lite3d_shader_program *program);
LITE3D_CEXPORT void lite3d_shader_program_bind(
    lite3d_shader_program *program);
LITE3D_CEXPORT void lite3d_shader_program_unbind(
    lite3d_shader_program *program);
LITE3D_CEXPORT int lite3d_shader_program_validate(
    lite3d_shader_program *program);

/* set uniform or sampler params to shader, shader must be bind */
LITE3D_CEXPORT int lite3d_shader_program_uniform_set(
    lite3d_shader_program *program, lite3d_shader_parameter_container *p);

LITE3D_CEXPORT void lite3d_shader_program_attribute_index(
    lite3d_shader_program *program, const char *name, int32_t location);

#endif	/* LITE3D_SHADER_PROGRAM_H */

