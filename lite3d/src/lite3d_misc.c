/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025  Sirius (Korolev Nikita)
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
#include <IL/il.h>
#include <IL/ilu.h>
#include <SDL_log.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_misc.h>

static uint8_t glOutOfMemory = LITE3D_FALSE;
static uint8_t glStackCorruption = LITE3D_FALSE;
static uint8_t glNotSupported = LITE3D_FALSE;

void lite3d_misc_gl_error_stack_clean(void)
{
#ifndef NDEBUG
    glNotSupported = LITE3D_FALSE;
    while (glGetError() != GL_NO_ERROR);
#endif
}

void lite3d_misc_il_error_stack_clean(void)
{
#ifndef NDEBUG
    while (ilGetError() != IL_NO_ERROR);
#endif
}

int lite3d_misc_check_gl_error(const char *func, int line)
{
    GLenum errNo;
    uint8_t err = LITE3D_FALSE;

    if (glNotSupported)
    {
        glNotSupported = LITE3D_FALSE;
        return LITE3D_TRUE;
    }

    while ((errNo = glGetError()) != GL_NO_ERROR)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s:%d %s (0x%X)", func, line,
            errNo == GL_INVALID_ENUM ? "GL_INVALID_ENUM" :
            (errNo == GL_INVALID_VALUE ? "GL_INVALID_VALUE" :
            (errNo == GL_INVALID_OPERATION ? "GL_INVALID_OPERATION" :
            (errNo == GL_INVALID_FRAMEBUFFER_OPERATION ? "GL_INVALID_FRAMEBUFFER_OPERATION" :
            (errNo == GL_OUT_OF_MEMORY ? "GL_OUT_OF_MEMORY" :
            (errNo == GL_STACK_UNDERFLOW ? "GL_STACK_UNDERFLOW" : "GL_STACK_OVERFLOW"))))), errNo);

        if (errNo == GL_OUT_OF_MEMORY)
            glOutOfMemory = LITE3D_TRUE;
        else if (errNo == GL_STACK_OVERFLOW || errNo == GL_STACK_UNDERFLOW)
            glStackCorruption = LITE3D_TRUE;

        err = LITE3D_TRUE;
    }

    return err;
}

int lite3d_misc_check_il_error(const char *func, int line)
{
    ILenum errNo;
    uint8_t err = LITE3D_FALSE;

    while ((errNo = ilGetError()) != IL_NO_ERROR)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s:%d IL %s", func, line, iluErrorString(errNo));

        err = LITE3D_TRUE;
    }

    return err;
}

int lite3d_misc_check_gl_out_of_mem(void)
{
    return glOutOfMemory;
}

int lite3d_misc_check_gl_stack_corruption(void)
{
    return glStackCorruption;
}

void lite3d_misc_gl_out_of_mem_reset(void)
{
    glOutOfMemory = LITE3D_FALSE;
}

void lite3d_misc_gl_stack_corruption_reset(void)
{
    glStackCorruption = LITE3D_FALSE;
}

void lite3d_misc_gl_set_not_supported(void)
{
    glNotSupported = LITE3D_TRUE;
}
