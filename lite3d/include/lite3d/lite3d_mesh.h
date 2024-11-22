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
#ifndef LITE3D_MESH_H
#define	LITE3D_MESH_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_array.h>
#include <lite3d/lite3d_list.h>
#include <lite3d/lite3d_vbo.h>
#include <lite3d/lite3d_vao.h>
#include <lite3d/lite3d_frustum.h>

#define LITE3D_INDEX_UNSIGNED_BYTE  0x1401
#define LITE3D_INDEX_UNSIGNED_SHORT 0x1403
#define LITE3D_INDEX_UNSIGNED_INT   0x1405

#define LITE3D_PRIMITIVE_POINT      0x0000
#define LITE3D_PRIMITIVE_LINE       0x0001
#define LITE3D_PRIMITIVE_TRIANGLE   0x0004

typedef struct lite3d_mesh
{
    uint32_t version;
    lite3d_vbo vertexBuffer;
    lite3d_vbo indexBuffer;
    lite3d_vbo *auxBuffer;
    uint32_t verticesCount;
    uint32_t elementsCount;
    lite3d_list chunks;
    lite3d_array drawQueue;
    void *userdata;
} lite3d_mesh;

typedef struct lite3d_mesh_chunk
{
    lite3d_list_node link;
    lite3d_vao vao;
    lite3d_array layout;
    uint32_t vertexStride;
    /* material index */
    uint32_t materialIndex;
    uint8_t hasIndexes;
    lite3d_bounding_vol boundingVol;
    lite3d_mesh *mesh;
} lite3d_mesh_chunk;

LITE3D_CEXPORT int lite3d_mesh_init(struct lite3d_mesh *mesh, uint16_t usage);
LITE3D_CEXPORT void lite3d_mesh_purge(struct lite3d_mesh *mesh);
LITE3D_CEXPORT int lite3d_mesh_extend(struct lite3d_mesh *mesh, 
    size_t verticesSize, size_t indexesSize);

LITE3D_CEXPORT lite3d_mesh_chunk *lite3d_mesh_append_chunk(struct lite3d_mesh *mesh,
    const struct lite3d_vao_layout *layout,
    uint32_t layoutCount,
    uint32_t stride,
    uint32_t indexesCount,
    size_t indexesSize,
    size_t indexesOffset,
    uint32_t verticesCount,
    size_t verticesSize,
    size_t verticesOffset);

LITE3D_CEXPORT int lite3d_mesh_chunk_init(struct lite3d_mesh_chunk *meshChunk, size_t layoutCount);
LITE3D_CEXPORT void lite3d_mesh_chunk_purge(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_bind(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_draw(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT void lite3d_mesh_chunk_draw_instanced(struct lite3d_mesh_chunk *meshChunk, uint32_t instancesCount);
LITE3D_CEXPORT void lite3d_mesh_chunk_unbind(struct lite3d_mesh_chunk *meshChunk);
LITE3D_CEXPORT lite3d_mesh_chunk *lite3d_mesh_chunk_get_by_material_index(struct lite3d_mesh *mesh, 
    uint32_t materialIndex);

// Отрисовать накопленный буфер команд одним вызовом! 
LITE3D_CEXPORT void lite3d_mesh_queue_draw(struct lite3d_mesh *mesh, uint8_t hasIndexes);
//
//  Добавление чанка в буфер команд для последующего рисования 
//  ВНИМАНИЕ! layout всех чанков в одном mesh (VBO) должен быть одинаковый так как они будут рисоваться одним вызовом, 
//  это касается и индексов, они должны быть у всех чанков или все чанки не должны содержать индексы.
//
LITE3D_CEXPORT void lite3d_mesh_queue_chunk(struct lite3d_mesh_chunk *meshChunk, uint32_t instancesCount);
// Очистка буфера команд для рисования 
LITE3D_CEXPORT void lite3d_mesh_queue_clean(struct lite3d_mesh *mesh);

#endif	/* LITE3D_MESH_H */

