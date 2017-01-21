/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2017  Sirius (Korolev Nikita)
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
#ifndef LITE3D_LIGHTING_H
#define	LITE3D_LIGHTING_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_kazmath.h>
#include <lite3d/lite3d_scene_node.h>

#define LITE3D_LIGHT_POINT              0x1
#define LITE3D_LIGHT_DIRECTIONAL        0x2
#define LITE3D_LIGHT_SPOT               0x3

/* align structure to 16 bytes texel, this ability gives us a 
 * chance to use each kmVec4 field in this structure as a texel in 
 * texture buffer */
#pragma pack(push,16)
typedef struct lite3d_light_params
{
    /* flags.x - type */
    /* flags.y - enabled */
    kmVec4 flags;
    /* light position */
    kmVec4 position;
    /* spot directional */
    /* take effect with spot and directional light */
    kmVec4 spotDirection;
    /* ambient color */
    kmVec4 ambient;
    /* diffuse color */
    kmVec4 diffuse;
    /* specular color */
    kmVec4 specular;
    /* attenuation factor */
    /* x - constant attenuation */
    /* y - linear attenuation */
    /* z - quadratic attenuation */
    kmVec4 attenuation;
    /* x - spot cutoff */
    /* y - spot cutoff exponent */
    kmVec4 spotFactor;
} lite3d_light_params;
#pragma pack(pop)

typedef struct lite3d_light_source
{
    lite3d_scene_node lightNode;
    lite3d_light_params params;
    void *userdata;
} lite3d_light_source;

#endif
