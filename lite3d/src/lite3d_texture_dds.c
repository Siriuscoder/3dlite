/******************************************************************************
 *    This file is part of lite3d (Light-weight 3d engine).
 *    Copyright (C) 2024 Sirius (Korolev Nikita)
 *
 *    Lite3D is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    Lite3D is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with Lite3D.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#include <SDL_assert.h>
#include <SDL_log.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_texture_unit.h>
#include <lite3d/lite3d_texture_dds.h>

#define DDS_PF_DXT1 0x1
#define DDS_PF_DXT3 0x3
#define DDS_PF_DXT5 0x5
#define DDS_PF_RXGB 0x6 // Doom3 normal maps
#define DDS_PF_BC4  0x7 // 3DC/BC4
#define DDS_PF_BC5  0x8 // 3DC/BC5

#define DDS_CAPS                0x00000001L
#define DDS_HEIGHT              0x00000002L
#define DDS_WIDTH               0x00000004L

#define DDS_RGB                 0x00000040L
#define DDS_PIXELFORMAT         0x00001000L

#define DDS_LUMINANCE           0x00020000L

#define DDS_ALPHAPIXELS         0x00000001L
#define DDS_ALPHA               0x00000002L
#define DDS_FOURCC              0x00000004L
#define DDS_PITCH               0x00000008L
#define DDS_COMPLEX             0x00000008L
#define DDS_TEXTURE             0x00001000L
#define DDS_MIPMAPCOUNT         0x00020000L
#define DDS_LINEARSIZE          0x00080000L
#define DDS_VOLUME              0x00200000L
#define DDS_MIPMAP              0x00400000L
#define DDS_DEPTH               0x00800000L

#define DDS_CUBEMAP             0x00000200L
#define DDS_CUBEMAP_POSITIVEX   0x00000400L
#define DDS_CUBEMAP_NEGATIVEX   0x00000800L
#define DDS_CUBEMAP_POSITIVEY   0x00001000L
#define DDS_CUBEMAP_NEGATIVEY   0x00002000L
#define DDS_CUBEMAP_POSITIVEZ   0x00004000L
#define DDS_CUBEMAP_NEGATIVEZ   0x00008000L
#define DDS_CUBEMAP_SIDES       6

#define DDS_MAKEFOURCC(ch0, ch1, ch2, ch3)  \
    ((int32_t)(int8_t)(ch0) | ((int32_t)(int8_t)(ch1) << 8) |   \
    ((int32_t)(int8_t)(ch2) << 16) | ((int32_t)(int8_t)(ch3) << 24 ))

uint32_t CubemapDirections[DDS_CUBEMAP_SIDES] = {
    DDS_CUBEMAP_POSITIVEX,
    DDS_CUBEMAP_NEGATIVEX,
    DDS_CUBEMAP_POSITIVEY,
    DDS_CUBEMAP_NEGATIVEY,
    DDS_CUBEMAP_POSITIVEZ,
    DDS_CUBEMAP_NEGATIVEZ
};

#pragma pack(push, 1)
typedef struct lite3d_dds_pixelformat
{
    uint32_t    Size;               // size of structure DDS_PIXELFORMAT
    uint32_t    Flags;              // pixel format flags
    uint32_t    FourCC;             // (FOURCC code)
    uint32_t    RGBBitCount;        // how many bits per pixel
    uint32_t    RBitMask;           // mask for red bit
    uint32_t    GBitMask;           // mask for green bits
    uint32_t    BBitMask;           // mask for blue bits
    uint32_t    RGBAlphaBitMask;    // mask for alpha channel
} lite3d_dds_pixelformat;

typedef struct lite3d_dds_head
{
    char        Signature[4];

    uint32_t    Size;               // size of the head structure (minus MagicNum)
    uint32_t    Flags;              // determines what fields are valid
    uint32_t    Height;             // height of surface to be created
    uint32_t    Width;              // width of input surface
    uint32_t    LinearSize;         // Formless late-allocated optimized surface size
    uint32_t    Depth;              // Depth if a volume texture
    uint32_t    MipMapCount;        // number of mip-map levels requested
    uint32_t    AlphaBitDepth;      // depth of alpha buffer requested
    uint32_t    NotUsed[10];

    lite3d_dds_pixelformat PixelFormat;

    uint32_t    ddsCaps1, ddsCaps2, ddsCaps3, ddsCaps4; // direct draw surface capabilities
    uint32_t    TextureStage;
} lite3d_dds_head;
#pragma pack(pop)

static const char *lite3d_dds_get_format_string(int compFormat)
{
    switch (compFormat)
    {
        case DDS_PF_DXT1:
            return "DXT1";
        case DDS_PF_DXT3:
            return "DXT3";
        case DDS_PF_DXT5:
            return "DXT5";
        case DDS_PF_RXGB:
            return "RXGB";
        case DDS_PF_BC4:
            return "3DC/BC4";
        case DDS_PF_BC5:
            return "3DC/BC5";
    }

    return "Unknown";
}

static int lite3d_check_dds_head(const struct lite3d_dds_head *head)
{
    if (strncmp(head->Signature, "DDS ", 4))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Invalid DDS file signature", LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    //note that if Size1 is "DDS " this is not a valid dds file according
    //to the file spec. Some broken tool out there seems to produce files
    //with this value in the size field, so we support reading them...
    if (head->Size != (sizeof(lite3d_dds_head)-4) && head->Size != DDS_MAKEFOURCC('D', 'D', 'S', ' '))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Invalid DDS header size: %u", LITE3D_CURRENT_FUNCTION, head->Size);
        return LITE3D_FALSE;
    }

    if (head->PixelFormat.Size != sizeof(lite3d_dds_pixelformat))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Invalid DDS header pixelformat size: %u", 
            LITE3D_CURRENT_FUNCTION, head->PixelFormat.Size);
        return LITE3D_FALSE;
    }
    
    if (head->Width == 0 || head->Height == 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Invalid DDS dimensions: %u", 
            LITE3D_CURRENT_FUNCTION, head->PixelFormat.Size);
        return LITE3D_FALSE;
    }

    if (!(head->ddsCaps1 & DDS_TEXTURE))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: DDS is not a texture", LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

static size_t lite3d_read_and_validate_dds_head(const uint8_t *buffer, size_t size, struct lite3d_dds_head *head, int *compFormat)
{
    if (sizeof(lite3d_dds_head) > size)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Invalid DDS file size", LITE3D_CURRENT_FUNCTION);
        return 0;
    }

    memcpy(head, buffer, sizeof(lite3d_dds_head));

    if (!lite3d_check_dds_head(head))
    {
        return 0;
    }

    if (head->PixelFormat.Flags & DDS_FOURCC) 
    {
        switch (head->PixelFormat.FourCC)
        {
            case DDS_MAKEFOURCC('D','X','T','1'):
                *compFormat = DDS_PF_DXT1;
                break;
            case DDS_MAKEFOURCC('D','X','T','3'):
                *compFormat = DDS_PF_DXT3;
                break;
            case DDS_MAKEFOURCC('D','X','T','5'):
                *compFormat = DDS_PF_DXT5;
                break;
            case DDS_MAKEFOURCC('R', 'X', 'G', 'B'):
                *compFormat = DDS_PF_RXGB;
                break;
            case DDS_MAKEFOURCC('A', 'T', 'I', '1'):
                *compFormat = DDS_PF_BC4;
                break;
            case DDS_MAKEFOURCC('A', 'T', 'I', '2'):
                *compFormat = DDS_PF_BC5;
                break;
            default:
            {
                SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, 
                    "%s: DDS FastLoad attempt failed: Unsupported compression format: %u", 
                    LITE3D_CURRENT_FUNCTION, head->PixelFormat.FourCC);
                return 0;
            }
        }
    }
    else
    {
        SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, 
            "%s: DDS FastLoad attempt failed: DDS does not contain compressed data", 
            LITE3D_CURRENT_FUNCTION);
        return 0;
    }

    return sizeof(lite3d_dds_head);
}

static uint32_t lite3d_get_linear_size(uint32_t width, uint32_t height, uint32_t depth, int compFormat)
{
    uint32_t linearSize = ((width+3)/4) * ((height+3)/4) * depth;

    switch (compFormat)
    {
        case DDS_PF_DXT1:
        case DDS_PF_BC4:
            linearSize *= 8;
            break;
        case DDS_PF_DXT3:
        case DDS_PF_DXT5:
        case DDS_PF_RXGB:
        case DDS_PF_BC5:
            linearSize *= 16;
            break;
    }

    return linearSize;
}

static void lite3d_fixup_dds_header(struct lite3d_dds_head *head, int compFormat)
{
    if (head->Depth == 0 || !(head->ddsCaps2 & DDS_VOLUME))
        head->Depth = 1;

    if (!(head->Flags & DDS_MIPMAPCOUNT) || head->MipMapCount == 0) 
    {
        head->MipMapCount = 1;
    }

    head->Flags |= DDS_LINEARSIZE;
    head->LinearSize = lite3d_get_linear_size(head->Width, head->Height, head->Depth, compFormat);
}

static void lite3d_dds_header_get_format(const struct lite3d_dds_head *head, int8_t srgb, int compFormat, uint16_t *imageFormat, 
    uint16_t *internalFormat)
{
    switch (compFormat)
    {
        case DDS_PF_DXT1:
            {
                if (head->PixelFormat.Flags & DDS_ALPHAPIXELS)
                {
                    *imageFormat = LITE3D_TEXTURE_FORMAT_RGBA;
                    *internalFormat = srgb ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT : GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
                }
                else
                {
                    *imageFormat = LITE3D_TEXTURE_FORMAT_RGB;
                    *internalFormat = srgb ? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT : GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                }
            }
            break;
        case DDS_PF_DXT3:
            *imageFormat = LITE3D_TEXTURE_FORMAT_RGBA;
            *internalFormat = srgb ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT : GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
            break;
        case DDS_PF_DXT5:
            *imageFormat = LITE3D_TEXTURE_FORMAT_RGBA;
            *internalFormat = srgb ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
        case DDS_PF_RXGB:
            *imageFormat = LITE3D_TEXTURE_FORMAT_RGB;
            *internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
            break;
        case DDS_PF_BC4:
            *imageFormat = LITE3D_TEXTURE_FORMAT_RED;
            *internalFormat = GL_COMPRESSED_RED_RGTC1_EXT;
            break;
        case DDS_PF_BC5:
            *imageFormat = LITE3D_TEXTURE_FORMAT_RG;
            *internalFormat = GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
            break;
    }
}

static uint8_t lite3d_dds_header_get_num_cubefaces(const struct lite3d_dds_head *head)
{
    uint8_t validCubeFaces = 0; 
    for (int i = 0; i < DDS_CUBEMAP_SIDES; i++) 
    {
        if (head->ddsCaps2 & CubemapDirections[i])
        {
            validCubeFaces++;
        }
    }

    return validCubeFaces;
}

static size_t lite3d_dds_load_face(struct lite3d_texture_unit *textureUnit, const struct lite3d_dds_head *head, 
    const uint8_t *buffer, size_t size, uint8_t face, int compFormat)
{
    uint32_t Width = head->Width, Height = head->Height, Depth = head->Depth;
    size_t originSize = size;

    for (uint32_t mip = 0; mip < head->MipMapCount; ++mip)
    {
        size_t linearSize = lite3d_get_linear_size(Width, Height, Depth, compFormat);
        if (linearSize > size)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Invalid DDS file size", LITE3D_CURRENT_FUNCTION);
            return 0;
        }

        /* use only 0 level and skip others if texture do not support mipmaps */
        if (mip == 0 || textureUnit->generatedMipmaps > 0)
        {
            if (!lite3d_texture_unit_set_compressed_pixels(textureUnit, 0, 0, 0, Width, Height, Depth, mip, face, 
                linearSize, buffer))
            {
                return 0;
            }
        }

        size -= linearSize;
        buffer += linearSize;

        Width = LITE3D_MAX(1, Width / 2);
        Height = LITE3D_MAX(1, Height / 2);
        Depth = LITE3D_MAX(1, Depth / 2);
    }

    return originSize - size;
}

