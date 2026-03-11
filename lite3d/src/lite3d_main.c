/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025  Sirius (Korolev Nikita)
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
 *****************************************************************************/


#include <stdlib.h>
#include <SDL.h>

#include <lite3d/lite3d_main.h>
#include <lite3d/lite3d_metrics.h>


static lite3d_global_settings gGlobalSettings;


static int sdl_init(void)
{
    uint32_t subSystems = SDL_INIT_VIDEO |
                          SDL_INIT_TIMER |
                          SDL_INIT_EVENTS;

    SDL_version compiledVers, linkedVers;

    SDL_VERSION(&compiledVers);
    SDL_GetVersion(&linkedVers);

    if (compiledVers.major != linkedVers.major)
    {
        SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
            "%s: SDL major version mismatch, compiled: %d, linked: %d",
            LITE3D_CURRENT_FUNCTION, (int)compiledVers.major, (int)linkedVers.major);

        return LITE3D_FALSE;
    }

    if (SDL_WasInit(subSystems) != subSystems)
    {

#ifdef SDL_HINT_APP_NAME
        SDL_SetHint(SDL_HINT_APP_NAME, "lite3d app");
#endif

#ifdef SDL_HINT_WINDOWS_DPI_AWARENESS
        // "permonitorv2": Request per-monitor V2 DPI awareness. (Windows 10, version 1607 and later). 
        // The most visible difference from "permonitor" is that window title bar will be scaled to the visually
        // correct size when dragging between monitors with different scale factors. This is the preferred DPI awareness level.
        SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");
#endif

#ifdef SDL_HINT_VIDEO_HIGHDPI_DISABLED
        // Allow HiDPI
        SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "0");
#endif

        if (SDL_Init(subSystems) != 0)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
                "%s: SDL startup failed: %s\n",
                LITE3D_CURRENT_FUNCTION, SDL_GetError());
            return LITE3D_FALSE;
        }
    }

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "SDL Version %d.%d.%d",
        (int) linkedVers.major,
        (int) linkedVers.minor,
        (int) linkedVers.patch);

    return LITE3D_TRUE;
}


int lite3d_main(const lite3d_global_settings *settings)
{
    int ret = LITE3D_FALSE;

    gGlobalSettings = *settings;

    /* begin lite3d initialization */

    /* setup memory */
    lite3d_memory_init(&gGlobalSettings.userAllocator);

    /* cleanup engine memory at exit */
    atexit(lite3d_memory_cleanup);

    /* setup logger */
    lite3d_logger_setup(gGlobalSettings.logFile);

    lite3d_logger_set_logParams(
        gGlobalSettings.logLevel,
        gGlobalSettings.logFlushAlways,
        gGlobalSettings.logMuteStd);

    if (!lite3d_metrics_global_init())
    {
        goto ret_release_logger;
    }

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "=================== LITE3D " LITE3D_FULL_VERSION " ===================");

    /* setup SDL */
    if (!sdl_init())
    {
        goto ret_metrics;
    }

    if (SDL_SetThreadPriority(SDL_THREAD_PRIORITY_HIGH))
    {
        SDL_LogWarn(
            SDL_LOG_CATEGORY_APPLICATION,
            "Could not increase thread priority %s",
            SDL_GetError());
    }

    /* setup video */
    if (!lite3d_video_open(
          &gGlobalSettings.videoSettings,
          settings->logMuteStd))
    {
        goto ret_sdl_quit;
    }

    /* setup textures technique */
    if (!lite3d_texture_technique_init(&gGlobalSettings.textureSettings))
    {
        goto ret_video_close;
    }

    /* setup textures technique */
    if (!lite3d_vbo_technique_init())
    {
        goto ret_texture_shut;
    }

    /* setup vao technique */
    if (!lite3d_vao_technique_init())
    {
        goto ret_texture_shut;
    }

    /* setup shaders technique */
    if (!lite3d_shader_program_technique_init())
    {
        goto ret_texture_shut;
    }

    if (!lite3d_timer_technique_init())
    {
        goto ret_texture_shut;
    }

    /* init shader global parameters */
    lite3d_shader_global_parameters_init();

    if (!lite3d_framebuffer_technique_init())
    {
        goto ret_texture_shut;
    }

    lite3d_query_technique_init();

    /* start main loop */
    lite3d_render_loop(&gGlobalSettings.renderLisneters);
    ret = LITE3D_TRUE;

#ifdef LITE3D_WITH_METRICS
    lite3d_metrics_global_write_to_log();
#endif

ret_texture_shut:
    lite3d_texture_technique_shut();

ret_video_close:
    lite3d_video_close();

ret_sdl_quit:
    SDL_Quit();

ret_metrics:
    lite3d_metrics_global_purge();

ret_release_logger:
    lite3d_logger_release();

    return ret;
}


const lite3d_global_settings *lite3d_get_global_settings(void)
{
    return &gGlobalSettings;
}
