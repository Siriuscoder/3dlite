/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2014  Sirius (Korolev Nikita)
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
#ifndef LITE3D_GL_TEXTURE_UNIT_H
#define	LITE3D_GL_TEXTURE_UNIT_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_pack.h>
#include <lite3d/lite3d_vbo.h>

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
#define LITE3D_TEXTURE_CUBE       0x8513
#define LITE3D_TEXTURE_BUFFER     0x8C2A
/* pre-loading texture manipulation */
#define LITE3D_ALIENIFY_FILTER      0x0001
#define LITE3D_BLURAVG_FILTER       0x0002
#define LITE3D_BLURGAUSSIAN_FILTER  0x0003
#define LITE3D_CONTRAST_FILTER      0x0004
#define LITE3D_GAMMACORRECT_FILTER  0x0005
#define LITE3D_MIRROR_FILTER        0x0006
#define LITE3D_NEGATIVE_FILTER      0x0007
#define LITE3D_NOISIFY_FILTER       0x0008
#define LITE3D_PIXELIZE_FILTER      0x0009
#define LITE3D_WAVE_FILTER          0x000A
#define LITE3D_SHARPEN_FILTER       0x000B
#define LITE3D_FLIP_FILTER          0x000C

#define LITE3D_MAX_FILTERS          10

#define LITE3D_TEXTURE_QL_NICEST    0x0001
#define LITE3D_TEXTURE_QL_LOW       0x0002
#define LITE3D_TEXTURE_QL_MEDIUM    0x0003

#define LITE3D_TEXTURE_CLAMP_TO_EDGE    0x0001
#define LITE3D_TEXTURE_REPEAT           0x0002

#define LITE3D_TEXTURE_FORMAT_ALPHA             0x1906
#define LITE3D_TEXTURE_FORMAT_RED               0x1903
#define LITE3D_TEXTURE_FORMAT_RG                0x8227
#define LITE3D_TEXTURE_FORMAT_RGB               0x1907
#define LITE3D_TEXTURE_FORMAT_RGBA              0x1908
#define LITE3D_TEXTURE_FORMAT_BRG               0x80E0
#define LITE3D_TEXTURE_FORMAT_BRGA              0x80E1
#define LITE3D_TEXTURE_FORMAT_LUMINANCE         0x1909
#define LITE3D_TEXTURE_FORMAT_LUMINANCE_ALPHA   0x190A
#define LITE3D_TEXTURE_FORMAT_DEPTH             0x1902

