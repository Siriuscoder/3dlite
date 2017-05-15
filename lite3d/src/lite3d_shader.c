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
#include <SDL_assert.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_shader.h>

int lite3d_shader_init(lite3d_shader *shader, uint8_t type)
{
    SDL_assert(shader);
    
    lite3d_misc_gl_error_stack_clean();
    /* cleanup shader firt */
    if (glIsShader(shader->shaderID))
        lite3d_shader_purge(shader);

    /* craate new shader object */
    shader->shaderID = glCreateShader(type == LITE3D_SHADER_TYPE_VERTEX ?
        GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
    shader->type = type;

    if (lite3d_misc_check_gl_error())
    {
        glDeleteShader(shader->shaderID);
        shader->shaderID = 0;
        return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

int lite3d_shader_compile(
    lite3d_shader *shader, uint32_t sources, const char **source, int32_t *length)
{
    GLint isCompiled = 0;
    GLint maxLogLength = 0;
    SDL_assert(shader);

    lite3d_misc_gl_error_stack_clean();
    if (!glIsShader(shader->shaderID))
        return LITE3D_FALSE;

    /* setup source and compile it */
    glShaderSource(shader->shaderID, sources, source, length ? (GLint *)length : NULL);
    glCompileShader(shader->shaderID);

    lite3d_misc_check_gl_error();

    /* check compile status */
    glGetShaderiv(shader->shaderID, GL_COMPILE_STATUS, &isCompiled);
    shader->success = isCompiled == GL_TRUE ? LITE3D_TRUE : LITE3D_FALSE;

    /* allocate string and copy compile info log into it */
    glGetShaderiv(shader->shaderID, GL_INFO_LOG_LENGTH, &maxLogLength);
    shader->statusString = (char *) lite3d_malloc(maxLogLength);
    glGetShaderInfoLog(shader->shaderID, maxLogLength, &maxLogLength, shader->statusString);

    return shader->success;
}

void lite3d_shader_purge(
    lite3d_shader *shader)
{
    SDL_assert(shader);

    if (glIsShader(shader->shaderID))
    {
        lite3d_free(shader->statusString);
        glDeleteShader(shader->shaderID);
        shader->shaderID = 0;
    }
}