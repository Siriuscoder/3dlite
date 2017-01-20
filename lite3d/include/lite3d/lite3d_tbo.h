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
#ifndef LITE3D_TBO_H
#define	LITE3D_TBO_H

#include <lite3d/lite3d_texture_unit.h>

#define LITE3D_TB_R8        0x8229
#define LITE3D_TB_R16       0x822A
#define LITE3D_TB_R16F      0x822D
#define LITE3D_TB_R32F      0x822E
#define LITE3D_TB_R8I       0x8231
#define LITE3D_TB_R16I 	    0x8233
#define LITE3D_TB_R32I      0x8235
#define LITE3D_TB_R8UI      0x8232
#define LITE3D_TB_R16UI     0x8234
#define LITE3D_TB_R32UI     0x8236
#define LITE3D_TB_RG8       0x822B
#define LITE3D_TB_RG16      0x822C
#define LITE3D_TB_RG16F     0x822F
#define LITE3D_TB_RG32F     0x8230
#define LITE3D_TB_RG8I      0x8237
#define LITE3D_TB_RG16I     0x8239
#define LITE3D_TB_RG32I     0x823B
#define LITE3D_TB_RG8UI     0x8238
#define LITE3D_TB_RG16UI    0x823A
#define LITE3D_TB_RG32UI    0x823C
#define LITE3D_TB_RGB32F    0x8815
#define LITE3D_TB_RGB32I    0x8D83
#define LITE3D_TB_RGB32UI   0x8D71
#define LITE3D_TB_RGBA8     0x8058
#define LITE3D_TB_RGBA16    0x805B
#define LITE3D_TB_RGBA16F   0x881A
#define LITE3D_TB_RGBA32F   0x8814
#define LITE3D_TB_RGBA8I    0x8D8E
#define LITE3D_TB_RGBA16I   0x8D88
#define LITE3D_TB_RGBA32I   0x8D82
#define LITE3D_TB_RGBA8UI   0x8D7C
#define LITE3D_TB_RGBA16UI  0x8D76
#define LITE3D_TB_RGBA32UI  0x8D70


LITE3D_CEXPORT int lite3d_texture_buffer_allocate(lite3d_texture_unit *textureUnit, 
    uint32_t texelsCount, const void *data, uint16_t bf, uint16_t access);

LITE3D_CEXPORT int lite3d_texture_buffer_purge(lite3d_texture_unit *textureUnit);

LITE3D_CEXPORT int8_t lite3d_texture_buffer_texel_size(uint16_t bf);

LITE3D_CEXPORT void *lite3d_texture_buffer_map(lite3d_texture_unit *textureUnit, uint16_t access);

LITE3D_CEXPORT void lite3d_texture_buffer_unmap(lite3d_texture_unit *textureUnit);

LITE3D_CEXPORT int lite3d_texture_buffer(lite3d_texture_unit *textureUnit,
    const void *buffer, size_t offset, size_t size);

LITE3D_CEXPORT int lite3d_texture_buffer_get(lite3d_texture_unit *textureUnit,
    void *buffer, size_t offset, size_t size);

LITE3D_CEXPORT int lite3d_texture_buffer_extend(lite3d_texture_unit *textureUnit, 
    size_t addSize, uint16_t access);


#endif	/* LITE3D_TBO_H */

