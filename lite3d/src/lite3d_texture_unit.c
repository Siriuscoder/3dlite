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

static lite3d_texture_technique_settings gTextureSettings;

static const char *glTextureFormats[] = {
    "UNKNOWN",
    "COLOR_INDEX",
    "ALPHA",
    "RGB",
    "RGBA",
    "BGR",
    "BGRA",
    "LUMINANCE",
    "LUMINANCE_ALPHA",
    "DEPTH_COMPONENT"
};

static lite3d_image_filter gFilters[LITE3D_MAX_FILTERS];
static int8_t gFiltersCount = 0;
static int maxTextureSize;
static int maxTextureImageUnits;
static int maxCombinedTextureImageUnits;

static void* LITE3D_DEVIL_CALL il_alloc(const ILsizei size)
{
    return lite3d_malloc(size);
}

static void LITE3D_DEVIL_CALL il_free(const void *ptr)
{
    lite3d_free((void *) ptr);
}

static const char *format_string(GLenum format)
{
    switch (format)
    {
#ifndef GLES
        case GL_COLOR_INDEX:
            return glTextureFormats[1];
        case GL_BGR:
            return glTextureFormats[5];
        case GL_BGRA:
            return glTextureFormats[6];
#endif
        case GL_ALPHA:
            return glTextureFormats[2];
        case GL_RGB:
            return glTextureFormats[3];
        case GL_RGBA:
            return glTextureFormats[4];
        case GL_LUMINANCE:
            return glTextureFormats[7];
        case GL_LUMINANCE_ALPHA:
            return glTextureFormats[8];
        case GL_DEPTH_COMPONENT:
            return glTextureFormats[9];
        default:
            return glTextureFormats[0];
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

    if (settings->useGLCompression)
    {
        /* check compression extentions */
        if (!lite3d_check_texture_compression() || !lite3d_check_texture_compression_s3tc()
            || !lite3d_check_texture_compression_dxt1())
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: Texture compression not supported, skiping", LITE3D_CURRENT_FUNCTION);
            gTextureSettings.useGLCompression = 0;
        }
    }

    if (!lite3d_check_texture_filter_anisotropic())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: EXT_texture_filter_anisotropic not supported..", LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }
    else
    {
        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,
            &gTextureSettings.maxAnisotropy);
        if (gTextureSettings.anisotropy > gTextureSettings.maxAnisotropy)
            gTextureSettings.anisotropy = gTextureSettings.maxAnisotropy;
    }

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureImageUnits);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &maxCombinedTextureImageUnits);

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Max texture image units: %d",
        maxTextureImageUnits);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Max combined texture image units: %d",
        maxCombinedTextureImageUnits);
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Max texture size: %d",
        maxTextureSize);

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
    const lite3d_file *resource, uint32_t imageType,
    uint32_t textureTarget, int8_t quality, uint8_t wrapping)
{
    ILuint imageDesc = 0, imageFormat;
    GLint mipLevel = 0;
    int32_t imageWidth, imageHeight, imageDepth;
    int8_t totalLevels = 0;

    SDL_assert(resource);
    SDL_assert(textureUnit);

    if (!resource->isLoaded || resource->fileSize == 0)
        return LITE3D_FALSE;


    lite3d_misc_il_error_stack_clean();
    /* gen IL image */
    imageDesc = ilGenImage();
    if (lite3d_misc_check_il_error())
        return LITE3D_FALSE;

    /* Bind IL image */
    ilBindImage(imageDesc);
    /* Load IL image from memory */
    if (!ilLoadL(imageType, resource->fileBuff, resource->fileSize))
    {
        lite3d_misc_check_il_error();
        return LITE3D_FALSE;
    }

    apply_image_filters();

    /* retrive */
    imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
    imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
    imageDepth = ilGetInteger(IL_IMAGE_DEPTH);
    /* matches openGL texture format */
    imageFormat = ilGetInteger(IL_IMAGE_FORMAT);
    /* allocate texture surface */
    if (!lite3d_texture_unit_allocate(textureUnit, textureTarget, quality,
        wrapping, imageFormat, imageWidth, imageHeight, imageDepth))
    {
        /* release IL image */
        ilDeleteImage(imageDesc);
        return LITE3D_FALSE;
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
            (textureUnit->textureTarget == LITE3D_TEXTURE_1D ? "TEXTURE_1D" :
            (textureUnit->textureTarget == LITE3D_TEXTURE_2D ? "TEXTURE_2D" :
            "TEXTURE_3D")),
            resource->name, textureUnit->imageBPP,
            ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL));
    }

    if (textureUnit->imageSize != (size_t) ilGetInteger(IL_IMAGE_SIZE_OF_DATA))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s: %s: "
            "representable image size does not mached with IL image size (%d-%d)",
            (textureUnit->textureTarget == LITE3D_TEXTURE_1D ? "TEXTURE_1D" :
            (textureUnit->textureTarget == LITE3D_TEXTURE_2D ? "TEXTURE_2D" :
            "TEXTURE_3D")),
            resource->name, textureUnit->imageBPP,
            ilGetInteger(IL_IMAGE_SIZE_OF_DATA));
    }

    textureUnit->loadedMipmaps = 0;

    /* make texture active */
    glBindTexture(textureTarget, textureUnit->textureID);

    /* calc saved mipmaps in image */
    textureUnit->loadedMipmaps = ilGetInteger(IL_NUM_MIPMAPS);
    /* Specifies the alignment requirements 
     * for the start of each pixel row in memory.*/
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    totalLevels = textureUnit->loadedMipmaps < textureUnit->generatedMipmaps ?
        textureUnit->loadedMipmaps : textureUnit->generatedMipmaps;
    for (mipLevel = 0; mipLevel <= totalLevels; ++mipLevel)
    {
        int32_t lWidth, lHeight, lDepth;
        /* workaround to prevent ilActiveMipmap bug */
        ilBindImage(imageDesc);
        ilActiveMipmap(mipLevel);
        
        lWidth = ilGetInteger(IL_IMAGE_WIDTH);
        lHeight = ilGetInteger(IL_IMAGE_HEIGHT);
        lDepth = ilGetInteger(IL_IMAGE_DEPTH);

        if (!lite3d_texture_unit_set_pixels(textureUnit, 0, 0, 0, 
            lWidth, lHeight, lDepth, mipLevel, ilGetData()))
        {
            ilDeleteImages(1, &imageDesc);
            lite3d_texture_unit_purge(textureUnit);
            return LITE3D_FALSE;
        }
    }

    /* make texture active */
    glBindTexture(textureTarget, textureUnit->textureID);
    /* ganerate mipmaps if not loaded */
    if (textureUnit->loadedMipmaps == 0 && quality == LITE3D_TEXTURE_QL_NICEST)
        glGenerateMipmap(textureTarget);

    if (lite3d_misc_check_gl_error())
    {
        ilDeleteImages(1, &imageDesc);
        lite3d_texture_unit_purge(textureUnit);
        return LITE3D_FALSE;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s: %s, "
        "%dx%dx%d, build-in/%s mipmaps %d/%d, %s, "
        "format %s",
        (textureUnit->textureTarget == LITE3D_TEXTURE_1D ? "TEXTURE_1D" :
        (textureUnit->textureTarget == LITE3D_TEXTURE_2D ? "TEXTURE_2D" :
        "TEXTURE_3D")),
        resource->name,
        textureUnit->imageWidth,
        textureUnit->imageHeight,
        textureUnit->imageDepth,
        textureUnit->loadedMipmaps == 0 ? "generated" : "allocated",
        textureUnit->loadedMipmaps,
        textureUnit->generatedMipmaps,
        gTextureSettings.useGLCompression ?
        (textureUnit->texiFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ?
        "Compressed DXT1" : "Compressed DXT5") : "Not compressed",
        format_string(imageFormat));

    /* release IL image */
    ilDeleteImage(imageDesc);

    textureUnit->isFbAttachment = LITE3D_FALSE;
    return LITE3D_TRUE;
}

