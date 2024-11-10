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
#ifndef LITE3D_TBO_H
#define	LITE3D_TBO_H

#include <lite3d/lite3d_texture_unit.h>

// use LITE3D_TEXTURE_INTERNAL in bf
LITE3D_CEXPORT int lite3d_texture_buffer_init(lite3d_texture_unit *textureUnit, 
    uint32_t texelsCount, const void *data, uint16_t bf, uint16_t usage);

LITE3D_CEXPORT int lite3d_texture_buffer_purge(lite3d_texture_unit *textureUnit);

// use LITE3D_TEXTURE_INTERNAL in bf
LITE3D_CEXPORT int8_t lite3d_texture_buffer_texel_size(uint16_t bf);

LITE3D_CEXPORT void *lite3d_texture_buffer_map(lite3d_texture_unit *textureUnit, uint16_t access);

LITE3D_CEXPORT void lite3d_texture_buffer_unmap(lite3d_texture_unit *textureUnit);

LITE3D_CEXPORT int lite3d_texture_buffer(lite3d_texture_unit *textureUnit,
    const void *buffer, size_t offset, size_t size);

LITE3D_CEXPORT int lite3d_texture_buffer_get(const lite3d_texture_unit *textureUnit,
    void *buffer, size_t offset, size_t size);

LITE3D_CEXPORT int lite3d_texture_buffer_extend(lite3d_texture_unit *textureUnit, 
    size_t addSize);


#endif	/* LITE3D_TBO_H */

