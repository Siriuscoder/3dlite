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
#ifndef LITE3D_BUFFERS_MANIP_H
#define	LITE3D_BUFFERS_MANIP_H

#include <lite3d/lite3d_common.h>
#include <lite3d/kazmath/vec4.h>

#define LITE3D_TEST_NEVER 0x0200
#define LITE3D_TEST_LESS 0x0201
#define LITE3D_TEST_EQUAL 0x0202
#define LITE3D_TEST_LEQUAL 0x0203
#define LITE3D_TEST_GREATER 0x0204
#define LITE3D_TEST_NOTEQUAL 0x0205
#define LITE3D_TEST_GEQUAL 0x0206
#define LITE3D_TEST_ALWAYS 0x0207

LITE3D_CEXPORT void lite3d_depth_test(uint8_t on);
LITE3D_CEXPORT void lite3d_depth_test_func(uint32_t func);
LITE3D_CEXPORT void lite3d_depth_output(uint8_t on);

LITE3D_CEXPORT void lite3d_stencil_test(uint8_t on);
LITE3D_CEXPORT void lite3d_stencil_test_func(uint32_t func, int32_t value);
LITE3D_CEXPORT void lite3d_stencil_output(uint8_t on);
LITE3D_CEXPORT void lite3d_stencil_value(int32_t value);

LITE3D_CEXPORT void lite3d_color_output(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

LITE3D_CEXPORT void lite3d_buffers_clear(uint8_t color, uint8_t depth, uint8_t stencil);
LITE3D_CEXPORT void lite3d_buffers_clear_values(const kmVec4 *color, float depth, int stencil);

#endif	/* LITE3D_BUFFERS_MANIP_H */