#define LITE3D_TEXTURE_INTERNAL_R8              0x8229
#define LITE3D_TEXTURE_INTERNAL_R8_SNORM        0x8F94
#define LITE3D_TEXTURE_INTERNAL_R16             0x822A
#define LITE3D_TEXTURE_INTERNAL_R16_SNORM       0x8F98
#define LITE3D_TEXTURE_INTERNAL_RG8             0x822B
#define LITE3D_TEXTURE_INTERNAL_RG8_SNORM       0x8F95
#define LITE3D_TEXTURE_INTERNAL_RG16            0x822C
#define LITE3D_TEXTURE_INTERNAL_RG16_SNORM      0x8F99
#define LITE3D_TEXTURE_INTERNAL_R3_G3_B2        0x2A10
#define LITE3D_TEXTURE_INTERNAL_RGB4            0x804F
#define LITE3D_TEXTURE_INTERNAL_RGB5            0x8050
#define LITE3D_TEXTURE_INTERNAL_RGB8            0x8051
#define LITE3D_TEXTURE_INTERNAL_RGB8_SNORM      0x8F96
#define LITE3D_TEXTURE_INTERNAL_RGB10           0x8052
#define LITE3D_TEXTURE_INTERNAL_RGB12           0x8053
#define LITE3D_TEXTURE_INTERNAL_RGB16_SNORM     0x8F9A
#define LITE3D_TEXTURE_INTERNAL_RGBA2           0x8055
#define LITE3D_TEXTURE_INTERNAL_RGBA4           0x8056
#define LITE3D_TEXTURE_INTERNAL_RGB5_A1         0x8057
#define LITE3D_TEXTURE_INTERNAL_RGBA8           0x8058
#define LITE3D_TEXTURE_INTERNAL_RGBA8_SNORM     0x8F97
#define LITE3D_TEXTURE_INTERNAL_RGB10_A2        0x8059
#define LITE3D_TEXTURE_INTERNAL_RGB10_A2UI      0x906F
#define LITE3D_TEXTURE_INTERNAL_RGBA12          0x805A
#define LITE3D_TEXTURE_INTERNAL_RGBA16          0x805B
#define LITE3D_TEXTURE_INTERNAL_SRGB8           0x8C41
#define LITE3D_TEXTURE_INTERNAL_SRGB8_ALPHA8    0x8C43
#define LITE3D_TEXTURE_INTERNAL_R16F            0x822D
#define LITE3D_TEXTURE_INTERNAL_RG16F           0x822F
#define LITE3D_TEXTURE_INTERNAL_RGB16F          0x881B
#define LITE3D_TEXTURE_INTERNAL_RGBA16F         0x881A
#define LITE3D_TEXTURE_INTERNAL_R32F            0x822E
#define LITE3D_TEXTURE_INTERNAL_RG32F           0x8230
#define LITE3D_TEXTURE_INTERNAL_RGB32F          0x8815
#define LITE3D_TEXTURE_INTERNAL_RGBA32F         0x8814
#define LITE3D_TEXTURE_INTERNAL_R11F_G11F_B10F  0x8C3A
#define LITE3D_TEXTURE_INTERNAL_RGB9_E5         0x8C3D
#define LITE3D_TEXTURE_INTERNAL_R8I             0x8231
#define LITE3D_TEXTURE_INTERNAL_R8UI            0x8232
#define LITE3D_TEXTURE_INTERNAL_R16I            0x8233
#define LITE3D_TEXTURE_INTERNAL_R16UI           0x8234
#define LITE3D_TEXTURE_INTERNAL_R32I            0x8235
#define LITE3D_TEXTURE_INTERNAL_R32UI           0x8236
#define LITE3D_TEXTURE_INTERNAL_RG8I            0x8237
#define LITE3D_TEXTURE_INTERNAL_RG8UI           0x8238
#define LITE3D_TEXTURE_INTERNAL_RG16I           0x8239
#define LITE3D_TEXTURE_INTERNAL_RG16UI          0x823A
#define LITE3D_TEXTURE_INTERNAL_RG32I           0x823B
#define LITE3D_TEXTURE_INTERNAL_RG32UI          0x823C
#define LITE3D_TEXTURE_INTERNAL_RGB8I           0x8D8F
#define LITE3D_TEXTURE_INTERNAL_RGB8UI          0x8D7D
#define LITE3D_TEXTURE_INTERNAL_RGB16I          0x8D89
#define LITE3D_TEXTURE_INTERNAL_RGB16UI         0x8D77
#define LITE3D_TEXTURE_INTERNAL_RGB32I          0x8D83
#define LITE3D_TEXTURE_INTERNAL_RGB32UI         0x8D71
#define LITE3D_TEXTURE_INTERNAL_RGBA8I          0x8D8E
#define LITE3D_TEXTURE_INTERNAL_RGBA8UI         0x8D7C
#define LITE3D_TEXTURE_INTERNAL_RGBA16I         0x8D88
#define LITE3D_TEXTURE_INTERNAL_RGBA16UI        0x8D76
#define LITE3D_TEXTURE_INTERNAL_RGBA32I         0x8D82
#define LITE3D_TEXTURE_INTERNAL_RGBA32UI        0x8D70

            
typedef struct lite3d_image_filter
{
    uint32_t filterID;
    float param1, param2;
} lite3d_image_filter;

typedef struct lite3d_texture_technique_settings
{
    int32_t anisotropy;
    int32_t maxAnisotropy;
    uint8_t useGLCompression;
} lite3d_texture_technique_settings;

typedef struct lite3d_texture_unit
{
    uint32_t textureID;
    uint32_t textureTarget;
    int32_t texFormat;
    int32_t texiFormat;
    int32_t imageType;
    int32_t imageHeight;
    int32_t imageWidth;
    int32_t imageDepth;
    size_t imageSize;
    size_t totalSize;
    int8_t imageBPP;
    int8_t loadedMipmaps;
    int8_t generatedMipmaps;
    int16_t minFilter;
    int16_t magFilter;
    uint8_t wrapping;
    uint8_t compressed;
    uint8_t isFbAttachment;
    lite3d_vbo tbo;
    uint8_t isTextureBuffer;
    void *userdata;
} lite3d_texture_unit;

