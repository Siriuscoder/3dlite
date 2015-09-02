/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
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
#include <stdlib.h>
#include <string.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_nedmalloc.h>

lite3d_alloca_f gAlloca_f = {NULL, NULL};
nedpool *globalMemPools[LITE3D_POOL_MAX] = {NULL, NULL, NULL, NULL, NULL, NULL};

void lite3d_memory_init(lite3d_alloca_f *allocator)
{
    int i;
    
    if (allocator && allocator->mallocf != NULL && allocator->freef != NULL)
    {
        gAlloca_f = *allocator;
    }
    else if(gAlloca_f.mallocf == NULL && gAlloca_f.freef == NULL)
    {
        gAlloca_f.mallocf = malloc;
        gAlloca_f.freef = free;
    }

    for (i = 0; i < LITE3D_POOL_MAX; ++i)
    {
        if (!globalMemPools[i])
            globalMemPools[i] = nedcreatepool(0x100000, 1);
    }
}

void lite3d_memory_cleanup(void)
{
    int i;
    for (i = 0; i < LITE3D_POOL_MAX; ++i)
    {
        if (globalMemPools[i])
        {
            neddestroypool(globalMemPools[i]);
            globalMemPools[i] = NULL;
        }
    }
}

void *lite3d_malloc(size_t size)
{
    if (gAlloca_f.mallocf)
        return gAlloca_f.mallocf(size);

    return NULL;
}

void *lite3d_calloc(size_t size)
{
    void *mem = lite3d_malloc(size);
    if (mem)
        memset(mem, 0, size);

    return mem;
}

void lite3d_free(void *p)
{
    if (gAlloca_f.freef)
        gAlloca_f.freef(p);
}

void *lite3d_malloc_pooled(uint8_t pollNo, size_t size)
{
    SDL_assert_release(pollNo < LITE3D_POOL_MAX);
    return nedpmalloc(globalMemPools[pollNo], size);
}

void *lite3d_calloc_pooled(uint8_t pollNo, size_t size)
{
    void *mem = lite3d_malloc_pooled(pollNo, size);
    if (mem)
        memset(mem, 0, size);

    return mem;
}

void lite3d_free_pooled(uint8_t pollNo, void *p)
{
    SDL_assert_release(pollNo < LITE3D_POOL_MAX);
    nedpfree(globalMemPools[pollNo], p);
}
