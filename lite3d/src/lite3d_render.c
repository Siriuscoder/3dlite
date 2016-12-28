/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
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
#include <string.h>
#include <SDL_timer.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_render.h>
#include <lite3d/lite3d_video.h>
#include <lite3d/lite3d_scene.h>
#include <lite3d/lite3d_buffers_manip.h>
#include <lite3d/lite3d_main.h>

typedef struct lookUnit
{
    lite3d_list_node rtLink;
    lite3d_camera *camera;
    uint16_t pass;
    int priority;
} lookUnit;

static uint64_t gLastMark = 0;
static uint64_t gPerfFreq = 0;
static int32_t gFPSCounter = 0;
static lite3d_list gRenderTargets;
static lite3d_render_listeners gRenderListeners;
static uint8_t gRenderStarted = LITE3D_TRUE;
static uint8_t gRenderActive = LITE3D_TRUE;
static lite3d_render_stats gRenderStats;
static lite3d_render_target gScreenRt;

static void refresh_render_stats(uint64_t beginFrame, uint64_t endFrame)
{
    gFPSCounter++;
    gRenderStats.lastFrameMs = ((float) (endFrame - beginFrame) / (float) gPerfFreq) * 1000.0;

    if (gRenderStats.bestFrameMs == 0)
        gRenderStats.bestFrameMs = gRenderStats.lastFrameMs;
    if (gRenderStats.worstFrameMs == 0)
        gRenderStats.worstFrameMs = gRenderStats.lastFrameMs;
    if (gRenderStats.avrFrameMs == 0)
        gRenderStats.avrFrameMs = gRenderStats.lastFrameMs;

    if (gRenderStats.bestFrameMs > gRenderStats.lastFrameMs)
        gRenderStats.bestFrameMs = gRenderStats.lastFrameMs;

    if (gRenderStats.worstFrameMs < gRenderStats.lastFrameMs)
        gRenderStats.worstFrameMs = gRenderStats.lastFrameMs;

    gRenderStats.avrFrameMs = (gRenderStats.lastFrameMs + gRenderStats.avrFrameMs) / 2;

    gRenderStats.triangleByBatch = gRenderStats.batchedByFrame ? gRenderStats.trianglesByFrame / gRenderStats.batchedByFrame : 0;
    gRenderStats.triangleMs = gRenderStats.trianglesByFrame ? (float) gRenderStats.lastFrameMs / (float) gRenderStats.trianglesByFrame : 0;

    /* second elapsed */
    if ((endFrame - gLastMark) > gPerfFreq)
    {
        gLastMark = endFrame;
        gRenderStats.lastFPS = gFPSCounter;
        gFPSCounter = 0;

        if (gRenderStats.avrFPS == 0)
            gRenderStats.avrFPS = gRenderStats.lastFPS;
        if (gRenderStats.bestFPS == 0)
            gRenderStats.bestFPS = gRenderStats.lastFPS;
        if (gRenderStats.worstFPS == 0)
            gRenderStats.worstFPS = gRenderStats.lastFPS;

        if (gRenderStats.bestFPS < gRenderStats.lastFPS)
            gRenderStats.bestFPS = gRenderStats.lastFPS;
        if (gRenderStats.worstFPS > gRenderStats.lastFPS || gRenderStats.worstFPS <= 1)
            gRenderStats.worstFPS = gRenderStats.lastFPS;

        gRenderStats.avrFPS = (gRenderStats.avrFPS + gRenderStats.lastFPS) / 2;
    }
}

