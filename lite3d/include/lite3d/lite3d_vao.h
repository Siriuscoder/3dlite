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
#ifndef LITE3D_VAO_H
#define	LITE3D_VAO_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_list.h>

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

LITE3D_CEXPORT int lite3d_vao_technique_init(void);
LITE3D_CEXPORT int lite3d_vao_support_instancing(void);

LITE3D_CEXPORT int lite3d_vao_init(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_purge(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_bind(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_draw_indexed(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_draw_indexed_instanced(struct lite3d_vao *vao, size_t count);
LITE3D_CEXPORT void lite3d_vao_draw(struct lite3d_vao *vao);
LITE3D_CEXPORT void lite3d_vao_draw_instanced(struct lite3d_vao *vao, size_t count);
LITE3D_CEXPORT void lite3d_vao_unbind(struct lite3d_vao *vao);

#endif	/* LITE3D_VBO_H */

