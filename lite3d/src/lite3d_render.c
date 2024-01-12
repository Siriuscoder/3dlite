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
#include <string.h>
#include <SDL_timer.h>
#include <SDL_assert.h>
#include <SDL_log.h>

#include <IL/il.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_render.h>
#include <lite3d/lite3d_video.h>
#include <lite3d/lite3d_buffers_manip.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_main.h>
#include <lite3d/lite3d_metrics.h>
#include <lite3d/lite3d_array.h>

#define LITE3D_LIMSTATS_SKIP_PER_FRAMES  10000

typedef struct lookUnit
{
    lite3d_list_node rtLink;
    lite3d_camera *camera;
    lite3d_scene *scene;
    uint16_t pass;
    int priority;
    uint32_t renderFlags;
    lite3d_framebuffer_layer layer[2];
    size_t layersCount;
} lookUnit;

static uint64_t gPerfFreq = 0;
static uint64_t gBeginFrameMark = 0;
static int32_t gFPSCounter = 0;
static lite3d_list gRenderTargets;
static lite3d_render_listeners gRenderListeners;
static uint8_t gRenderStarted = LITE3D_TRUE;
static uint8_t gRenderActive = LITE3D_TRUE;
static lite3d_render_stats gRenderStats;
static lite3d_render_target gScreenRt;
static lite3d_array gInvalidatedCameras;

static void validate_cameras(void)
{
    lite3d_camera **camera;
    LITE3D_ARR_FOREACH(&gInvalidatedCameras, lite3d_camera*, camera)
    {
        (*camera)->cameraNode.invalidated = LITE3D_FALSE;
    }

    lite3d_array_clean(&gInvalidatedCameras);
}

static int validate_renderflags(uint32_t *renderFlags)
{
    if (*renderFlags & LITE3D_RENDER_INSTANCING)
    {
        if (!lite3d_check_instanced_arrays())
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: GLEW_ARB_instanced_arrays is not supported", LITE3D_CURRENT_FUNCTION);
            *renderFlags &= ~LITE3D_RENDER_INSTANCING;
        }
    }

    if (*renderFlags & LITE3D_RENDER_OCCLUSION_QUERY)
    {
        if (!lite3d_check_occlusion_query())
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: GLEW_ARB_occlusion_query2 is not supported", LITE3D_CURRENT_FUNCTION);
            *renderFlags &= ~LITE3D_RENDER_OCCLUSION_QUERY;
        }
    }

    if (*renderFlags & LITE3D_RENDER_OCCLUSION_CULLING)
    {
        if (!lite3d_check_occlusion_query())
        {
            SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "%s: GLEW_ARB_occlusion_query2 is not supported", LITE3D_CURRENT_FUNCTION);
            *renderFlags &= ~LITE3D_RENDER_OCCLUSION_CULLING;
        }
    }

    return LITE3D_TRUE;
}

static void refresh_render_stats(uint64_t beginFrame, uint64_t endFrame)
{
    gFPSCounter++;
    gRenderStats.framesCount++;
    gRenderStats.lastFrameMs = ((float) (endFrame - beginFrame) / (float) gPerfFreq) * 1000.0;

    if (gRenderStats.bestFrameMs == 0 || !(gRenderStats.framesCount % LITE3D_LIMSTATS_SKIP_PER_FRAMES))
        gRenderStats.bestFrameMs = gRenderStats.lastFrameMs;
    if (gRenderStats.worstFrameMs == 0 || !(gRenderStats.framesCount % LITE3D_LIMSTATS_SKIP_PER_FRAMES))
        gRenderStats.worstFrameMs = gRenderStats.lastFrameMs;
    if (gRenderStats.bestFPS == 0 || !(gRenderStats.framesCount % LITE3D_LIMSTATS_SKIP_PER_FRAMES))
        gRenderStats.bestFPS = gRenderStats.lastFPS;
    if (gRenderStats.worstFPS == 0 || !(gRenderStats.framesCount % LITE3D_LIMSTATS_SKIP_PER_FRAMES))
        gRenderStats.worstFPS = gRenderStats.lastFPS;

    gRenderStats.bestFrameMs = LITE3D_MIN(gRenderStats.lastFrameMs, gRenderStats.bestFrameMs);
    gRenderStats.worstFrameMs = LITE3D_MAX(gRenderStats.lastFrameMs, gRenderStats.worstFrameMs);

    gRenderStats.triangleByBatch = gRenderStats.batchCalled ? gRenderStats.trianglesByFrame / gRenderStats.batchCalled : 0;
    gRenderStats.triangleMs = gRenderStats.trianglesByFrame ? (float) gRenderStats.lastFrameMs / (float) gRenderStats.trianglesByFrame : 0;
}

