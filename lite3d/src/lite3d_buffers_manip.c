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

#include <lite3d/lite3d_buffers_manip.h>

static uint8_t gDepthTestOn = 0xff;
static uint8_t gStencilTestOn = 0xff;
static uint8_t gBlendingOn = 0xff;
static uint8_t gBlendigMode = 0xff;
static uint8_t gColorOutOn = 0xff;
static uint8_t gDepthOutOn = 0xff;
static uint8_t gStencilOutOn = 0xff;
static uint8_t gBackFaceCullingOn = 0x99;
static uint8_t gDeptTestFunc = 0;

#ifndef GLES
static uint8_t gPolygonMode = 0;
static GLenum polygonModeEnum[] = {
    GL_POINT, GL_LINE, GL_FILL
};
#endif

static GLenum testFuncEnum[] = {
    GL_NEVER, GL_LESS, GL_EQUAL, GL_LEQUAL, GL_GREATER, GL_NOTEQUAL, GL_GEQUAL, GL_ALWAYS
};

static GLenum cullfaceModeEnum[] = {
    0x0, GL_FRONT, GL_BACK, GL_FRONT_AND_BACK
};

void lite3d_depth_test(uint8_t on)
{
    if (on != gDepthTestOn)
    {
        on == LITE3D_TRUE ? glEnable(GL_DEPTH_TEST) : glDisable(GL_DEPTH_TEST);
        gDepthTestOn = on;
    }
}

void lite3d_depth_test_func(uint8_t func)
{
    if (func != gDeptTestFunc)
    {
        glDepthFunc(testFuncEnum[func]);
        gDeptTestFunc = func;
    }
}

void lite3d_stencil_test(uint8_t on)
{
    if (on != gStencilTestOn)
    {
        on == LITE3D_TRUE ? glEnable(GL_STENCIL_TEST) : glDisable(GL_STENCIL_TEST);
        gStencilTestOn = on;
    }
}

void lite3d_stencil_test_func(uint8_t func, int32_t refValue)
{
    glStencilFunc(testFuncEnum[func], refValue, 0xFF);
}

void lite3d_depth_output(uint8_t on)
{
    if (on != gDepthOutOn)
    {
        glDepthMask(on == LITE3D_TRUE ? GL_TRUE : GL_FALSE);
        gDepthOutOn = on;
    }
}

void lite3d_color_output(uint8_t on)
{
    if (on != gColorOutOn)
    {
        if (on)
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        else
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        gColorOutOn = on;
    }
}

void lite3d_stencil_output(uint8_t on)
{
    if (on != gStencilOutOn)
    {
        glStencilMask(on == LITE3D_TRUE ? 0xFF : 0x00);
        gStencilOutOn = on;
    }
}

void lite3d_stencil_value(int32_t value)
{
    glStencilFunc(GL_NEVER, value, 0xFF);
    glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
}

void lite3d_buffers_clear(uint8_t color, uint8_t depth, uint8_t stencil)
{
    GLbitfield clearbits = 0;
    if (color)
    {
        lite3d_color_output(LITE3D_TRUE);
        clearbits |= GL_COLOR_BUFFER_BIT;
    }

    if (depth)
    {
        lite3d_depth_output(LITE3D_TRUE);
        clearbits |= GL_DEPTH_BUFFER_BIT;
    }

    if (stencil)
    {
        lite3d_stencil_output(LITE3D_TRUE);
        clearbits |= GL_STENCIL_BUFFER_BIT;
    }

    if (color || depth || stencil)
        glClear(clearbits);
}

void lite3d_buffers_clear_values(const kmVec4 *color, float depth, int stencil)
{
    glClearColor(color->x, color->y, color->z, color->w);
    glClearDepthf(depth);
    glClearStencil(stencil);
}

/* 
    Traditional blend algorithm:
    Orgb = Sa * Srgb + (1 - Sa) * Drgb
    Oa = 1 * Sa + 0 * Da = Sa
*/
static void blending_mode_traditional(void)
{
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);
}

/* 
    Additive blend algorithm:
    Orgb = 1 * Srgb + 1 * Drgb
    Oa = 1 * Sa + 0 * Da = Sa
*/
static void blending_mode_additive(void)
{
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ONE, GL_ZERO);
}

/* 
    Traditional blend algorithm with alpha blend:
    Orgb = Sa * Srgb + (1 - Sa) * Drgb
    Oa = 1 * Sa + (1 - Sa) * Da = Sa
*/
static void blending_mode_traditional_alpha_blend(void)
{
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
}

void lite3d_blending(uint8_t on)
{
    if (on != gBlendingOn)
    {
        on == LITE3D_TRUE ? glEnable(GL_BLEND) : glDisable(GL_BLEND);
        gBlendingOn = on;
    }
}

lite3d_blend_mode_t gBlendModes[] = {
    blending_mode_traditional,
    blending_mode_traditional_alpha_blend,
    blending_mode_additive
};

void lite3d_blending_mode_set(uint8_t mode)
{
    SDL_assert(mode < (sizeof(gBlendModes)/sizeof(lite3d_blend_mode_t)));

    if (mode != gBlendigMode)
    {
        gBlendModes[mode]();
        gBlendigMode = mode;
    }
}

void lite3d_polygon_mode(uint8_t mode)
{
    /* In OpenGL ES, GL_FILL is the only available polygon mode. */
#ifndef GLES
    if (gPolygonMode != mode)
    {
        glPolygonMode(GL_FRONT_AND_BACK, polygonModeEnum[mode]);
        gPolygonMode = mode;
    }
#else
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "glPolygonMode is not supported in GLES");
#endif
}

void lite3d_backface_culling(uint8_t mode)
{
    if (gBackFaceCullingOn != mode)
    {
        if (mode != LITE3D_CULLFACE_NEVER)
        {
            glEnable(GL_CULL_FACE);
            glCullFace(cullfaceModeEnum[mode]);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }

        gBackFaceCullingOn = mode;
    }
}
