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
#include <3dlite/3dlite_list.h>

#define LITE3D_RENDER_TARGET_NAME   20

typedef int (*lite3d_pre_render_t)(void *userdata);
typedef int (*lite3d_post_render_t)(void *userdata);
typedef int (*lite3d_pre_frame_t)(void *userdata);
typedef int (*lite3d_post_frame_t)(void *userdata);
typedef int (*lite3d_render_frame_t)(void *userdata);
typedef int (*lite3d_process_event_t)(SDL_Event *levent, void *userdata);
typedef void (*lite3d_render_target_pre_update_t)(void *userdata);
typedef void (*lite3d_render_target_post_update_t)(void *userdata);

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
    int32_t lastFPS;
    int32_t avrFPS;
    int32_t bestFPS;
    int32_t worstFPS;
    float lastFrameMs;
    float avrFrameMs;
    float bestFrameMs;
    float worstFrameMs;
    int32_t renderTargets;
} lite3d_render_stats;

typedef struct lite3d_render_target
{
    lite3d_list_node node;
    int32_t ID;
    int32_t height;
    int32_t width;
    int8_t isRoot;
    void *userdata;
    uint8_t enabled;
    lite3d_render_target_pre_update_t preUpdate;
    lite3d_render_target_post_update_t postUpdate;
} lite3d_render_target;

LITE3D_CEXPORT int lite3d_render_init(void);
LITE3D_CEXPORT void lite3d_render_loop(lite3d_render_listeners *callbacks);
LITE3D_CEXPORT lite3d_render_stats *lite3d_get_render_stats(void);
LITE3D_CEXPORT lite3d_render_target *lite3d_add_render_target(int32_t ID, int32_t width,
    int32_t height, int8_t isRoot, void *userdata);
LITE3D_CEXPORT void lite3d_erase_render_target(int32_t ID);
LITE3D_CEXPORT lite3d_render_target *lite3d_get_render_target(int32_t ID);
LITE3D_CEXPORT void lite3d_erase_all_render_targets(void);
LITE3D_CEXPORT void lite3d_suspend_render(void);
LITE3D_CEXPORT void lite3d_pause_render(void);
LITE3D_CEXPORT void lite3d_stop_render(void);

#endif	/* RENDER_H */

