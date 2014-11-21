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
#include <string.h>

#include <SDL_assert.h>
#include <SDL_log.h>
#include <SDL_rwops.h>

#include <3dlite/filepack.h>
#include <3dlite/alloc.h>

static lite3d_resource_file *lookup_or_create_resource(lite3d_resource_pack *pack, const char *key)
{

}

static void purge_resource(lite3d_resource_file *resource)
{
    if(resource->isLoaded)
    {
        lite3d_free(resource->fileBuff);
        resource->fileBuff = NULL;
        resource->fileSize = 0;
    }

    resource->isLoaded = 0;
}

static void resource_index_delete(lite3d_rb_node *x)
{
    lite3d_resource_file *resource = 
        MEMBERCAST(lite3d_resource_file, x, cached);

    purge_resource(resource);
    /* for fast resources alloc/free operations use NO1 memory pool */
    lite3d_free_pooled(LITE3D_POOL_NO1, resource);
}

lite3d_resource_pack *lite3d_open_pack(const char *path, uint8_t compressed, 
    size_t memoryLimit)
{
    lite3d_resource_pack *pack = NULL;

    /* compressed packs case */
    if(compressed)
    {

    }

    pack = (lite3d_resource_pack *)lite3d_malloc(sizeof(lite3d_resource_pack));
    SDL_assert_release(pack);
    
    pack->isCompressed = compressed;
    pack->memoryLimit = memoryLimit;
    pack->fileCache = lite3d_rb_tree_create(lite3d_rb_tree_c_string_comparator,
        resource_index_delete);
    
    lite3d_list_init(&pack->priorityList);
    strncpy(pack->pathto, path, sizeof(pack->pathto)-1);

    return pack;
}

void lite3d_close_pack(lite3d_resource_pack *pack)
{
    SDL_assert_release(pack);
    /* empty list */
    lite3d_list_init(&pack->priorityList);
    /* release all resources and release resource index */
    lite3d_rb_tree_destroy(pack->fileCache);

    /* release compressed pack logic */
    if(pack->isCompressed)
    {

    }

    lite3d_free(pack);
}

lite3d_resource_file *lite3d_load_resource_file(lite3d_resource_pack *pack, const char *file)
{
    SDL_assert_release(pack);
    char *fileBuffer;
    size_t fileSize;

    if(!pack->isCompressed)
    {
        char fullPath[LITE3D_MAX_FILE_PATH];

        if((strlen(file) + strlen(pack->pathto)) >= LITE3D_MAX_FILE_PATH)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "lite3d_load_resource_file: path too long..");
            return NULL;
        }

        strcpy(fullPath, pack->pathto);
        strcat(fullPath, file);

        /* check open file */
        SDL_RWops *wops = SDL_RWFromFile(fullPath, "rb");
        if(!wops)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "lite3d_load_resource_file: file %s: %s",
                fullPath, SDL_GetError());
            return NULL;
        }

        /* begin to read file in memory */
        fileSize = SDL_RWsize(wops);
    }

    lite3d_resource_file *resource = lookup_or_create_resource(pack, file);
}