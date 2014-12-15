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

#include <3dlite/3dlite_file_cache.h>
#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_7zloader.h>

static lite3d_resource_file *lookup_resource_index(lite3d_resource_pack *pack, const char *key)
{
    lite3d_rb_node *index = lite3d_rb_tree_exact_query(pack->fileCache, key);
    lite3d_resource_file *resource = NULL;
    if(index)
    {
        /* OK, found */
        resource = MEMBERCAST(lite3d_resource_file, index, cached);
    }
    
    return resource;
}

static lite3d_resource_file *create_resource_index(lite3d_resource_pack *pack, const char *key)
{
    lite3d_resource_file *resource = (lite3d_resource_file *)
        lite3d_malloc_pooled(LITE3D_POOL_NO1, sizeof(lite3d_resource_file));
    
    SDL_assert_release(resource);
    
    strcpy(resource->name, key);
    resource->cached.key = resource->name;
    resource->packer = pack;
    
    /* put index into cache tree */
    lite3d_rb_tree_insert(pack->fileCache, &resource->cached);
    /* init priority queue link */
    lite3d_list_link_init(&resource->priority);
    return resource;
}

static void resource_index_delete(lite3d_rb_node *x)
{
    lite3d_resource_file *resource = 
        MEMBERCAST(lite3d_resource_file, x, cached);

    lite3d_purge_resource_file(resource);
    lite3d_list_unlink_link(&resource->priority);
    
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, 
        "resource_index_delete: '%s' unindexed",    
        resource->name);
    /* for fast resources alloc/free operations use NO1 memory pool */
    lite3d_free_pooled(LITE3D_POOL_NO1, resource);
}

static void resource_purge_iter(lite3d_rb_tree* tree, lite3d_rb_node *x)
{
    lite3d_resource_file *resource = 
        MEMBERCAST(lite3d_resource_file, x, cached);
    
    lite3d_purge_resource_file(resource);
}

static void pack_7z_iterator(lite3d_7z_pack *pack,
    const char *path, int32_t index, void *userdata)
{
    lite3d_resource_pack *rpack = (lite3d_resource_pack *)userdata;
    lite3d_resource_file *resource = create_resource_index(rpack, path);
    
    if(resource)
        resource->dbIndex = index;
}

static int check_pack_memory_limit(lite3d_resource_pack *pack, size_t size)
{
    if (size > 0x6400000)
    {
        return 0;
    }

memValidate:
    /* validate memory limit */
    if ((pack->memoryUsed + size) > pack->memoryLimit)
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "lite3d_load_resource_file: memory limit is reached "
            "(%d bytes vs %d bytes limit) cleanup old data..",
            (int)(pack->memoryUsed + size), (int)pack->memoryLimit);
        lite3d_cleanup_out_of_use(pack);
        goto memValidate;
    }

    return 1;
}

lite3d_resource_pack *lite3d_open_pack(const char *path, uint8_t compressed, 
    size_t memoryLimit)
{
    lite3d_resource_pack *pack = NULL;
    lite3d_7z_pack *pack7z = NULL;

    /* compressed packs case */
    if(compressed)
    {
        if((pack7z = lite3d_7z_pack_open(path)) == NULL)
            return NULL;
    }

    pack = (lite3d_resource_pack *)lite3d_malloc(sizeof(lite3d_resource_pack));
    SDL_assert_release(pack);
    
    pack->isCompressed = compressed;
    pack->memoryLimit = memoryLimit;
    pack->memoryUsed = 0;
    pack->fileCache = lite3d_rb_tree_create(lite3d_rb_tree_c_string_comparator,
        resource_index_delete);
    
    lite3d_list_init(&pack->priorityList);
    strncpy(pack->pathto, path, sizeof(pack->pathto)-1);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
        "lite3d_open_pack: '%s' pack opened (%s) limit %d bytes",
        path, compressed ? "compressed" : "filesystem", (int)memoryLimit);
    
    /* begin indexing 7z pack */
    if(compressed)
    {
        pack->internal7z = pack7z;
        lite3d_7z_pack_iterate(pack7z, pack_7z_iterator, pack);
    }

    return pack;
}

void lite3d_close_pack(lite3d_resource_pack *pack)
{
    SDL_assert(pack);
    
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
        "lite3d_close_pack: begin to closing pack '%s' (%s)",
        pack->pathto, pack->isCompressed ? "compressed" : "filesystem");
    
    /* empty list */
    lite3d_list_init(&pack->priorityList);
    /* release all resources and release resource index */
    lite3d_rb_tree_destroy(pack->fileCache);

    /* release compressed pack logic */
    if(pack->isCompressed)
    {
        lite3d_7z_pack *pack7z = (lite3d_7z_pack *)pack->internal7z;
        lite3d_7z_pack_close(pack7z);
    }
    
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, 
        "lite3d_close_pack: '%s' pack (%s) closed ", pack->pathto,
        pack->isCompressed ? "compressed" : "filesystem");
    lite3d_free(pack);
}

