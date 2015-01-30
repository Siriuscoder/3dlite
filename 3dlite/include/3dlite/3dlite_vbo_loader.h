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
#ifndef LITE3D_VBO_LOADER_H
#define	LITE3D_VBO_LOADER_H

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_vbo.h>
#include <3dlite/3dlite_resource_pack.h>

typedef struct lite3d_buffer_component
{
    uint8_t binding;
    uint8_t type;
    uint8_t count;
    uint8_t size;
} lite3d_buffer_component;
/*
    note:
        function load mesh model with specified *name* from *file*,
        if *name* is NULL then first model node from *file* will be load.
*/
LITE3D_CEXPORT int lite3d_vbo_load(lite3d_vbo *vbo, lite3d_resource_file *resource, 
    const char *name);


LITE3D_CEXPORT int lite3d_vbo_load_from_memory(lite3d_vbo *vbo, 
    void *vertices, 
    size_t verticesCount, 
    lite3d_buffer_component *layout,
    size_t layoutCount,
    void *indexes, 
    size_t indexesCount, 
    uint8_t indexComponents, 
    uint8_t componentSize);

LITE3D_CEXPORT int lite3d_vbo_extend_from_memory(lite3d_vbo *vbo, 
    void *vertices, 
    size_t verticesCount, 
    lite3d_buffer_component *layout,
    size_t layoutCount,
    void *indexes, 
    size_t indexesCount, 
    uint8_t indexComponents, 
    uint8_t componentSize);


#endif	/* LITE3D_VBO_LOADER_H */

