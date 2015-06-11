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
#ifndef LITE3D_MISC_H
#define	LITE3D_MISC_H

#include <3dlite/3dlite_common.h>

LITE3D_CEXPORT void lite3d_misc_gl_error_stack_clean(void);
LITE3D_CEXPORT void lite3d_misc_il_error_stack_clean(void);

LITE3D_CEXPORT int lite3d_misc_check_gl_error(void);
LITE3D_CEXPORT int lite3d_misc_check_il_error(void);

LITE3D_CEXPORT int lite3d_misc_check_gl_out_of_mem(void);
LITE3D_CEXPORT void lite3d_misc_gl_out_of_mem_reset(void);
LITE3D_CEXPORT int lite3d_misc_check_gl_stack_corruption(void);
LITE3D_CEXPORT void lite3d_misc_gl_stack_corruption_reset(void);
#endif	/* LITE3D_MISC_H */

