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
#ifndef LITE3D_BUFFERS_MANIP_H
#define	LITE3D_BUFFERS_MANIP_H

#include <lite3d/lite3d_common.h>
#include <lite3d/kazmath/vec4.h>

#define LITE3D_TEST_NEVER           0x0
#define LITE3D_TEST_LESS            0x1
#define LITE3D_TEST_EQUAL           0x2
#define LITE3D_TEST_LEQUAL          0x3
#define LITE3D_TEST_GREATER         0x4
#define LITE3D_TEST_NOTEQUAL        0x5
#define LITE3D_TEST_GEQUAL          0x6
#define LITE3D_TEST_ALWAYS          0x7

#define LITE3D_BLENDING_MODE_RGB_LINEAR_SOURCE_ALPHA        0
#define LITE3D_BLENDING_MODE_RGBA_LINEAR_SOURCE_ALPHA       1
#define LITE3D_BLENDING_MODE_RGB_ADDITIVE                   2

#define LITE3D_POLYMODE_POINT        0x0
#define LITE3D_POLYMODE_LINE         0x1
#define LITE3D_POLYMODE_FILL         0x2

#define LITE3D_CULLFACE_NEVER                   0x0
#define LITE3D_CULLFACE_FRONT                   0x1
#define LITE3D_CULLFACE_BACK                    0x2
#define LITE3D_CULLFACE_FRONT_AND_BACK          0x3

typedef void (*lite3d_blend_mode_t)(void);

LITE3D_CEXPORT void lite3d_depth_test(uint8_t on);
LITE3D_CEXPORT void lite3d_depth_test_func(uint8_t func);
LITE3D_CEXPORT void lite3d_depth_output(uint8_t on);
LITE3D_CEXPORT uint8_t lite3d_depth_output_get(void);

LITE3D_CEXPORT void lite3d_stencil_test(uint8_t on);
LITE3D_CEXPORT void lite3d_stencil_test_func(uint8_t func, int32_t refValue);
LITE3D_CEXPORT void lite3d_stencil_output(uint8_t on);
LITE3D_CEXPORT void lite3d_stencil_value(int32_t value);

LITE3D_CEXPORT void lite3d_color_output(uint8_t on);

LITE3D_CEXPORT void lite3d_buffers_clear(uint8_t color, uint8_t depth, uint8_t stencil);
LITE3D_CEXPORT void lite3d_buffers_clear_values(const kmVec4 *color, float depth, int stencil);

/* 
    LITE3D_BLENDING_MODE_RGB_LINEAR_SOURCE_ALPHA
    Orgb = Sa * Srgb + (1 - Sa) * Drgb
    Oa = 1 * Sa + 0 * Da = Sa

    LITE3D_BLENDING_MODE_RGBA_LINEAR_SOURCE_ALPHA
    Orgb = Sa * Srgb + (1 - Sa) * Drgb
    Oa = 1 * Sa + (1 - Sa) * Da
    
    LITE3D_BLENDING_MODE_RGB_ADDITIVE
    Orgb = 1 * Srgb + 1 * Drgb
    Oa = 1 * Sa + 0 * Da = Sa
 */
LITE3D_CEXPORT void lite3d_blending_mode_set(uint8_t mode);
LITE3D_CEXPORT void lite3d_blending(uint8_t on);

LITE3D_CEXPORT void lite3d_polygon_mode(uint8_t mode);
LITE3D_CEXPORT void lite3d_backface_culling(uint8_t mode);
LITE3D_CEXPORT uint8_t lite3d_backface_culling_get(void);

#endif	/* LITE3D_BUFFERS_MANIP_H */

