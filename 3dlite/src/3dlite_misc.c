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
#include <3dlite/GL/glew.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <SDL_log.h>

#include <3dlite/3dlite_misc.h>

void lite3d_misc_gl_error_stack_clean(void)
{
    while (glGetError() != GL_NO_ERROR);
}

void lite3d_misc_il_error_stack_clean(void)
{
    while (ilGetError() != IL_NO_ERROR);
}

int lite3d_misc_check_gl_error(void)
{
    GLenum errNo;
    uint8_t err = LITE3D_FALSE;

    while ((errNo = glGetError()) != GL_NO_ERROR)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "GL: code %d(%s)", errNo,
                errNo == GL_INVALID_ENUM ? "GL_INVALID_ENUM" : 
                (errNo == GL_INVALID_VALUE ? "GL_INVALID_VALUE" : 
                (errNo == GL_INVALID_OPERATION ? "GL_INVALID_OPERATION" : 
                (errNo == GL_INVALID_FRAMEBUFFER_OPERATION ? "GL_INVALID_FRAMEBUFFER_OPERATION" : 
                (errNo == GL_OUT_OF_MEMORY ? "GL_OUT_OF_MEMORY" : 
                (errNo == GL_STACK_UNDERFLOW ? "GL_STACK_UNDERFLOW" : "GL_STACK_OVERFLOW"))))));

        err = LITE3D_TRUE;
    }

    return err;
}

int lite3d_misc_check_il_error(void)
{
    ILenum errNo;
    uint8_t err = LITE3D_FALSE;

    while ((errNo = ilGetError()) != IL_NO_ERROR)
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "IL: %s", iluErrorString(errNo));
      
        err = LITE3D_TRUE;
    }

    return err;
}