int lite3d_texture_unit_set_pixels(lite3d_texture_unit *textureUnit, 
    int32_t widthOff, int32_t heightOff, int32_t depthOff, 
    int32_t width, int32_t height, int32_t depth,
    int8_t level, const void *pixels)
{
    SDL_assert(textureUnit);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;
    
    /* make texture active */
    glBindTexture(textureUnit->textureTarget, textureUnit->textureID);
    lite3d_misc_gl_error_stack_clean();
    
    switch (textureUnit->textureTarget)
    {
        case LITE3D_TEXTURE_1D:
            glTexSubImage1D(textureUnit->textureTarget, level, widthOff,
                width, textureUnit->texFormat, GL_UNSIGNED_BYTE, pixels);
            break;
        case LITE3D_TEXTURE_2D:
            glTexSubImage2D(textureUnit->textureTarget, level, widthOff,
                heightOff, width, height, textureUnit->texFormat,
                GL_UNSIGNED_BYTE, pixels);
            break;
        case LITE3D_TEXTURE_3D:
            glTexSubImage3D(textureUnit->textureTarget, level, widthOff,
                heightOff, depthOff, width, height, depth,
                textureUnit->texFormat, GL_UNSIGNED_BYTE, pixels);
            break;
    }
    
    return lite3d_misc_check_gl_error() ? LITE3D_FALSE : LITE3D_TRUE;
}

