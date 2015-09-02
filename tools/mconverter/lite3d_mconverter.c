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
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <SDL_assert.h>
#include <SDL_log.h>
#include <SDL_rwops.h>
#include <lite3d/lite3d_main.h>
#include <lite3d/lite3d_m_codec.h>

#define DEFAULT_WIDTH           800
#define DEFAULT_HEIGHT          600

static char inputFilePath[1024] = {0};
static char outputFolder[1024] = {0};
static int optimize = LITE3D_FALSE;
static int flipUV = LITE3D_FALSE;
static int nonameCounter = 0;
static lite3d_indexed_mesh model;

static int save_buffer(void *buffer, size_t size, const char *path)
{
    SDL_RWops *output;
    output = SDL_RWFromFile(path, "wb");
    if (!output)
        return LITE3D_FALSE;

    if (SDL_RWwrite(output, buffer, size, 1) != 1)
    {
        SDL_RWclose(output);
        return LITE3D_FALSE;
    }

    SDL_RWclose(output);
    return LITE3D_TRUE;
}

static lite3d_indexed_mesh *mesh_init(void)
{
    if (!lite3d_indexed_mesh_init(&model))
        return NULL;

    return &model;
}

static void mesh_loaded(lite3d_indexed_mesh *mesh, const char *name)
{
    void *encodeBuffer = NULL;
    size_t encodeBufferSize;
    char encodedFile[1024] = {0};

    if (name == NULL || name[0] == 0)
    {
        sprintf(encodedFile, "%s%snoname%d.m", outputFolder, outputFolder[0] == 0 ? "" : "/",
            ++nonameCounter);
    }
    else
    {
        sprintf(encodedFile, "%s%s%s.m", outputFolder, outputFolder[0] == 0 ? "" : "/",
            name);
    }

    printf("Encoding %s ... ", encodedFile); 
    fflush(stdout);
    
    encodeBufferSize = lite3d_indexed_mesh_m_encode_size(mesh);
    encodeBuffer = lite3d_malloc(encodeBufferSize);
    if (!lite3d_indexed_mesh_m_encode(mesh, encodeBuffer, encodeBufferSize))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: encode failed..",
            __FUNCTION__);
    }
    else
    {
        if (save_buffer(encodeBuffer, encodeBufferSize, encodedFile))
        {
            printf("done\n");
            fflush(stdout);
        }
    }

    lite3d_free(encodeBuffer);
    lite3d_indexed_mesh_purge(mesh);
}

static int convert_mesh(void *userdata)
{
    lite3d_pack *pack;
    lite3d_file *meshFile;
    uint32_t loadFlags = 0;

    if (!(pack = lite3d_pack_open("./", LITE3D_FALSE, 0)))
        return LITE3D_FALSE;
    if (!(meshFile = lite3d_pack_file_load(pack, inputFilePath)))
        return LITE3D_FALSE;


    if (optimize)
        loadFlags |= LITE3D_OPTIMIZE_MESH_FLAG;
    if (flipUV)
        loadFlags |= LITE3D_FLIP_UV_FLAG;

    lite3d_indexed_mesh_load_recursive(meshFile, mesh_init, mesh_loaded,
        LITE3D_VBO_STATIC_READ, loadFlags);
    lite3d_pack_close(pack);

    return LITE3D_FALSE;
}

static void print_help_and_exit()
{
    printf("Lite3d conversion utility.\n");
    printf("Conversion from formats supported by Assimp to internal lite3d format (m).\n");
    printf("Engine version %s\n\n", LITE3D_VERSION_STRING);

    printf("Usage: -i[input] file -o[output] folder -O[optimize mesh] -F[flip UVs]\n\n");
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

    settings.renderLisneters.preRender = convert_mesh;

    if (argc < 3)
        print_help_and_exit();

    for (i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-i") == 0)
        {
            if ((i + 1) < argc)
                strncpy(inputFilePath, args[i + 1], sizeof (inputFilePath) - 1);
            else
                print_help_and_exit();
        }
        else if (strcmp(args[i], "-o") == 0)
        {
            if ((i + 1) < argc)
                strncpy(outputFolder, args[i + 1], sizeof (outputFolder) - 1);
            else
                print_help_and_exit();
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

    if (inputFilePath[0] == 0)
        print_help_and_exit();

    return !lite3d_main(&settings);
}