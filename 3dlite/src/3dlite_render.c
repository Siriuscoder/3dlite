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

#include <3dlite/3dlite_render.h>
#include <3dlite/3dlite_video.h>

static uint64_t gLastMark = 0;
static uint64_t gPerfFreq = 0;
static int32_t gFPSCounter = 0;
static lite3d_render_listeners gRenderListeners;
static lite3d_render_stats gRenderStats = {
    0, 0, 0, 0, 0.0, 0.0, 0.0, 0.0
};

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

static void update_render_targets(void)
{
    
}

void lite3d_render_loop(lite3d_render_listeners *callbacks)
{
    SDL_Event wevent;
    uint64_t beginFrameMark;
    gPerfFreq = SDL_GetPerformanceFrequency();
    uint8_t starting = LITE3D_TRUE;
    gRenderListeners = *callbacks;
    memset(&gRenderStats, 0, sizeof (gRenderStats));

    if (gRenderListeners.preRender && !gRenderListeners.preRender(gRenderListeners.userdata))
        return;

    while (starting)
    {
        beginFrameMark = SDL_GetPerformanceCounter();
        if (gRenderListeners.preFrame && !gRenderListeners.preFrame(gRenderListeners.userdata))
            break;
        if (gRenderListeners.renderFrame && !gRenderListeners.renderFrame(gRenderListeners.userdata))
            break;
        if (gRenderListeners.postFrame && !gRenderListeners.postFrame(gRenderListeners.userdata))
            break;

        lite3d_swap_buffers();
        calc_render_stats(beginFrameMark, SDL_GetPerformanceCounter());

        while (SDL_PollEvent(&wevent))
        {
            if (gRenderListeners.processEvent && !gRenderListeners.processEvent(&wevent, gRenderListeners.userdata))
            {
                starting = LITE3D_FALSE;
                break;
            }
        }
    }

    if (gRenderListeners.postRender)
        gRenderListeners.postRender(gRenderListeners.userdata);
}

lite3d_render_stats *lite3d_get_render_stats(void)
{
    return &gRenderStats;
}