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
#ifndef LITE3D_VAO_H
#define	LITE3D_VAO_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_list.h>
#include <lite3d/lite3d_vbo.h>

/* legacy types, now not used  */
#define LITE3D_BUFFER_BINDING_VERTEX          0x0
#define LITE3D_BUFFER_BINDING_COLOR           0x1
#define LITE3D_BUFFER_BINDING_NORMAL          0x2
#define LITE3D_BUFFER_BINDING_TEXCOORD        0x3
/* may be more then one, attribute index will be increased */
#define LITE3D_BUFFER_BINDING_ATTRIBUTE       0x4
#define LITE3D_BUFFER_BINDING_TANGENT         0x5
#define LITE3D_BUFFER_BINDING_BINORMAL        0x6

typedef struct lite3d_vao
{
    uint32_t vaoID;
    uint32_t elementsCount;
    uint32_t indexesCount;
    size_t indexesSize;
    size_t indexesOffset;
    uint32_t verticesCount;
    size_t verticesSize;
    size_t verticesOffset;
} lite3d_vao;

typedef struct lite3d_vao_layout
{
    uint8_t binding;
    uint8_t count; /* count elements in component */
} lite3d_vao_layout;

typedef struct lite3d_multidraw_indexed_command
{
    uint32_t count;
    uint32_t instanceCount;
    uint32_t firstIndex;
    int32_t  baseVertex;
    uint32_t baseInstance;
} lite3d_multidraw_indexed_command;

typedef struct lite3d_multidraw_command
{
    uint32_t count;
    uint32_t instanceCount;
    uint32_t first;
    uint32_t baseInstance;
} lite3d_multidraw_command;

LITE3D_CEXPORT int lite3d_vao_technique_init(void);
LITE3D_CEXPORT int lite3d_vao_support_instancing(void);

LITE3D_CEXPORT int lite3d_vao_init(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_purge(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_bind(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_draw_indexed(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_multidraw_indexed(struct lite3d_vao *vao, size_t offset, size_t count);
LITE3D_CEXPORT void lite3d_vao_draw_indexed_instanced(struct lite3d_vao *vao, uint32_t count);
LITE3D_CEXPORT void lite3d_vao_draw(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_multidraw(struct lite3d_vao *vao, size_t offset, size_t count);
LITE3D_CEXPORT void lite3d_vao_draw_instanced(struct lite3d_vao *vao, uint32_t count);
LITE3D_CEXPORT void lite3d_vao_unbind(struct lite3d_vao *vao);

LITE3D_CEXPORT int lite3d_vao_init_layout(struct lite3d_vbo *vertexBuffer,
    struct lite3d_vbo *indexBuffer,
    struct lite3d_vbo *auxBuffer,
    struct lite3d_vao *vao, 
    const struct lite3d_vao_layout *layout,
    uint32_t layoutCount,
    uint32_t stride,
    uint32_t indexesCount,
    size_t indexesSize,
    size_t indexesOffset,
    uint32_t verticesCount,
    size_t verticesSize,
    size_t verticesOffset);

#endif	/* LITE3D_VBO_H */

