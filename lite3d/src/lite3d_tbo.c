/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024 Sirius (Korolev Nikita)
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
#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_tbo.h>

extern GLenum textureTargetEnum[];

/*
Overview

    This extension provides a new texture type, called a buffer texture.
    Buffer textures are one-dimensional arrays of texels whose storage comes
    from an attached buffer object.  When a buffer object is bound to a buffer
    texture, a format is specified, and the data in the buffer object is
    treated as an array of texels of the specified format.

    The use of a buffer object to provide storage allows the texture data to
    be specified in a number of different ways:  via buffer object loads
    (BufferData), direct CPU writes (MapBuffer), framebuffer readbacks
    (EXT_pixel_buffer_object extension).  A buffer object can also be loaded
    by transform feedback (NV_transform_feedback extension), which captures
    selected transformed attributes of vertices processed by the GL.  Several
    of these mechanisms do not require an extra data copy, which would be
    required when using conventional TexImage-like entry points.

    Buffer textures do not support mipmapping, texture lookups with normalized
    floating-point texture coordinates, and texture filtering of any sort, and
    may not be used in fixed-function fragment processing.  They can be
    accessed via single texel fetch operations in programmable shaders.  For
    assembly shaders (NV_gpu_program4), the TXF instruction is used.  For GLSL
    (EXT_gpu_shader4), a new sampler type and texel fetch function are used.

    While buffer textures can be substantially larger than equivalent
    one-dimensional textures; the maximum texture size supported for buffer
    textures in the initial implementation of this extension is 2^27 texels,
    versus 2^13 (8192) texels for otherwise equivalent one-dimensional
    textures.  When a buffer object is attached to a buffer texture, a size is
    not specified; rather, the number of texels in the texture is taken by
    dividing the size of the buffer object by the size of each texel.
*/

int lite3d_texture_buffer_init(lite3d_texture_unit *textureUnit, 
    uint32_t texelsCount, const void *data, uint16_t bf, uint16_t usage)
{
    int TBOMaxSize;
    if (!lite3d_check_tbo())
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: Texture buffer object not supported..", LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }
    
    SDL_assert(textureUnit);
    memset(textureUnit, 0, sizeof(lite3d_texture_unit));
    lite3d_misc_gl_error_stack_clean();

    textureUnit->imageType = LITE3D_IMAGE_ANY;

    /* what BPP ? */
    if ((textureUnit->imageBPP = lite3d_texture_buffer_texel_size(bf)) == 0)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: Invalid buffer format", LITE3D_CURRENT_FUNCTION);
        return LITE3D_FALSE;
    }

    textureUnit->totalSize = textureUnit->imageSize = texelsCount * textureUnit->imageBPP;
    textureUnit->textureTarget = LITE3D_TEXTURE_BUFFER;

    lite3d_vbo_get_limitations(NULL, &TBOMaxSize, NULL);
    if (TBOMaxSize > 0 && textureUnit->totalSize > TBOMaxSize)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: TBO is too large, limit is %d bytes, requested %zu bytes", LITE3D_CURRENT_FUNCTION,
            TBOMaxSize, textureUnit->totalSize);
        return LITE3D_FALSE;
    }

    if (!lite3d_vbo_init(&textureUnit->tbo, usage))
        return LITE3D_FALSE;
    if (!lite3d_vbo_buffer(&textureUnit->tbo, data, textureUnit->imageSize))
        return LITE3D_FALSE;
    
    glGenTextures(1, &textureUnit->textureID);
    glBindTexture(textureTargetEnum[textureUnit->textureTarget], textureUnit->textureID);
    glTexBuffer(textureTargetEnum[textureUnit->textureTarget], bf, textureUnit->tbo.vboID);
    
    if (LITE3D_CHECK_GL_ERROR)
    {
        lite3d_vbo_purge(&textureUnit->tbo);
        lite3d_texture_unit_purge(textureUnit);
        return LITE3D_FALSE;
    }
    
    textureUnit->isTextureBuffer = LITE3D_TRUE;
    textureUnit->internalFormat = bf;
    return LITE3D_TRUE;
}

int lite3d_texture_buffer_purge(lite3d_texture_unit *textureUnit)
{
    SDL_assert(textureUnit);

    lite3d_vbo_purge(&textureUnit->tbo);
    lite3d_texture_unit_purge(textureUnit);
    return LITE3D_TRUE;
}

