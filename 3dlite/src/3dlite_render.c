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
#include <3dlite/3dlite_render.h>
#include <3dlite/3dlite_video.h>

static lite3d_render_listeners gRenderCallbacks;
static lite3d_render_stats gRenderStats;

void lite3d_render_loop(lite3d_render_listeners *callbacks)
{
    SDL_Event wevent;
    uint8_t starting = LITE3D_TRUE;
    gRenderCallbacks = *callbacks;
    memset(&gRenderStats, 0, sizeof (gRenderStats));

    if (gRenderCallbacks.preRender && !gRenderCallbacks.preRender(gRenderCallbacks.userdata))
        return;

    while (starting)
    {
        if (gRenderCallbacks.preFrame && !gRenderCallbacks.preFrame(gRenderCallbacks.userdata))
            break;
        if (gRenderCallbacks.renderFrame && !gRenderCallbacks.renderFrame(gRenderCallbacks.userdata))
            break;
        if (gRenderCallbacks.postFrame && !gRenderCallbacks.postFrame(gRenderCallbacks.userdata))
            break;

        lite3d_swap_buffers();
        while (SDL_PollEvent(&wevent))
        {
            if (gRenderCallbacks.processEvent && !gRenderCallbacks.processEvent(&wevent, gRenderCallbacks.userdata))
            {
                starting = LITE3D_FALSE;
                break;
            }
        }
    }

    if (gRenderCallbacks.postRender)
        gRenderCallbacks.postRender(gRenderCallbacks.userdata);
}

lite3d_render_stats *lite3d_get_render_stats(void)
{
    return &gRenderStats;
}