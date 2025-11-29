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
#include <lite3d/lite3d_main.h>

#include <stdlib.h>
#include <stdio.h>
#include <SDL.h>

#include <lite3d/lite3d_metrics.h>

static lite3d_global_settings gGlobalSettings;

static int lite3d_sdl_init(void)
{
    SDL_InitFlags subSystems = SDL_INIT_VIDEO | SDL_INIT_EVENTS;
    int32_t compiledVers = SDL_VERSION;  /* hardcoded number from SDL headers */
    int32_t linkedVers = SDL_GetVersion();  /* reported by linked SDL library */

    if (SDL_VERSIONNUM_MAJOR(compiledVers) != 
        SDL_VERSIONNUM_MAJOR(linkedVers))
    {
        printf("%s: SDL major version mismatch, compiled: %d, present: %d",
            LITE3D_CURRENT_FUNCTION,
            SDL_VERSIONNUM_MAJOR(compiledVers),
            SDL_VERSIONNUM_MAJOR(linkedVers));

        return LITE3D_FALSE;
    }

    if (gGlobalSettings.appID[0])
    {
        if (!SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_IDENTIFIER_STRING, gGlobalSettings.appID))
        {
            printf("%s: set identifier string failed: %s",
                LITE3D_CURRENT_FUNCTION,
                SDL_GetError());
        }
    }

    if (gGlobalSettings.appName[0])
    {
        if (!SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_NAME_STRING, gGlobalSettings.appName))
        {
            printf("%s: set app name failed: %s",
                LITE3D_CURRENT_FUNCTION,
                SDL_GetError());
        }
    }

    if (gGlobalSettings.appVersion[0])
    {
        if (!SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_VERSION_STRING, gGlobalSettings.appVersion))
        {
            printf("%s: set version failed: %s",
                LITE3D_CURRENT_FUNCTION,
                SDL_GetError());
        }
    }

    if (gGlobalSettings.appCreator[0])
    {
        if (!SDL_SetAppMetadataProperty(SDL_PROP_APP_METADATA_CREATOR_STRING, gGlobalSettings.appCreator))
        {
            printf("%s: set creator failed: %s",
                LITE3D_CURRENT_FUNCTION,
                SDL_GetError());
        }
    }

    if (SDL_WasInit(subSystems) != subSystems)
    {
        if (!SDL_Init(subSystems))
        {
            printf("%s: SDL startup failed: %s",
                LITE3D_CURRENT_FUNCTION, SDL_GetError());
            return LITE3D_FALSE;
        }
    }

    SDL_LogInfo(
        SDL_LOG_CATEGORY_APPLICATION,
        "SDL subsystems is initialized, version %d.%d.%d",
        SDL_VERSIONNUM_MAJOR(linkedVers),
        SDL_VERSIONNUM_MINOR(linkedVers),
        SDL_VERSIONNUM_MICRO(linkedVers));

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
    lite3d_logger_setup(settings->logFile);

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
    if (!lite3d_sdl_init())
    {
        goto ret_metrics;
    }

    if (!SDL_SetCurrentThreadPriority(SDL_THREAD_PRIORITY_HIGH))
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Could not insrease thread priority %s",
            SDL_GetError());
    }

    /* setup video */
    if (!lite3d_video_open(
          &gGlobalSettings.videoSettings,
          settings->logMuteStd))
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Could not setup video");

        goto ret_sdl_quit;
    }

    /* setup textures technique */
    if (!lite3d_texture_technique_init(&gGlobalSettings.textureSettings))
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Could not setup textures technique "
            "(lite3d_texture_technique_init)");

        goto ret_video_close;
    }

    /* setup textures technique */
    if (!lite3d_vbo_technique_init())
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Could not setup VBO technique");
        goto ret_texture_shut;
    }

    /* setup vao technique */
    if (!lite3d_vao_technique_init())
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Could not setup VAO technique");

        goto ret_texture_shut;
    }

    /* setup shaders technique */
    if (!lite3d_shader_program_technique_init())
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Could not setup shaders technique");

        goto ret_texture_shut;
    }

    if (!lite3d_timer_technique_init())
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Could not setup timer technique");

        goto ret_texture_shut;
    }

    /* init shader global parameters */
    lite3d_shader_global_parameters_init();

    if (!lite3d_framebuffer_technique_init())
    {
        SDL_LogError(
            SDL_LOG_CATEGORY_APPLICATION,
            "Could not setup framebuffer techique");

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
