/******************************************************************************
*	This file is part of lite3d (Light-weight 3d engine).
*	Copyright (C) 2026 Sirius (Korolev Nikita)
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

#define LITE3D_MEASUREMENTS_WARMUP 1000
#define LITE3D_MEASUREMENTS_GROUPS 7

/*
 * Metrics collect raw measurements only during warmup. When the warmup buffer
 * reaches LITE3D_MEASUREMENTS_WARMUP samples, the current min/max range is used
 * to build LITE3D_MEASUREMENTS_GROUPS logarithmic buckets. The first bucket
 * always starts at 0 and ends at minMcs; the remaining buckets cover minMcs..
 * maxMcs with narrower ranges near minMcs and wider ranges near maxMcs.
 *
 * Warmup samples are replayed into the buckets once, then the raw buffer is
 * cleared. Further inserts update only the matching bucket hit counter, so
 * distribution tracking does not keep growing with the number of calls.
 */
typedef struct lite3d_metric_node
{
    /* rb tree node entity */
    lite3d_rb_node cached;
    char name[LITE3D_MAX_METRIC_NAME];
    double maxMcs;
    double minMcs;
    double avgMcs;
    uint64_t count;
    uint8_t distributionReady;

    struct lite3d_metric_distribution 
    {
        double lo;
        double hi;
        uint64_t hit;
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
LITE3D_CEXPORT int lite3d_metrics_global_insert(const char *name, double mcs);
LITE3D_CEXPORT int lite3d_metrics_global_write_to_log(void);

LITE3D_CEXPORT int lite3d_metrics_init(lite3d_metrics *metrics);
LITE3D_CEXPORT int lite3d_metrics_purge(lite3d_metrics *metrics);
LITE3D_CEXPORT int lite3d_metrics_insert(lite3d_metrics *metrics, const char *name, double mcs);
LITE3D_CEXPORT int lite3d_metrics_write_to_log(lite3d_metrics *metrics);

#ifdef LITE3D_WITH_METRICS
#define LITE3D_METRIC_CALL(method, args) \
    { \
        double call_delta_; \
        uint64_t call_bt_ = SDL_GetPerformanceCounter(); \
        method args; \
        call_delta_ = (SDL_GetPerformanceCounter() - call_bt_) / (SDL_GetPerformanceFrequency() / 1000000.0f); \
        lite3d_metrics_global_insert(STR(method), call_delta_); \
    }

#define LITE3D_METRIC_CALLRET(method, ret, args) \
    { \
        double call_delta_; \
        uint64_t call_bt_ = SDL_GetPerformanceCounter(); \
        ret = method args; \
        call_delta_ = (SDL_GetPerformanceCounter() - call_bt_) / (SDL_GetPerformanceFrequency() / 1000000.0f); \
        lite3d_metrics_global_insert(STR(method), call_delta_); \
    }
#else
#define LITE3D_METRIC_CALL(method, args) method args;
#define LITE3D_METRIC_CALLRET(method, ret, args) ret = method args;
#endif

#endif	/* LITE3D_METRICS_H */
