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
#ifndef LITE3D_MESH_ASSIMP_LOADER_H
#define	LITE3D_MESH_ASSIMP_LOADER_H

#ifdef INCLUDE_ASSIMP

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_mesh.h>
#include <lite3d/lite3d_pack.h>
#include <lite3d/lite3d_kazmath.h>
#include <lite3d/lite3d_lighting.h>

#define LITE3D_OPTIMIZE_MESH_FLAG             0x1
#define LITE3D_FLIP_UV_FLAG                   0x2
#define LITE3D_MERGE_NODES_FLAG               0x4

typedef struct lite3d_assimp_loader_ctx
{
    lite3d_mesh *(*onAllocMesh)(void *userdata);
    void (*onMesh)(lite3d_mesh *, const kmMat4 *transform, const char *name, void *userdata);
    void (*onMaterial)(const char *matName, 
        uint32_t matIndex,
        const kmVec4 *ambient,
        const kmVec4 *diffuse,
        const kmVec4 *specular,
        const kmVec4 *emissive,
        const kmVec4 *reflective,
        const kmVec4 *transparent,
        const char *diffuseTextureFile,
        const char *normalTextureFile,
        const char *reflectionTextureFile,
        void *userdata);
    void (*onLight)(const char *lightName,
        const lite3d_light_params *params,
        const kmMat4 *transform,
        void *userdata);

    void (*onLevelPush)(void *userdata);
    void (*onLevelPop)(void *userdata);
    void *userdata;
} lite3d_assimp_loader_ctx;

/*
    note:
        function load mesh model with specified *name* from *resource*,
        if *name* is NULL then first model node from *resource* will be load.
*/
LITE3D_CEXPORT int lite3d_assimp_mesh_load(lite3d_mesh *mesh, const lite3d_file *resource, 
    const char *name, uint16_t access, uint32_t flags);

/*
    note:
        recursive load all meshes from *resource*
*/
LITE3D_CEXPORT int lite3d_assimp_mesh_load_recursive(const lite3d_file *resource, 
    lite3d_assimp_loader_ctx ctx,
    uint16_t access, uint32_t flags);

LITE3D_CEXPORT void lite3d_assimp_logging_init(void);
LITE3D_CEXPORT void lite3d_assimp_logging_level(int8_t level);
LITE3D_CEXPORT void lite3d_assimp_logging_release(void);

#endif
#endif	/* LITE3D_MESH_ASSIMP_LOADER_H */

