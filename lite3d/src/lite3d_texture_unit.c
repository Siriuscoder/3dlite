/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024  Sirius (Korolev Nikita)
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
#include <string.h>

#include <SDL_assert.h>
#include <SDL_log.h>

#include <IL/il.h>
#include <IL/ilu.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_texture_unit.h>
#include <lite3d/lite3d_texture_dds.h>

static lite3d_texture_technique_settings gTextureSettings;

static const ILuint imageTypeEnum[] = {
    0x0,
    IL_BMP,
    IL_JPG,
    IL_PNG,
    IL_TGA,
    IL_TIF,
    IL_GIF,
    IL_DDS,
    IL_PSD,
    IL_HDR
};

const GLenum textureTargetEnum[] = {
    GL_TEXTURE_1D,
    GL_TEXTURE_2D,
    GL_TEXTURE_3D,
    GL_TEXTURE_CUBE_MAP,
    GL_TEXTURE_2D_ARRAY,
    GL_TEXTURE_BUFFER,
    GL_TEXTURE_2D_MULTISAMPLE,
    GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
    GL_TEXTURE_2D,
    GL_TEXTURE_2D_ARRAY
};

static lite3d_image_filter gFilters[LITE3D_MAX_FILTERS];
static int8_t gFiltersCount = 0;
static int maxTextureSize;
static int maxTexture3DSize;
static int maxTextureImageUnits;
static int maxCombinedTextureImageUnits;
static int textureCompression = LITE3D_TRUE;

static void* LITE3D_DEVIL_CALL il_alloc(const ILsizei size)
{
    return lite3d_malloc(size);
}

static void LITE3D_DEVIL_CALL il_free(const void *ptr)
{
    lite3d_free((void *) ptr);
}

const char *lite3d_texture_unit_format_string(const lite3d_texture_unit *texture)
{
    switch (texture->texFormat)
    {
        case LITE3D_TEXTURE_FORMAT_BRG:
            return "BGR";
        case LITE3D_TEXTURE_FORMAT_BRGA:
            return "BGRA";
        case LITE3D_TEXTURE_FORMAT_RGB:
            return "RGB";
        case LITE3D_TEXTURE_FORMAT_RGBA:
            return "RGBA";
        case LITE3D_TEXTURE_FORMAT_DEPTH:
            return "DEPTH_COMPONENT";
        case LITE3D_TEXTURE_FORMAT_RED:
            return "RED";
        case LITE3D_TEXTURE_FORMAT_RG:
            return "RG";
        default:
            return "?";
    }
}

const char *lite3d_texture_unit_target_string(uint32_t textureTarget)
{
    switch (textureTargetEnum[textureTarget])
    {
    case GL_TEXTURE_1D:
        return "TEXTURE_1D";
    case GL_TEXTURE_2D:
        return "TEXTURE_2D";
    case GL_TEXTURE_3D:
        return "TEXTURE_3D";
    case GL_TEXTURE_CUBE_MAP:
        return "TEXTURE_CUBE_MAP";
    case GL_TEXTURE_BUFFER:
        return "TBO";
    case GL_TEXTURE_2D_MULTISAMPLE:
        return "TEXTURE_2D_MULTISAMPLE";
    case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        return "TEXTURE_2D_MULTISAMPLE_ARRAY";
    case GL_TEXTURE_2D_ARRAY:
        return "TEXTURE_2D_ARRAY";
    default:
        return "INVALID";
    }
}

static int texture_internal_format_compressed(uint32_t iformat)
{
    switch (iformat)
    {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
    case GL_COMPRESSED_RED_RGTC1_EXT:
    case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        return LITE3D_TRUE;
    }

    return LITE3D_FALSE;
}

