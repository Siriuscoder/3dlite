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

#include <3dlite/3dlite_video.h>

static lite3d_video_settings gVideoSettings;

int lite3d_setup_video(const lite3d_video_settings *settings)
{
    gVideoSettings = *settings;

    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_EVENTS);
}

const lite3d_video_settings *lite3d_get_video_settings(void)
{
    return &gVideoSettings;
}

int lite3d_close_video(void)
{

}
