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

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_pack.h>

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

#define LITE3D_MAX_FILTERS          10

#define LITE3D_TEXTURE_QL_NICEST    0x0001
#define LITE3D_TEXTURE_QL_LOW       0x0002
#define LITE3D_TEXTURE_QL_MEDIUM    0x0003

#define LITE3D_TEXTURE_CLAMP_TO_EDGE    0x0001
#define LITE3D_TEXTURE_REPEAT           0x0002

#define LITE3D_TEXTURE_FORMAT_ALPHA             0x1906
#define LITE3D_TEXTURE_FORMAT_RGB               0x1907
#define LITE3D_TEXTURE_FORMAT_RGBA              0x1908            
#define LITE3D_TEXTURE_FORMAT_BRG               0x80E0            
#define LITE3D_TEXTURE_FORMAT_BRGA              0x80E1
#define LITE3D_TEXTURE_FORMAT_LUMINANCE         0x1909
#define LITE3D_TEXTURE_FORMAT_LUMINANCE_ALPHA   0x190A
#define LITE3D_TEXTURE_FORMAT_DEPTH             0x1902
            
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
    int8_t imageBPP;
    int8_t loadedMipmaps;
    int8_t generatedMipmaps;
    int16_t minFilter;
    int16_t magFilter;
    uint8_t wrapping;
    uint8_t isFbAttachment;
    void *userdata;
} lite3d_texture_unit;

LITE3D_CEXPORT void lite3d_texture_technique_add_image_filter(lite3d_image_filter *filter);
LITE3D_CEXPORT void lite3d_texture_technique_reset_filters(void);

LITE3D_CEXPORT int lite3d_texture_technique_init(const lite3d_texture_technique_settings *settings);
LITE3D_CEXPORT void lite3d_texture_technique_shut(void);

/* load texture from resource file using Devil */
LITE3D_CEXPORT int lite3d_texture_unit_from_resource(lite3d_texture_unit *textureUnit, 
    const lite3d_file *resource, uint32_t imageType, uint32_t textureTarget, 
    int8_t quality, uint8_t wrapping);

/* allocate empty texture object */
LITE3D_CEXPORT int lite3d_texture_unit_allocate(lite3d_texture_unit *textureUnit, 
    uint32_t textureTarget, int8_t quality, uint8_t wrapping, uint16_t format,
    int32_t height, int32_t width, int32_t depth);

/* update specified mipmap surface */
LITE3D_CEXPORT int lite3d_texture_unit_set_pixels(lite3d_texture_unit *textureUnit, 
    uint32_t level, int32_t width, int32_t height, int32_t depth,
    int32_t widthOff, int32_t heightOff, int32_t depthOff, void *pixels);

LITE3D_CEXPORT void lite3d_texture_unit_purge(lite3d_texture_unit *texture);
LITE3D_CEXPORT void lite3d_texture_unit_bind(lite3d_texture_unit *texture, uint16_t layer);
LITE3D_CEXPORT void lite3d_texture_unit_unbind(lite3d_texture_unit *texture, uint16_t layer);

LITE3D_CEXPORT void lite3d_texture_unit_compression(uint8_t on);

#endif