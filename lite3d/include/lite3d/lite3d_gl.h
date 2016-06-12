/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2016  Sirius (Korolev Nikita)
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
#ifndef LITE3D_GL_H
#define	LITE3D_GL_H

#ifdef WITH_GLES2
#   include <GLES2/gl2.h>
#   include <GLES2/gl2ext.h>
#   define GLES
#elif defined WITH_GLES3
#   include <GLES3/gl3.h>
#   include <GLES3/gl3ext.h>
#   define GLES
#else
#   include <lite3d/GL/glew.h>

#   ifdef PLATFORM_Windows
#       include <lite3d/GL/wglew.h>
#   elif defined PLATFORM_Linux
#       include <lite3d/GL/glxew.h>
#   endif
#endif

#endif
