/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2024  Sirius (Korolev Nikita)
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
#ifndef LITE3D_ALLOC_H
#define	LITE3D_ALLOC_H

#include <lite3d/lite3d_common.h>

#ifdef LITE3D_OFFSETOF
#undef LITE3D_OFFSETOF
#endif

#ifdef LITE3D_MEMBERCAST
#undef LITE3D_MEMBERCAST
#endif

#define LITE3D_OFFSETOF(tstruct, field)             ((size_t) &((tstruct*)0x0)->field)
#define LITE3D_MEMBERCAST(tstruct, pfield, field)   ((tstruct*)((size_t)pfield - \
    LITE3D_OFFSETOF(tstruct, field)))
#define LITE3D_ALIGN_SIZE(size, align)              (size + (size % align))

#define LITE3D_BUFFER_OFFSET(i)                     ((char *)NULL + (i)) 

#define LITE3D_MEMMODEL_MALLOC      0x01
#define LITE3D_MEMMODEL_NEDPOOL     0x02

#define LITE3D_POOL_MAX             0x06
#define LITE3D_POOL_COMMON          0x00
/* More little chunks */
#define LITE3D_POOL_NO1             0x01
/* C++ objects pool */
#define LITE3D_POOL_NO2             0x02
/* Mesh chunks only */
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
LITE3D_CEXPORT void *lite3d_calloc_pooled(uint8_t pollNo, size_t size);
LITE3D_CEXPORT void lite3d_free_pooled(uint8_t pollNo, void *p);

LITE3D_CEXPORT void lite3d_memory_init(lite3d_alloca_f *allocator);
LITE3D_CEXPORT void lite3d_memory_cleanup(void);

LITE3D_CEXPORT char *lite3d_strdup(const char *str);

#endif	/* ALLOC_H */

