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

#include <SDL_assert.h>
#include <SDL_log.h>

#include <IL/il.h>
#include <IL/ilu.h>

#include <3dlite/GL/glew.h>

#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_misc.h>
#include <3dlite/3dlite_texture_unit.h>

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
    "LUMINANCE_ALPHA"
};

static lite3d_image_filter gFilters[LITE3D_MAX_FILTERS];
static int8_t gFiltersCount = 0;
static int maxTextureSize;
static int maxTextureImageUnits;
static int maxCombinedTextureImageUnits;

static void* DEVIL_CALL il_alloc(const ILsizei size)
{
    return lite3d_malloc_pooled(LITE3D_POOL_NO2, size);
}

static void DEVIL_CALL il_free(const void *ptr)
{
    lite3d_free_pooled(LITE3D_POOL_NO2, (void *) ptr);
}

static const char *format_string(GLenum format)
{
    switch (format)
    {
        case GL_COLOR_INDEX:
            return glTextureFormats[1];
        case GL_ALPHA:
            return glTextureFormats[2];
        case GL_RGB:
            return glTextureFormats[3];
        case GL_RGBA:
            return glTextureFormats[4];
        case GL_BGR:
            return glTextureFormats[5];
        case GL_BGRA:
            return glTextureFormats[6];
        case GL_LUMINANCE:
            return glTextureFormats[7];
        case GL_LUMINANCE_ALPHA:
            return glTextureFormats[8];
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
                iluBlurAvg(gFilters[i].param1.intVal);
                break;
            case LITE3D_BLURGAUSSIAN_FILTER:
                iluBlurGaussian(gFilters[i].param1.intVal);
                break;
            case LITE3D_CONTRAST_FILTER:
                iluContrast(gFilters[i].param1.floatVal);
                break;
            case LITE3D_GAMMACORRECT_FILTER:
                iluGammaCorrect(gFilters[i].param1.floatVal);
                break;
            case LITE3D_MIRROR_FILTER:
                iluMirror();
                break;
            case LITE3D_NEGATIVE_FILTER:
                iluNegative();
                break;
            case LITE3D_NOISIFY_FILTER:
                iluNoisify(gFilters[i].param1.floatVal);
                break;
            case LITE3D_PIXELIZE_FILTER:
                iluPixelize(gFilters[i].param1.intVal);
                break;
            case LITE3D_WAVE_FILTER:
                iluWave(gFilters[i].param1.floatVal);
                break;
            case LITE3D_SHARPEN_FILTER:
                iluSharpen(gFilters[i].param1.floatVal,
                    gFilters[i].param2.intVal);
                break;
        }
    }
}

void lite3d_texture_technique_add_image_filter(lite3d_image_filter *filter)
{
    SDL_assert(filter);

    if (gFiltersCount >= LITE3D_MAX_FILTERS)
        lite3d_texture_technique_reset_filters();

    gFilters[gFiltersCount] = *filter;
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
        if (!GLEW_ARB_texture_compression || !GLEW_EXT_texture_compression_s3tc)
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: Texture compression not supported, skiping", __FUNCTION__);
            gTextureSettings.useGLCompression = 0;
        }
    }

    if (!GLEW_SGIS_generate_mipmap)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: SGIS_generate_mipmap not supported..", __FUNCTION__);
        return LITE3D_FALSE;
    }

    if (!GLEW_EXT_texture_filter_anisotropic)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: EXT_texture_filter_anisotropic not supported..", __FUNCTION__);
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
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Max texture image units: %d",
        maxTextureImageUnits);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Max combined texture image units: %d",
        maxCombinedTextureImageUnits);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Max texture size: %d",
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

int lite3d_texture_unit_from_resource(lite3d_texture_unit *texture,
    const lite3d_resource_file *resource, uint32_t imageType,
    uint32_t textureTarget, int8_t quality, uint8_t wrapping)
{
    SDL_assert(resource);

    if (!resource->isLoaded || resource->fileSize == 0)
        return LITE3D_FALSE;

    return lite3d_texture_unit_from_memory(texture, resource->fileBuff,
        resource->fileSize, imageType, textureTarget, quality, wrapping);
}