const char *lite3d_texture_unit_internal_format_string(const lite3d_texture_unit *texture)
{
    switch (texture->texiFormat)
    {
    case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        return "Compressed RGB DXT1";
    case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
        return "Compressed sRGB DXT1";
    case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        return "Compressed RGBA DXT1";
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
        return "Compressed sRGBA DXT1";
    case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        return "Compressed RGBA DXT3";
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
        return "Compressed sRGBA DXT3";
    case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        return "Compressed RGBA DXT5";
    case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
        return "Compressed sRGBA DXT5";
    case GL_COMPRESSED_RED_RGTC1_EXT:
        return "Compressed RGTC1";
    case GL_COMPRESSED_RED_GREEN_RGTC2_EXT:
        return "Compressed RGTC2";
    case LITE3D_TEXTURE_FORMAT_RGB:
        return "RGB";
    case LITE3D_TEXTURE_FORMAT_RGBA:
        return "RGBA";
    case LITE3D_TEXTURE_FORMAT_DEPTH:
        return "DEPTH_COMPONENT";
    case GL_DEPTH_COMPONENT32:
        return "DEPTH_COMPONENT32";
    case LITE3D_TEXTURE_FORMAT_RED:
        return "RED";
    case LITE3D_TEXTURE_FORMAT_RG:
        return "RG";
    case LITE3D_TEXTURE_INTERNAL_R8:
        return "R8";
    case LITE3D_TEXTURE_INTERNAL_R8_SNORM:
        return "R8_SNORM";
    case LITE3D_TEXTURE_INTERNAL_R16:
        return "R16";
    case LITE3D_TEXTURE_INTERNAL_R16_SNORM:
        return "R16_SNORM";
    case LITE3D_TEXTURE_INTERNAL_RG8:
        return "RG8";
    case LITE3D_TEXTURE_INTERNAL_RG8_SNORM:
        return "RG8_SNORM";
    case LITE3D_TEXTURE_INTERNAL_RG16:
        return "RG16";
    case LITE3D_TEXTURE_INTERNAL_RG16_SNORM:
        return "RG16_SNORM";
    case LITE3D_TEXTURE_INTERNAL_R3_G3_B2:
        return "R3_G3_B2";
    case LITE3D_TEXTURE_INTERNAL_RGB4:
        return "RGB4";
    case LITE3D_TEXTURE_INTERNAL_RGB5:
        return "RGB5";
    case LITE3D_TEXTURE_INTERNAL_RGB8:
        return "RGB8";
    case LITE3D_TEXTURE_INTERNAL_RGB8_SNORM:
        return "RGB8_SNORM";
    case LITE3D_TEXTURE_INTERNAL_RGB10:
        return "RGB10";
    case LITE3D_TEXTURE_INTERNAL_RGB12:
        return "RGB12";
    case LITE3D_TEXTURE_INTERNAL_RGB16_SNORM:
        return "RGB16_SNORM";
    case LITE3D_TEXTURE_INTERNAL_RGBA2:
        return "RGBA2";
    case LITE3D_TEXTURE_INTERNAL_RGBA4:
        return "RGBA4";
    case LITE3D_TEXTURE_INTERNAL_RGB5_A1:
        return "RGB5_A1";
    case LITE3D_TEXTURE_INTERNAL_RGBA8:
        return "RGBA8";
    case LITE3D_TEXTURE_INTERNAL_RGBA8_SNORM:
        return "RGBA8_SNORM";
    case LITE3D_TEXTURE_INTERNAL_RGB10_A2:
        return "RGB10_A2";
    case LITE3D_TEXTURE_INTERNAL_RGB10_A2UI:
        return "RGB10_A2UI";
    case LITE3D_TEXTURE_INTERNAL_RGBA12: 
        return "RGBA12";
    case LITE3D_TEXTURE_INTERNAL_RGBA16: 
        return "RGBA16";
    case LITE3D_TEXTURE_INTERNAL_SRGB8:
        return "SRGB8";
    case LITE3D_TEXTURE_INTERNAL_SRGB8_ALPHA8:
        return "SRGB8_ALPHA8";
    case LITE3D_TEXTURE_INTERNAL_SRGB:
        return "SRGB";
    case LITE3D_TEXTURE_INTERNAL_SRGB_ALPHA:
        return "SRGB_ALPHA";
    case LITE3D_TEXTURE_INTERNAL_R16F: 
        return "R16F";
    case LITE3D_TEXTURE_INTERNAL_RG16F:
        return "RG16F";
    case LITE3D_TEXTURE_INTERNAL_RGB16F: 
        return "RGB16F";
    case LITE3D_TEXTURE_INTERNAL_RGBA16F:
        return "RGBA16F";
    case LITE3D_TEXTURE_INTERNAL_R32F: 
        return "R32F";
    case LITE3D_TEXTURE_INTERNAL_RG32F:
        return "RG32F";
    case LITE3D_TEXTURE_INTERNAL_RGB32F: 
        return "RGB32F";
    case LITE3D_TEXTURE_INTERNAL_RGBA32F:
        return "RGBA32F";
    case LITE3D_TEXTURE_INTERNAL_R11F_G11F_B10F:
        return "R11F_G11F_B10F";
    case LITE3D_TEXTURE_INTERNAL_RGB9_E5:
        return "RGB9_E5";
    case LITE3D_TEXTURE_INTERNAL_R8I:
        return "R8I";
    case LITE3D_TEXTURE_INTERNAL_R8UI: 
        return "R8UI";
    case LITE3D_TEXTURE_INTERNAL_R16I: 
        return "R16I";
    case LITE3D_TEXTURE_INTERNAL_R16UI:
        return "R16UI";
    case LITE3D_TEXTURE_INTERNAL_R32I: 
        return "R32I";
    case LITE3D_TEXTURE_INTERNAL_R32UI:
        return "R32UI";
    case LITE3D_TEXTURE_INTERNAL_RG8I: 
        return "RG8I";
    case LITE3D_TEXTURE_INTERNAL_RG8UI:
        return "RG8UI";
    case LITE3D_TEXTURE_INTERNAL_RG16I:
        return "RG16I";
    case LITE3D_TEXTURE_INTERNAL_RG16UI:
        return "RG16UI";
    case LITE3D_TEXTURE_INTERNAL_RG32I:
        return "RG32I";
    case LITE3D_TEXTURE_INTERNAL_RG32UI:
        return "RG32UI";
    case LITE3D_TEXTURE_INTERNAL_RGB8I:
        return "RGB8I";
    case LITE3D_TEXTURE_INTERNAL_RGB8UI:
        return "RGB8UI";
    case LITE3D_TEXTURE_INTERNAL_RGB16I:
        return "RGB16I";
    case LITE3D_TEXTURE_INTERNAL_RGB16UI:
        return "RGB16UI";
    case LITE3D_TEXTURE_INTERNAL_RGB32I:
        return "RGB32I";
    case LITE3D_TEXTURE_INTERNAL_RGB32UI:
        return "RGB32UI";
    case LITE3D_TEXTURE_INTERNAL_RGBA8I:
        return "RGBA8I";
    case LITE3D_TEXTURE_INTERNAL_RGBA8UI:
        return "RGBA8UI";
    case LITE3D_TEXTURE_INTERNAL_RGBA16I:
        return "RGBA16I";
    case LITE3D_TEXTURE_INTERNAL_RGBA16UI:
        return "RGBA16UI";
    case LITE3D_TEXTURE_INTERNAL_RGBA32I:
        return "RGBA32I";
    case LITE3D_TEXTURE_INTERNAL_RGBA32UI:
        return "RGBA32UI";
    default:
        return "?";
    }
}

static void apply_image_filters(void)
{
    int8_t i;
    for (i = 0; i < gFiltersCount; ++i)
    {
        switch (gFilters[i].filterID)
        {
            case LITE3D_ALIENIFY_FILTER:
                iluAlienify();
                break;
            case LITE3D_BLURAVG_FILTER:
                iluBlurAvg((ILint)gFilters[i].param1);
                break;
            case LITE3D_BLURGAUSSIAN_FILTER:
                iluBlurGaussian((ILint)gFilters[i].param1);
                break;
            case LITE3D_CONTRAST_FILTER:
                iluContrast(gFilters[i].param1);
                break;
            case LITE3D_GAMMACORRECT_FILTER:
                iluGammaCorrect(gFilters[i].param1);
                break;
            case LITE3D_FLIP_FILTER:
                iluFlipImage();
                break;                
            case LITE3D_MIRROR_FILTER:
                iluMirror();
                break;
            case LITE3D_NEGATIVE_FILTER:
                iluNegative();
                break;
            case LITE3D_NOISIFY_FILTER:
                iluNoisify(gFilters[i].param1);
                break;
            case LITE3D_PIXELIZE_FILTER:
                iluPixelize((ILint)gFilters[i].param1);
                break;
            case LITE3D_WAVE_FILTER:
                iluWave(gFilters[i].param1);
                break;
            case LITE3D_SHARPEN_FILTER:
                iluSharpen(gFilters[i].param1,
                    (ILint)gFilters[i].param2);
                break;
        }
    }
}

