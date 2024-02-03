/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2024 Sirius (Korolev Nikita)
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
#ifndef LITE3D_METRICS_H
#define	LITE3D_METRICS_H

#include <lite3d/lite3d_common.h>
#include <lite3d/lite3d_list.h>
#include <lite3d/lite3d_rb_tree.h>
#include <lite3d/lite3d_array.h>

#define LITE3D_MEASUREMENTS_MAX 3000
#define LITE3D_MEASUREMENTS_GROUPS 7

typedef struct lite3d_metric_node
{
    /* rb tree node entity */
    lite3d_rb_node cached;
    char name[LITE3D_MAX_METRIC_NAME];
    uint64_t maxMcs;
    uint64_t minMcs;
    uint64_t avgMcs;
    uint64_t count;

    struct lite3d_metric_distribution 
    {
        uint64_t lo;
        uint64_t hi;
        uint64_t hit;
        float percentage;
    } distribution[LITE3D_MEASUREMENTS_GROUPS];

    lite3d_array measurements;
} lite3d_metric_node;

typedef struct lite3d_metrics
{
    lite3d_rb_tree *metricsCache;
} lite3d_metrics;

LITE3D_CEXPORT int lite3d_metrics_global_init(void);
LITE3D_CEXPORT int lite3d_metrics_global_purge(void);
LITE3D_CEXPORT lite3d_metrics *lite3d_metrics_global_get(void);
LITE3D_CEXPORT int lite3d_metrics_global_insert(const char *name, uint64_t mcs);
LITE3D_CEXPORT int lite3d_metrics_global_write_to_log(void);

LITE3D_CEXPORT int lite3d_metrics_init(lite3d_metrics *metrics);
LITE3D_CEXPORT int lite3d_metrics_purge(lite3d_metrics *metrics);
LITE3D_CEXPORT int lite3d_metrics_insert(lite3d_metrics *metrics, const char *name, uint64_t mcs);
LITE3D_CEXPORT int lite3d_metrics_write_to_log(lite3d_metrics *metrics);

#ifdef LITE3D_WITH_METRICS
#define LITE3D_METRIC_CALL(method, args) \
    { \
        uint64_t call_delta_; \
        uint64_t call_bt_ = SDL_GetPerformanceCounter(); \
        method args; \
        call_delta_ = (SDL_GetPerformanceCounter() - call_bt_) / (SDL_GetPerformanceFrequency() / 1000000); \
        lite3d_metrics_global_insert(STR(method), call_delta_); \
    }

#define LITE3D_METRIC_CALLRET(method, ret, args) \
    { \
        uint64_t call_delta_; \
        uint64_t call_bt_ = SDL_GetPerformanceCounter(); \
        ret = method args; \
        call_delta_ = (SDL_GetPerformanceCounter() - call_bt_) / (SDL_GetPerformanceFrequency() / 1000000); \
        lite3d_metrics_global_insert(STR(method), call_delta_); \
    }
#else
#define LITE3D_METRIC_CALL(method, args) method args;
#define LITE3D_METRIC_CALLRET(method, ret, args) ret = method args;
#endif

#endif	/* LITE3D_METRICS_H */