LITE3D_CEXPORT void lite3d_texture_technique_add_image_filter(lite3d_image_filter *filter);
LITE3D_CEXPORT void lite3d_texture_technique_reset_filters(void);

LITE3D_CEXPORT int lite3d_texture_technique_init(const lite3d_texture_technique_settings *settings);
LITE3D_CEXPORT void lite3d_texture_technique_shut(void);

/* texture mipmap level size */
LITE3D_CEXPORT int32_t lite3d_texture_unit_get_level_width(lite3d_texture_unit *textureUnit,
    int8_t level, uint8_t cubeface);
LITE3D_CEXPORT int32_t lite3d_texture_unit_get_level_height(lite3d_texture_unit *textureUnit,
    int8_t level, uint8_t cubeface);
LITE3D_CEXPORT int32_t lite3d_texture_unit_get_level_depth(lite3d_texture_unit *textureUnit,
    int8_t level, uint8_t cubeface);

/* load texture from resource file using Devil */
/* CUBEFACE description (takes in account for LITE3D_TEXTURE_CUBE textures only )
    GL_TEXTURE_CUBE_MAP_POSITIVE_X	Right   = 0
    GL_TEXTURE_CUBE_MAP_NEGATIVE_X	Left    = 1 
    GL_TEXTURE_CUBE_MAP_POSITIVE_Y	Top     = 2
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y	Bottom  = 3
    GL_TEXTURE_CUBE_MAP_POSITIVE_Z	Back    = 4
    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z	Front   = 5
*/
LITE3D_CEXPORT int lite3d_texture_unit_from_resource(lite3d_texture_unit *textureUnit, 
    const lite3d_file *resource, uint32_t imageType, uint32_t textureTarget, 
    int8_t quality, uint8_t wrapping, uint8_t cubeface);

/* allocate empty texture object */
/* set iformat = 0 to specify what internal format does not matter */
LITE3D_CEXPORT int lite3d_texture_unit_allocate(lite3d_texture_unit *textureUnit, 
    uint32_t textureTarget, int8_t quality, uint8_t wrapping, uint16_t format,
    uint16_t iformat, int32_t width, int32_t height, int32_t depth);

/* update specified mipmap level */
LITE3D_CEXPORT int lite3d_texture_unit_set_pixels(lite3d_texture_unit *textureUnit, 
    int32_t widthOff, int32_t heightOff, int32_t depthOff, 
    int32_t width, int32_t height, int32_t depth,
    int8_t level, uint8_t cubeface, const void *pixels);

/* update specified mipmap compressed level */
LITE3D_CEXPORT int lite3d_texture_unit_set_compressed_pixels(lite3d_texture_unit *textureUnit, 
    int32_t widthOff, int32_t heightOff, int32_t depthOff, 
    int32_t width, int32_t height, int32_t depth,
    int8_t level, uint8_t cubeface, size_t pixelsSize, const void *pixels);

/* get data from mipmap level */
LITE3D_CEXPORT int lite3d_texture_unit_get_pixels(lite3d_texture_unit *textureUnit, 
    int8_t level, uint8_t cubeface, void *pixels);
LITE3D_CEXPORT int lite3d_texture_unit_get_compressed_pixels(lite3d_texture_unit *textureUnit, 
    int8_t level, uint8_t cubeface, void *pixels);

/* size of mipmap levels */
LITE3D_CEXPORT int lite3d_texture_unit_get_level_size(lite3d_texture_unit *textureUnit, 
    int8_t level, uint8_t cubeface, size_t *size);
LITE3D_CEXPORT int lite3d_texture_unit_get_compressed_level_size(lite3d_texture_unit *textureUnit, 
    int8_t level, uint8_t cubeface, size_t *size);
/* regenerate mipmaps */
LITE3D_CEXPORT int lite3d_texture_unit_generate_mipmaps(lite3d_texture_unit *textureUnit);

LITE3D_CEXPORT void lite3d_texture_unit_purge(lite3d_texture_unit *texture);
LITE3D_CEXPORT void lite3d_texture_unit_bind(lite3d_texture_unit *texture, uint16_t layer);
LITE3D_CEXPORT void lite3d_texture_unit_unbind(lite3d_texture_unit *texture, uint16_t layer);

LITE3D_CEXPORT void lite3d_texture_unit_compression(uint8_t on);

#endif