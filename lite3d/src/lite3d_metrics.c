/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_metrics.h>
#include <lite3d/lite3d_alloc.h>

static lite3d_metrics globalMetrics = { NULL };

static void metric_node_delete(lite3d_rb_node *x)
{
    lite3d_metric_node *node = LITE3D_MEMBERCAST(lite3d_metric_node, x, cached);
    lite3d_array_purge(&node->measurements);
    lite3d_free(node);
}

int lite3d_metrics_init(lite3d_metrics *metrics)
{
    SDL_assert(metrics);
    metrics->metricsCache = lite3d_rb_tree_create(lite3d_rb_tree_c_string_comparator,
        metric_node_delete);
    return LITE3D_TRUE;
}

int lite3d_metrics_purge(lite3d_metrics *metrics)
{
    SDL_assert(metrics);
    SDL_assert(metrics->metricsCache);
    lite3d_rb_tree_destroy(metrics->metricsCache);
    metrics->metricsCache = NULL;
    return LITE3D_TRUE;
}

static void lite3d_metrics_recalc_distrib(lite3d_metric_node *node)
{
    int i = 0, count = sizeof(node->distribution) / sizeof(node->distribution[0]);
    uint64_t interval = (node->maxMcs - node->minMcs) / count;
    uint64_t *mi;

    if (interval == 0)
        return;

    for (; i < count; i++)
    {
        node->distribution[i].lo = node->minMcs + (i * interval);
        node->distribution[i].hi = node->distribution[i].lo + interval - 1;
        node->distribution[i].hit = 0;
        node->distribution[i].percentage = 0.0;
    }

    node->maxMcs = node->minMcs = node->avgMcs;

    LITE3D_ARR_FOREACH(&node->measurements, uint64_t, mi)
    {
        for (i = 0; i < count; i++)
        {
            if (*mi >= node->distribution[i].lo && *mi <= node->distribution[i].hi)
            {
                node->distribution[i].hit++;
                node->distribution[i].percentage = (float)node->distribution[i].hit / LITE3D_MEASUREMENTS_MAX * 100.0f;
                break;
            }
        }
    }
}

int lite3d_metrics_insert(lite3d_metrics *metrics, const char *name, uint64_t mcs)
{
    lite3d_rb_node *indexNode;
    lite3d_metric_node *node;
    SDL_assert(metrics);
    SDL_assert(metrics->metricsCache);

    indexNode = lite3d_rb_tree_exact_query(metrics->metricsCache, name);
    if (indexNode)
    {
        /* OK, found */
        node = LITE3D_MEMBERCAST(lite3d_metric_node, indexNode, cached);
        node->maxMcs = LITE3D_MAX(node->maxMcs, mcs);
        node->minMcs = LITE3D_MIN(node->minMcs, mcs);
        node->avgMcs = (node->avgMcs + mcs) / 2;
        node->count++;
        
        LITE3D_ARR_ADD_ELEM(&node->measurements, uint64_t, mcs);
        if (node->measurements.size == LITE3D_MEASUREMENTS_MAX)
        {
            lite3d_metrics_recalc_distrib(node);
            lite3d_array_clean(&node->measurements);
        }

        return LITE3D_TRUE;
    }

    node = lite3d_calloc(sizeof(lite3d_metric_node));
    SDL_assert(node);

    strncpy(node->name, name, sizeof(node->name) - 1);
    node->cached.key = node->name;
    node->minMcs = mcs;
    node->maxMcs = mcs;
    node->avgMcs = mcs;
    node->count++;
    lite3d_array_init(&node->measurements, sizeof(uint64_t), LITE3D_MEASUREMENTS_MAX);
    LITE3D_ARR_ADD_ELEM(&node->measurements, uint64_t, mcs);
    return lite3d_rb_tree_insert(metrics->metricsCache, &node->cached) ? LITE3D_TRUE : LITE3D_FALSE;
}

int lite3d_metrics_global_init(void)
{
    return lite3d_metrics_init(&globalMetrics);
}

int lite3d_metrics_global_purge(void)
{
    return lite3d_metrics_purge(&globalMetrics);
}

lite3d_metrics *lite3d_metrics_global_get(void)
{
    return &globalMetrics;
}

int lite3d_metrics_global_insert(const char *name, uint64_t mcs)
{
    return lite3d_metrics_insert(&globalMetrics, name, mcs);
}

static void node_write_to_log(lite3d_rb_tree* tree, lite3d_rb_node *x)
{
    lite3d_metric_node *node = LITE3D_MEMBERCAST(lite3d_metric_node, x, cached);
    int i = 0, count = sizeof(node->distribution) / sizeof(node->distribution[0]), wr;
    char output[2048];

    wr = snprintf(output, sizeof(output), "\n%30s | min %7"PRIu64" mcs | max %7"PRIu64" mcs | avg %7"PRIu64" mcs | %10"PRIu64" called |\n",
        node->name, node->minMcs, node->maxMcs, node->avgMcs, node->count);

    for (i = 0; i < count && wr < sizeof(output); i++)
    {
        wr += snprintf(output + wr, sizeof(output) - wr, "%17"PRIu64" - %6"PRIu64" mcs |%16"PRIu64" |%16.2f |\n",
            node->distribution[i].lo, node->distribution[i].hi, node->distribution[i].hit, node->distribution[i].percentage);
    }

    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, output);
}

int lite3d_metrics_write_to_log(lite3d_metrics *metrics)
{
    SDL_assert(metrics);
    SDL_assert(metrics->metricsCache);
    lite3d_rb_tree_iterate(metrics->metricsCache, node_write_to_log);
    return LITE3D_TRUE;
}

int lite3d_metrics_global_write_to_log(void)
{
    return lite3d_metrics_write_to_log(&globalMetrics);
}