int lite3d_texture_unit_set_compressed_pixels(lite3d_texture_unit *textureUnit, 
    int32_t widthOff, int32_t heightOff, int32_t depthOff, 
    int32_t width, int32_t height, int32_t depth,
    int8_t level, size_t pixelsSize, const void *pixels)
{
    SDL_assert(textureUnit);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;
    
    /* make texture active */
    glBindTexture(textureUnit->textureTarget, textureUnit->textureID);
    lite3d_misc_gl_error_stack_clean();
#ifndef GLES
    {
        int32_t compressed;
        glGetTexLevelParameteriv(textureUnit->textureTarget, level,
            GL_TEXTURE_COMPRESSED, &compressed);
    
        if(compressed == GL_FALSE)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s texture level %d is not a compressed format...",
                LITE3D_CURRENT_FUNCTION, level);
            glBindTexture(textureUnit->textureTarget, 0);
            return LITE3D_FALSE;
        }
    }
#endif
    
    switch (textureUnit->textureTarget)
    {
        case LITE3D_TEXTURE_1D:
            glCompressedTexSubImage1D(textureUnit->textureTarget, level, widthOff,
                width, textureUnit->texFormat, pixelsSize, pixels);
            break;
        case LITE3D_TEXTURE_2D:
            glCompressedTexSubImage2D(textureUnit->textureTarget, level, widthOff,
                heightOff, width, height, textureUnit->texFormat,
                pixelsSize, pixels);
            break;
        case LITE3D_TEXTURE_3D:
            glCompressedTexSubImage3D(textureUnit->textureTarget, level, widthOff,
                heightOff, depthOff, width, height, depth,
                textureUnit->texFormat, pixelsSize, pixels);
            break;
    }

    return lite3d_misc_check_gl_error() ? LITE3D_FALSE : LITE3D_TRUE;  
}

int lite3d_texture_unit_get_level_size(lite3d_texture_unit *textureUnit, 
    int8_t level, size_t *size)
{
#ifndef GLES
    int32_t imageWidth, imageHeight, imageDepth;

    SDL_assert(textureUnit);
    SDL_assert(size);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;
    
    /* make texture active */
    glBindTexture(textureUnit->textureTarget, textureUnit->textureID);

    imageWidth = lite3d_texture_unit_get_level_width(textureUnit, level);
    imageHeight = lite3d_texture_unit_get_level_height(textureUnit, level);
    imageDepth = lite3d_texture_unit_get_level_depth(textureUnit, level);

    *size = imageWidth * imageHeight * imageDepth * textureUnit->imageBPP;

    return LITE3D_TRUE;
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s texture dumping is not supported..",
        LITE3D_CURRENT_FUNCTION);
    return LITE3D_FALSE;
#endif    
}

int lite3d_texture_unit_get_compressed_level_size(lite3d_texture_unit *textureUnit, 
    int8_t level, size_t *size)
{
#ifndef GLES
    int32_t compressed;

    SDL_assert(textureUnit);
    SDL_assert(size);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;

    /* make texture active */
    glBindTexture(textureUnit->textureTarget, textureUnit->textureID);
    glGetTexLevelParameteriv(textureUnit->textureTarget, level,
        GL_TEXTURE_COMPRESSED, &compressed);
    
    if(compressed == GL_FALSE)
        return LITE3D_FALSE;

    glGetTexLevelParameteriv(textureUnit->textureTarget, level, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, (GLint *)size);

    return LITE3D_TRUE;
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s texture dumping is not supported..",
        LITE3D_CURRENT_FUNCTION);
    return LITE3D_FALSE;
