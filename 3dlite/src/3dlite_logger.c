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
#include <stdio.h>
#include <SDL_log.h>

#include <assimp/cimport.h>
#include <3dlite/3dlite_logger.h>
    
struct aiLogStream aiLogStream;

static void log_to_file(FILE *desc, int category, 
    SDL_LogPriority priority, const char* message)
{
    fprintf(desc, "%s(%s): %s\n", (priority == SDL_LOG_PRIORITY_VERBOSE ? "note" :
        (priority == SDL_LOG_PRIORITY_DEBUG ? "debug" : 
        (priority == SDL_LOG_PRIORITY_INFO ? "info" :
        (priority == SDL_LOG_PRIORITY_WARN ? "warn" :
        (priority == SDL_LOG_PRIORITY_ERROR ? "error" : "critical"))))),
        category == SDL_LOG_CATEGORY_APPLICATION ? "app" : "sdl",
        message);
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

void lite3d_logger_set_loglevel(int8_t level)
{
    SDL_LogSetAllPriority(level == LITE3D_LOGLEVEL_ERROR ? SDL_LOG_PRIORITY_ERROR : 
        (level == LITE3D_LOGLEVEL_INFO ? SDL_LOG_PRIORITY_INFO : SDL_LOG_PRIORITY_VERBOSE));

    aiEnableVerboseLogging(level == LITE3D_LOGLEVEL_VERBOSE ? AI_TRUE : AI_FALSE);
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

