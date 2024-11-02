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
#include <stdlib.h>
#include <string.h>
#include <SDL_log.h>
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
    void *newmem = NULL;
    if (gAlloca_f.mallocf)
    {
        if ((newmem = gAlloca_f.mallocf(size)) == NULL)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
                "%s: out of memory, failed to allocate %lu bytes", LITE3D_CURRENT_FUNCTION, size);
        }
    }

    return newmem;
}

void *lite3d_calloc(size_t size)
{
    void *mem = NULL;
    if ((mem = lite3d_malloc(size)) == NULL)
    {
        return NULL;
    }

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
    void *newmem = NULL;
    SDL_assert(pollNo < LITE3D_POOL_MAX);

    if ((newmem = nedpmalloc(globalMemPools[pollNo], size)) == NULL)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "%s: out of memory at pool %d, failed to allocate %lu bytes", LITE3D_CURRENT_FUNCTION, pollNo, size);
    }

    return newmem;
}

void *lite3d_calloc_pooled(uint8_t pollNo, size_t size)
{
    void *newmem = NULL;
    if ((newmem = lite3d_malloc_pooled(pollNo, size)) == NULL)
    {
        return NULL;
    }

    memset(newmem, 0, size);
    return newmem;
}

void lite3d_free_pooled(uint8_t pollNo, void *p)
{
    SDL_assert(pollNo < LITE3D_POOL_MAX);
    nedpfree(globalMemPools[pollNo], p);
}

char *lite3d_strdup(const char *str)
{
    void *strmem = lite3d_malloc(strlen(str) + 1);
    if (!strmem)
    {
        return NULL;
    }

    strcpy(strmem, str);
    return strmem;
}
