/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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
#ifndef LITE3D_TIMER_H
#define	LITE3D_TIMER_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_list.h>

typedef struct lite3d_timer
{
    lite3d_list_node link;
    void (*ontimer)(struct lite3d_timer *timer);
    int32_t interval;
    uint64_t lastTimeUpdate;
    uint8_t enabled;
    int32_t lag;
    void *userdata;
} lite3d_timer;

typedef void (*lite3d_timer_func)(lite3d_timer *timer);

LITE3D_CEXPORT int lite3d_timer_technique_init();
LITE3D_CEXPORT lite3d_timer *lite3d_timer_add(int32_t milli, 
    lite3d_timer_func timerCallback, void *userdata);
LITE3D_CEXPORT void lite3d_timer_purge(lite3d_timer *timer);

void lite3d_timer_induce(uint64_t timeMark, uint64_t freq);

#endif	/* LITE3D_TIMER_H */