int lite3d_texture_unit_dds_fast_load(struct lite3d_texture_unit *textureUnit, const struct lite3d_file *resource, 
    uint32_t textureTarget, int8_t srgb, int8_t filtering, uint8_t wrapping, uint8_t cubeface)
{
    lite3d_dds_head head;
    int compFormat;
    size_t processed = 0, remains = resource->fileSize;
    uint16_t imageFormat, internalFormat;
    const uint8_t *buffer = (const uint8_t*)resource->fileBuff;
    uint8_t cubefacesNum = 0;
    lite3d_texture_unit textureUnitCopy = *textureUnit;

    if ((processed = lite3d_read_and_validate_dds_head(buffer, remains, &head, &compFormat)) == 0)
    {
        return LITE3D_FALSE;
    }

    textureUnitCopy.imageType = LITE3D_IMAGE_DDS;
    remains -= processed;
    buffer += processed;

    lite3d_fixup_dds_header(&head, compFormat);
    lite3d_dds_header_get_format(&head, srgb, compFormat, &imageFormat, &internalFormat);

    /* allocate texture surface if not allocated yet */
    if (textureUnitCopy.imageWidth != head.Width || textureUnitCopy.imageHeight != head.Height ||
        textureUnitCopy.imageDepth != head.Depth)
    {
        if (!lite3d_texture_unit_allocate(&textureUnitCopy, textureTarget, filtering,
            wrapping, imageFormat, internalFormat, head.Width, head.Height, head.Depth, 1))
        {
            return LITE3D_FALSE;
        }
    }

    /* check mipmaps consistency */
    if (head.Flags & DDS_MIPMAPCOUNT && textureUnitCopy.generatedMipmaps > 0)
    {
        if (head.MipMapCount != (textureUnitCopy.generatedMipmaps + 1))
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: DDS mipmap count mismatch, expected %d, fact %d", 
                LITE3D_CURRENT_FUNCTION, textureUnitCopy.generatedMipmaps, head.MipMapCount);
            lite3d_texture_unit_purge(&textureUnitCopy);
            return LITE3D_FALSE;
        }

        textureUnitCopy.loadedMipmaps = textureUnitCopy.generatedMipmaps;
    }

    do 
    {
        /* For CubeMap textures */
        if (textureTarget == LITE3D_TEXTURE_CUBE && head.ddsCaps1 & DDS_COMPLEX && head.ddsCaps2 & DDS_CUBEMAP)
        {
            cubefacesNum = lite3d_dds_header_get_num_cubefaces(&head);
            if (cubefacesNum == DDS_CUBEMAP_SIDES) // All faces are present
            {
                for (uint8_t face = 0; face < cubefacesNum; ++face)
                {
                    if ((processed = lite3d_dds_load_face(&textureUnitCopy, &head, buffer, remains, face, compFormat)) == 0)
                    {
                        lite3d_texture_unit_purge(&textureUnitCopy);
                        return LITE3D_FALSE;
                    }

                    remains -= processed;
                    buffer += processed;
                }

                break;
            }
        }

        /* Load regular image or requested face of cubemap */
        if ((processed = lite3d_dds_load_face(&textureUnitCopy, &head, buffer, remains, cubeface, compFormat)) == 0)
        {
            lite3d_texture_unit_purge(&textureUnitCopy);
            return LITE3D_FALSE;
        }
        
        remains -= processed;
        buffer += processed;
    } while(LITE3D_FALSE);

    /* ganerate mipmaps if not loaded */
    if (textureUnitCopy.loadedMipmaps == 0)
        lite3d_texture_unit_generate_mipmaps(&textureUnitCopy);

    if (LITE3D_CHECK_GL_ERROR)
    {
        lite3d_texture_unit_purge(&textureUnitCopy);
        return LITE3D_FALSE;
    }

    *textureUnit = textureUnitCopy;
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s(FastDDS): %s, "
        "%dx%dx%d, %d mipmaps %s, cubefaces %u, image: %s, storage: %s, data: %s",
        lite3d_texture_unit_target_string(textureUnit->textureTarget),
        resource->name,
        textureUnit->imageWidth,
        textureUnit->imageHeight,
        textureUnit->imageDepth,
        textureUnit->generatedMipmaps,
        textureUnit->loadedMipmaps == 0 ? "generated" : "loaded",
        cubefacesNum, 
        lite3d_dds_get_format_string(compFormat),
        lite3d_texture_unit_internal_format_string(textureUnit),
        lite3d_texture_unit_format_string(textureUnit));

    return LITE3D_TRUE;
}