static void update_render_target(lite3d_render_target *target)
{
    lite3d_list_node *node;
    lite3d_scene *scene;
    lookUnit *look;
    /* switch target framebuffer */
    lite3d_framebuffer_switch(&target->fb);
    /* clear target */
    lite3d_buffers_clear_values(&target->cleanColor, target->cleanDepth, target->cleanStencil);
    lite3d_buffers_clear(target->clearColorBuffer, target->clearDepthBuffer, target->clearStencilBuffer);

    /* do paint by render queue */
    for (node = target->lookSequence.l.next; node != &target->lookSequence.l; node = lite3d_list_next(node))
    {
        look = LITE3D_MEMBERCAST(lookUnit, node, rtLink);
        scene = (lite3d_scene *) look->camera->cameraNode.scene;

        if(look->camera->cameraNode.enabled)
        {
            lite3d_scene_render(scene, look->camera, look->pass, target->renderFlags);

            /* accamulate statistics */
            gRenderStats.trianglesByFrame += scene->stats.trianglesRendered;
            gRenderStats.verticesByFrame += scene->stats.verticesRendered;
            gRenderStats.nodesTotal += scene->stats.nodesTotal;
            gRenderStats.batchesTotal += scene->stats.batchesTotal;
            gRenderStats.batchedByFrame += scene->stats.batchesCalled;
            gRenderStats.materialsTotal += scene->stats.materialBlocks;
            gRenderStats.materialsPassedByFrame += scene->stats.materialPassed;
            gRenderStats.textureUnitsByFrame += scene->stats.textureUnitsBinded;
        }
    }
}

static int update_render_targets(void)
{
    int32_t targetsCount = 0;
    lite3d_list_node *node;
    lite3d_render_target *target;

    for (node = gRenderTargets.l.next; node != &gRenderTargets.l; node = lite3d_list_next(node))
    {
        target = LITE3D_MEMBERCAST(lite3d_render_target, node, node);
        if (!target->enabled)
            continue;

        if (target->preUpdate)
            target->preUpdate(target);

        update_render_target(target);

        if (target->postUpdate)
            target->postUpdate(target);

        targetsCount++;

        /* flush gl commands */
        glFlush();
    }

    gRenderStats.renderTargets = targetsCount;
    return targetsCount ? LITE3D_TRUE : LITE3D_FALSE;
}

void lite3d_render_loop(lite3d_render_listeners *callbacks)
{
    SDL_Event wevent;
    uint64_t beginFrameMark;
    gRenderListeners = *callbacks;

    gPerfFreq = SDL_GetPerformanceFrequency();

    /* depth test enable by default */
    lite3d_depth_test(LITE3D_TRUE);
    lite3d_depth_test_func(LITE3D_TEST_LESS);

    lite3d_buffers_clear_values(&KM_VEC4_ZERO, 1.0f, 0);
    lite3d_buffers_clear(LITE3D_TRUE, LITE3D_TRUE, LITE3D_TRUE);
    lite3d_video_swap_buffers();

    /* clean statistic */
    memset(&gRenderStats, 0, sizeof (gRenderStats));
    /* init screen render target */
    lite3d_render_target_init(&gScreenRt,
        lite3d_get_global_settings()->videoSettings.screenWidth,
        lite3d_get_global_settings()->videoSettings.screenHeight);
    
    lite3d_render_target_fullscreen(&gScreenRt, lite3d_get_global_settings()->videoSettings.fullscreen);

    lite3d_list_init(&gRenderTargets);
    lite3d_render_target_add(&gScreenRt, 0xFFFFFFF);

    /* start user initialization */
    if (!gRenderListeners.preRender || (gRenderListeners.preRender &&
        gRenderListeners.preRender(gRenderListeners.userdata)))
    {
        /* get time mark */
        beginFrameMark = SDL_GetPerformanceCounter();
        
        /* begin render loop */
        while (gRenderStarted)
        {
            gRenderStats.trianglesByFrame =
                gRenderStats.nodesTotal =
                gRenderStats.batchesTotal =
                gRenderStats.batchedByFrame = 
                gRenderStats.materialsTotal =
                gRenderStats.materialsPassedByFrame =
                gRenderStats.textureUnitsByFrame =
                gRenderStats.verticesByFrame = 0;

            if (gRenderActive)
            {
                if (gRenderListeners.preFrame &&
                    !gRenderListeners.preFrame(gRenderListeners.userdata))
                    break;
                if (!gRenderStarted)
                    break;
                if (!update_render_targets())
                    break;
                if (gRenderListeners.postFrame &&
                    !gRenderListeners.postFrame(gRenderListeners.userdata))
                    break;
            }
            
            /* refresh render statistic, render time span used */
            refresh_render_stats(beginFrameMark, SDL_GetPerformanceCounter());
            /* get time mark */
            beginFrameMark = SDL_GetPerformanceCounter();
            /* induce timers */
            lite3d_timer_induce(beginFrameMark, gPerfFreq);

            while (SDL_PollEvent(&wevent))
            {
                if (gRenderListeners.processEvent &&
                    !gRenderListeners.processEvent(&wevent, gRenderListeners.userdata))
                {
                    lite3d_render_stop();
                    break;
                }
            }
            
            /* finish gl operations and swap buffers */
            lite3d_video_swap_buffers();
        }
    }

    if (gRenderListeners.postRender)
        gRenderListeners.postRender(gRenderListeners.userdata);

    lite3d_render_target_erase_all();
    lite3d_render_target_purge(&gScreenRt);
}

