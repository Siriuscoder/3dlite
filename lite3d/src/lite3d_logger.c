/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
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
#include <stdio.h>
#include <SDL_log.h>
#include <SDL_timer.h>

#include <assimp/cimport.h>
#include <lite3d/lite3d_logger.h>
    
static struct aiLogStream aiLogStream;
static uint8_t gFlushAlways = LITE3D_FALSE;

static void log_to_file(FILE *desc, int category, 
    SDL_LogPriority priority, const char* message)
{
    fprintf(desc, "[%10d]:%s:%s : %s\n", 
        SDL_GetTicks(),
        (priority == SDL_LOG_PRIORITY_VERBOSE ? "note" :
        (priority == SDL_LOG_PRIORITY_DEBUG ? "debug" : 
        (priority == SDL_LOG_PRIORITY_INFO ? "info" :
        (priority == SDL_LOG_PRIORITY_WARN ? "warn" :
        (priority == SDL_LOG_PRIORITY_ERROR ? "error" : "critical"))))),
        category == SDL_LOG_CATEGORY_APPLICATION ? "app" : "sdl",
        message);

    if(gFlushAlways)
        fflush(desc);
}

static void std_output_function(void* userdata, int category, 
    SDL_LogPriority priority, const char* message)
{
    FILE *outdesc = stdout;
    
    if(priority >= SDL_LOG_PRIORITY_ERROR)
        outdesc = stderr;
        
    log_to_file(outdesc, category, priority, message);
}

static void aiLogFunc(const char* message , char* user)
{
    SDL_LogDebug(SDL_LOG_CATEGORY_APPLICATION, "Assimp: %s",
        message);
}

static void file_output_function(void* userdata, int category, 
    SDL_LogPriority priority, const char* message)
{

}

void lite3d_logger_set_logParams(int8_t level, int8_t flushAlways)
{
    SDL_LogSetAllPriority(level == LITE3D_LOGLEVEL_ERROR ? SDL_LOG_PRIORITY_ERROR : 
        (level == LITE3D_LOGLEVEL_INFO ? SDL_LOG_PRIORITY_INFO : SDL_LOG_PRIORITY_VERBOSE));

    aiEnableVerboseLogging(level == LITE3D_LOGLEVEL_VERBOSE ? AI_TRUE : AI_FALSE);
    flushAlways = flushAlways;
}

void lite3d_logger_setup_stdout(void)
{
    SDL_LogSetOutputFunction(std_output_function, NULL);
    aiLogStream.callback = aiLogFunc;
    aiLogStream.user = NULL;

	aiAttachLogStream(&aiLogStream);
}

void lite3d_logger_release(void)
{
    aiDetachAllLogStreams();
}
