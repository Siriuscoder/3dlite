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

#define LITE3D_LIGHT_UNDEFINED          0x0
#define LITE3D_LIGHT_POINT              0x1
#define LITE3D_LIGHT_DIRECTIONAL        0x2
#define LITE3D_LIGHT_SPOT               0x3

/* align structure to 16 bytes texel, this ability gives us a 
 * chance to use each kmVec4 field in this structure as a texel in 
 * texture buffer */
#pragma pack(push,16)
typedef struct lite3d_light_params
{
    /* block1.x - type */
    /* block1.y - enabled */
    /* block1.z - influence distance */
    /* block1.w - influence min radiance */
    kmVec4 block1;

    /* block2.x - diffuse.r */
    /* block2.y - diffuse.g  */
    /* block2.z - diffuse.b */
    /* block2.w - radiance */
    kmVec4 block2;

    /* block3.x - position.x */
    /* block3.y - position.y */
    /* block3.z - position.z */
    /* block3.w - size */
    kmVec4 block3;

    /* block4.x - direction.x */
    /* block4.y - direction.y */
    /* block4.z - direction.z */
    /* block4.w - attenuation constant */
    kmVec4 block4;

    /* block5.x - attenuation linear */
    /* block5.y - attenuation quadratic */
    /* block5.z - angle inner cone */
    /* block5.w - angle outer cone */
    kmVec4 block5;

} lite3d_light_params;
#pragma pack(pop)

typedef struct lite3d_light_source
{
    lite3d_light_params params;
    void *userdata;
} lite3d_light_source;

#endif
