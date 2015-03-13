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
#ifndef LITE3D_FRAMEBUFFER_H
#define	LITE3D_FRAMEBUFFER_H

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_list.h>

typedef struct lite3d_framebuffer
{
    uint32_t framebufferId;
    uint32_t renderBuffersIds[3];
    size_t renderBuffersUsed;
    int32_t height;
    int32_t width;
} lite3d_framebuffer;

LITE3D_CEXPORT int lite3d_framebuffer_technique_init(void);


#endif	/* LITE3D_FRAMEBUFFER_H */

