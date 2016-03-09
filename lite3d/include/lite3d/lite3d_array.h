/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2014-2016  Sirius (Korolev Nikita)
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
#ifndef LITE3D_ARRAY_H
#define	LITE3D_ARRAY_H

#include <lite3d/lite3d_common.h>

typedef struct lite3d_array
{
    void *data;
    size_t size;
    size_t elemSize;
    size_t capacity;
} lite3d_array;

LITE3D_CEXPORT void lite3d_array_init(lite3d_array *a, size_t elemSize, size_t size);
LITE3D_CEXPORT void lite3d_array_clean(lite3d_array *a);
LITE3D_CEXPORT void lite3d_array_purge(lite3d_array *a);


#endif	/* LITE3D_ARRAY_H */

