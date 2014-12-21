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
#ifndef LITE3D_RENDER_H
#define	LITE3D_RENDER_H

#include <SDL_events.h>
#include <3dlite/3dlite_common.h>

typedef int (*lite3d_pre_render_t)(void *userdata);
typedef int (*lite3d_post_render_t)(void *userdata);
typedef int (*lite3d_pre_frame_t)(void *userdata);
typedef int (*lite3d_post_frame_t)(void *userdata);
typedef int (*lite3d_render_frame_t)(void *userdata);
typedef int (*lite3d_process_event_t)(SDL_Event *levent, void *userdata);

typedef struct lite3d_render_listeners
{
    lite3d_pre_render_t preRender;
    lite3d_post_render_t postRender;
    lite3d_pre_frame_t preFrame;
    lite3d_post_frame_t postFrame;
    lite3d_render_frame_t renderFrame;
    lite3d_process_event_t processEvent;
    void *userdata;
} lite3d_render_listeners;

typedef struct lite3d_render_stats
{
    int32_t FPS;
    float frameTimeMs;
} lite3d_render_stats;


LITE3D_CEXPORT void lite3d_render_loop(lite3d_render_listeners *callbacks);
LITE3D_CEXPORT lite3d_render_stats *lite3d_get_render_stats(void);


#endif	/* RENDER_H */

