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
#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_7z_loader.h>
#include <lite3d/lite3d_alloc.h>

static uint8_t gCRC_gen = 0;
static Byte kUtf8Limits[5] = { 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };

static void *alloc7z(void *p, size_t size)
{
    return lite3d_malloc(size);
}

static void free7z(void *p, void *addr)
{
    lite3d_free(addr);
}

static int utf16_to_utf8(Byte *dest, size_t *destLen, const UInt16 *src)
{
    size_t destPos = 0, srcPos = 0, srcLen = 0;
    for (; src[srcLen] != '\0'; srcLen++); srcLen++;
    
    for (;;)
    {
        unsigned numAdds;
        UInt32 value;
        if (srcPos == srcLen)
        {
            *destLen = destPos;
            return 1;
        }
        value = src[srcPos++];
        if (value < 0x80)
        {
            if (dest)
                dest[destPos] = (char) value;
            destPos++;
            continue;
        }
        if (value >= 0xD800 && value < 0xE000)
        {
            UInt32 c2;
            if (value >= 0xDC00 || srcPos == srcLen)
                break;
            c2 = src[srcPos++];
            if (c2 < 0xDC00 || c2 >= 0xE000)
                break;
            value = (((value - 0xD800) << 10) | (c2 - 0xDC00)) + 0x10000;
        }
        for (numAdds = 1; numAdds < 5; numAdds++)
            if (value < (((UInt32) 1) << (numAdds * 5 + 6)))
                break;
        if (dest)
            dest[destPos] = (char) (kUtf8Limits[numAdds - 1] + (value >> (6 * numAdds)));
        destPos++;
        do
        {
            numAdds--;
            if (dest)
                dest[destPos] = (char) (0x80 + ((value >> (6 * numAdds)) & 0x3F));
            destPos++;
        }
        while (numAdds != 0);
    }
    *destLen = destPos;
    return 0;
}

lite3d_7z_pack *lite3d_7z_pack_open(const char *path)
{
    lite3d_7z_pack *pack;
    CSzFile zFile;

    SDL_assert(path);
    
    if(!gCRC_gen)
    {
        CrcGenerateTable();
        gCRC_gen = 1;
    }
    
    if (InFile_Open(&zFile, path))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: '%s' open failed..", LITE3D_CURRENT_FUNCTION, path);
        return NULL;
    }
    
    pack = (lite3d_7z_pack *)lite3d_malloc_pooled(LITE3D_POOL_NO1, sizeof(lite3d_7z_pack));
    SDL_assert_release(pack);
    
    pack->allocImp.Alloc = alloc7z;
    pack->allocImp.Free = free7z;
    pack->archiveStream.file = zFile;

    FileInStream_CreateVTable(&pack->archiveStream);
    LookToRead_CreateVTable(&pack->lookStream, False);
  
    pack->lookStream.realStream = &pack->archiveStream.s;
    LookToRead_Init(&pack->lookStream);
    
    SzArEx_Init(&pack->db);
    if(SzArEx_Open(&pack->db, &pack->lookStream.s, &pack->allocImp, &pack->allocImp) != SZ_OK)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: '%s' extract index failed, bad archive..", LITE3D_CURRENT_FUNCTION, path);
        
        SzArEx_Free(&pack->db, &pack->allocImp);
        File_Close(&pack->archiveStream.file);
        
        lite3d_free(pack);
        return NULL;
    }
    
    pack->blockIndex = 0xffffffff;
    pack->outBuffer = NULL;
    pack->outBufferSize = 0;
    
    return pack;
}

void lite3d_7z_pack_close(lite3d_7z_pack *pack)
{
    SDL_assert(pack);
    
    /* release unpack buffer */
    if(pack->outBufferSize > 0)
    {
        IAlloc_Free(&pack->allocImp, pack->outBuffer);
    }
    
    SzArEx_Free(&pack->db, &pack->allocImp);
    File_Close(&pack->archiveStream.file);
    
    lite3d_free_pooled(LITE3D_POOL_NO1, pack);
}

void lite3d_7z_pack_iterate(lite3d_7z_pack *pack, lite3d_7z_iterator iter, void *userdata)
{
    UInt32 index;
    const CSzFileItem *f;
    size_t nameLen;
    size_t destPos;
    Byte utf8name[LITE3D_MAX_FILE_NAME];
    UInt16 utf16name[LITE3D_MAX_FILE_NAME];

    SDL_assert(pack);
    
    for(index = 0; index < pack->db.db.NumFiles; ++index)
    {
        f = pack->db.db.Files + index;
        if(f->IsDir)
            continue;
        
        nameLen = SzArEx_GetFileNameUtf16(&pack->db, index, NULL);
        if(nameLen >= LITE3D_MAX_FILE_NAME)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: skip index %d, name too long..", LITE3D_CURRENT_FUNCTION,
                (int)index);
            continue;
        }
        
        SzArEx_GetFileNameUtf16(&pack->db, index, utf16name);
        utf16name[nameLen] = 0;
        /* encode name */
        if(!utf16_to_utf8(utf8name, &destPos, utf16name))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s: to utf8 name decode failed..");
            continue;
        }
            
        utf8name[destPos+1] = 0;
        
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION,
            "%s: '%s' index %d", LITE3D_CURRENT_FUNCTION,
            utf8name, (int)index);
        iter(pack, (char *)utf8name, index, userdata);
    }
}

void *lite3d_7z_pack_file_extract(lite3d_7z_pack *pack, uint32_t index, size_t *outSize)
{
    void *fileMem;
    size_t offset = 0;
    size_t outSizeProcessed = 0;
    SDL_assert(pack);

    if(SzArEx_Extract(&pack->db, &pack->lookStream.s, index,
        &pack->blockIndex, &pack->outBuffer, &pack->outBufferSize,
        &offset, &outSizeProcessed,
        &pack->allocImp, &pack->allocImp) != SZ_OK)
    {
        *outSize = 0;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: index %d extract failed..", LITE3D_CURRENT_FUNCTION, (int)index);
        return NULL;
    }
    
    fileMem = lite3d_malloc(outSizeProcessed);
    memcpy(fileMem, pack->outBuffer + offset, outSizeProcessed);
    *outSize = outSizeProcessed;
    return fileMem;
}

size_t lite3d_7z_pack_file_size(lite3d_7z_pack *pack, uint32_t index)
{
    const CSzFileItem *f;
    SDL_assert(pack);
    
    if(index >= pack->db.db.NumFiles)
        return 0;
    
    f = pack->db.db.Files + index;
    
    if(f->IsDir)
        return 0;
    
    return f->Size;
}