/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024 Sirius (Korolev Nikita)
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

#define LITE3D_LIGHT_UNDEFINED                  0x0
#define LITE3D_LIGHT_POINT                      (1u << 0)
#define LITE3D_LIGHT_DIRECTIONAL                (1u << 1)
#define LITE3D_LIGHT_SPOT                       (1u << 2)
#define LITE3D_LIGHT_ENABLED                    (1u << 3)
#define LITE3D_LIGHT_CASTSHADOW                 (1u << 4)
#define LITE3D_LIGHT_CASTSHADOW_PCF3x3          (1u << 5)
#define LITE3D_LIGHT_CASTSHADOW_PCF4x4          (1u << 6)
#define LITE3D_LIGHT_CASTSHADOW_POISSON         (1u << 7)
#define LITE3D_LIGHT_CASTSHADOW_SSS             (1u << 8)

#pragma pack(push,1)
typedef struct lite3d_light_params
{
    kmVec3 position;
    uint32_t pad1;
    kmVec3 direction;
    uint32_t pad2;
    kmVec3 diffuse;
    uint32_t pad3;
    float radiance;
    float influenceDistance;
    float maxInfluence;
    float minRadianceCutOff;
    float innerCone; // Rad
    float outerCone; // Rad
    float attenuationConstant;
    float attenuationLinear;
    float attenuationQuadratic;
    float lightSize;
    uint32_t shadowIndex;
    uint32_t flags;
} lite3d_light_params;
#pragma pack(pop)

typedef struct lite3d_light_source
{
    lite3d_light_params params;
    void *userdata;
} lite3d_light_source;

#endif
