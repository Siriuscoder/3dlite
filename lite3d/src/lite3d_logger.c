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
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <SDL_log.h>
#include <SDL_timer.h>

#include <lite3d/lite3d_logger.h>
#include <lite3d/lite3d_mesh_assimp_loader.h>

static int gFlushAlways = LITE3D_FALSE;
static FILE *gOutFile = NULL;
static int gMuteStd = LITE3D_FALSE;

static void log_to_file(FILE *desc, int category,
    SDL_LogPriority priority, const char* message)
{
    if (!desc)
        return;

    fprintf(desc, "[%10d]:%s:%s : %s\n",
        SDL_GetTicks(),
        (priority == SDL_LOG_PRIORITY_VERBOSE ? "note" :
        (priority == SDL_LOG_PRIORITY_DEBUG ? "debug" :
        (priority == SDL_LOG_PRIORITY_INFO ? "info" :
        (priority == SDL_LOG_PRIORITY_WARN ? "warn" :
        (priority == SDL_LOG_PRIORITY_ERROR ? "error" : "critical"))))),
        category == SDL_LOG_CATEGORY_APPLICATION ? "app" : "sdl",
        message);

    if (gFlushAlways)
        fflush(desc);
}

static void std_output_function(void* userdata, int category,
    SDL_LogPriority priority, const char* message)
{
    FILE *outdesc = stdout;
    if (gMuteStd)
        return;

    if (priority >= SDL_LOG_PRIORITY_ERROR)
        outdesc = stderr;

    log_to_file(outdesc, category, priority, message);
}

static void file_output_function(void* userdata, int category,
    SDL_LogPriority priority, const char* message)
{
    std_output_function(userdata, category, priority, message);
    log_to_file(gOutFile, category, priority, message);
}

void lite3d_logger_set_logParams(int level, int flushAlways, int muteStd)
{
    SDL_LogSetAllPriority(level == LITE3D_LOGLEVEL_ERROR ? SDL_LOG_PRIORITY_WARN :
        (level == LITE3D_LOGLEVEL_INFO ? SDL_LOG_PRIORITY_INFO : SDL_LOG_PRIORITY_VERBOSE));

    gFlushAlways = flushAlways;
    gMuteStd = muteStd;
#ifdef INCLUDE_ASSIMP
    lite3d_assimp_logging_level(level);
#endif
}

void lite3d_logger_setup_stdout(void)
{
    SDL_LogSetOutputFunction(std_output_function, NULL);

#ifdef INCLUDE_ASSIMP
    lite3d_assimp_logging_init();
#endif

}

void lite3d_logger_setup_file(const char *logfile)
{
    SDL_LogSetOutputFunction(file_output_function, NULL);
    if ((gOutFile = fopen(logfile, "a")) == NULL)
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s: Could not open log file: %s",
        LITE3D_CURRENT_FUNCTION, strerror(errno));

#ifdef INCLUDE_ASSIMP
    lite3d_assimp_logging_init();
#endif
}

void lite3d_logger_release(void)
{
    if (gOutFile)
    {
        fclose(gOutFile);
        gOutFile = NULL;
    }

#ifdef INCLUDE_ASSIMP
    lite3d_assimp_logging_release();
#endif
}

