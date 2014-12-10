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

#include <3dlite/alloc.h>
#include <3dlite/gl_texture_unit.h>

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

static void* DEVIL_CALL ilAlloc(const ILsizei size)
{
    return lite3d_malloc_pooled(LITE3D_POOL_NO2, size);
}

static void DEVIL_CALL ilFree(const void *ptr)
{
    lite3d_free_pooled(LITE3D_POOL_NO2, (void *) ptr);
}

static const char *glFormatString(GLenum format)
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
    for(i = 0; i < gFiltersCount; ++i)
    {
        switch(gFilters[i].filterID)
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

    if(gFiltersCount >= LITE3D_MAX_FILTERS)
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
        return 0;
    }

    if (!GLEW_EXT_texture_filter_anisotropic)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: EXT_texture_filter_anisotropic not supported..", __FUNCTION__);
        return 0;
    }
    else
    {
        glGetIntegerv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,
            &gTextureSettings.maxAnisotropy);
        if (gTextureSettings.anisotropy > gTextureSettings.maxAnisotropy)
            gTextureSettings.anisotropy = gTextureSettings.maxAnisotropy;
    }

    ilSetMemory(ilAlloc, ilFree);
    ilInit();
    iluInit();

    glEnable(GL_TEXTURE_1D);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_TEXTURE_3D);
    lite3d_texture_technique_reset_filters();
    return 1;
}

lite3d_texture_unit *lite3d_texture_unit_from_resource(const lite3d_resource_file *resource,
    uint32_t imageType, uint32_t textureTarget)
{
    SDL_assert(resource);

    if (!resource->isLoaded || resource->fileSize == 0)
        return NULL;

    return lite3d_texture_unit_from_memory(resource->name, resource->fileBuff,
        resource->fileSize, imageType, textureTarget);
}

lite3d_texture_unit *lite3d_texture_unit_from_memory(const char *textureName,
    const void *buffer, size_t size, uint32_t imageType, uint32_t textureTarget)
{
    ILuint imageDesc, imageFormat, internalFormat;
    GLint mipLevel = 0;
    ILenum errNo;
    int32_t imageHeight, imageWidth, imageDepth;
    lite3d_texture_unit *textureUnit;

    SDL_assert(buffer);
    SDL_assert(size > 0);
    /* gen IL image */
    ilGenImages(1, &imageDesc);

    if ((errNo = ilGetError()) != IL_NO_ERROR)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: %s", __FUNCTION__, iluErrorString(errNo));
        return NULL;
    }

    /* Bind IL image */
    ilBindImage(imageDesc);
    /*   
        if(gTextureSettings.useGLCompression)
            ilEnable(IL_KEEP_DXTC_DATA);
        else
            ilDisable(IL_KEEP_DXTC_DATA);
     */
    /* Load IL image from memory */
    if (!ilLoadL(imageType, buffer, size))
    {
        errNo = ilGetError();
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: %s", __FUNCTION__, iluErrorString(errNo));
        return NULL;
    }

    apply_image_filters();

    if (imageType == LITE3D_IMAGE_ANY)
    {
        imageType = ilGetInteger(IL_IMAGE_TYPE);
    }

    textureUnit =
        (lite3d_texture_unit *) lite3d_calloc(sizeof (lite3d_texture_unit));
    SDL_assert_release(textureUnit);

    strcpy(textureUnit->textureName, textureName);
    textureUnit->imageType = imageType;
    textureUnit->imageWidth = imageWidth = ilGetInteger(IL_IMAGE_WIDTH);
    textureUnit->imageHeight = imageHeight = ilGetInteger(IL_IMAGE_HEIGHT);
    textureUnit->imageDepth = imageDepth = ilGetInteger(IL_IMAGE_DEPTH);
    textureUnit->imageBPP = (int8_t)ilGetInteger(IL_IMAGE_BYTES_PER_PIXEL);
    /* first mipmap size */
    textureUnit->imageSize = ilGetInteger(IL_IMAGE_SIZE_OF_DATA);
    textureUnit->loadedMipmaps = 0;
    /* matches openGL texture format */
    imageFormat = ilGetInteger(IL_IMAGE_FORMAT);

    switch (textureUnit->imageBPP)
    {
        case 3:
            internalFormat = gTextureSettings.useGLCompression ?
                GL_COMPRESSED_RGB_S3TC_DXT1_EXT : GL_RGB;
            break;
        case 4:
            internalFormat = gTextureSettings.useGLCompression ?
                GL_COMPRESSED_RGBA_S3TC_DXT5_EXT : GL_RGBA;
            break;
        default:
            internalFormat = textureUnit->imageBPP;
            break;
    }

    /* enable texture target */
    glGenTextures(1, &textureUnit->textureID);
    if ((errNo = glGetError()) != GL_NO_ERROR)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: %s, code %d", __FUNCTION__, glewGetErrorString(errNo), errNo);
        ilDeleteImages(1, &imageDesc);
        lite3d_free(textureUnit);
        return NULL;
    }

    /* make texture active */
    glBindTexture(textureTarget, textureUnit->textureID);
    /* set trilinear filtration */
    textureUnit->minFilter = GL_LINEAR_MIPMAP_LINEAR;
    textureUnit->magFilter = GL_LINEAR;

    glTexParameteri(textureTarget, GL_TEXTURE_MIN_FILTER, textureUnit->minFilter);
    glTexParameteri(textureTarget, GL_TEXTURE_MAG_FILTER, textureUnit->magFilter);

    /* calc saved mipmaps in image */
    while (ilActiveMipmap(textureUnit->loadedMipmaps))
        textureUnit->loadedMipmaps++;
    /* if were no saved mipmaps - generate it */
    glTexParameteri(textureTarget, GL_GENERATE_MIPMAP_SGIS, 
        textureUnit->loadedMipmaps == 1 ? GL_TRUE : GL_FALSE);

    /* set anisotropic angle */
    glTexParameteri(textureTarget, GL_TEXTURE_MAX_ANISOTROPY_EXT,
        gTextureSettings.anisotropy);
    /* Specifies the alignment requirements 
     * for the start of each pixel row in memory.*/
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (mipLevel = 0; ilActiveMipmap(mipLevel) == IL_TRUE; ++mipLevel)
    {
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
    
    glTexParameteri(textureTarget, GL_GENERATE_MIPMAP_SGIS, GL_FALSE);

    if ((errNo = glGetError()) != GL_NO_ERROR)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: %s, code %d", __FUNCTION__, glewGetErrorString(errNo), errNo);
        ilDeleteImages(1, &imageDesc);
        lite3d_free(textureUnit);
        return NULL;
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Texture %s (%s)loaded: "
        "width %d, height %d, depth %d, also loaded mipmaps levels %d, %s, "
        "format %s", textureUnit->textureName,
        (textureUnit->textureTarget == LITE3D_TEXTURE_1D ? "TEXTURE_1D" :
        (textureUnit->textureTarget == LITE3D_TEXTURE_2D ? "TEXTURE_2D" :
        "TEXTURE_3D")),
        textureUnit->imageWidth,
        textureUnit->imageHeight,
        textureUnit->imageDepth,
        textureUnit->loadedMipmaps - 1,
        gTextureSettings.useGLCompression ?
        (internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT ?
        "Compressed DXT1" : "Compressed DXT5") : "Plain",
        glFormatString(imageFormat));

    /* release IL image */
    ilDeleteImages(1, &imageDesc);
    return textureUnit;
}

void lite3d_texture_unit_purge(lite3d_texture_unit *texture)
{
    SDL_assert(texture);
    glDeleteTextures(1, &texture->textureID);
    lite3d_free(texture);
}