int lite3d_texture_unit_from_memory(lite3d_texture_unit *textureUnit, const void *buffer,
    size_t size, uint32_t imageType, uint32_t textureTarget, int8_t quality, uint8_t wrapping)
{
    ILuint imageDesc = 0, imageFormat, internalFormat;
    GLint mipLevel = 0;
    int32_t imageHeight, imageWidth, imageDepth;

    SDL_assert(buffer);
    SDL_assert(size > 0);
    SDL_assert(textureUnit);

    memset(textureUnit, 0, sizeof (lite3d_texture_unit));
    lite3d_misc_il_error_stack_clean();
    /* gen IL image */
    imageDesc = ilGenImage();
    if (lite3d_misc_check_il_error())
        return LITE3D_FALSE;

    /* Bind IL image */
    ilBindImage(imageDesc);
    /* Load IL image from memory */
    if (!ilLoadL(imageType, buffer, size))
    {
        lite3d_misc_check_il_error();
        return LITE3D_FALSE;
    }

    apply_image_filters();

    if (imageType == LITE3D_IMAGE_ANY)
    {
        imageType = ilGetInteger(IL_IMAGE_TYPE);
    }

    textureUnit->imageType = imageType;
    textureUnit->imageWidth = imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
    textureUnit->imageHeight = imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
    textureUnit->imageDepth = imageDepth = ilGetInteger(IL_IMAGE_DEPTH);
    textureUnit->imageBPP = (int8_t) ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
    /* first mipmap size */
    textureUnit->imageSize = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);
    textureUnit->loadedMipmaps = 0;
    textureUnit->textureTarget = textureTarget;
    textureUnit->wrapping = wrapping;
    /* matches openGL texture format */
    imageFormat = ilGetInteger(IL_IMAGE_FORMAT);

    switch (textureUnit->imageBPP)
    {
        case 3:
            internalFormat = gTextureSettings.useGLCompression ?
                GL_COMPRESSED_RGB_S3TC_DXT1_EXT : 3;
            break;
        case 4:
            internalFormat = gTextureSettings.useGLCompression ?
                GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : 4;
            break;
        default:
            internalFormat = textureUnit->imageBPP;
            break;
    }

    /* enable texture target */
    glGenTextures(1, &textureUnit->textureID);
    if (lite3d_misc_check_gl_error())
    {
        ilDeleteImages(1, &imageDesc);
        return LITE3D_FALSE;
    }

    /* make texture active */
    glBindTexture(textureTarget, textureUnit->textureID);

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
    glTexParameteri(textureTarget, GL_TEXTURE_WRAP_R,
        textureUnit->wrapping == LITE3D_TEXTURE_REPEAT ? GL_REPEAT : GL_CLAMP_TO_EDGE);

    /* calc saved mipmaps in image */
    textureUnit->loadedMipmaps = ilGetInteger(IL_NUM_MIPMAPS);
    //while (ilActiveMipmap(textureUnit->loadedMipmaps + 1))
    //    textureUnit->loadedMipmaps++;
    /* if were no saved mipmaps - generate it */
    glTexParameteri(textureTarget, GL_GENERATE_MIPMAP_SGIS,
        textureUnit->loadedMipmaps == 0 ? GL_TRUE : GL_FALSE);

    glTexParameteri(textureTarget, GL_TEXTURE_BASE_LEVEL, 0);
    /* Specifies the alignment requirements 
     * for the start of each pixel row in memory.*/
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    if (quality == LITE3D_TEXTURE_QL_NICEST)
    {
        if (gTextureSettings.anisotropy > 1)
        {
            /* set anisotropic angle */
            glTexParameteri(textureTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                gTextureSettings.anisotropy);
        }
    }

    for (mipLevel = 0; mipLevel <= textureUnit->loadedMipmaps; ++mipLevel)
    {
        /* workaround to prevent ilActiveMipmap bug */
        ilBindImage(imageDesc);
        ilActiveMipmap(mipLevel);

        imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
        imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
        imageDepth = ilGetInteger(IL_IMAGE_DEPTH);

        switch (textureTarget)
        {
            case LITE3D_TEXTURE_1D:
                glTexImage1D(textureTarget, mipLevel, internalFormat, imageWidth,
                    0, imageFormat, GL_UNSIGNED_BYTE, ilGetData());
                break;
            case LITE3D_TEXTURE_2D:
                glTexImage2D(textureTarget, mipLevel, internalFormat, imageWidth,
                    imageHeight, 0, imageFormat, GL_UNSIGNED_BYTE, ilGetData());
                break;
            case LITE3D_TEXTURE_3D:
                glTexImage3D(textureTarget, mipLevel, internalFormat, imageWidth,
                    imageHeight, imageDepth, 0, imageFormat, GL_UNSIGNED_BYTE,
                    ilGetData());
                break;
        }
    }

    if (lite3d_misc_check_gl_error())
    {
        ilDeleteImages(1, &imageDesc);
        return LITE3D_FALSE;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s: "
        "%dx%dx%d, build-in mipmaps %d, %s, "
        "format %s",
        (textureUnit->textureTarget == LITE3D_TEXTURE_1D ? "TEXTURE_1D" :
        (textureUnit->textureTarget == LITE3D_TEXTURE_2D ? "TEXTURE_2D" :
        "TEXTURE_3D")),
        textureUnit->imageWidth,
        textureUnit->imageHeight,
        textureUnit->imageDepth,
        textureUnit->loadedMipmaps,
        gTextureSettings.useGLCompression ?
        (internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ?
        "Compressed DXT1" : "Compressed DXT5") : "Plain",
        format_string(imageFormat));

    glTexParameteri(textureTarget, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);
    /* release IL image */
    ilDeleteImage(imageDesc);

    textureUnit->isFbAttachment = LITE3D_FALSE;
    glBindTexture(textureTarget, 0);

    return LITE3D_TRUE;
}

void lite3d_texture_unit_purge(lite3d_texture_unit *texture)
{
    SDL_assert(texture);
    glDeleteTextures(1, &texture->textureID);
    texture->textureID = 0;
}

void lite3d_texture_unit_bind(lite3d_texture_unit *texture, uint16_t layer)
{
    SDL_assert(texture);
    SDL_assert_release(layer < maxCombinedTextureImageUnits);

    glActiveTexture(GL_TEXTURE0 + layer);
    glBindTexture(texture->textureTarget, texture->textureID);
    
    if(texture->isFbAttachment)
        glGenerateMipmapEXT(texture->textureTarget);
}

void lite3d_texture_unit_unbind(lite3d_texture_unit *texture, uint16_t layer)
{
    SDL_assert(texture);
    SDL_assert_release(layer < maxCombinedTextureImageUnits);

    glActiveTexture(GL_TEXTURE0 + layer);
    glBindTexture(texture->textureTarget, 0);
}

