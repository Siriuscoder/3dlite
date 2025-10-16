/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2025  Sirius (Korolev Nikita)
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

#define LITE3D_SHADER_PROGRAM_TYPE_COMMON_PIPELINE  0x1
#define LITE3D_SHADER_PROGRAM_TYPE_COMPUTE          0x2

typedef struct lite3d_shader_program
{
    uint32_t programID;
    char *statusString;
    uint8_t success;
    uint8_t validated;
    uint8_t type;
    uint32_t syncFlags;
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
    int8_t direction;
} lite3d_shader_parameter_container;

LITE3D_CEXPORT int lite3d_shader_program_technique_init(void);
LITE3D_CEXPORT void lite3d_shader_program_get_limitations(int *maxGeometryOutputVertices, 
    int *maxGeometryOutputComponents, int *maxGeometryTotalOutputComponents);
LITE3D_CEXPORT int lite3d_shader_program_init(struct lite3d_shader_program *program);
LITE3D_CEXPORT int lite3d_shader_program_link(
    struct lite3d_shader_program *program, lite3d_shader *shaders, size_t count);
LITE3D_CEXPORT void lite3d_shader_program_purge(
    struct lite3d_shader_program *program);
LITE3D_CEXPORT void lite3d_shader_program_bind(
    struct lite3d_shader_program *program);
LITE3D_CEXPORT void lite3d_shader_program_unbind(
    struct lite3d_shader_program *program);
LITE3D_CEXPORT int lite3d_shader_program_validate(
    struct lite3d_shader_program *program);

/* set uniform or sampler params to shader, shader must be bind */
LITE3D_CEXPORT int lite3d_shader_program_uniform_set(
    struct lite3d_shader_program *program, struct lite3d_shader_parameter_container *p);

LITE3D_CEXPORT void lite3d_shader_program_attribute_index(
    struct lite3d_shader_program *program, const char *name, int32_t location);

LITE3D_CEXPORT int lite3d_shader_program_validate_current(void);

LITE3D_CEXPORT void lite3d_shader_program_compute_dispatch(struct lite3d_shader_program *program, uint32_t numGroupsX,
    uint32_t numGroupsY, uint32_t numGroupsZ);

LITE3D_CEXPORT void lite3d_shader_program_compute_dispatch_sync(struct lite3d_shader_program *program, uint32_t numGroupsX,
    uint32_t numGroupsY, uint32_t numGroupsZ);

#endif	/* LITE3D_SHADER_PROGRAM_H */

