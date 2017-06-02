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
#ifndef LITE3D_MESH_H
#define	LITE3D_MESH_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_vbo.h>
#include <lite3d/lite3d_vao.h>
#include <lite3d/lite3d_frustum.h>

/* legacy types, now not used  */
#define LITE3D_BUFFER_BINDING_VERTEX          0x0
#define LITE3D_BUFFER_BINDING_COLOR           0x1
#define LITE3D_BUFFER_BINDING_NORMAL          0x2
#define LITE3D_BUFFER_BINDING_TEXCOORD        0x3
/* may be more then one, attribute index will be increased */
#define LITE3D_BUFFER_BINDING_ATTRIBUTE       0x4
#define LITE3D_BUFFER_BINDING_TANGENT         0x5
#define LITE3D_BUFFER_BINDING_BINORMAL        0x6

#define LITE3D_INDEX_UNSIGNED_BYTE  0x1401
#define LITE3D_INDEX_UNSIGNED_SHORT 0x1403
#define LITE3D_INDEX_UNSIGNED_INT   0x1405

#define LITE3D_PRIMITIVE_POINT      0x0000
#define LITE3D_PRIMITIVE_LINE       0x0001
#define LITE3D_PRIMITIVE_TRIANGLE   0x0004

typedef struct lite3d_mesh_layout
{
    uint8_t binding;
    uint8_t count; /* count elements in component */
} lite3d_mesh_layout;

typedef struct lite3d_mesh
{
    uint32_t version;
    lite3d_vbo vertexBuffer;
    lite3d_vbo indexBuffer;
    lite3d_vbo *auxBuffer;
    size_t verticesCount;
    size_t elementsCount;
    size_t chunkCount;
    lite3d_list chunks;
    void *userdata;
} lite3d_mesh;

typedef struct lite3d_mesh_chunk
{
    lite3d_list_node node;
    lite3d_vao vao;
    size_t layoutEntriesCount;
    lite3d_mesh_layout *layout;
    /* material index */
    uint32_t materialIndex;
    uint8_t hasIndexes;
    lite3d_bounding_vol boundingVol;
    lite3d_mesh *mesh;
} lite3d_mesh_chunk;

LITE3D_CEXPORT int lite3d_mesh_init(struct lite3d_mesh *mesh);
LITE3D_CEXPORT void lite3d_mesh_purge(struct lite3d_mesh *mesh);
LITE3D_CEXPORT int lite3d_mesh_extend(struct lite3d_mesh *mesh, 
    size_t verticesSize, size_t indexesSize, uint16_t access);

LITE3D_CEXPORT lite3d_mesh_chunk *lite3d_mesh_append_chunk(lite3d_mesh *mesh,
    const lite3d_mesh_layout *layout,
    size_t layoutCount,
    size_t stride,
    uint16_t componentType,
    size_t indexesCount,
    size_t indexesSize,
    size_t indexesOffset,
    size_t verticesCount,
    size_t verticesSize,
    size_t verticesOffset);

LITE3D_CEXPORT int lite3d_mesh_chunk_init(struct lite3d_mesh_chunk *meshChunk, uint8_t indexed);
LITE3D_CEXPORT void lite3d_mesh_chunk_purge(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_bind(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_draw(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_draw_instanced(struct lite3d_mesh_chunk *meshChunk, size_t instancesCount);
LITE3D_CEXPORT void lite3d_mesh_chunk_unbind(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT lite3d_mesh_chunk *lite3d_mesh_chunk_get_by_index(struct lite3d_mesh *mesh, 
    uint32_t materialIndex);

LITE3D_CEXPORT uint16_t lite3d_index_component_type_by_size(uint8_t size); 
LITE3D_CEXPORT uint8_t lite3d_size_by_index_type(uint16_t type);

#endif	/* LITE3D_MESH_H */

