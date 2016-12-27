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

LITE3D_CEXPORT int lite3d_texture_buffer_allocate(lite3d_texture_unit *textureUnit, 
    uint32_t size);

LITE3D_CEXPORT int lite3d_texture_buffer_purge(lite3d_texture_unit *textureUnit);

#endif	/* LITE3D_TBO_H */