static int8_t max_mipmaps_count(int32_t width, int32_t height, int32_t depth,
    int32_t maxMipmapsSupported)
{
    int8_t count = 0;
    do
    {
        if (width > 1) width = width / 2;
        if (height > 1) height = height / 2;
        if (depth > 1) depth = depth / 2;

        count++;

        if (count > maxMipmapsSupported)
            return 0;
    }
    while (!(width == 1 && height == 1 && depth == 1));

    return count;
}

static int check_texture_target(uint32_t textureTarget)
{
    if (textureTarget >= (sizeof(textureTargetEnum) / sizeof(textureTargetEnum[0])))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Invalid texture target %d",
            LITE3D_CURRENT_FUNCTION, textureTarget);
        return LITE3D_FALSE;
    }

#ifdef GLES
    switch (textureTargetEnum[textureTarget])
    {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_BUFFER:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Texture target %s(%d) is not supported in GLES",
                LITE3D_CURRENT_FUNCTION, lite3d_texture_unit_target_string(textureTarget), textureTarget);
            return LITE3D_FALSE;
        }
    }
#endif

    return LITE3D_TRUE;
}

static int set_internal_format(lite3d_texture_unit *textureUnit, uint16_t format,
    uint16_t iformat, uint32_t *internalFormat)
{
#ifdef GLES
    switch (format)
    {
        case LITE3D_TEXTURE_FORMAT_BRG:
        case LITE3D_TEXTURE_FORMAT_BRGA:
#ifdef WITH_GLES2
        case LITE3D_TEXTURE_FORMAT_RED:
        case LITE3D_TEXTURE_FORMAT_RG:
#endif
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Texture format %d is not supported in GLES",
                LITE3D_CURRENT_FUNCTION, format);
            return LITE3D_FALSE;
        }
    }
#endif

    /* what BPP ? */
    switch (format)
    {
        case LITE3D_TEXTURE_FORMAT_RGB:
        case LITE3D_TEXTURE_FORMAT_BRG:
        {
            textureUnit->imageBPP = 3;
            *internalFormat = GL_RGB;
            if (textureCompression && lite3d_check_texture_compression_s3tc())
            {
                if (iformat == LITE3D_TEXTURE_INTERNAL_SRGB)
                {
                    iformat = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
                }
                else if (iformat == 0)
                {
                    iformat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
                }
            }
            break;
        }
        case LITE3D_TEXTURE_FORMAT_DEPTH:
        {
            if (gTextureSettings.useDepthFormat == LITE3D_TEXTURE_IFORMAT_DEPTH_32)
            {
                textureUnit->imageBPP = 4;
                *internalFormat = GL_DEPTH_COMPONENT32;
            }
            else
            {
                textureUnit->imageBPP = 3;
                *internalFormat = GL_DEPTH_COMPONENT;
            }
            break;
        }
        case LITE3D_TEXTURE_FORMAT_RGBA:
        case LITE3D_TEXTURE_FORMAT_BRGA:
        {
            textureUnit->imageBPP = 4;
            *internalFormat = GL_RGBA;
            if (textureCompression && lite3d_check_texture_compression_s3tc())
            {
                if (iformat == LITE3D_TEXTURE_INTERNAL_SRGB_ALPHA)
                {
                    iformat = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
                }
                else if (iformat == 0)
                {
                    iformat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
                }
            }
            break;
        }
        case LITE3D_TEXTURE_FORMAT_RED:
        {
            textureUnit->imageBPP = 1;
            *internalFormat = GL_RED;
            if (textureCompression && iformat == 0 && lite3d_check_texture_compression_rgtc())
            {
                iformat = GL_COMPRESSED_RED_RGTC1_EXT;
            }
            break;
        }
        case LITE3D_TEXTURE_FORMAT_RG:
        {
            textureUnit->imageBPP = 2;
            *internalFormat = GL_RG;
            if (textureCompression && iformat == 0 && lite3d_check_texture_compression_rgtc())
            {
                iformat = GL_COMPRESSED_RED_GREEN_RGTC2_EXT;
            }
            break;
        }
        default:
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "%s: Unknown texture format %d",
                LITE3D_CURRENT_FUNCTION, format);
            return LITE3D_FALSE;
        }
    }

    if (iformat > 0)
    {
        *internalFormat = iformat;
    }

    textureUnit->compressed = texture_internal_format_compressed(*internalFormat);
    return LITE3D_TRUE;
}

void lite3d_texture_technique_add_image_filter(lite3d_image_filter *filter)
{
    SDL_assert(filter);

    if (gFiltersCount < LITE3D_MAX_FILTERS)
    {
        gFilters[gFiltersCount++] = *filter;
    }
}

void lite3d_texture_technique_reset_filters(void)
{
    gFiltersCount = 0;
}

