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
#ifndef LITE3D_VBO_H
#define	LITE3D_VBO_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_list.h>

#define LITE3D_VBO_STREAM_DRAW 0x88E0
#define LITE3D_VBO_STREAM_READ 0x88E1
#define LITE3D_VBO_STREAM_COPY 0x88E2
#define LITE3D_VBO_STATIC_DRAW 0x88E4
#define LITE3D_VBO_STATIC_READ 0x88E5
#define LITE3D_VBO_STATIC_COPY 0x88E6
#define LITE3D_VBO_DYNAMIC_DRAW 0x88E8
#define LITE3D_VBO_DYNAMIC_READ 0x88E9
#define LITE3D_VBO_DYNAMIC_COPY 0x88EA

#define LITE3D_VBO_MAP_READ_ONLY 0x88B8
#define LITE3D_VBO_MAP_WRITE_ONLY 0x88B9
#define LITE3D_VBO_MAP_READ_WRITE 0x88BA

typedef struct lite3d_vbo
{
    uint32_t vboID;
    size_t size;
} lite3d_vbo;

typedef struct lite3d_vao
{
    uint32_t vaoID;
    uint32_t elementsCount;
    uint32_t indexesCount;
    uint32_t indexesSize;
    uint32_t indexesOffset;
    uint32_t verticesCount;
    uint32_t verticesSize;
    uint32_t verticesOffset;
    uint16_t elementType; /* GL_POINTS, GL_LINES, GL_TRIANGLES (GL value) */
    uint16_t indexType; /* Byte, short, int (GL value) */
} lite3d_vao;

LITE3D_CEXPORT int lite3d_vbo_technique_init(void);
LITE3D_CEXPORT int lite3d_vbo_support_instancing(void);

LITE3D_CEXPORT int lite3d_vbo_init(struct lite3d_vbo *vbo);
LITE3D_CEXPORT void lite3d_vbo_purge(struct lite3d_vbo *vbo);
LITE3D_CEXPORT void *lite3d_vbo_map(struct lite3d_vbo *vbo,
    uint16_t access);
LITE3D_CEXPORT void lite3d_vbo_unmap(struct lite3d_vbo *vbo);
LITE3D_CEXPORT int lite3d_vbo_extend(struct lite3d_vbo *vbo, 
    size_t addSize, uint16_t access);
LITE3D_CEXPORT int lite3d_vbo_buffer(struct lite3d_vbo *vbo, 
    const void *buffer, size_t size, uint16_t access);
LITE3D_CEXPORT int lite3d_vbo_subbuffer(struct lite3d_vbo *vbo, 
    const void *buffer, size_t offset, size_t size);

LITE3D_CEXPORT int lite3d_vao_init(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_purge(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_bind(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_draw_indexed(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_draw_indexed_instanced(struct lite3d_vao *vao, size_t count);
LITE3D_CEXPORT void lite3d_vao_draw(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_draw_instanced(struct lite3d_vao *vao, size_t count);
LITE3D_CEXPORT void lite3d_vao_unbind(struct lite3d_vao *vao);

#endif	/* LITE3D_VBO_H */

