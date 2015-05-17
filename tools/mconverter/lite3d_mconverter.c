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
#include <stdio.h>

#include <SDL_assert.h>
#include <SDL_log.h>
#include <SDL_rwops.h>
#include <3dlite/3dlite_main.h>
#include <3dlite/3dlite_m_codec.h>

#include "3dlite/7zdec/Types.h"

#define DEFAULT_WIDTH           800
#define DEFAULT_HEIGHT          600

static char inputFilePath[1024];
static char outputFilePath[1024];
static int optimize = LITE3D_FALSE;
static int flipUV = LITE3D_FALSE;

static int saveBuffer(void *buffer, size_t size)
{
    SDL_RWops *output;
    output = SDL_RWFromFile(outputFilePath, "wb");
    if (!output)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: '%s': %s",
            __FUNCTION__, outputFilePath, SDL_GetError());
        return LITE3D_FALSE;
    }

    if (SDL_RWwrite(output, buffer, size, 1) != 1)
    {
        SDL_RWclose(output);
        return LITE3D_FALSE;
    }

    SDL_RWclose(output);
    return LITE3D_TRUE;
}

static int loadMesh(void *userdata)
{
    lite3d_resource_pack *pack;
    lite3d_resource_file *meshFile;
    lite3d_indexed_mesh model;
    void *encodeBuffer = NULL;
    size_t encodeBufferSize;
    uint32_t loadFlags = 0;
    if (!(pack = lite3d_resource_pack_open("./", LITE3D_FALSE, 0)))
        return LITE3D_FALSE;
    if (!(meshFile = lite3d_resource_pack_file_load(pack, inputFilePath)))
        return LITE3D_FALSE;

    if (!lite3d_indexed_mesh_init(&model))
        goto exit2;


    if (optimize)
        loadFlags |= LITE3D_OPTIMIZE_MESH_FLAG;
    if (flipUV)
        loadFlags |= LITE3D_FLIP_UV_FLAG;

    if (!lite3d_indexed_mesh_load(&model, meshFile, NULL, LITE3D_VBO_STATIC_DRAW, loadFlags))
        goto exit2;

    encodeBufferSize = lite3d_indexed_mesh_m_encode_size(&model);
    encodeBuffer = lite3d_malloc(encodeBufferSize);
    if (!lite3d_indexed_mesh_m_encode(&model, encodeBuffer, encodeBufferSize))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: %s encode failed..",
            __FUNCTION__, inputFilePath);
        goto exit1;
    }

    if (!saveBuffer(encodeBuffer, encodeBufferSize))
        goto exit1;

exit1:
    lite3d_free(encodeBuffer);
    lite3d_indexed_mesh_purge(&model);
exit2:
    lite3d_resource_pack_close(pack);

    return LITE3D_FALSE;
}

static void printHelpAndExit()
{
    printf("Lite3d conversion utility.\n");
    printf("Conversion from formats supported by Assimp to internal lite3d format (m).\n\n");
    printf("Engine version %s\n\n", LITE3D_VERSION_STRING);

    printf("Usage: -i[input] path -o[output] path -O[optimize mesh] -F[flip UVs]\n\n");
    exit(1);
}

int main(int argc, char *args[])
{
    int i = 0;
    lite3d_global_settings settings;
    memset(&settings, 0, sizeof (settings));

    settings.logLevel = LITE3D_LOGLEVEL_ERROR;
    settings.maxFileCacheSize = 0x700000;
    settings.textureSettings.anisotropy = 8;
    settings.textureSettings.useGLCompression = LITE3D_TRUE;
    settings.videoSettings.FSAA = 1;
    strcpy(settings.videoSettings.caption, "TEST window");
    settings.videoSettings.colorBits = 32;
    settings.videoSettings.fullscreen = LITE3D_FALSE;
    settings.videoSettings.screenWidth = DEFAULT_WIDTH;
    settings.videoSettings.screenHeight = DEFAULT_HEIGHT;
    settings.videoSettings.vsync = LITE3D_TRUE;
    settings.videoSettings.hidden = LITE3D_TRUE;

    settings.renderLisneters.preRender = loadMesh;

    if (argc < 3)
        printHelpAndExit();

    for (i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-i") == 0)
        {
            if ((i + 1) < argc)
                strcpy(inputFilePath, args[i + 1]);
            else
                printHelpAndExit();
        }
        else if (strcmp(args[i], "-o") == 0)
        {
            if ((i + 1) < argc)
                strcpy(outputFilePath, args[i + 1]);
            else
                printHelpAndExit();
        }
        else if (strcmp(args[i], "-O") == 0)
        {
            optimize = LITE3D_TRUE;
        }
        else if (strcmp(args[i], "-F") == 0)
        {
            flipUV = LITE3D_TRUE;
        }
    }

    return !lite3d_main(&settings);
}