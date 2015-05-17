/******************************************************************************
*	This file is part of 3dlite (Light-weight 3d engine).
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
#ifndef LITE3D_MESH_H
#define	LITE3D_MESH_H

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_vbo.h>

/* legacy types, now not used  */
#define LITE3D_BUFFER_BINDING_VERTEX          0x0
#define LITE3D_BUFFER_BINDING_COLOR           0x1
#define LITE3D_BUFFER_BINDING_NORMAL          0x2
#define LITE3D_BUFFER_BINDING_TEXCOORD        0x3
/* may be more then one, attribute index will be increased */
#define LITE3D_BUFFER_BINDING_ATTRIBUTE       0x4

#define LITE3D_INDEX_UNSIGNED_BYTE  0x1401
#define LITE3D_INDEX_UNSIGNED_SHORT 0x1403
#define LITE3D_INDEX_UNSIGNED_INT   0x1405

#define LITE3D_PRIMITIVE_POINT      0x0000
#define LITE3D_PRIMITIVE_LINE       0x0001
#define LITE3D_PRIMITIVE_TRIANGLE   0x0004

typedef struct lite3d_indexed_mesh_layout
{
    uint8_t binding;
    uint8_t count; /* count elements in component */
} lite3d_indexed_mesh_layout;

typedef struct lite3d_indexed_mesh
{
    lite3d_vbo vertexBuffer;
    lite3d_vbo indexBuffer;
    size_t verticesCount;
    size_t elementsCount;
    size_t chunkCount;
    lite3d_list chunks;
} lite3d_indexed_mesh;

typedef struct lite3d_mesh_chunk
{
    lite3d_list_node node;
    lite3d_vao vao;
    lite3d_indexed_mesh *ownMesh;
    size_t layoutEntriesCount;
    lite3d_indexed_mesh_layout *layout;
    /* material index */
    uint32_t materialIndex;
} lite3d_mesh_chunk;

LITE3D_CEXPORT int lite3d_indexed_mesh_init(struct lite3d_indexed_mesh *mesh);
LITE3D_CEXPORT void lite3d_indexed_mesh_purge(struct lite3d_indexed_mesh *mesh);
LITE3D_CEXPORT int lite3d_indexed_mesh_extend(struct lite3d_indexed_mesh *mesh, 
    size_t verticesSize, size_t indexesSize, uint16_t access);
LITE3D_CEXPORT void lite3d_indexed_mesh_draw(struct lite3d_indexed_mesh *mesh);
LITE3D_CEXPORT void lite3d_indexed_mesh_draw_instanced(struct lite3d_indexed_mesh *mesh, size_t count);

LITE3D_CEXPORT lite3d_mesh_chunk *lite3d_indexed_mesh_append_chunk(lite3d_indexed_mesh *mesh,
    const lite3d_indexed_mesh_layout *layout,
    size_t layoutCount,
    size_t stride,
    uint16_t componentType,
    uint16_t indexPrimitive,
    size_t indexesCount,
    size_t indexesSize,    
    size_t indexesOffset,
    size_t verticesCount,
    size_t verticesSize,
    size_t verticesOffset);

LITE3D_CEXPORT int lite3d_mesh_chunk_init(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_purge(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_bind(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_indexed_mesh_chunk_draw(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_indexed_mesh_chunk_draw_instanced(struct lite3d_mesh_chunk *meshChunk, size_t count);
LITE3D_CEXPORT void lite3d_mesh_chunk_draw(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_draw_instanced(struct lite3d_mesh_chunk *meshChunk, size_t count);
LITE3D_CEXPORT void lite3d_mesh_chunk_unbind(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT lite3d_mesh_chunk *lite3d_mesh_chunk_get_by_index(struct lite3d_indexed_mesh *mesh, 
    uint32_t materialIndex);

LITE3D_CEXPORT uint16_t lite3d_index_component_type_by_size(uint8_t size); 
LITE3D_CEXPORT uint16_t lite3d_index_primitive_by_components(uint8_t count);

#endif	/* LITE3D_MESH_H */
