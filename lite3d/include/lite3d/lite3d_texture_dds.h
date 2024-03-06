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
#ifndef LITE3D_TEXTURE_DDS_H
#define	LITE3D_TEXTURE_DDS_H

int lite3d_texture_unit_dds_fast_load(struct lite3d_texture_unit *textureUnit, const struct lite3d_file *resource, 
    uint32_t textureTarget, int8_t srgb, int8_t filtering, uint8_t wrapping, uint8_t cubeface);

#endif	/* LITE3D_TEXTURE_DDS_H */

