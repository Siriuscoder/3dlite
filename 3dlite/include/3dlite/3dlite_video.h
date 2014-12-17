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
#ifndef LITE3D_VIDEO_H
#define	LITE3D_VIDEO_H

#include <3dlite/3dlite_common.h>

#define LITE3D_CAPTION_MAX 50

typedef struct lite3d_video_settings
{
    int32_t screenHeight;
    int32_t screenWidth;
    int8_t FSAA;
    int8_t fullscreen;
    char caption[LITE3D_CAPTION_MAX];
    int8_t vsync;
    int32_t vsyncInterval;
} lite3d_video_settings;

LITE3D_CEXPORT int lite3d_setup_video(const lite3d_video_settings *settings);
LITE3D_CEXPORT const lite3d_video_settings *lite3d_get_video_settings(void);
LITE3D_CEXPORT int lite3d_close_video(void);


#endif	/* VIDEO_H */

