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
#ifndef LITE3D_LOGGER_H
#define	LITE3D_LOGGER_H

#include <3dlite/3dlite_common.h>

#define LITE3D_LOGLEVEL_ERROR           0x1
#define LITE3D_LOGLEVEL_INFO            0x2
#define LITE3D_LOGLEVEL_VERBOSE         0x3

LITE3D_CEXPORT void lite3d_logger_setup_stdout(void);
LITE3D_CEXPORT void lite3d_logger_set_logParams(int8_t level, int8_t flushAlways);
LITE3D_CEXPORT void lite3d_logger_release(void);

#endif	/* LOGGER_H */