#endif
}

int lite3d_texture_unit_get_pixels(lite3d_texture_unit *textureUnit, 
    int8_t level, void *pixels)
{
#ifndef GLES
    SDL_assert(textureUnit);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;

    /* make texture active */
    glBindTexture(textureUnit->textureTarget, textureUnit->textureID);
    lite3d_misc_gl_error_stack_clean();

    glGetTexImage(textureUnit->textureTarget, level, textureUnit->texFormat,
        GL_UNSIGNED_BYTE, pixels);

    return lite3d_misc_check_gl_error() ? LITE3D_FALSE : LITE3D_TRUE;
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s texture dumping is not supported..",
        LITE3D_CURRENT_FUNCTION);
    return LITE3D_FALSE;
#endif
}

int lite3d_texture_unit_get_compressed_pixels(lite3d_texture_unit *textureUnit, 
    int8_t level, void *pixels)
{
#ifndef GLES
    int32_t compressed;

    SDL_assert(textureUnit);
    if (textureUnit->generatedMipmaps < level)
        return LITE3D_FALSE;

    /* make texture active */
    glBindTexture(textureUnit->textureTarget, textureUnit->textureID);
    lite3d_misc_gl_error_stack_clean();

    glGetTexLevelParameteriv(textureUnit->textureTarget, level,
        GL_TEXTURE_COMPRESSED, &compressed);

    if(compressed == GL_FALSE)
        return LITE3D_FALSE;

    glGetCompressedTexImage(textureUnit->textureTarget, level, pixels);
    return lite3d_misc_check_gl_error() ? LITE3D_FALSE : LITE3D_TRUE;
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s texture dumping is not supported..",
        LITE3D_CURRENT_FUNCTION);
    return LITE3D_FALSE;
#endif
}

int lite3d_texture_unit_generate_mipmaps(lite3d_texture_unit *textureUnit)
{
    if(textureUnit->generatedMipmaps > 0)
    {
        glBindTexture(textureUnit->textureTarget, textureUnit->textureID);
        glGenerateMipmap(textureUnit->textureTarget);
        return LITE3D_TRUE;
    }
    
    return LITE3D_FALSE;
}

