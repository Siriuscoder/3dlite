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
#ifndef LITE3D_M_CODEC_H
#define	LITE3D_M_CODEC_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_mesh.h>
#include <lite3d/lite3d_pack.h>

/*  
    .m files containes optimised and prepared meshes in simplest format 
    so that it can be loaded into VBO directly to make process of loading 
    models more faster.

    .m file format:

    Header:
    ---------------------------------------
    SIG | VERS | SIZE | CHUNKS |
    ---------------------------------------

    Chunk no 1:
    ---------------------------------------
    SH | SV | SI | L | options |
    ---------------------------------------
    
    Chunk no 2:
    ...

    Vertex section:
    -----------------------------------
    BINARY
    -----------------------------------

    Index section:
    -----------------------------------
    BINARY
    -----------------------------------

*/

LITE3D_CEXPORT int lite3d_mesh_m_decode(lite3d_mesh *mesh, 
    const void *buffer, size_t size, uint16_t access);

LITE3D_CEXPORT size_t lite3d_mesh_m_encode_size(lite3d_mesh *mesh);
LITE3D_CEXPORT int lite3d_mesh_m_encode(lite3d_mesh *mesh, 
    void *buffer, size_t size);

#endif	/* LITE3D_M_CODEC_H */

