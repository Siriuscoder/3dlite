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
#ifndef LITE3D_VBO_LOADER_H
#define	LITE3D_VBO_LOADER_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_mesh.h>
#include <lite3d/lite3d_pack.h>

#define LITE3D_OPTIMIZE_MESH_FLAG             0x1
#define LITE3D_FLIP_UV_FLAG                   0x2
#define LITE3D_MERGE_NODES_FLAG               0x4

/*
    note:
        function load mesh model from m file represented by *resource*
*/
LITE3D_CEXPORT int lite3d_mesh_load_from_m_file(lite3d_mesh *mesh, const lite3d_file *resource);


LITE3D_CEXPORT int lite3d_mesh_indexed_load_from_memory(lite3d_mesh *mesh, 
    const void *vertices, 
    uint32_t verticesCount,
    const lite3d_vao_layout *layout,
    uint32_t layoutCount,
    const void *indexes, 
    uint32_t elementsCount);

LITE3D_CEXPORT int lite3d_mesh_indexed_append_from_memory(lite3d_mesh *mesh, 
    const void *vertices, 
    uint32_t verticesCount,
    const lite3d_vao_layout *layout,
    uint32_t layoutCount,
    const void *indexes, 
    uint32_t elementsCount);

LITE3D_CEXPORT int lite3d_mesh_load_from_memory(lite3d_mesh *mesh, 
    const void *vertices, 
    uint32_t verticesCount,
    const lite3d_vao_layout *layout,
    uint32_t layoutCount);

LITE3D_CEXPORT int lite3d_mesh_append_from_memory(lite3d_mesh *mesh, 
    const void *vertices, 
    uint32_t verticesCount,
    const lite3d_vao_layout *layout,
    uint32_t layoutCount);

LITE3D_CEXPORT void lite3d_mesh_order_mat_indexes(lite3d_mesh *mesh);

#endif	/* LITE3D_VBO_LOADER_H */

