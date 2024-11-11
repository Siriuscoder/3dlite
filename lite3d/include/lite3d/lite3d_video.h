/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2024  Sirius (Korolev Nikita)
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
#ifndef LITE3D_VIDEO_H
#define	LITE3D_VIDEO_H

#include <lite3d/lite3d_common.h>

#define LITE3D_CAPTION_MAX 50

#define LITE3D_GL_PROFILE_DEFAULT 0
#define LITE3D_GL_PROFILE_CORE 1
#define LITE3D_GL_PROFILE_COMPATIBILITY 2


typedef struct lite3d_video_settings
{
    int8_t colorBits; /* 24 or 32 valid */
    int32_t screenHeight;
    int32_t screenWidth;
    int8_t MSAA;
    int8_t fullscreen;
    char caption[LITE3D_CAPTION_MAX];
    int8_t vsync;
    int8_t hidden;
    int8_t glProfile;
    int8_t glVersionMajor;
    int8_t glVersionMinor;
    int8_t debug;
} lite3d_video_settings;


LITE3D_CEXPORT int lite3d_video_open(lite3d_video_settings *settings, int hideConsole);
LITE3D_CEXPORT int lite3d_video_close(void);
LITE3D_CEXPORT void lite3d_video_swap_buffers(void);
LITE3D_CEXPORT void lite3d_video_set_mouse_pos(int32_t x, int32_t y);
LITE3D_CEXPORT void lite3d_video_resize(int32_t width, int32_t height);
LITE3D_CEXPORT int lite3d_video_get_display_size(int32_t *width, int32_t *height);
LITE3D_CEXPORT void lite3d_video_set_fullscreen(int8_t flag);
LITE3D_CEXPORT void lite3d_video_view_system_cursor(int8_t flag);
LITE3D_CEXPORT void lite3d_video_wait_async_complete(void);
LITE3D_CEXPORT const char *lite3d_video_get_vendor(void);


#endif	/* VIDEO_H */
