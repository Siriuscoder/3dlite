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
#ifndef LITE3D_MISC_H
#define	LITE3D_MISC_H

#include <lite3d/lite3d_common.h>

#ifdef NDEBUG 
#   define LITE3D_CHECK_GL_ERROR (0)
#   define LITE3D_CHECK_IL_ERROR (0)
#else
#   define LITE3D_CHECK_GL_ERROR \
        lite3d_misc_check_gl_error(LITE3D_CURRENT_FUNCTION, __LINE__)

#   define LITE3D_CHECK_IL_ERROR \
        lite3d_misc_check_il_error(LITE3D_CURRENT_FUNCTION, __LINE__)
#endif

LITE3D_CEXPORT void lite3d_misc_gl_error_stack_clean(void);
LITE3D_CEXPORT void lite3d_misc_il_error_stack_clean(void);

LITE3D_CEXPORT int lite3d_misc_check_gl_error(const char *func, int line);
LITE3D_CEXPORT int lite3d_misc_check_il_error(const char *func, int line);

LITE3D_CEXPORT int lite3d_misc_check_gl_out_of_mem(void);
LITE3D_CEXPORT void lite3d_misc_gl_out_of_mem_reset(void);
LITE3D_CEXPORT int lite3d_misc_check_gl_stack_corruption(void);
LITE3D_CEXPORT void lite3d_misc_gl_stack_corruption_reset(void);
LITE3D_CEXPORT void lite3d_misc_gl_set_not_supported(void);

#endif	/* LITE3D_MISC_H */

