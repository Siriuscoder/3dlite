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
#include <stdlib.h>
#include <assert.h>

#include <3dlite/alloc.h>
#include <3dlite/nedmalloc.h>

lite3d_alloca_f gAlloca_f = 
    { NULL, NULL };
nedpool *globalMemPools[LITE3D_POOL_MAX] = 
    { NULL, NULL, NULL, NULL, NULL, NULL };

static void *nedpool_malloc_global(size_t size)
{
    return lite3d_malloc_pooled(LITE3D_POOL_COMMON, size);
}

static void nedpool_free_global(void *p)
{
    lite3d_free_pooled(LITE3D_POOL_COMMON, p);
}

void lite3d_init_memory(int32_t flags)
{
    if(gAlloca_f.mallocf == NULL && gAlloca_f.freef == NULL)
    {
        if(flags & LITE3D_MEMMODEL_NEDPOOL)
        {
            int i;
            globalMemPools[0] = nedcreatepool(0x500000, 1);

            for(i = 1; i < LITE3D_POOL_MAX; ++i)
                globalMemPools[i] = nedcreatepool(0, 1);
            
            gAlloca_f.mallocf = nedpool_malloc_global;
            gAlloca_f.freef = nedpool_free_global;
        }
        else if(flags & LITE3D_MEMMODEL_MALLOC)
        {
            gAlloca_f.mallocf = malloc;
            gAlloca_f.freef = free;                
        }
    }
}

void lite3d_cleanup_memory(void)
{
    int i;
    for(i = 0; i < LITE3D_POOL_MAX; ++i)
    {
        if(globalMemPools[i])
           neddestroypool(globalMemPools[i]); 
    }
}

void *lite3d_malloc(size_t size)
{
    if(gAlloca_f.mallocf)
        return gAlloca_f.mallocf(size);
    
    return NULL;
}

void lite3d_free(void *p)
{
    if(gAlloca_f.freef)
        gAlloca_f.freef(p);
}

void *lite3d_malloc_pooled(uint8_t pollNo, size_t size)
{
    if(pollNo < LITE3D_POOL_MAX)
        return nedpmalloc(globalMemPools[pollNo], size);
    
    return NULL;
}

void lite3d_free_pooled(uint8_t pollNo, void *p)
{
    if(pollNo < LITE3D_POOL_MAX)
        nedpfree(globalMemPools[pollNo], p);
}
