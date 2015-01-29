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

/*
    note:
        function load mesh model with specified *name* from *file*,
        if *name* is NULL then first model node from *file* will be load.
*/
LITE3D_CEXPORT int lite3d_vbo_load(lite3d_vbo *vbo, lite3d_resource_file *resource, 
    const char *name);
LITE3D_CEXPORT int lite3d_vbo_load(lite3d_vbo *vbo, const char *file, 
    const char *name);

#endif	/* LITE3D_VBO_LOADER_H */

