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
#ifndef LITE3D_FILEPACK_H
#define	LITE3D_FILEPACK_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_rb_tree.h>
#include <lite3d/lite3d_list.h>

typedef struct lite3d_pack
{
    lite3d_rb_tree *fileCache;
    lite3d_list priorityList;
    uint8_t isCompressed;
    size_t memoryLimit;
    size_t memoryUsed;
    char pathto[LITE3D_MAX_FILE_PATH];
    void *internal7z;
} lite3d_pack;

typedef struct lite3d_file
{
    /* rb tree node entity */
    lite3d_rb_node cached;
    /* node of priority */
    lite3d_list_node priority;

    lite3d_pack *packer;
    char name[LITE3D_MAX_FILE_NAME];
    void *fileBuff;
    size_t fileSize;
    uint8_t isLoaded;
    /* for 7z */
    int32_t dbIndex;
} lite3d_file;

LITE3D_CEXPORT lite3d_pack *lite3d_pack_open(const char *path, uint8_t compressed, size_t memoryLimit);
LITE3D_CEXPORT void lite3d_pack_close(lite3d_pack *pack);

LITE3D_CEXPORT lite3d_file *lite3d_pack_file_load(lite3d_pack *pack, const char *file);
LITE3D_CEXPORT lite3d_file *lite3d_pack_file_find(lite3d_pack *pack, const char *file);
LITE3D_CEXPORT void lite3d_pack_file_purge(lite3d_file *resource);
LITE3D_CEXPORT void lite3d_pack_purge(lite3d_pack *pack);
LITE3D_CEXPORT void lite3d_pack_purge_unused(lite3d_pack *pack);

#endif	/* LITE3D_FILEPACK_H */

