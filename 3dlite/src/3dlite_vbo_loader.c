/******************************************************************************
*	This file is part of 3dlite (Light-weight 3d engine).
*	Copyright (C) 2015  Sirius (Korolev Nikita)
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

#include <SDL_log.h>
#include <SDL_assert.h>

#include <3dlite/GL/glew.h>
#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_misc.h>
#include <3dlite/3dlite_vbo_loader.h>

int lite3d_vbo_load_from_memory(lite3d_vbo *vbo, 
    void *vertices, 
    size_t verticesCount, 
    lite3d_buffer_component *layout,
    size_t layoutCount,
    void *indexes, 
    size_t indexesCount, 
    uint8_t indexComponents, 
    uint8_t componentSize)
{
    lite3d_vao *vao;
    size_t verticesSize, indexesSize;
    SDL_assert(vbo);

    lite3d_misc_gl_error_stack_clean();
    glBindBuffer(GL_ARRAY_BUFFER, vbo->vboVerticesID);

    if(!lite3d_misc_check_gl_error())
        return LITE3D_FALSE;

    
    //glBufferData(GL_ARRAY_BUFFER, 
}

int lite3d_vbo_extend_from_memory(lite3d_vbo *vbo, 
    void *vertices, 
    size_t verticesCount, 
    lite3d_buffer_component *layout,
    size_t layoutCount,
    void *indexes, 
    size_t indexesCount, 
    uint8_t indexComponents, 
    uint8_t componentSize)
{
    SDL_assert(vbo);


}