int lite3d_texture_technique_init(const lite3d_texture_technique_settings *settings)
{
    SDL_assert(settings);
    gTextureSettings = *settings;

    /* check compression extentions */
    if (!lite3d_check_texture_compression_s3tc())
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "%s: Texture compression s3tc not supported", LITE3D_CURRENT_FUNCTION);
    }

    if (!lite3d_check_texture_compression_rgtc())
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "%s: Texture compression rgtc not supported", LITE3D_CURRENT_FUNCTION);
    }

    textureCompression = gTextureSettings.useGLCompression;

    if (!lite3d_check_texture_filter_anisotropic())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: EXT_texture_filter_anisotropic not supported..", LITE3D_CURRENT_FUNCTION);
        gTextureSettings.anisotropy = 0;
    }
    else
    {
        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,
            &gTextureSettings.maxAnisotropy);
        if (gTextureSettings.anisotropy > gTextureSettings.maxAnisotropy)
            gTextureSettings.anisotropy = gTextureSettings.maxAnisotropy;
    }

    if (lite3d_check_seamless_cube_map())
    {
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    }

    if (gTextureSettings.useDepthFormat == LITE3D_TEXTURE_IFORMAT_DEPTH_32 && !lite3d_check_depth32())
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
            "%s: Format depth32 is not supported, setting default depth format", LITE3D_CURRENT_FUNCTION);
        gTextureSettings.useDepthFormat = LITE3D_TEXTURE_IFORMAT_DEPTH_DEFAULT;
    }
    
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    glGetIntegerv(GL_MAX_3D_TEXTURE_SIZE, &maxTexture3DSize);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTextureImageUnits);

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "GL_MAX_TEXTURE_IMAGE_UNITS: %d",
        maxTextureImageUnits);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS: %d",
        maxCombinedTextureImageUnits);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "GL_MAX_TEXTURE_SIZE: %d",
        maxTextureSize);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "GL_MAX_3D_TEXTURE_SIZE: %d",
        maxTexture3DSize);

    ilSetMemory(il_alloc, il_free);
    ilInit();
    iluInit();

    lite3d_texture_technique_reset_filters();
    return LITE3D_TRUE;
}

void lite3d_texture_technique_shut(void)
{
    ilShutDown();
}

int lite3d_texture_unit_from_resource(lite3d_texture_unit *textureUnit,
    const lite3d_file *resource, uint32_t imageType, uint32_t textureTarget, 
    int8_t srgb, int8_t filtering, uint8_t wrapping, uint8_t cubeface)
{
    ILuint imageDesc = 0, imageFormat = 0, internalFormat = 0;
    GLint mipLevel = 0;
    int32_t imageWidth, imageHeight, imageDepth;
    int8_t totalLevels = 0;
    uint8_t totalFaces = 0;
    uint8_t imageFace = 0;

    SDL_assert(resource);
    SDL_assert(textureUnit);

    if (!resource->isLoaded || resource->fileSize == 0)
        return LITE3D_FALSE;

    if (srgb && !lite3d_check_srgb())
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s sRGB format is not supported",
            lite3d_texture_unit_target_string(textureUnit->textureTarget));
        srgb = LITE3D_FALSE;
    }

    if (textureTarget > LITE3D_TEXTURE_CUBE)
    {
        textureUnit->textureTarget = textureTarget;
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s not supplied by %s method",
            lite3d_texture_unit_target_string(textureUnit->textureTarget), LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    if (textureCompression && gTextureSettings.useCompressedDataOnLoad && imageType == LITE3D_IMAGE_DDS)
    {
        if (lite3d_texture_unit_dds_fast_load(textureUnit, resource, textureTarget, srgb, filtering, wrapping, cubeface))
        {
            return LITE3D_TRUE;
        }
    }

    lite3d_misc_il_error_stack_clean();
    /* gen IL image */
    imageDesc = ilGenImage();
    if (LITE3D_CHECK_IL_ERROR)
        return LITE3D_FALSE;

    /* Bind IL image */
    ilBindImage(imageDesc);
    /* Load IL image from memory */
    if (!ilLoadL(imageTypeEnum[imageType], resource->fileBuff, (ILuint)resource->fileSize))
    {
        LITE3D_CHECK_IL_ERROR;
        ilDeleteImage(imageDesc);
        return LITE3D_FALSE;
    }

    /* retrive */
    imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
    imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
    imageDepth = ilGetInteger(IL_IMAGE_DEPTH);

    /* matches openGL texture format */
    imageFormat = ilGetInteger(IL_IMAGE_FORMAT);
    if (srgb && imageFormat == LITE3D_TEXTURE_FORMAT_RGB)
        internalFormat = LITE3D_TEXTURE_INTERNAL_SRGB;
    else if (srgb && imageFormat == LITE3D_TEXTURE_FORMAT_RGBA)
        internalFormat = LITE3D_TEXTURE_INTERNAL_SRGB_ALPHA;

    /* allocate texture surface if not allocated yet */
    if (textureUnit->imageWidth != imageWidth || textureUnit->imageHeight != imageHeight ||
        textureUnit->imageDepth != imageDepth)
    {
        if (!lite3d_texture_unit_allocate(textureUnit, textureTarget, filtering,
            wrapping, imageFormat, internalFormat, imageWidth, imageHeight, imageDepth, 1))
        {
            /* release IL image */
            ilDeleteImage(imageDesc);
            return LITE3D_FALSE;
        }
    }

    if (imageType == LITE3D_IMAGE_ANY)
    {
        imageType = ilGetInteger(IL_IMAGE_TYPE);
    }

    textureUnit->imageType = imageType;

    if (textureUnit->imageBPP != (int8_t) ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s: %s: "
            "representable image BPP does not mached with IL image BPP (%d-%d)",
            lite3d_texture_unit_target_string(textureUnit->textureTarget),
            resource->name, textureUnit->imageBPP,
            ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL));
    }

    if (textureUnit->imageSize != (size_t) ilGetInteger(IL_IMAGE_SIZE_OF_DATA))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s: %s: "
            "representable image size does not mached with IL image size (%d-%d)",
            lite3d_texture_unit_target_string(textureUnit->textureTarget),
            resource->name, textureUnit->imageBPP,
            ilGetInteger(IL_IMAGE_SIZE_OF_DATA));
    }

    textureUnit->loadedMipmaps = 0;

    /* make texture active */
    glBindTexture(textureTargetEnum[textureTarget], textureUnit->textureID);

    /* calc saved mipmaps in image */
    textureUnit->loadedMipmaps = ilGetInteger(IL_NUM_MIPMAPS);

    if (textureTarget == LITE3D_TEXTURE_CUBE)
    {
        /* expected exact 6 faces for cubemaping */
        if ((totalFaces = ilGetInteger(IL_NUM_FACES)) != 5)
            totalFaces = 0;
    }

    totalLevels = LITE3D_MIN(textureUnit->loadedMipmaps, textureUnit->generatedMipmaps);
    for (imageFace = 0; imageFace <= totalFaces; ++imageFace)
    {
        for (mipLevel = 0; mipLevel <= totalLevels; ++mipLevel)
        {
            int32_t lWidth, lHeight, lDepth;
            /* workaround to prevent ilActiveMipmap bug */
            ilBindImage(imageDesc);
            ilActiveFace(imageFace);
            ilActiveMipmap(mipLevel);

            apply_image_filters();

            lWidth = ilGetInteger(IL_IMAGE_WIDTH);
            lHeight = ilGetInteger(IL_IMAGE_HEIGHT);
            lDepth = ilGetInteger(IL_IMAGE_DEPTH);

            if (!lite3d_texture_unit_set_pixels(textureUnit, 0, 0, 0, 
                lWidth, lHeight, lDepth, mipLevel, totalFaces == 0 ? cubeface : imageFace, ilGetData()))
            {
                ilDeleteImage(imageDesc);
                lite3d_texture_unit_purge(textureUnit);
                return LITE3D_FALSE;
            }
        }
    }

    /* ganerate mipmaps if not loaded */
    if (textureUnit->loadedMipmaps == 0)
        lite3d_texture_unit_generate_mipmaps(textureUnit);

    if (LITE3D_CHECK_GL_ERROR)
    {
        ilDeleteImage(imageDesc);
        lite3d_texture_unit_purge(textureUnit);
        return LITE3D_FALSE;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s: %s, "
        "%dx%dx%d, %d mipmaps %s, storage: %s, data: %s",
        lite3d_texture_unit_target_string(textureUnit->textureTarget),
        resource->name,
        textureUnit->imageWidth,
        textureUnit->imageHeight,
        textureUnit->imageDepth,
        textureUnit->generatedMipmaps,
        textureUnit->loadedMipmaps == 0 ? "generated" : "loaded",
        lite3d_texture_unit_internal_format_string(textureUnit),
        lite3d_texture_unit_format_string(textureUnit));

    /* release IL image */
    ilDeleteImage(imageDesc);

    textureUnit->isFbAttachment = LITE3D_FALSE;
    return LITE3D_TRUE;
}

