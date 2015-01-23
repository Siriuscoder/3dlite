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
#include <string.h>
#include <SDL_timer.h>
#include <SDL_assert.h>

#include <3dlite/GL/glew.h>
#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_render.h>
#include <3dlite/3dlite_video.h>
#include <3dlite/3dlite_scene.h>

static uint64_t gLastMark = 0;
static uint64_t gPerfFreq = 0;
static int32_t gFPSCounter = 0;
static lite3d_list gRenderTargets;
static lite3d_render_listeners gRenderListeners;
static uint8_t gRenderStarted = LITE3D_TRUE;
static uint8_t gRenderActive = LITE3D_TRUE;
static lite3d_render_stats gRenderStats;

static void calc_render_stats(uint64_t beginFrame, uint64_t endFrame)
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

    gRenderStats.triangleByBatch = gRenderStats.batchesByFrame ? gRenderStats.trianglesByFrame / gRenderStats.batchesByFrame : 0;
    gRenderStats.triangleMs = gRenderStats.trianglesByFrame ? (float)gRenderStats.lastFPS / (float)gRenderStats.trianglesByFrame : 0;

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
        if (gRenderStats.worstFPS > gRenderStats.lastFPS)
            gRenderStats.worstFPS = gRenderStats.worstFPS;

        gRenderStats.avrFPS = (gRenderStats.avrFPS + gRenderStats.lastFPS) / 2;
    }
}

static void update_render_target(lite3d_render_target *target)
{
    lite3d_list_node *node;
    lite3d_scene *scene;
    lite3d_camera *camera;
    /* TODO: switch target framebuffer */
    /* set viewport */
    glViewport(0, 0, target->width, target->height);
    /* clear target */
    glClear(target->cleanMask);
    /* do paint by render queue */
    for (node = target->renderQueue.l.next; node != &target->renderQueue.l; node = lite3d_list_next(node))
    {
        camera = MEMBERCAST(lite3d_camera, node, renderTargetLink);
        scene = (lite3d_scene *)camera->cameraNode.scene;
        lite3d_scene_render(scene, camera);

        /* accamulate statistic */
        gRenderStats.trianglesByFrame += scene->stats.trianglesRendered;
        gRenderStats.objectsByFrame += scene->stats.objectsRendered;
        gRenderStats.batchesByFrame += scene->stats.batches;
        gRenderStats.materialsByFrame += scene->stats.materialBlocks;
        gRenderStats.materialsPassedByFrame += scene->stats.materialPassed;
        gRenderStats.textureUnitsByFrame += scene->stats.textureUnits;
    }
}

static int update_render_targets(void)
{
    int32_t targetsCount = 0;
    lite3d_list_node *node;
    lite3d_render_target *target;

    for (node = gRenderTargets.l.next; node != &gRenderTargets.l; node = lite3d_list_next(node))
    {
        target = MEMBERCAST(lite3d_render_target, node, node);
        if (!target->enabled)
            continue;

        if (target->preUpdate)
            target->preUpdate(target);

        update_render_target(target);

        if (target->postUpdate)
            target->postUpdate(target);

        if (target->isRoot)
            lite3d_video_swap_buffers();

        targetsCount++;
    }

    gRenderStats.renderTargets = targetsCount;
    return targetsCount ? LITE3D_TRUE : LITE3D_FALSE;
}

void lite3d_render_loop(lite3d_render_listeners *callbacks)
{
    SDL_Event wevent;
    uint64_t beginFrameMark;
    gRenderListeners = *callbacks;

    memset(&gRenderStats, 0, sizeof(gRenderStats));
    if (gRenderListeners.preRender && !gRenderListeners.preRender(gRenderListeners.userdata))
        return;

    while (gRenderStarted)
    {
        gRenderStats.trianglesByFrame = 
        gRenderStats.objectsByFrame = 
        gRenderStats.batchesByFrame = 
        gRenderStats.materialsByFrame = 
        gRenderStats.materialsPassedByFrame = 
        gRenderStats.textureUnitsByFrame = 0;

        beginFrameMark = SDL_GetPerformanceCounter();

        if (gRenderActive)
        {
            if (gRenderListeners.preFrame && !gRenderListeners.preFrame(gRenderListeners.userdata))
                break;
            if (!update_render_targets())
                break;
            if (gRenderListeners.postFrame && !gRenderListeners.postFrame(gRenderListeners.userdata))
                break;
        }

        calc_render_stats(beginFrameMark, SDL_GetPerformanceCounter());

        while (SDL_PollEvent(&wevent))
        {
            if (gRenderListeners.processEvent && !gRenderListeners.processEvent(&wevent, gRenderListeners.userdata))
            {
                lite3d_render_stop();
                break;
            }
        }
    }

    if (gRenderListeners.postRender)
        gRenderListeners.postRender(gRenderListeners.userdata);

    lite3d_render_target_erase_all();
}

