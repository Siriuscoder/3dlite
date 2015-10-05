/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
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
#include <lite3d/GL/glew.h>

#include <lite3d/lite3d_buffers_manip.h>

void lite3d_depth_test(uint8_t on)
{
    on == LITE3D_TRUE ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
}

void lite3d_depth_test_func(uint32_t func)
{
    glDepthFunc(func);
}

void lite3d_stencil_test(uint8_t on)
{
    on == LITE3D_TRUE ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
}

void lite3d_stencil_test_func(uint32_t func, int32_t value)
{
    glStencilFunc(func, value, 0xFF);
}

void lite3d_depth_output(uint8_t on)
{
    glDepthMask(on == LITE3D_TRUE ? GL_TRUE : GL_FALSE);
}

void lite3d_color_output(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
    glColorMask(r == LITE3D_TRUE ? GL_TRUE : GL_FALSE,
        g == LITE3D_TRUE ? GL_TRUE : GL_FALSE,
        b == LITE3D_TRUE ? GL_TRUE : GL_FALSE,
        a == LITE3D_TRUE ? GL_TRUE : GL_FALSE);
}

void lite3d_stencil_output(uint8_t on)
{
    glStencilMask(on == LITE3D_TRUE ? 0xFF : 0x00);
}

void lite3d_stencil_value(int32_t value)
{
    glStencilFunc(GL_NEVER, value, 0xFF);
    glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
}

void lite3d_buffers_clear(uint8_t color, uint8_t depth, uint8_t stencil)
{
    glClear((color ? GL_COLOR_BUFFER_BIT : 0) | 
        (depth ? GL_DEPTH_BUFFER_BIT : 0) | 
        (stencil ? GL_STENCIL_BUFFER_BIT : 0));
}

void lite3d_buffers_clear_values(const kmVec4 *color, float depth)
{
    glClearColor(color->x, color->y, color->z, color->w);
    glClearDepth(depth);
}