int lite3d_texture_unit_set_pixels(lite3d_texture_unit *textureUnit, 
    int32_t widthOff, int32_t heightOff, int32_t depthOff, 
    int32_t width, int32_t height, int32_t depth,
    int8_t level, uint8_t cubeface, const void *pixels)
{
    SDL_assert(textureUnit);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;
    
    /* make texture active */
    glBindTexture(textureTargetEnum[textureUnit->textureTarget], textureUnit->textureID);
    lite3d_misc_gl_error_stack_clean();
    
    switch (textureUnit->textureTarget)
    {
        case LITE3D_TEXTURE_1D:
            glTexSubImage1D(textureTargetEnum[textureUnit->textureTarget], level, widthOff,
                width, textureUnit->texFormat, GL_UNSIGNED_BYTE, pixels);
            break;
        case LITE3D_TEXTURE_2D:
        case LITE3D_TEXTURE_CUBE:
        case LITE3D_TEXTURE_2D_SHADOW:
            glTexSubImage2D(textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ?
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget],
                level, widthOff, heightOff, width, height, textureUnit->texFormat,
                GL_UNSIGNED_BYTE, pixels);
            break;
        case LITE3D_TEXTURE_3D:
        case LITE3D_TEXTURE_2D_ARRAY:
        case LITE3D_TEXTURE_2D_SHADOW_ARRAY:
            glTexSubImage3D(textureTargetEnum[textureUnit->textureTarget], level, widthOff,
                heightOff, depthOff, width, height, depth,
                textureUnit->texFormat, GL_UNSIGNED_BYTE, pixels);
            break;
    }
    
    return LITE3D_CHECK_GL_ERROR ? LITE3D_FALSE : LITE3D_TRUE;
}

int lite3d_texture_unit_set_compressed_pixels(lite3d_texture_unit *textureUnit, 
    int32_t widthOff, int32_t heightOff, int32_t depthOff, 
    int32_t width, int32_t height, int32_t depth,
    int8_t level, uint8_t cubeface, size_t pixelsSize, const void *pixels)
{
    SDL_assert(textureUnit);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;
    
    /* make texture active */
    glBindTexture(textureTargetEnum[textureUnit->textureTarget], textureUnit->textureID);
    lite3d_misc_gl_error_stack_clean();
#ifndef GLES
    {
        int32_t compressed;
        glGetTexLevelParameteriv(textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ? 
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget],
            level, GL_TEXTURE_COMPRESSED, &compressed);
    
        if(compressed == GL_FALSE)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s texture level %d is not a compressed format...",
                LITE3D_CURRENT_FUNCTION, level);
            glBindTexture(textureTargetEnum[textureUnit->textureTarget], 0);
            return LITE3D_FALSE;
        }
    }
#endif
    
    switch (textureUnit->textureTarget)
    {
        case LITE3D_TEXTURE_1D:
            glCompressedTexSubImage1D(textureTargetEnum[textureUnit->textureTarget], level, widthOff,
                width, textureUnit->texiFormat, (GLsizei)pixelsSize, pixels);
            break;
        case LITE3D_TEXTURE_2D:
        case LITE3D_TEXTURE_CUBE:
        case LITE3D_TEXTURE_2D_SHADOW:
            glCompressedTexSubImage2D(textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ? 
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget],
                level, widthOff, heightOff, width, height, textureUnit->texiFormat,
                (GLsizei)pixelsSize, pixels);
            break;
        case LITE3D_TEXTURE_3D:
        case LITE3D_TEXTURE_2D_ARRAY:
        case LITE3D_TEXTURE_2D_SHADOW_ARRAY:
            glCompressedTexSubImage3D(textureTargetEnum[textureUnit->textureTarget], level, widthOff,
                heightOff, depthOff, width, height, depth,
                textureUnit->texiFormat, (GLsizei)pixelsSize, pixels);
            break;
    }

    return LITE3D_CHECK_GL_ERROR ? LITE3D_FALSE : LITE3D_TRUE;  
}

