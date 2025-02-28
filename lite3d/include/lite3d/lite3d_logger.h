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
#ifndef LITE3D_LOGGER_H
#define	LITE3D_LOGGER_H

#include <lite3d/lite3d_common.h>

#define LITE3D_LOGLEVEL_ERROR           0x1
#define LITE3D_LOGLEVEL_INFO            0x2
#define LITE3D_LOGLEVEL_VERBOSE         0x3

LITE3D_CEXPORT void lite3d_logger_setup(const char *logfile);
LITE3D_CEXPORT void lite3d_logger_set_logParams(int level, int flushAlways, int muteStd);
LITE3D_CEXPORT void lite3d_logger_release(void);

#endif	/* LOGGER_H */