static void timer_render_stats_tick(lite3d_timer *timer)
{
    gRenderStats.lastFPS = gFPSCounter;
    gFPSCounter = 0;

    if (gRenderStats.avrFPS == 0)
        gRenderStats.avrFPS = gRenderStats.lastFPS;
    if (gRenderStats.avrFrameMs == 0)
        gRenderStats.avrFrameMs = gRenderStats.lastFrameMs;

    gRenderStats.bestFPS = LITE3D_MAX(gRenderStats.lastFPS, gRenderStats.bestFPS);
    gRenderStats.worstFPS = LITE3D_MIN(gRenderStats.lastFPS, gRenderStats.worstFPS);
    gRenderStats.avrFrameMs = (gRenderStats.bestFrameMs + gRenderStats.worstFrameMs + gRenderStats.lastFrameMs) / 3;
    gRenderStats.avrFPS = (gRenderStats.bestFPS + gRenderStats.worstFPS + gRenderStats.lastFPS) / 3;
}

static void update_render_target(lite3d_render_target *target)
{
    lite3d_list_node *node;
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

        if (look->camera->cameraNode.enabled)
        {
            lite3d_framebuffer_switch_layer(&target->fb, look->layer, look->layersCount);

            lite3d_buffers_clear((look->renderFlags & LITE3D_RENDER_CLEAN_COLOR_BUFF) ? LITE3D_TRUE : LITE3D_FALSE,
                (look->renderFlags & LITE3D_RENDER_CLEAN_DEPTH_BUFF) ? LITE3D_TRUE : LITE3D_FALSE,
                (look->renderFlags & LITE3D_RENDER_CLEAN_STENCIL_BUFF) ? LITE3D_TRUE : LITE3D_FALSE);

            lite3d_depth_test((look->renderFlags & LITE3D_RENDER_DEPTH_TEST) ? LITE3D_TRUE : LITE3D_FALSE);
            lite3d_color_output((look->renderFlags & LITE3D_RENDER_COLOR_OUTPUT) ? LITE3D_TRUE : LITE3D_FALSE);
            lite3d_depth_output((look->renderFlags & LITE3D_RENDER_DEPTH_OUTPUT) ? LITE3D_TRUE : LITE3D_FALSE);
            lite3d_stencil_output((look->renderFlags & LITE3D_RENDER_STENCIL_OUTPUT) ? LITE3D_TRUE : LITE3D_FALSE);
            
            LITE3D_METRIC_CALL(lite3d_scene_render, (look->scene, look->camera, look->pass, look->renderFlags))

            if (look->camera->cameraNode.invalidated)
                LITE3D_ARR_ADD_ELEM(&gInvalidatedCameras, lite3d_camera *, look->camera);

            /* accamulate statistics */
            gRenderStats.trianglesByFrame += look->scene->stats.trianglesRendered;
            gRenderStats.verticesByFrame += look->scene->stats.verticesRendered;
            gRenderStats.nodesTotal += look->scene->stats.nodesTotal;
            gRenderStats.batchTotal += look->scene->stats.batchTotal;
            gRenderStats.batchCalled += look->scene->stats.batchCalled;
            gRenderStats.batchInstancedCalled += look->scene->stats.batchInstancedCalled;
            gRenderStats.batchOccluded += look->scene->stats.batchOccluded;
            gRenderStats.materialsTotal += look->scene->stats.materialBlocks;
            gRenderStats.materialsPassedByFrame += look->scene->stats.materialPassed;
            gRenderStats.textureUnitsByFrame += look->scene->stats.textureUnitsBinded;
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

        if (target->preUpdate && !target->preUpdate(target))
            continue;

        LITE3D_METRIC_CALL(update_render_target, (target))

        if (target->postUpdate)
            target->postUpdate(target);

        targetsCount++;
    }

    validate_cameras();

    gRenderStats.renderTargets = targetsCount;
    return targetsCount ? LITE3D_TRUE : LITE3D_FALSE;
}

