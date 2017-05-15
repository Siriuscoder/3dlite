/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2014  Sirius (Korolev Nikita)
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
#ifndef LITE3D_SHADER_H
#define	LITE3D_SHADER_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_shader_params.h>

#define LITE3D_SHADER_TYPE_VERTEX       0x1
#define LITE3D_SHADER_TYPE_FRAGMENT     0x2

typedef struct lite3d_shader
{
    uint32_t shaderID;
    char *statusString;
    uint8_t success;
    uint8_t type;
} lite3d_shader;

LITE3D_CEXPORT int lite3d_shader_init(lite3d_shader *shader, uint8_t type);
LITE3D_CEXPORT int lite3d_shader_compile(
    lite3d_shader *shader, uint32_t sources, const char **source, int32_t *length);
LITE3D_CEXPORT void lite3d_shader_purge(
    lite3d_shader *shader);

#endif	/* LITE3D_SHADER_H */

