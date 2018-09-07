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
#include <string.h>

#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_metrics.h>
#include <lite3d/lite3d_alloc.h>

static lite3d_metrics globalMetrics = { NULL };

static void metric_node_delete(lite3d_rb_node *x)
{
    lite3d_metric_node *node = LITE3D_MEMBERCAST(lite3d_metric_node, x, cached);
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
        node->avgMcs = mcs;
        node->count++;

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
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%-30s |%20lld |%20lld |%20lld |%20lld |",
        node->name, node->avgMcs, node->minMcs, node->maxMcs, node->count);
}

int lite3d_metrics_write_to_log(lite3d_metrics *metrics)
{
    SDL_assert(metrics);
    SDL_assert(metrics->metricsCache);
    SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%-30s |%20s |%20s |%20s |%20s |", "Latency Metrics", 
        "avg mcs", "min mcs", "tmax mcs", "times");
    lite3d_rb_tree_iterate(metrics->metricsCache, node_write_to_log);
    return LITE3D_TRUE;
}

int lite3d_metrics_global_write_to_log(void)
{
    return lite3d_metrics_write_to_log(&globalMetrics);
}
