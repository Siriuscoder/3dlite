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
#ifndef LITE3D_7ZLOADER_H
#define	LITE3D_7ZLOADER_H

#include <3dlite/common.h>

#include <3dlite/7zdec/7z.h>
#include <3dlite/7zdec/7zCrc.h>
#include <3dlite/7zdec/7zFile.h>
#include <3dlite/7zdec/7zVersion.h>


typedef struct lite3d_7z_pack
{
    CFileInStream archiveStream;
    CLookToRead lookStream;
    CSzArEx db;
    ISzAlloc allocImp;
    
    /*
      if you need cache, use these 3 variables.
      if you use external function, you can make these variable as static.
    */
    UInt32 blockIndex; /* it can have any value before first call (if outBuffer = 0) */
    Byte *outBuffer; /* it must be 0 before first call for each new archive. */
    size_t outBufferSize;  /* it can have any value before first call (if outBuffer = 0) */
} lite3d_7z_pack;

typedef void (*lite3d_7z_iterator)(lite3d_7z_pack *pack,
    const char *path, int32_t index, void *userdata);

LITE3D_CEXPORT lite3d_7z_pack *lite3d_7z_pack_open(const char *path);
LITE3D_CEXPORT void lite3d_7z_pack_close(lite3d_7z_pack *pack);
LITE3D_CEXPORT void lite3d_7z_pack_iterate(lite3d_7z_pack *pack, lite3d_7z_iterator iter, void *userdata);
LITE3D_CEXPORT void *lite3d_7z_pack_file_extract(lite3d_7z_pack *pack, int index, size_t *outSize);
LITE3D_CEXPORT int32_t lite3d_7z_pack_file_size(lite3d_7z_pack *pack, int index);

#endif