lite3d_render_stats *lite3d_render_stats_get(void)
{
    return &gRenderStats;
}

int lite3d_render_target_init(lite3d_render_target *rt,
    int32_t width, int32_t height)
{
    SDL_assert(rt);

    memset(rt, 0, sizeof (lite3d_render_target));
    lite3d_list_link_init(&rt->node);

    rt->enabled = LITE3D_TRUE;
    lite3d_list_init(&rt->lookSequence);
    rt->clearColorBuffer = LITE3D_TRUE;
    rt->clearDepthBuffer = LITE3D_TRUE;
    rt->clearStencilBuffer = LITE3D_TRUE;
    rt->cleanColor.x = rt->cleanColor.y = rt->cleanColor.z = 0.3f;
    rt->cleanColor.w = 0.0f;
    rt->cleanDepth = 1.0f;
    rt->cleanStencil = 0;
    rt->width = width;
    rt->height = height;
    rt->renderFlags = LITE3D_RENDER_STAGE_FIRST | LITE3D_RENDER_STAGE_SECOND;

    if (rt != &gScreenRt)
    {
        if (!lite3d_framebuffer_init(&rt->fb, width, height))
            return LITE3D_FALSE;
    }
    else
    {
        if (!lite3d_framebuffer_screen_init(&rt->fb, width, height))
            return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

void lite3d_render_target_purge(lite3d_render_target *rt)
{
    lite3d_list_node *node = NULL;
    lookUnit *look = NULL;

    SDL_assert(rt);

    lite3d_framebuffer_purge(&rt->fb);
    while ((node = lite3d_list_remove_first_link(&rt->lookSequence)) != NULL)
    {
        look = LITE3D_MEMBERCAST(lookUnit, node, rtLink);
        lite3d_free_pooled(LITE3D_POOL_NO1, look);
    }
}

void lite3d_render_target_add(lite3d_render_target *newRt, int priority)
{
    lite3d_render_target *rtPtr = NULL;
    lite3d_list_node *node = NULL;

    newRt->priority = priority;
    lite3d_render_target_erase(newRt);

    node = &gRenderTargets.l;
    while ((node = lite3d_list_next(node)) != &gRenderTargets.l)
    {
        rtPtr = LITE3D_MEMBERCAST(lite3d_render_target, node, node);
        if (priority <= rtPtr->priority)
        {
            lite3d_list_insert_before_link(&newRt->node, &rtPtr->node);
            return;
        }
    }

    lite3d_list_add_first_link(&newRt->node, &gRenderTargets);
}

void lite3d_render_target_erase(lite3d_render_target *rt)
{
    lite3d_list_unlink_link(&rt->node);
}

void lite3d_render_target_erase_all(void)
{
    lite3d_list_node *node = NULL;
    while ((node = lite3d_list_remove_first_link(&gRenderTargets)) != NULL);
}

void lite3d_render_resume(void)
{
    gRenderActive = LITE3D_TRUE;
}

void lite3d_render_suspend(void)
{
    gRenderActive = LITE3D_FALSE;
}

void lite3d_render_stop(void)
{
    gRenderStarted = LITE3D_FALSE;
}

int lite3d_render_target_attach_camera(lite3d_render_target *target, lite3d_camera *camera,
    uint16_t pass, int priority)
{
    lookUnit *look = NULL;
    lookUnit *lookIns = NULL;
    lite3d_list_node *node = NULL;
    SDL_assert(target && camera);


    lookIns = (lookUnit *) lite3d_calloc_pooled(LITE3D_POOL_NO1, sizeof (lookUnit));
    SDL_assert_release(lookIns);

    lookIns->camera = camera;
    lookIns->pass = pass;
    lookIns->priority = priority;
    lite3d_list_link_init(&lookIns->rtLink);

    /* check camera already attached to the render target */
    node = &target->lookSequence.l;
    while ((node = lite3d_list_next(node)) != &target->lookSequence.l)
    {
        look = LITE3D_MEMBERCAST(lookUnit, node, rtLink);
        if (look->camera == camera && look->pass == pass &&
            look->priority == priority)
        {
            lite3d_free_pooled(LITE3D_POOL_NO1, lookIns);
            return LITE3D_FALSE;
        }

        if (priority <= look->priority)
        {
            lite3d_list_insert_before_link(&lookIns->rtLink, &look->rtLink);
            return LITE3D_TRUE;
        }
    }

    lite3d_list_add_last_link(&lookIns->rtLink, &target->lookSequence);
    return LITE3D_TRUE;
}

int lite3d_render_target_dettach_camera(lite3d_render_target *rt, lite3d_camera *camera,
    uint16_t pass, int priority)
{
    lookUnit *look = NULL;
    lite3d_list_node *node = NULL;
    SDL_assert(rt && camera);

    /* lookup for the camera */
    node = &rt->lookSequence.l;
    while ((node = lite3d_list_next(node)) != &rt->lookSequence.l)
    {
        look = LITE3D_MEMBERCAST(lookUnit, node, rtLink);
        if (look->camera == camera && look->pass == pass &&
            look->priority == priority)
        {
            lite3d_list_unlink_link(node);
            lite3d_free_pooled(LITE3D_POOL_NO1, look);
            return LITE3D_FALSE;
        }
    }

    return LITE3D_TRUE;
}

int lite3d_render_target_screen_attach_camera(lite3d_camera *camera,
    uint16_t pass, int priority)
{
    return lite3d_render_target_attach_camera(&gScreenRt, camera,
        pass, priority);
}

int lite3d_render_target_screen_dettach_camera(lite3d_camera *camera,
    uint16_t pass, int priority)
{
    return lite3d_render_target_dettach_camera(&gScreenRt, camera,
        pass, priority);
}

lite3d_render_target *lite3d_render_target_screen_get(void)
{
    return &gScreenRt;
}

void lite3d_render_target_resize(lite3d_render_target *rt, int32_t width, int32_t height)
{
    SDL_assert(rt);

    if (width == 0 || height == 0)
    {
        if (!lite3d_video_get_display_size(&width, &height))
            return;
    }

    rt->width = width;
    rt->height = height;
    lite3d_video_resize(width, height);
    lite3d_framebuffer_resize(&rt->fb, width, height);
}

void lite3d_render_target_fullscreen(lite3d_render_target *rt, int8_t flag)
{
    SDL_assert(rt);
    if (rt == &gScreenRt && rt->fullscreen != flag)
    {
        lite3d_video_set_fullscreen(flag);
        rt->fullscreen = flag;
    }
}
