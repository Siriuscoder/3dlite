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
#pragma once

#ifndef LITE3D_SCENE_H
#define	LITE3D_SCENE_H

#include <3dlite/3dlite_common.h>
#include <3dlite/3dlite_camera.h>

typedef struct lite3d_scene
{
    uint8_t tmp;
} lite3d_scene;

LITE3D_CEXPORT void lite3d_render_scene(lite3d_scene *scene, lite3d_camera *camera);

#endif	/* LITE3D_SCENE_H */

