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
#ifndef LITE3D_VIDEO_H
#define	LITE3D_VIDEO_H

#include <lite3d/lite3d_common.h>

#define LITE3D_CAPTION_MAX 50

typedef struct lite3d_video_settings
{
    int8_t colorBits; /* 24 or 32 valid */
    int32_t screenHeight;
    int32_t screenWidth;
    int8_t FSAA;
    int8_t fullscreen;
    char caption[LITE3D_CAPTION_MAX];
    int8_t vsync;
    int8_t hidden;
} lite3d_video_settings;

LITE3D_CEXPORT int lite3d_video_open(const lite3d_video_settings *settings);
LITE3D_CEXPORT const lite3d_video_settings *lite3d_video_get_settings(void);
LITE3D_CEXPORT int lite3d_video_close(void);
LITE3D_CEXPORT void lite3d_video_swap_buffers(void);


#endif	/* VIDEO_H */

