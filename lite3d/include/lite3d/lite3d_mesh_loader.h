/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2015  Sirius (Korolev Nikita)
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
#ifndef LITE3D_VBO_LOADER_H
#define	LITE3D_VBO_LOADER_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_mesh.h>
#include <lite3d/lite3d_pack.h>

#define LITE3D_OPTIMIZE_MESH_FLAG             0x1
#define LITE3D_FLIP_UV_FLAG                   0x2
#define LITE3D_MERGE_NODES_FLAG               0x4

typedef lite3d_indexed_mesh *(*lite3d_retrieve_mesh)(void);
typedef void (*lite3d_mesh_loaded)(lite3d_indexed_mesh *, const char *name);

/*
    note:
        function load mesh model with specified *name* from *resource*,
        if *name* is NULL then first model node from *resource* will be load.
*/
LITE3D_CEXPORT int lite3d_indexed_mesh_load(lite3d_indexed_mesh *mesh, const lite3d_file *resource, 
    const char *name, uint16_t access, uint32_t flags);

/*
    note:
        recursive load all meshes from *resource*
*/
LITE3D_CEXPORT int lite3d_indexed_mesh_load_recursive(const lite3d_file *resource, 
    lite3d_retrieve_mesh retrieveMesh, lite3d_mesh_loaded meshLoaded, uint16_t access, uint32_t flags);

/*
    note:
        function load mesh model from m file represented by *resource*
*/
LITE3D_CEXPORT int lite3d_indexed_mesh_load_from_m_file(lite3d_indexed_mesh *mesh, const lite3d_file *resource, 
    uint16_t access);


LITE3D_CEXPORT int lite3d_indexed_mesh_load_from_memory(lite3d_indexed_mesh *mesh, 
    const void *vertices, 
    size_t verticesCount, 
    const lite3d_indexed_mesh_layout *layout,
    size_t layoutCount,
    const void *indexes, 
    size_t elementsCount, 
    uint8_t indexComponents, 
    uint16_t access);

LITE3D_CEXPORT int lite3d_indexed_mesh_extend_from_memory(lite3d_indexed_mesh *mesh, 
    const void *vertices, 
    size_t verticesCount, 
    const lite3d_indexed_mesh_layout *layout,
    size_t layoutCount,
    const void *indexes, 
    size_t elementsCount, 
    uint8_t indexComponents, 
    uint16_t access);

LITE3D_CEXPORT void lite3d_indexed_mesh_order_mat_indexes(lite3d_indexed_mesh *mesh);

#endif	/* LITE3D_VBO_LOADER_H */