int lite3d_render_loop_pump_event(void)
{
    SDL_Event wevent;

    // process only one event
    if (SDL_PollEvent(&wevent))
    {
        if (gRenderListeners.processEvent)
        {
            int res;
            LITE3D_METRIC_CALLRET(gRenderListeners.processEvent, res, (&wevent, gRenderListeners.userdata))

            if (!res)
            {
                lite3d_render_stop();
                return LITE3D_FALSE;
            }
        }

        return LITE3D_TRUE;
    }

    return LITE3D_FALSE;
}

int lite3d_render_frame(void)
{
    gRenderStats.trianglesByFrame =
        gRenderStats.nodesTotal =
        gRenderStats.batchTotal =
        gRenderStats.batchCalled =
        gRenderStats.batchInstancedCalled = 
        gRenderStats.batchOccluded = 
        gRenderStats.materialsTotal =
        gRenderStats.materialsPassedByFrame =
        gRenderStats.textureUnitsByFrame =
        gRenderStats.verticesByFrame = 0;

    if (gRenderActive)
    {
        if (gRenderListeners.preFrame &&
            !gRenderListeners.preFrame(gRenderListeners.userdata))
        {
            lite3d_render_stop();
            return LITE3D_FALSE;
        }

        if (!gRenderStarted)
            return LITE3D_FALSE;

        if (!update_render_targets())
        {
            lite3d_render_stop();
            return LITE3D_FALSE;
        }

        if (gRenderListeners.postFrame &&
            !gRenderListeners.postFrame(gRenderListeners.userdata))
        {
            lite3d_render_stop();
            return LITE3D_FALSE;
        }
    }

    /* refresh render statistic, render time span used */
    refresh_render_stats(gBeginFrameMark, SDL_GetPerformanceCounter());
    /* get time mark */
    gBeginFrameMark = SDL_GetPerformanceCounter();
    /* induce timers */
    LITE3D_METRIC_CALL(lite3d_timer_induce, (gBeginFrameMark, gPerfFreq))

    /* pump all available events */
    while (lite3d_render_loop_pump_event())
    {}
    /* finish gl operations and swap buffers */
    LITE3D_METRIC_CALL(lite3d_video_swap_buffers, ())
    return LITE3D_TRUE;
}

