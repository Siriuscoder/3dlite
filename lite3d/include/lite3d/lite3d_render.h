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
#ifndef LITE3D_RENDER_H
#define	LITE3D_RENDER_H

#include <SDL_events.h>
#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_list.h>
#include <lite3d/lite3d_camera.h>
#include <lite3d/lite3d_framebuffer.h>

#define LITE3D_RENDER_TARGET_NAME   20
#define CLEAN_COLOR_BUFFER 0x00004000
#define CLEAN_DEPTH_BUFFER 0x00000100
#define CLEAN_STENCIL_BUFFER 0x00000400

typedef int (*lite3d_pre_render_t)(void *userdata);
typedef int (*lite3d_post_render_t)(void *userdata);
typedef int (*lite3d_pre_frame_t)(void *userdata);
typedef int (*lite3d_post_frame_t)(void *userdata);
typedef int (*lite3d_process_event_t)(SDL_Event *levent, void *userdata);

typedef struct lite3d_render_listeners
{
    lite3d_pre_render_t preRender;
    lite3d_post_render_t postRender;
    lite3d_pre_frame_t preFrame;
    lite3d_post_frame_t postFrame;
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
    int32_t trianglesByFrame;
    int32_t verticesByFrame;
    float triangleMs;
    int32_t triangleByBatch;
    int32_t objectsByFrame;
    int32_t batchesByFrame;
    int32_t materialsByFrame;
    int32_t materialsPassedByFrame;
    int32_t textureUnitsByFrame;
} lite3d_render_stats;

typedef struct lite3d_render_target
{
    lite3d_list_node node;
    lite3d_framebuffer fb;
    int32_t height;
    int32_t width;
    void *userdata;
    uint8_t enabled;
    void (*preUpdate)(struct lite3d_render_target *target);
    void (*postUpdate)(struct lite3d_render_target *target);
    lite3d_list lookSequence;
    uint32_t cleanMask;
    kmVec4 cleanColor;
    int priority;
} lite3d_render_target;

LITE3D_CEXPORT void lite3d_render_loop(lite3d_render_listeners *callbacks);
LITE3D_CEXPORT lite3d_render_stats *lite3d_render_stats_get(void);

LITE3D_CEXPORT int lite3d_render_target_init(lite3d_render_target *rt, 
    int32_t width, int32_t height);
LITE3D_CEXPORT void lite3d_render_target_purge(lite3d_render_target *rt);
LITE3D_CEXPORT void lite3d_render_target_add(lite3d_render_target *rt, int priority);
LITE3D_CEXPORT void lite3d_render_target_erase(lite3d_render_target *rt);
LITE3D_CEXPORT void lite3d_render_target_erase_all(void);

LITE3D_CEXPORT void lite3d_render_resume(void);
LITE3D_CEXPORT void lite3d_render_suspend(void);
LITE3D_CEXPORT void lite3d_render_stop(void);
LITE3D_CEXPORT void lite3d_render_depth_test(uint8_t on);

LITE3D_CEXPORT int lite3d_render_target_attach_camera(lite3d_render_target *rt, lite3d_camera *camera, uint16_t pass, int priority);
LITE3D_CEXPORT int lite3d_render_target_dettach_camera(lite3d_render_target *rt, lite3d_camera *camera, uint16_t pass, int priority);
LITE3D_CEXPORT int lite3d_render_target_screen_attach_camera(lite3d_camera *camera, uint16_t pass, int priority);
LITE3D_CEXPORT int lite3d_render_target_screen_dettach_camera(lite3d_camera *camera, uint16_t pass, int priority);
LITE3D_CEXPORT lite3d_render_target *lite3d_render_target_screen_get(void);
#endif	/* RENDER_H */
