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
#include <3dlite/3dlite_material.h>

typedef struct lite3d_vbo
{
    uint32_t vboVerticesID;
    uint32_t vboIndexesID;
    size_t verticesSize;
    size_t indexesSize;
    size_t verticesCount;
    size_t elementsCount;
    size_t vaosCount;
    lite3d_list vaos;
    void (*bindOn)(struct lite3d_vbo *vbo);
    void (*bindOff)(struct lite3d_vbo *vbo);
} lite3d_vbo;

typedef struct lite3d_vao
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

    /* material index */
    uint32_t materialIndex;
    void (*batchBegin)(struct lite3d_vbo *vbo, struct lite3d_vao *vao);
    void (*batchEnd)(struct lite3d_vbo *vbo, struct lite3d_vao *vao);
} lite3d_vao;

LITE3D_CEXPORT int lite3d_vbo_technique_init(void);
LITE3D_CEXPORT int lite3d_vbo_init(struct lite3d_vbo *vbo);
LITE3D_CEXPORT void lite3d_vbo_purge(struct lite3d_vbo *vbo);
LITE3D_CEXPORT int lite3d_vbo_extend(struct lite3d_vbo *vbo, 
    size_t verticesSize, size_t indexesSize, uint16_t access);
LITE3D_CEXPORT void lite3d_vbo_draw(struct lite3d_vbo *vbo);

LITE3D_CEXPORT int lite3d_vao_init(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_purge(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_draw(struct lite3d_vao *vao);
LITE3D_CEXPORT lite3d_vao *lite3d_vao_get_by_index(struct lite3d_vbo *vbo, 
    uint32_t materialIndex);

#endif	/* LITE3D_VBO_H */