lite3d_resource_file *lite3d_load_resource_file(lite3d_resource_pack *pack, const char *file)
{
    void *fileBuffer = NULL;
    size_t fileSize = 0, chunks = 0;
    char *pIt;
    lite3d_resource_file *resource;

    SDL_assert(pack);
    SDL_assert(file);
    
    if(strlen(file) >= LITE3D_MAX_FILE_NAME)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
            "create_resource_index: '%s' file name too long..", file);
        return NULL;
    }
    
    resource = lookup_resource_index(pack, file);
    
    if(resource && resource->isLoaded)
    {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, 
            "lite3d_load_resource_file: '%s' loaded from index (size: %d bytes)",
            file, (int)resource->fileSize);
            
        /* move resource to the head of priority queue */
        lite3d_list_unlink_link(&resource->priority);
        lite3d_list_add_first_link(&resource->priority, &pack->priorityList);
        return resource;
    }


    if(!pack->isCompressed)
    {
        char fullPath[LITE3D_MAX_FILE_PATH];
        SDL_RWops *desc;

        if((strlen(file) + strlen(pack->pathto)) >= LITE3D_MAX_FILE_PATH)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "lite3d_load_resource_file: path too long..");
            return NULL;
        }

        strcpy(fullPath, pack->pathto);
        strcat(fullPath, file);

        /* check open file */
        desc = SDL_RWFromFile(fullPath, "rb");
        if(!desc)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
                "lite3d_load_resource_file: file '%s': %s",
                fullPath, SDL_GetError());
            return NULL;
        }

        /* get file size */
        fileSize = SDL_RWsize(desc);
        if(!check_pack_memory_limit(pack, fileSize))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "lite3d_load_resource_file: file %s too big: %d bytes (limit 100M)",
                fullPath, (int) fileSize);
            SDL_RWclose(desc);
            return NULL;
        }
        
        fileBuffer = lite3d_malloc(fileSize);
        SDL_assert_release(fileBuffer);
        /* begin to read file into the memory */
        
        pIt = (char *)fileBuffer;
        while(SDL_RWread(desc, pIt, 1024, 1) > 0)
        {
            chunks++;
            pIt += 1024;
        }
        
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, 
            "lite3d_load_resource_file: '%s' loaded (size: %d bytes, chunks %d)",
            file, (int)fileSize, (int)chunks);
        SDL_RWclose(desc);
    }
    else
    {
        lite3d_7z_pack *pack7z;
        if(!resource)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, 
               "lite3d_load_resource_file: file %s not found..",
               file);
            return NULL;
        }
        
        pack7z = (lite3d_7z_pack *)pack->internal7z;
        fileSize = lite3d_7z_pack_file_size(pack7z, resource->dbIndex);
        if(!check_pack_memory_limit(pack, fileSize))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "lite3d_load_resource_file: file %s too big: %d bytes (limit 100M)",
                file, (int) fileSize);
            return NULL;
        }

        fileBuffer = lite3d_7z_pack_file_extract(pack7z, 
            resource->dbIndex, &fileSize);
        
        if(fileBuffer == NULL || fileSize == 0)
        {
            return NULL;
        }
        
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, 
            "lite3d_load_resource_file: '%s' uncompressed (size: %d bytes)",
            file, (int)fileSize);
    }
    
    if(!resource)
    {
        resource = create_resource_index(pack, file);
    }
    
    resource->fileBuff = fileBuffer;
    resource->fileSize = fileSize;
    resource->isLoaded = 1;
    /* move resource to the head of priority queue */
    lite3d_list_unlink_link(&resource->priority);
    lite3d_list_add_first_link(&resource->priority, &pack->priorityList);
    
    pack->memoryUsed += fileSize;
    return resource;
}

void lite3d_purge_resource_file(lite3d_resource_file *resource)
{
    SDL_assert(resource);
    if(resource->isLoaded)
    {
        lite3d_free(resource->fileBuff);
        resource->fileBuff = NULL;     
        resource->packer->memoryUsed -= resource->fileSize;
        resource->fileSize = 0;
        lite3d_list_unlink_link(&resource->priority);
        
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, 
            "lite3d_purge_resource_file: '%s' unloaded",    
            resource->name);
    }
    
    resource->isLoaded = 0;
}

void lite3d_purge_resources(lite3d_resource_pack *pack)
{
    lite3d_rb_tree_iterate(pack->fileCache, resource_purge_iter);
}

void lite3d_cleanup_out_of_use(lite3d_resource_pack *pack)
{
    lite3d_list_node *last;
    lite3d_resource_file *resource;
    SDL_assert(pack);

    if(lite3d_list_is_empty(&pack->priorityList))
        return;
    
    last = lite3d_list_last_link(&pack->priorityList);
    resource = MEMBERCAST(lite3d_resource_file, last, priority);
    
    lite3d_purge_resource_file(resource);
}