int lite3d_texture_unit_get_level_size(lite3d_texture_unit *textureUnit, 
    int8_t level, uint8_t cubeface, size_t *size)
{
    int32_t imageWidth, imageHeight, imageDepth;

    SDL_assert(textureUnit);
    SDL_assert(size);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;

    imageWidth = lite3d_texture_unit_get_level_width(textureUnit, level, cubeface);
    imageHeight = lite3d_texture_unit_get_level_height(textureUnit, level, cubeface);
    imageDepth = lite3d_texture_unit_get_level_depth(textureUnit, level, cubeface);

#ifndef GLES
    {
        GLenum levelType = textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ? 
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget];
            
        if (textureUnit->texFormat == LITE3D_TEXTURE_FORMAT_DEPTH)
        {
            int32_t sizeDepth;
            glGetTexLevelParameteriv(levelType, level, GL_TEXTURE_DEPTH_SIZE, &sizeDepth);
            *size = imageWidth * imageHeight * imageDepth * sizeDepth / 8;
        }
        else
        {
            int32_t sizeR, sizeG, sizeB, sizeA;
            glGetTexLevelParameteriv(levelType, level, GL_TEXTURE_RED_SIZE, &sizeR);
            glGetTexLevelParameteriv(levelType, level, GL_TEXTURE_GREEN_SIZE, &sizeG);
            glGetTexLevelParameteriv(levelType, level, GL_TEXTURE_BLUE_SIZE, &sizeB);
            glGetTexLevelParameteriv(levelType, level, GL_TEXTURE_ALPHA_SIZE, &sizeA);
            *size = imageWidth * imageHeight * imageDepth * (sizeR + sizeG + sizeB + sizeA) / 8;
        }
    }
#else
    *size = imageWidth * imageHeight * imageDepth * textureUnit->imageBPP;
#endif
    return LITE3D_TRUE;
}

int lite3d_texture_unit_get_compressed_level_size(lite3d_texture_unit *textureUnit, 
    int8_t level, uint8_t cubeface, size_t *size)
{
#ifndef GLES
    int32_t compressed;

    SDL_assert(textureUnit);
    SDL_assert(size);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;

    /* make texture active */
    glBindTexture(textureTargetEnum[textureUnit->textureTarget], textureUnit->textureID);
    glGetTexLevelParameteriv(textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ? 
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget],
        level, GL_TEXTURE_COMPRESSED, &compressed);
    
    if(compressed == GL_FALSE)
        return LITE3D_FALSE;

    glGetTexLevelParameteriv(textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ? 
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget],
        level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, (GLint *)size);

    return LITE3D_TRUE;
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s texture dumping is not supported..",
        LITE3D_CURRENT_FUNCTION);
    return LITE3D_FALSE;
#endif
}

int lite3d_texture_unit_get_pixels(lite3d_texture_unit *textureUnit, 
    int8_t level, uint8_t cubeface, void *pixels)
{
#ifndef GLES
    SDL_assert(textureUnit);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;

    /* make texture active */
    glBindTexture(textureTargetEnum[textureUnit->textureTarget], textureUnit->textureID);
    lite3d_misc_gl_error_stack_clean();

    glGetTexImage(textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ? 
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget],
        level, textureUnit->texFormat, GL_UNSIGNED_BYTE, pixels);

    return LITE3D_CHECK_GL_ERROR ? LITE3D_FALSE : LITE3D_TRUE;
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s texture dumping is not supported..",
        LITE3D_CURRENT_FUNCTION);
    return LITE3D_FALSE;
#endif
}

int lite3d_texture_unit_get_compressed_pixels(lite3d_texture_unit *textureUnit, 
    int8_t level, uint8_t cubeface, void *pixels)
{
#ifndef GLES
    int32_t compressed;

    SDL_assert(textureUnit);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;

    /* make texture active */
    glBindTexture(textureTargetEnum[textureUnit->textureTarget], textureUnit->textureID);
    lite3d_misc_gl_error_stack_clean();

    glGetTexLevelParameteriv(textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ? 
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget],
        level, GL_TEXTURE_COMPRESSED, &compressed);

    if(compressed == GL_FALSE)
        return LITE3D_FALSE;

    glGetCompressedTexImage(textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ? 
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget],
        level, pixels);
    return LITE3D_CHECK_GL_ERROR ? LITE3D_FALSE : LITE3D_TRUE;
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s texture dumping does not supported..",
        LITE3D_CURRENT_FUNCTION);
    return LITE3D_FALSE;
#endif
}

int lite3d_texture_unit_generate_mipmaps(lite3d_texture_unit *textureUnit)
{
    if(textureUnit->generatedMipmaps > 0)
    {
        glBindTexture(textureTargetEnum[textureUnit->textureTarget], textureUnit->textureID);
        glGenerateMipmap(textureTargetEnum[textureUnit->textureTarget]);
        return LITE3D_TRUE;
    }
    
    return LITE3D_FALSE;
}

void lite3d_texture_unit_calc_total_size(lite3d_texture_unit *textureUnit)
{
    typedef int (*tsizefunc)(lite3d_texture_unit *, int8_t, uint8_t, size_t *);
    tsizefunc sf;
    size_t levelSize = 0;
    textureUnit->totalSize = 0;

    sf = textureUnit->compressed ? lite3d_texture_unit_get_compressed_level_size : 
        lite3d_texture_unit_get_level_size;
    
    for (int icube = 0; icube < 6; icube++)
    {
        uint8_t li = 0;
        while(sf(textureUnit, li++, icube, &levelSize))
            textureUnit->totalSize += levelSize;
        if (textureUnit->textureTarget != LITE3D_TEXTURE_CUBE)
            break;
    }
}