int lite3d_texture_unit_allocate(lite3d_texture_unit *textureUnit,
    uint32_t textureTarget, int8_t quality, uint8_t wrapping, uint16_t format,
    int32_t width, int32_t height, int32_t depth)
{
    uint32_t internalFormat;
    int32_t textureMaxLevels;
    SDL_assert(textureUnit);

    memset(textureUnit, 0, sizeof (lite3d_texture_unit));
    lite3d_misc_gl_error_stack_clean();


    textureUnit->imageType = LITE3D_IMAGE_ANY;
    textureUnit->imageWidth = width;
    textureUnit->imageHeight = height;
    textureUnit->imageDepth = depth;

    /* what BPP ? */
    switch (format)
    {
        case LITE3D_TEXTURE_FORMAT_RGB:
        case LITE3D_TEXTURE_FORMAT_BRG:
            textureUnit->imageBPP = 3;
            break;
        case LITE3D_TEXTURE_FORMAT_RGBA:
        case LITE3D_TEXTURE_FORMAT_BRGA:
        case LITE3D_TEXTURE_FORMAT_DEPTH:
            textureUnit->imageBPP = 4;
            break;
        case LITE3D_TEXTURE_FORMAT_ALPHA:
        case LITE3D_TEXTURE_FORMAT_LUMINANCE_ALPHA:
        case LITE3D_TEXTURE_FORMAT_LUMINANCE:
            textureUnit->imageBPP = 1;
            break;
        default:
            return LITE3D_FALSE;
    }

    textureUnit->imageSize = width * height * depth * textureUnit->imageBPP;
    textureUnit->loadedMipmaps = 0;
    textureUnit->generatedMipmaps = 0;
    textureUnit->textureTarget = textureTarget;
    textureUnit->wrapping = wrapping;
    textureUnit->texFormat = format;

    /* determine internal format */
    switch (textureUnit->imageBPP)
    {
        case 3:
            internalFormat = gTextureSettings.useGLCompression ?
                GL_COMPRESSED_RGB_S3TC_DXT1_EXT : 3;
            break;
        case 4:
            internalFormat = format == LITE3D_TEXTURE_FORMAT_DEPTH ? GL_DEPTH_COMPONENT :
                (gTextureSettings.useGLCompression ? GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : 4);
            break;
        default:
            internalFormat = textureUnit->imageBPP;
            break;
    }

    textureUnit->texiFormat = internalFormat;

    /* check depth texture consistency */
    if (format == LITE3D_TEXTURE_FORMAT_DEPTH && textureTarget != LITE3D_TEXTURE_2D)
        return LITE3D_FALSE;

    /* enable texture target */
    glGenTextures(1, &textureUnit->textureID);
    if (lite3d_misc_check_gl_error())
    {
        return LITE3D_FALSE;
    }

    /* make texture active */
    glBindTexture(textureTarget, textureUnit->textureID);

    if (quality == LITE3D_TEXTURE_QL_NICEST)
    {
        /* check  mipmaps consistency */
#ifdef GL_TEXTURE_MAX_LEVEL
        glGetTexParameteriv(textureTarget, GL_TEXTURE_MAX_LEVEL, &textureMaxLevels);
#else
        textureMaxLevels = 1000;
#endif
        textureUnit->generatedMipmaps = max_mipmaps_count(width,
            height, depth, textureMaxLevels);

        if (textureUnit->generatedMipmaps == 0)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: mipmaps not supported for this dimensions %dx%dx%d",
                (textureUnit->textureTarget == LITE3D_TEXTURE_1D ? "TEXTURE_1D" :
                (textureUnit->textureTarget == LITE3D_TEXTURE_2D ? "TEXTURE_2D" :
                "TEXTURE_3D")),
                width, height, depth);

            quality = LITE3D_TEXTURE_QL_MEDIUM;
        }
    }

    textureUnit->minFilter = (quality == LITE3D_TEXTURE_QL_NICEST ?
        GL_LINEAR_MIPMAP_LINEAR : (quality == LITE3D_TEXTURE_QL_LOW ?
        GL_NEAREST : GL_LINEAR));

    textureUnit->magFilter = (quality == LITE3D_TEXTURE_QL_LOW ?
        GL_NEAREST : GL_LINEAR);

    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, textureUnit->minFilter);
    glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, textureUnit->magFilter);
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_S,
        textureUnit->wrapping == LITE3D_TEXTURE_REPEAT ? GL_REPEAT : GL_CLAMP_TO_EDGE);
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_T,
        textureUnit->wrapping == LITE3D_TEXTURE_REPEAT ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    if (quality == LITE3D_TEXTURE_QL_NICEST)
    {
        if (gTextureSettings.anisotropy > 1)
        {
            /* set anisotropic angle */
            glTexParameteri(textureTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                gTextureSettings.anisotropy);
        }
    }

    /* allocate texture memory */
    switch (textureTarget)
    {
        case LITE3D_TEXTURE_1D:
            glTexImage1D(textureTarget, 0, internalFormat, width,
                0, format, GL_UNSIGNED_BYTE, NULL);
            break;
        case LITE3D_TEXTURE_2D:
            glTexImage2D(textureTarget, 0, internalFormat, width,
                height, 0, format, GL_UNSIGNED_BYTE, NULL);
            break;
        case LITE3D_TEXTURE_3D:
            glTexImage3D(textureTarget, 0, internalFormat, width,
                height, depth, 0, format, GL_UNSIGNED_BYTE, NULL);
            break;
    }

    /*
     * You must reserve memory for other mipmaps levels as well either by making a 
     * series of calls to glTexImage2D or use glGenerateMipmap(GL_TEXTURE_2D).
     * Here, we'll use :
     */
    if (quality == LITE3D_TEXTURE_QL_NICEST)
        glGenerateMipmap(textureTarget);

    /* calculate texture total size */
    if (internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT || 
        internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT)
    {
        size_t levelSize = 0;
        uint8_t li = 0;
        textureUnit->totalSize = 0;
        while(lite3d_texture_unit_get_compressed_level_size(textureUnit, li++, &levelSize))
            textureUnit->totalSize += levelSize;

        textureUnit->compressed = LITE3D_TRUE;
    }
    else
    {
        size_t levelSize = 0;
        uint8_t li = 0;
        textureUnit->totalSize = 0;
        while(lite3d_texture_unit_get_level_size(textureUnit, li++, &levelSize))
            textureUnit->totalSize += levelSize;

        textureUnit->compressed = LITE3D_FALSE;
    }

    if (lite3d_misc_check_gl_error())
    {
        lite3d_texture_unit_purge(textureUnit);
        return LITE3D_FALSE;
    }

    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "%s: "
        "%dx%dx%d surface allocated (%lu bytes), levels %d, %s, "
        "format %s",
        (textureUnit->textureTarget == LITE3D_TEXTURE_1D ? "TEXTURE_1D" :
        (textureUnit->textureTarget == LITE3D_TEXTURE_2D ? "TEXTURE_2D" :
        "TEXTURE_3D")),
        textureUnit->imageWidth,
        textureUnit->imageHeight,
        textureUnit->imageDepth,
        textureUnit->totalSize,
        textureUnit->generatedMipmaps,
        gTextureSettings.useGLCompression ?
        (internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ?
        "Compressed DXT1" : "Compressed DXT5") : "Not compressed",
        format_string(format));

    textureUnit->isFbAttachment = LITE3D_FALSE;
    return LITE3D_TRUE;
}

