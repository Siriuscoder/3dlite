/******************************************************************************
*	This file is part of 3dlite (Light-weight 3d engine).
*	Copyright (C) 2014  Sirius (Korolev Nikita)
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
#ifndef LITE3D_GL_TEXTURE_UNIT_H
#define	LITE3D_GL_TEXTURE_UNIT_H

#include <3dlite/common.h>

typedef struct lite3d_texture_technique_settings
{
    uint8_t anisotropic;
    uint8_t useGLCompression;
} lite3d_texture_technique_settings;

typedef struct lite3d_texture_unit
{
    int32_t textureID;

} lite3d_texture_unit;

LITE3D_CEXPORT void lite3d_texture_technique_init(const lite3d_texture_technique_settings *settings);

#endif