int lite3d_texture_unit_allocate(lite3d_texture_unit *textureUnit,
    uint32_t textureTarget, int8_t filtering, uint8_t wrapping, uint16_t format,
    uint16_t iformat, int32_t width, int32_t height, int32_t depth, int32_t samples)
{
    uint32_t internalFormat;
    int32_t textureMaxLevels;
    SDL_assert(textureUnit);

    memset(textureUnit, 0, sizeof (lite3d_texture_unit));
    lite3d_misc_gl_error_stack_clean();


    textureUnit->isTextureBuffer = LITE3D_FALSE;
    textureUnit->imageType = LITE3D_IMAGE_ANY;
    textureUnit->imageWidth = width;
    textureUnit->imageHeight = height;
    textureUnit->imageDepth = depth;

    if (!check_texture_target(textureTarget))
    {
        return LITE3D_FALSE;
    }

    if ((textureTarget == LITE3D_TEXTURE_2D_MULTISAMPLE || textureTarget == LITE3D_TEXTURE_3D_MULTISAMPLE) &&
        !lite3d_check_texture_multisample())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Multisample texture not supported",
            LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    if (textureTarget == LITE3D_TEXTURE_2D_SHADOW && !lite3d_check_shadow_samplers())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Shadow2D textures not supported",
            LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    if (!set_internal_format(textureUnit, format, iformat, &internalFormat))
    {
        return LITE3D_FALSE;
    }

    textureUnit->imageSize = width * height * depth * textureUnit->imageBPP;
    textureUnit->loadedMipmaps = 0;
    textureUnit->generatedMipmaps = 0;
    textureUnit->textureTarget = textureTarget;
    textureUnit->wrapping = wrapping;
    textureUnit->texFormat = format;
    textureUnit->texiFormat = internalFormat;

    if ((textureTarget >= LITE3D_TEXTURE_BUFFER && textureTarget <= LITE3D_TEXTURE_3D_MULTISAMPLE && filtering > LITE3D_TEXTURE_FILTER_NEAREST) ||
        (textureTarget == LITE3D_TEXTURE_2D_SHADOW && textureTarget == LITE3D_TEXTURE_2D_SHADOW_ARRAY && filtering > LITE3D_TEXTURE_FILTER_BILINEAR))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: incorrect filtering option %d for texture %s",
            LITE3D_CURRENT_FUNCTION, filtering, lite3d_texture_unit_target_string(textureUnit->textureTarget));
        return LITE3D_FALSE;
    }

    if (textureTarget == LITE3D_TEXTURE_2D_SHADOW && textureTarget == LITE3D_TEXTURE_2D_SHADOW_ARRAY && 
        format != LITE3D_TEXTURE_FORMAT_DEPTH)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: incorrect format %s for texture %s",
            LITE3D_CURRENT_FUNCTION, lite3d_texture_unit_format_string(textureUnit), lite3d_texture_unit_target_string(textureUnit->textureTarget));
        return LITE3D_FALSE;
    }

    /* enable texture target */
    glGenTextures(1, &textureUnit->textureID);
    if (LITE3D_CHECK_GL_ERROR)
    {
        return LITE3D_FALSE;
    }

    /* make texture active */
    glBindTexture(textureTargetEnum[textureTarget], textureUnit->textureID);

    if (filtering == LITE3D_TEXTURE_FILTER_TRILINEAR && textureTarget < LITE3D_TEXTURE_BUFFER)
    {
        /* check  mipmaps consistency */
#ifdef GL_TEXTURE_MAX_LEVEL
        glGetTexParameteriv(textureTargetEnum[textureTarget], GL_TEXTURE_MAX_LEVEL, &textureMaxLevels);
#else
        textureMaxLevels = 1000;
#endif
        textureUnit->generatedMipmaps = max_mipmaps_count(width,
            height, depth, textureMaxLevels);
        if (textureUnit->generatedMipmaps == 0)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: mipmaps not supported for this dimensions %dx%dx%d",
                lite3d_texture_unit_target_string(textureUnit->textureTarget),
                width, height, depth);
            filtering = LITE3D_TEXTURE_FILTER_BILINEAR;
        }
    }

    textureUnit->minFilter = (filtering == LITE3D_TEXTURE_FILTER_TRILINEAR ?
        GL_LINEAR_MIPMAP_LINEAR : (filtering == LITE3D_TEXTURE_FILTER_NEAREST ?
            GL_NEAREST : GL_LINEAR));
    textureUnit->magFilter = (filtering == LITE3D_TEXTURE_FILTER_NEAREST ?
        GL_NEAREST : GL_LINEAR);

    if (textureTarget < LITE3D_TEXTURE_BUFFER || textureTarget == LITE3D_TEXTURE_2D_SHADOW || 
        textureTarget == LITE3D_TEXTURE_2D_SHADOW_ARRAY)
    {
        glTexParameteri(textureTargetEnum[textureTarget], GL_TEXTURE_MIN_FILTER, textureUnit->minFilter);
        glTexParameteri(textureTargetEnum[textureTarget], GL_TEXTURE_MAG_FILTER, textureUnit->magFilter);
        glTexParameteri(textureTargetEnum[textureTarget], GL_TEXTURE_WRAP_S,
            textureUnit->wrapping == LITE3D_TEXTURE_REPEAT ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        glTexParameteri(textureTargetEnum[textureTarget], GL_TEXTURE_WRAP_T,
            textureUnit->wrapping == LITE3D_TEXTURE_REPEAT ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        if (textureTarget == LITE3D_TEXTURE_3D || textureTarget == LITE3D_TEXTURE_2D_ARRAY ||
            textureTarget == LITE3D_TEXTURE_2D_SHADOW_ARRAY)
        {
            glTexParameteri(textureTargetEnum[textureTarget], GL_TEXTURE_WRAP_R,
                textureUnit->wrapping == LITE3D_TEXTURE_REPEAT ? GL_REPEAT : GL_CLAMP_TO_EDGE);
        }

        if (filtering == LITE3D_TEXTURE_FILTER_TRILINEAR)
        {
            if (gTextureSettings.anisotropy > 1)
            {
                /* set anisotropic angle */
                glTexParameteri(textureTargetEnum[textureTarget], GL_TEXTURE_MAX_ANISOTROPY_EXT,
                    gTextureSettings.anisotropy);
            }
        }
    }

    /* Enable depth texture comparison mode for shadowmaps */
    if (textureTarget == LITE3D_TEXTURE_2D_SHADOW || 
        textureTarget == LITE3D_TEXTURE_2D_SHADOW_ARRAY)
    {
        glTexParameteri(textureTargetEnum[textureTarget], GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(textureTargetEnum[textureTarget], GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    }

    if (LITE3D_CHECK_GL_ERROR)
    {
        lite3d_texture_unit_purge(textureUnit);
        return LITE3D_FALSE;
    }

    /* allocate texture memory */
    switch (textureTarget)
    {
        case LITE3D_TEXTURE_1D:
            glTexImage1D(textureTargetEnum[textureTarget], 0, internalFormat, width,
                0, format, GL_UNSIGNED_BYTE, NULL);
            break;
        case LITE3D_TEXTURE_2D:
        case LITE3D_TEXTURE_2D_SHADOW:
            glTexImage2D(textureTargetEnum[textureTarget], 0, internalFormat, width,
                height, 0, format, GL_UNSIGNED_BYTE, NULL);
            break;
        case LITE3D_TEXTURE_3D:
        case LITE3D_TEXTURE_2D_ARRAY:
        case LITE3D_TEXTURE_2D_SHADOW_ARRAY:
            glTexImage3D(textureTargetEnum[textureTarget], 0, internalFormat, width,
                height, depth, 0, format, GL_UNSIGNED_BYTE, NULL);
            break;
        case LITE3D_TEXTURE_2D_MULTISAMPLE:
            glTexImage2DMultisample(textureTargetEnum[textureTarget], samples, internalFormat, width,
                height, GL_TRUE);
            break;
        case LITE3D_TEXTURE_3D_MULTISAMPLE:
            glTexImage3DMultisample(textureTargetEnum[textureTarget], samples, internalFormat, width,
                height, depth, GL_TRUE);
            break;
        case LITE3D_TEXTURE_CUBE:
        {
            int icube = 0;
            for (icube = 0; icube < 6; icube++)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + icube, 0, internalFormat, width,
                    height, 0, format, GL_UNSIGNED_BYTE, NULL);
            }
            break;
        }
    }

    if (LITE3D_CHECK_GL_ERROR)
    {
        lite3d_texture_unit_purge(textureUnit);
        return LITE3D_FALSE;
    }

    /*
     * You must reserve memory for other mipmaps levels as well either by making a 
     * series of calls to glTexImage2D or use glGenerateMipmap(GL_TEXTURE_2D).
     * Here, we'll use :
     */
    lite3d_texture_unit_generate_mipmaps(textureUnit);
    /* calculate texture total size */
    lite3d_texture_unit_calc_total_size(textureUnit);

    if (LITE3D_CHECK_GL_ERROR)
    {
        lite3d_texture_unit_purge(textureUnit);
        return LITE3D_FALSE;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s: "
        "%dx%dx%d surface allocated (%lu bytes), %d mipmaps, storage: %s, data: %s",
        lite3d_texture_unit_target_string(textureUnit->textureTarget),
        textureUnit->imageWidth,
        textureUnit->imageHeight,
        textureUnit->imageDepth,
        textureUnit->totalSize,
        textureUnit->generatedMipmaps,
        lite3d_texture_unit_internal_format_string(textureUnit),
        lite3d_texture_unit_format_string(textureUnit));

    textureUnit->isFbAttachment = LITE3D_FALSE;
    return LITE3D_TRUE;
}

void lite3d_texture_unit_purge(lite3d_texture_unit *textureUnit)
{
    SDL_assert(textureUnit);
    glDeleteTextures(1, &textureUnit->textureID);
    textureUnit->textureID = 0;
    textureUnit->totalSize = 0;
}

void lite3d_texture_unit_bind(lite3d_texture_unit *textureUnit, uint16_t layer)
{
    SDL_assert(textureUnit);
    SDL_assert_release(layer < maxCombinedTextureImageUnits);

    glActiveTexture(GL_TEXTURE0 + layer);
    glBindTexture(textureTargetEnum[textureUnit->textureTarget], textureUnit->textureID);

    if (textureUnit->isFbAttachment &&
        textureUnit->generatedMipmaps > 0)
        glGenerateMipmap(textureTargetEnum[textureUnit->textureTarget]);
}

void lite3d_texture_unit_unbind(lite3d_texture_unit *textureUnit, uint16_t layer)
{
    SDL_assert(textureUnit);
    SDL_assert_release(layer < maxCombinedTextureImageUnits);

    glActiveTexture(GL_TEXTURE0 + layer);
    glBindTexture(textureTargetEnum[textureUnit->textureTarget], 0);
}

void lite3d_texture_unit_compression(uint8_t on)
{
    /* skip if not supported or disabled */
    if (!gTextureSettings.useGLCompression)
    {
        return;
    }

    textureCompression = on;
}

int32_t lite3d_texture_unit_get_level_width(lite3d_texture_unit *textureUnit,
    int8_t level, uint8_t cubeface)
{
#ifndef GLES
    int32_t result = 0;
    SDL_assert(textureUnit);
    glBindTexture(textureTargetEnum[textureUnit->textureTarget], textureUnit->textureID);
    glGetTexLevelParameteriv(textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ? 
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget],
        level, GL_TEXTURE_WIDTH, &result);
    return result;
#else
    SDL_assert(textureUnit);
    return textureUnit->imageWidth >> level;
#endif
}

int32_t lite3d_texture_unit_get_level_height(lite3d_texture_unit *textureUnit,
    int8_t level, uint8_t cubeface)
{
#ifndef GLES
    int32_t result = 0;
    SDL_assert(textureUnit);
    glBindTexture(textureTargetEnum[textureUnit->textureTarget], textureUnit->textureID);
    glGetTexLevelParameteriv(textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ? 
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget],
        level, GL_TEXTURE_HEIGHT, &result);
    return result;
#else
    SDL_assert(textureUnit);
    return textureUnit->imageHeight >> level;
#endif
}

int32_t lite3d_texture_unit_get_level_depth(lite3d_texture_unit *textureUnit,
    int8_t level, uint8_t cubeface)
{
#ifndef GLES
    int32_t result = 0;
    SDL_assert(textureUnit);
    glBindTexture(textureTargetEnum[textureUnit->textureTarget], textureUnit->textureID);
    glGetTexLevelParameteriv(textureUnit->textureTarget == LITE3D_TEXTURE_CUBE ? 
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubeface : textureTargetEnum[textureUnit->textureTarget],
        level, GL_TEXTURE_DEPTH, &result);
    return result;
#else
    SDL_assert(textureUnit);
    return textureUnit->imageDepth >> level;
#endif
}