void lite3d_render_loop(lite3d_render_listeners *callbacks)
{
    lite3d_timer *frameStatsTimer = NULL;
    gRenderListeners = *callbacks;

    gPerfFreq = SDL_GetPerformanceFrequency();

    /* depth test enable by default */
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
    lite3d_array_init(&gInvalidatedCameras, sizeof(lite3d_camera *), 1);

    /* launch frame statistic compute timer */
    frameStatsTimer = lite3d_timer_add(1000, timer_render_stats_tick, NULL);

    /* start user initialization */
    if (!gRenderListeners.preRender || (gRenderListeners.preRender &&
        gRenderListeners.preRender(gRenderListeners.userdata)))
    {
        /* get time mark */
        gBeginFrameMark = SDL_GetPerformanceCounter();
        
        /* begin render loop */
        while (gRenderStarted)
        {
            LITE3D_METRIC_CALL(lite3d_render_frame, ())
        }
    }

    if (gRenderListeners.postRender)
        gRenderListeners.postRender(gRenderListeners.userdata);

    lite3d_timer_purge(frameStatsTimer);
    lite3d_render_target_erase_all();
    lite3d_render_target_purge(&gScreenRt);
    lite3d_array_purge(&gInvalidatedCameras);
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

    lite3d_render_target_erase(rt);
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

int lite3d_render_target_attach_camera(lite3d_render_target *target, lite3d_camera *camera, lite3d_scene *scene,
    uint16_t pass, const lite3d_framebuffer_layer *layer, size_t layersCount, int priority, uint32_t renderFlags)
{
    lookUnit *look = NULL;
    lookUnit *lookIns = NULL;
    lite3d_list_node *node = NULL;
    SDL_assert(target);
    SDL_assert(scene);
    SDL_assert(camera);

    if (!validate_renderflags(&renderFlags))
        return LITE3D_FALSE;

    lookIns = (lookUnit *) lite3d_calloc_pooled(LITE3D_POOL_NO1, sizeof (lookUnit));
    SDL_assert_release(lookIns);

    lookIns->camera = camera;
    lookIns->pass = pass;
    lookIns->priority = priority;
    lookIns->scene = scene;
    lookIns->renderFlags = renderFlags;
    lookIns->layersCount = layersCount;
    memcpy(lookIns->layer, layer, LITE3D_MIN(layersCount * sizeof(lite3d_framebuffer_layer), sizeof(lookIns->layer)));
    lite3d_list_link_init(&lookIns->rtLink);

    /* check camera already attached to the render target */
    node = &target->lookSequence.l;
    while ((node = lite3d_list_next(node)) != &target->lookSequence.l)
    {
        look = LITE3D_MEMBERCAST(lookUnit, node, rtLink);
        if (look->priority == priority)
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

int lite3d_render_target_dettach_camera(lite3d_render_target *rt, lite3d_camera *camera, int priority)
{
    lookUnit *look = NULL;
    lite3d_list_node *node = NULL;
    SDL_assert(rt);
    SDL_assert(camera);

    /* lookup for the camera */
    node = &rt->lookSequence.l;
    while ((node = lite3d_list_next(node)) != &rt->lookSequence.l)
    {
        look = LITE3D_MEMBERCAST(lookUnit, node, rtLink);
        if (look->camera == camera && look->priority == priority)
        {
            lite3d_list_unlink_link(node);
            lite3d_free_pooled(LITE3D_POOL_NO1, look);
            return LITE3D_TRUE;
        }
    }

    return LITE3D_FALSE;
}

int lite3d_render_target_screen_attach_camera(lite3d_camera *camera, lite3d_scene *scene,
    uint16_t pass, int priority, uint32_t renderFlags)
{
    return lite3d_render_target_attach_camera(&gScreenRt, camera, scene,
        pass, NULL, 0, priority, renderFlags);
}

int lite3d_render_target_screen_dettach_camera(lite3d_camera *camera, int priority)
{
    return lite3d_render_target_dettach_camera(&gScreenRt, camera, priority);
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
    if (rt == &gScreenRt)
    {
        lite3d_video_resize(width, height);
    }

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

void lite3d_render_target_screenshot(lite3d_render_target *rt, const char *filename)
{
    ILuint imageId;
    uint8_t *pixels;
    size_t pixelsSize = lite3d_framebuffer_size(&rt->fb, LITE3D_FRAMEBUFFER_READ_RGBA_INT8);
    
    SDL_assert(rt);
    pixels = lite3d_malloc(pixelsSize);
    if (!lite3d_framebuffer_read(&rt->fb, 0, LITE3D_FRAMEBUFFER_READ_RGBA_INT8, pixels))
    {
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: framebuffer read failed", LITE3D_CURRENT_FUNCTION);
        lite3d_free(pixels);
        return;
    }

    /* Reset alpha to full untransparent */
    for (int alpha = 3; alpha < pixelsSize; alpha += 4)
    {
        pixels[alpha] = 255;
    }
    
    lite3d_misc_il_error_stack_clean();
    imageId = ilGenImage();
    if (LITE3D_CHECK_IL_ERROR)
    {
        lite3d_free(pixels);
        return;
    }
    
    ilBindImage(imageId);
    ilTexImage(rt->fb.width, rt->fb.height, 1, 4, IL_RGBA, IL_UNSIGNED_BYTE, pixels);
    lite3d_free(pixels);
    
    if (LITE3D_CHECK_IL_ERROR)
        return;

    ilEnable(IL_FILE_OVERWRITE);
    ilSaveImage(filename);
    if (LITE3D_CHECK_IL_ERROR)
    {
        ilDeleteImage(imageId);
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
            "%s: save image failed", LITE3D_CURRENT_FUNCTION);
        return;
    }
    
    ilDeleteImage(imageId);
}

