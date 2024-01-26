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
#include <SDL_log.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
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
    switch (type)
    {
    case LITE3D_SHADER_TYPE_VERTEX:
        shader->shaderID = glCreateShader(GL_VERTEX_SHADER);
        break;
    case LITE3D_SHADER_TYPE_FRAGMENT:
        shader->shaderID = glCreateShader(GL_FRAGMENT_SHADER);
        break;
    case LITE3D_SHADER_TYPE_GEOMETRY:
        if (!lite3d_check_geometry_shader())
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Geometry shaders are not supported", LITE3D_CURRENT_FUNCTION);
            return LITE3D_FALSE;
        }
        shader->shaderID = glCreateShader(GL_GEOMETRY_SHADER);
        break;
    default:
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "%s: Unknown shader type '%d'", LITE3D_CURRENT_FUNCTION, type);
        return LITE3D_FALSE;
    }

    shader->type = type;
    if (LITE3D_CHECK_GL_ERROR)
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

    LITE3D_CHECK_GL_ERROR;

    /* check compile status */
    glGetShaderiv(shader->shaderID, GL_COMPILE_STATUS, &isCompiled);
    shader->success = isCompiled == GL_TRUE ? LITE3D_TRUE : LITE3D_FALSE;

    /* allocate string and copy compile info log into it */
    glGetShaderiv(shader->shaderID, GL_INFO_LOG_LENGTH, &maxLogLength);
    shader->statusString = maxLogLength > 0 ? (char *) lite3d_calloc(maxLogLength) : NULL;
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
