/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2025 Sirius (Korolev Nikita)
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
#ifndef LITE3D_VBO_H
#define	LITE3D_VBO_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_list.h>

#define LITE3D_VBO_STREAM_DRAW 0x0
#define LITE3D_VBO_STREAM_READ 0x1
#define LITE3D_VBO_STREAM_COPY 0x2
#define LITE3D_VBO_STATIC_DRAW 0x3
#define LITE3D_VBO_STATIC_READ 0x4
#define LITE3D_VBO_STATIC_COPY 0x5
#define LITE3D_VBO_DYNAMIC_DRAW 0x6
#define LITE3D_VBO_DYNAMIC_READ 0x7
#define LITE3D_VBO_DYNAMIC_COPY 0x8

#define LITE3D_VBO_MAP_READ_ONLY 0x0
#define LITE3D_VBO_MAP_WRITE_ONLY 0x1
#define LITE3D_VBO_MAP_READ_WRITE 0x2

typedef struct lite3d_vbo
{
    uint32_t vboID;
    size_t size;
    uint16_t usage;
    uint16_t role;
    void *userdata;
} lite3d_vbo;

LITE3D_CEXPORT int lite3d_vbo_technique_init(void);
LITE3D_CEXPORT void lite3d_vbo_get_limitations(int *UBOMaxSize, int *TBOMaxSize, int *SSBOMaxSize);

LITE3D_CEXPORT void lite3d_vbo_bind(const struct lite3d_vbo *vbo);
LITE3D_CEXPORT void lite3d_vbo_unbind(const struct lite3d_vbo *vbo);
/* use this to init vertex buffer object */
LITE3D_CEXPORT int lite3d_vbo_init(struct lite3d_vbo *vbo, 
    uint16_t usage);
/* use this to init index buffer object */
LITE3D_CEXPORT int lite3d_ibo_init(struct lite3d_vbo *vbo, 
    uint16_t usage);
/* use this to init shader storage buffer object */
LITE3D_CEXPORT int lite3d_ssbo_init(struct lite3d_vbo *vbo, 
    uint16_t usage);
/* use this to init uniform buffer object */
LITE3D_CEXPORT int lite3d_ubo_init(struct lite3d_vbo *vbo, 
    uint16_t usage);
/* use this to init indirect buffer object */
LITE3D_CEXPORT int lite3d_vbo_indirect_init(struct lite3d_vbo *vbo, 
    uint16_t usage);

LITE3D_CEXPORT void lite3d_vbo_purge(struct lite3d_vbo *vbo);
LITE3D_CEXPORT void *lite3d_vbo_map(struct lite3d_vbo *vbo,
    uint16_t access);
LITE3D_CEXPORT void lite3d_vbo_unmap(struct lite3d_vbo *vbo);
LITE3D_CEXPORT int lite3d_vbo_extend(struct lite3d_vbo *vbo, 
    size_t addSize);
LITE3D_CEXPORT int lite3d_vbo_buffer_alloc(struct lite3d_vbo *vbo, 
    const void *buffer, size_t size);
LITE3D_CEXPORT int lite3d_vbo_subbuffer(struct lite3d_vbo *vbo, 
    const void *buffer, size_t offset, size_t size);
LITE3D_CEXPORT int lite3d_vbo_get_buffer(const struct lite3d_vbo *vbo, 
    void *buffer, size_t offset, size_t size);
LITE3D_CEXPORT int lite3d_vbo_subbuffer_extend(struct lite3d_vbo *vbo, 
    const void *buffer, size_t offset, size_t size);
LITE3D_CEXPORT int lite3d_vbo_buffer_set(struct lite3d_vbo *vbo, 
    const void *buffer, size_t size);


#endif	/* LITE3D_VBO_H */

