/******************************************************************************
*	This file is part of 3dlite (Light-weight 3d engine).
*	Copyright (C) 2014  Sirius (Korolev Nikita)
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

#ifndef ALLOC_H
#define	ALLOC_H

#include <3dlite/common.h>

#define LITE3D_MEMMODEL_MALLOC      0x01
#define LITE3D_MEMMODEL_NEDPOOL     0x02

typedef void *(*lite3d_mallocf)(size_t size);
typedef void (*lite3d_freef)(void *);
typedef struct __lite3d_alloca_f
{
    lite3d_mallocf mallocf;
    lite3d_freef freef;
} lite3d_alloca_f;


LITE3D_EXTERN void lite3d_set_allocator(lite3d_alloca_f *alloca);
LITE3D_EXTERN void *lite3d_malloc(size_t size);
LITE3D_EXTERN void lite3d_free(void *p);

void lite3d_init_memory(int32_t flags);
void lite3d_cleanup_memory();

#endif	/* ALLOC_H */