void lite3d_texture_unit_purge(lite3d_texture_unit *textureUnit)
{
    SDL_assert(textureUnit);
    glDeleteTextures(1, &textureUnit->textureID);
    textureUnit->textureID = 0;
}

void lite3d_texture_unit_bind(lite3d_texture_unit *textureUnit, uint16_t layer)
{
    SDL_assert(textureUnit);
    SDL_assert_release(layer < maxCombinedTextureImageUnits);

    glActiveTexture(GL_TEXTURE0 + layer);
    glBindTexture(textureUnit->textureTarget, textureUnit->textureID);

    if (textureUnit->isFbAttachment &&
        textureUnit->generatedMipmaps > 0)
        glGenerateMipmap(textureUnit->textureTarget);
}

void lite3d_texture_unit_unbind(lite3d_texture_unit *textureUnit, uint16_t layer)
{
    SDL_assert(textureUnit);
    SDL_assert_release(layer < maxCombinedTextureImageUnits);

    glActiveTexture(GL_TEXTURE0 + layer);
    glBindTexture(textureUnit->textureTarget, 0);
}

void lite3d_texture_unit_compression(uint8_t on)
{
    gTextureSettings.useGLCompression = on;
}

int32_t lite3d_texture_unit_get_level_width(lite3d_texture_unit *textureUnit,
    int8_t level)
{
#ifndef GLES
    int32_t result = 0;
    SDL_assert(textureUnit);
    glGetTexLevelParameteriv(textureUnit->textureTarget, level,
        GL_TEXTURE_WIDTH, &result);
    return result;
#else
    SDL_assert(textureUnit);
    return textureUnit->imageWidth >> level;
#endif
}

int32_t lite3d_texture_unit_get_level_height(lite3d_texture_unit *textureUnit,
    int8_t level)
{
#ifndef GLES
    int32_t result = 0;
    SDL_assert(textureUnit);
    glGetTexLevelParameteriv(textureUnit->textureTarget, level,
        GL_TEXTURE_HEIGHT, &result);
    return result;
#else
    SDL_assert(textureUnit);
    return textureUnit->imageHeight >> level;
#endif
}

int32_t lite3d_texture_unit_get_level_depth(lite3d_texture_unit *textureUnit,
    int8_t level)
{
#ifndef GLES
    int32_t result = 0;
    SDL_assert(textureUnit);
    glGetTexLevelParameteriv(textureUnit->textureTarget, level,
        GL_TEXTURE_DEPTH, &result);
    return result;
#else
    SDL_assert(textureUnit);
    return textureUnit->imageDepth >> level;
#endif
}
