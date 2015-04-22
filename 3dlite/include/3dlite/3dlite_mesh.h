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
#ifndef LITE3D_VBO_H
#define	LITE3D_VBO_H

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_list.h>

#define LITE3D_VBO_STREAM_DRAW 0x88E0
#define LITE3D_VBO_STREAM_READ 0x88E1
#define LITE3D_VBO_STREAM_COPY 0x88E2
#define LITE3D_VBO_STATIC_DRAW 0x88E4
#define LITE3D_VBO_STATIC_READ 0x88E5
#define LITE3D_VBO_STATIC_COPY 0x88E6
#define LITE3D_VBO_DYNAMIC_DRAW 0x88E8
#define LITE3D_VBO_DYNAMIC_READ 0x88E9
#define LITE3D_VBO_DYNAMIC_COPY 0x88EA

typedef struct lite3d_indexed_mesh
{
    uint32_t vboVerticesID;
    uint32_t vboIndexesID;
    size_t verticesSize;
    size_t indexesSize;
    size_t verticesCount;
    size_t elementsCount;
    size_t vaosCount;
    lite3d_list vaos;
} lite3d_indexed_mesh;

typedef struct lite3d_mesh_chunk
{
    lite3d_list_node inVbo;
    uint32_t vaoID;
    size_t elementsCount;
    size_t indexesCount;
    size_t indexesSize;
    size_t indexesOffset;
    size_t verticesCount;
    size_t verticesSize;
    size_t verticesOffset;
    uint16_t elementType; /* GL_POINTS, GL_LINES, GL_TRIANGLES (GL value)*/
    uint16_t indexType; /* Byte, short, int (GL value)*/

    lite3d_indexed_mesh *ownVbo;
    /* material index */
    uint32_t materialIndex;
} lite3d_mesh_chunk;

LITE3D_CEXPORT int lite3d_indexed_mesh_technique_init(void);
LITE3D_CEXPORT int lite3d_indexed_mesh_init(struct lite3d_indexed_mesh *mesh);
LITE3D_CEXPORT void lite3d_indexed_mesh_purge(struct lite3d_indexed_mesh *mesh);
LITE3D_CEXPORT int lite3d_indexed_mesh_extend(struct lite3d_indexed_mesh *mesh, 
    size_t verticesSize, size_t indexesSize, uint16_t access);
LITE3D_CEXPORT void lite3d_indexed_mesh_draw(struct lite3d_indexed_mesh *mesh);
LITE3D_CEXPORT void lite3d_indexed_mesh_draw_instanced(struct lite3d_mesh_chunk *meshChunk, size_t count);

LITE3D_CEXPORT int lite3d_mesh_chunk_init(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_purge(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_bind(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_draw(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_unbind(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT lite3d_mesh_chunk *lite3d_mesh_chunk_get_by_index(struct lite3d_indexed_mesh *mesh, 
    uint32_t materialIndex);

#endif	/* LITE3D_VBO_H */

