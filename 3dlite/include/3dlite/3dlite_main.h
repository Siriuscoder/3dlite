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
#ifndef LITE3D_MAIN_H
#define	LITE3D_MAIN_H

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_video.h>
#include <3dlite/3dlite_gl_texture_unit.h>
#include <3dlite/3dlite_alloc.h>
#include <3dlite/3dlite_logger.h>

typedef int (*lite3d_user_init_completed_t)(void *userdata);
typedef int (*lite3d_user_pre_shut_t)(void *userdata);

typedef struct lite3d_global_settings
{
    lite3d_video_settings videoSettings;
    lite3d_texture_technique_settings textureSettings;
    lite3d_alloca_f userAllocator;

    lite3d_user_init_completed_t initCompleted;
    lite3d_user_pre_shut_t preShut;
    int8_t logLevel;
} lite3d_global_settings;

LITE3D_CEXPORT int lite3d_main(const lite3d_global_settings *settings);
LITE3D_CEXPORT const lite3d_global_settings *lite3d_get_global_settings(void);


#endif	/* MAIN_H */
