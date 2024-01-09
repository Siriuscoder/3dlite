/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2024  Sirius (Korolev Nikita)
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
#ifndef LITE3D_MAIN_H
#define LITE3D_MAIN_H


#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_video.h>
#include <lite3d/lite3d_texture_unit.h>
#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_logger.h>
#include <lite3d/lite3d_render.h>
#include <lite3d/lite3d_mesh.h>
#include <lite3d/lite3d_mesh_loader.h>
#include <lite3d/lite3d_scene.h>
#include <lite3d/lite3d_timer.h>
#include <lite3d/lite3d_query.h>


typedef int (*lite3d_user_init_completed_t)(void *userdata);
typedef int (*lite3d_user_shut_t)(void *userdata);


typedef struct lite3d_global_settings
{
    lite3d_video_settings videoSettings;
    lite3d_texture_technique_settings textureSettings;
    lite3d_alloca_f userAllocator;
    lite3d_render_listeners renderLisneters;

    size_t maxFileCacheSize;
    int logLevel;
    int logFlushAlways;
    int logMuteStd;
    char logFile[50];
} lite3d_global_settings;


LITE3D_CEXPORT int lite3d_main(const lite3d_global_settings *settings);
LITE3D_CEXPORT const lite3d_global_settings *lite3d_get_global_settings(void);


#endif	/* MAIN_H */