lite3d_render_stats *lite3d_render_get_stats(void)
{
    return &gRenderStats;
}

lite3d_render_target *lite3d_render_target_add(int32_t ID, int32_t width,
    int32_t height, int8_t isRoot, void *userdata)
{
    lite3d_render_target *target;
    if(lite3d_render_target_get(ID))
        return NULL;

    target = (lite3d_render_target *) lite3d_calloc(sizeof (lite3d_render_target));
    SDL_assert(target);

    lite3d_list_link_init(&target->node);

    target->height = height;
    target->width = width;
    target->isRoot = isRoot;
    target->userdata = userdata;
    target->enabled = LITE3D_TRUE;
    target->ID = ID;
    lite3d_list_init(&target->renderQueue);
    target->cleanMask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
    
    lite3d_list_add_first_link(&target->node, &gRenderTargets);
    return target;
}

void lite3d_render_target_erase(int32_t ID)
{
    lite3d_list_node *node;
    lite3d_render_target *target;
    for (node = gRenderTargets.l.next; node != &gRenderTargets.l; node = lite3d_list_next(node))
    {
        target = MEMBERCAST(lite3d_render_target, node, node);
        if (target->ID == ID)
        {
            lite3d_list_unlink_link(node);
            lite3d_free(target);
            break;
        }
    }
}

lite3d_render_target *lite3d_render_target_get(int32_t ID)
{
    lite3d_list_node *node;
    lite3d_render_target *target;
    for (node = gRenderTargets.l.next; node != &gRenderTargets.l; node = lite3d_list_next(node))
    {
        target = MEMBERCAST(lite3d_render_target, node, node);
        if (target->ID == ID)
            return target;
    }

    return NULL;
}

void lite3d_render_target_erase_all(void)
{
    lite3d_list_node *node = NULL;
    while ((node = lite3d_list_remove_first_link(&gRenderTargets)) != NULL)
        lite3d_free(MEMBERCAST(lite3d_render_target, node, node));
}

void lite3d_render_suspend(void)
{
    gRenderActive = LITE3D_TRUE;
}

void lite3d_render_pause(void)
{
    gRenderActive = LITE3D_FALSE;
}

void lite3d_render_stop(void)
{
    gRenderStarted = LITE3D_FALSE;
}

int lite3d_render_init(void)
{
    gPerfFreq = SDL_GetPerformanceFrequency();
    memset(&gRenderStats, 0, sizeof (gRenderStats));
    lite3d_list_init(&gRenderTargets);
    
    /* init common GL variables */
    glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
    /* enable depth test */
    glClearDepth(1.0);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    /* for fixed pipeline GL operations */
    glShadeModel(GL_SMOOTH);

    return LITE3D_TRUE;
}

void lite3d_render_target_attach_camera(lite3d_render_target *target, lite3d_camera *camera)
{
    SDL_assert(target && camera);

    lite3d_list_link_init(&camera->renderTargetLink);
    lite3d_list_add_first_link(&camera->renderTargetLink, &target->renderQueue);
}

void lite3d_render_target_dettach_camera(lite3d_camera *camera)
{
    SDL_assert(camera);
    lite3d_list_unlink_link(&camera->renderTargetLink);
}

void lite3d_render_target_root_attach_camera(lite3d_camera *camera)
{
    lite3d_render_target_attach_camera(lite3d_render_target_get(0), camera);
}