int8_t lite3d_texture_buffer_texel_size(uint16_t bf)
{
    switch (bf)
    {
        case LITE3D_TEXTURE_INTERNAL_R8:
        case LITE3D_TEXTURE_INTERNAL_R8I:
        case LITE3D_TEXTURE_INTERNAL_R8UI:
            return 1 * 1;
        case LITE3D_TEXTURE_INTERNAL_R16:
        case LITE3D_TEXTURE_INTERNAL_R16F:
        case LITE3D_TEXTURE_INTERNAL_R16I:
        case LITE3D_TEXTURE_INTERNAL_R16UI:
            return 1 * 2;
        case LITE3D_TEXTURE_INTERNAL_R32I:
        case LITE3D_TEXTURE_INTERNAL_R32F:
        case LITE3D_TEXTURE_INTERNAL_R32UI:
            return 1 * 4;
        case LITE3D_TEXTURE_INTERNAL_RG8:
        case LITE3D_TEXTURE_INTERNAL_RG8I:
        case LITE3D_TEXTURE_INTERNAL_RG8UI:
            return 2 * 1;
        case LITE3D_TEXTURE_INTERNAL_RG16:
        case LITE3D_TEXTURE_INTERNAL_RG16F:
        case LITE3D_TEXTURE_INTERNAL_RG16I:
        case LITE3D_TEXTURE_INTERNAL_RG16UI:
            return 2 * 2;
        case LITE3D_TEXTURE_INTERNAL_RG32F:
        case LITE3D_TEXTURE_INTERNAL_RG32I:
        case LITE3D_TEXTURE_INTERNAL_RG32UI:
            return 2 * 4;
        case LITE3D_TEXTURE_INTERNAL_RGB32F:
        case LITE3D_TEXTURE_INTERNAL_RGB32I:
        case LITE3D_TEXTURE_INTERNAL_RGB32UI:
            return 3 * 4;
        case LITE3D_TEXTURE_INTERNAL_RGBA8:
        case LITE3D_TEXTURE_INTERNAL_RGBA8I:
        case LITE3D_TEXTURE_INTERNAL_RGBA8UI:
            return 4 * 1;
        case LITE3D_TEXTURE_INTERNAL_RGBA16:
        case LITE3D_TEXTURE_INTERNAL_RGBA16F:
        case LITE3D_TEXTURE_INTERNAL_RGBA16I:
        case LITE3D_TEXTURE_INTERNAL_RGBA16UI:
            return 4 * 2;
        case LITE3D_TEXTURE_INTERNAL_RGBA32F:
        case LITE3D_TEXTURE_INTERNAL_RGBA32I:
        case LITE3D_TEXTURE_INTERNAL_RGBA32UI:
            return 4 * 4;
    }
    
    return 0;
}

void *lite3d_texture_buffer_map(lite3d_texture_unit *textureUnit, uint16_t access)
{
    SDL_assert(textureUnit);
    return lite3d_vbo_map(&textureUnit->tbo, access);
}

void lite3d_texture_buffer_unmap(lite3d_texture_unit *textureUnit)
{
    SDL_assert(textureUnit);
    lite3d_vbo_unmap(&textureUnit->tbo);
}

int lite3d_texture_buffer(lite3d_texture_unit *textureUnit,
    const void *buffer, size_t offset, size_t size)
{
    SDL_assert(textureUnit);
    return lite3d_vbo_subbuffer(&textureUnit->tbo, buffer, offset, size);
}

int lite3d_texture_buffer_get(const lite3d_texture_unit *textureUnit,
    void *buffer, size_t offset, size_t size)
{
    SDL_assert(textureUnit);
    return lite3d_vbo_get_buffer(&textureUnit->tbo, buffer, offset, size);
}

int lite3d_texture_buffer_extend(lite3d_texture_unit *textureUnit, size_t addSize)
{
    int TBOMaxSize;
    SDL_assert(textureUnit);
    
    lite3d_vbo_get_limitations(NULL, &TBOMaxSize, NULL);
    if (TBOMaxSize > 0 && textureUnit->totalSize + addSize > TBOMaxSize)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: TBO is too large, limit is %d bytes, requested %zu bytes", LITE3D_CURRENT_FUNCTION,
            TBOMaxSize, textureUnit->totalSize + addSize);
        return LITE3D_FALSE;
    }

    if (lite3d_vbo_extend(&textureUnit->tbo, addSize))
    {
        textureUnit->totalSize += addSize;
        textureUnit->imageSize += addSize;
        return LITE3D_TRUE;
    }

    return LITE3D_FALSE;
}
