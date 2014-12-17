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
#include <SDL.h>

#include <3dlite/3dlite_main.h>
#include <3dlite/3dlite_logger.h>

static lite3d_global_settings gGlobalSettings;

int lite3d_main(const lite3d_global_settings *settings)
{
    SDL_version sdlVers;
    gGlobalSettings = *settings;
    /* begin 3dlite initialization */
    /* setup memory */
    lite3d_init_memory(&gGlobalSettings.userAllocator);
    /* setup SDL */
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
    /* setup logger */
    lite3d_setup_stdout_logger();
    lite3d_set_loglevel(gGlobalSettings.logLevel);

    SDL_GetVersion(&sdlVers);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "===== 3dlite started ======");
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION,
        "SDL Version %d:%d:%d", (int)sdlVers.major, (int)sdlVers.minor, (int)sdlVers.patch);
    /* setup video */
    if(!lite3d_setup_video(&gGlobalSettings.videoSettings))
    {
        lite3d_cleanup_memory();
        return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

const lite3d_global_settings *lite3d_get_global_settings(void)
{
    return &gGlobalSettings;
}

