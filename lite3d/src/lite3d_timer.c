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
#include <SDL_assert.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_timer.h>

lite3d_list gTimers;

int lite3d_timer_technique_init()
{
    lite3d_list_init(&gTimers);
    return LITE3D_TRUE;
}

lite3d_timer *lite3d_timer_add(int32_t milli,
    lite3d_timer_func timerCallback, void *userdata)
{
    lite3d_timer *timer = (lite3d_timer *)
        lite3d_calloc_pooled(LITE3D_POOL_NO1, sizeof (lite3d_timer));

    lite3d_list_link_init(&timer->link);
    lite3d_list_add_last_link(&timer->link, &gTimers);

    timer->interval = milli;
    timer->ontimer = timerCallback;
    timer->lastTick = 0;
    timer->enabled = LITE3D_TRUE;
    timer->userdata = userdata;

    return timer;
}

void lite3d_timer_purge(lite3d_timer *timer)
{
    SDL_assert(timer);
    lite3d_list_unlink_link(&timer->link);
    lite3d_free_pooled(LITE3D_POOL_NO1, timer);
}

void lite3d_timer_induce(uint64_t timeMark, uint64_t freq)
{
    lite3d_list_node *link;
    lite3d_timer *timer;

    /* check all timers */
    for (link = gTimers.l.next; link != &gTimers.l; link = lite3d_list_next(link))
    {
        timer = LITE3D_MEMBERCAST(lite3d_timer, link, link);
        if (!timer->enabled)
            continue;
        
        freq /= 1000;
        if ((timeMark - timer->lastTick) >= (freq * timer->interval))
        {
            if (timer->lastTick && timer->ontimer)
                timer->ontimer(timer);

            timer->lastTick = timeMark;
        }
    }
}
