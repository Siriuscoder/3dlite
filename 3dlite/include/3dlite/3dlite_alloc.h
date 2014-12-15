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
#ifndef LITE3D_ALLOC_H
#define	LITE3D_ALLOC_H

#include <3dlite/3dlite_common.h>

#ifdef OFFSETOF
#undef OFFSETOF
#endif

#ifdef MEMBERCAST
#undef MEMBERCAST
#endif

#define OFFSETOF(tstruct, field)            ((size_t) &((tstruct*)0x0)->field)
#define MEMBERCAST(tstruct, pfield, field)  ((tstruct*)((size_t)pfield - \
    OFFSETOF(tstruct, field)))

#define LITE3D_MEMMODEL_MALLOC      0x01
#define LITE3D_MEMMODEL_NEDPOOL     0x02

#define LITE3D_POOL_MAX             0x06
#define LITE3D_POOL_COMMON          0x00
#define LITE3D_POOL_NO1             0x01
#define LITE3D_POOL_NO2             0x02
#define LITE3D_POOL_NO3             0x03
#define LITE3D_POOL_NO4             0x04
#define LITE3D_POOL_NO5             0x05

typedef void *(*lite3d_mallocf)(size_t size);
typedef void (*lite3d_freef)(void *);
typedef struct lite3d_alloca_f
{
    lite3d_mallocf mallocf;
    lite3d_freef freef;
} lite3d_alloca_f;


LITE3D_CEXPORT void lite3d_set_allocator(lite3d_alloca_f *alloca);
LITE3D_CEXPORT void *lite3d_malloc(size_t size);
LITE3D_CEXPORT void *lite3d_calloc(size_t size);
LITE3D_CEXPORT void lite3d_free(void *p);

LITE3D_CEXPORT void *lite3d_malloc_pooled(uint8_t pollNo, size_t size);
LITE3D_CEXPORT void lite3d_free_pooled(uint8_t pollNo, void *p);

LITE3D_CEXPORT void lite3d_init_memory(lite3d_alloca_f *allocator);
LITE3D_CEXPORT void lite3d_cleanup_memory(void);

#endif	/* ALLOC_H */

