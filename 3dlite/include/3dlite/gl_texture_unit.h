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
#ifndef LITE3D_GL_TEXTURE_UNIT_H
#define	LITE3D_GL_TEXTURE_UNIT_H

#include <3dlite/common.h>
#include <3dlite/file_cache.h>

#include "GL/glew.h"

// Image types (IL enum compatible)
#define LITE3D_IMAGE_ANY          0x0000
#define LITE3D_IMAGE_BMP          0x0420  //Microsoft Windows Bitmap - .bmp extension
#define LITE3D_IMAGE_JPG          0x0425  //JPEG - .jpg, .jpe and .jpeg extensions
#define LITE3D_IMAGE_PNG          0x042A  //Portable Network Graphics - .png extension
#define LITE3D_IMAGE_TGA          0x042D  //TrueVision Targa File - .tga, .vda, .icb and .vst extensions
#define LITE3D_IMAGE_TIF          0x042E  //Tagged Image File Format - .tif and .tiff extensions
#define LITE3D_IMAGE_GIF          0x0436  //Graphics Interchange Format - .gif extension
#define LITE3D_IMAGE_DDS          0x0437  //DirectDraw Surface - .dds extension
#define LITE3D_IMAGE_PSD          0x0439  //Adobe PhotoShop - .psd extension
#define LITE3D_IMAGE_HDR          0x043F  //Radiance High Dynamic Range - .hdr extension

#define LITE3D_TEXTURE_1D         0x0DE0
#define LITE3D_TEXTURE_2D         0x0DE1
#define LITE3D_TEXTURE_3D         0x806F

typedef struct lite3d_texture_technique_settings
{
    uint8_t anisotropic;
    int32_t maxAnisotropic;
    uint8_t useGLCompression;
} lite3d_texture_technique_settings;

typedef struct lite3d_texture_unit
{
    uint32_t textureID;
    int16_t textureTarget;
    char textureName[LITE3D_MAX_NAME];
    int16_t imageType;
    int16_t imageGLFormat;
    int16_t imageHeight;
    int16_t imageWidth;
    int16_t imageDepth;
    size_t imageSize;
    int8_t imageBPP;
    int8_t numMipmaps;
    int16_t minFilter;
    int16_t magFilter;
} lite3d_texture_unit;

LITE3D_CEXPORT int lite3d_texture_technique_init(const lite3d_texture_technique_settings *settings);
LITE3D_CEXPORT lite3d_texture_unit *lite3d_texture_unit_from_resource(const lite3d_resource_file *resource, 
    uint16_t imageType, uint32_t textureTarget);
LITE3D_CEXPORT lite3d_texture_unit *lite3d_texture_unit_from_memory(const char *textureName, 
    const void *buffer, size_t size, uint16_t imageType, uint32_t textureTarget);

#endif