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
#include <inttypes.h>
#include <math.h>
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

static void lite3d_metrics_distrib_insert(lite3d_metric_node *node, double mcs)
{
    int i = 0, count = sizeof(node->distribution) / sizeof(node->distribution[0]);

    if (!node->distributionReady)
        return;

    for (; i < count; i++)
    {
        if ((i == 0 && mcs <= node->distribution[i].hi) ||
            (i == count - 1 && mcs >= node->distribution[i].lo) ||
            (mcs >= node->distribution[i].lo && mcs < node->distribution[i].hi))
        {
            node->distribution[i].hit++;
            break;
        }
    }
}

static void lite3d_metrics_init_distrib(lite3d_metric_node *node)
{
    int i = 0, count = sizeof(node->distribution) / sizeof(node->distribution[0]);
    double logRange = node->minMcs > 0.0 && node->maxMcs > node->minMcs ?
        log(node->maxMcs / node->minMcs) : 0.0;
    double *mi;

    for (; i < count; i++)
    {
        if (i == 0)
        {
            node->distribution[i].lo = 0.0;
            node->distribution[i].hi = node->minMcs;
        }
        else
        {
            double hiFactor = (double)i / (count - 1);
            node->distribution[i].lo = node->distribution[i - 1].hi;
            node->distribution[i].hi = logRange > 0.0 ?
                node->minMcs * exp(logRange * hiFactor) : node->maxMcs;
        }

        node->distribution[i].hit = 0;
    }

    node->distributionReady = LITE3D_TRUE;

    LITE3D_ARR_FOREACH(&node->measurements, double, mi)
    {
        lite3d_metrics_distrib_insert(node, *mi);
    }

    lite3d_array_clean(&node->measurements);
}

int lite3d_metrics_insert(lite3d_metrics *metrics, const char *name, double mcs)
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
        
        if (node->distributionReady)
        {
            lite3d_metrics_distrib_insert(node, mcs);
        }
        else
        {
            LITE3D_ARR_ADD_ELEM(&node->measurements, double, mcs);
            if (node->measurements.size == LITE3D_MEASUREMENTS_WARMUP)
            {
                lite3d_metrics_init_distrib(node);
            }
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
    lite3d_array_init(&node->measurements, sizeof(double), LITE3D_MEASUREMENTS_WARMUP);
    LITE3D_ARR_ADD_ELEM(&node->measurements, double, mcs);
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

int lite3d_metrics_global_insert(const char *name, double mcs)
{
    return lite3d_metrics_insert(&globalMetrics, name, mcs);
}

static void node_write_to_log(lite3d_rb_tree* tree, lite3d_rb_node *x)
{
    lite3d_metric_node *node = LITE3D_MEMBERCAST(lite3d_metric_node, x, cached);
    int i = 0, count = sizeof(node->distribution) / sizeof(node->distribution[0]), wr;
    char output[4096];

    wr = snprintf(output, sizeof(output),
        "\n"
        "+----------------------------------------------------------------------------+\n"
        "| Metric: %-66.66s |\n"
        "+----------------+----------------+----------------+------------------------+\n"
        "| Min, mcs       | Max, mcs       | Avg, mcs       | Calls                  |\n"
        "+----------------+----------------+----------------+------------------------+\n"
        "| %14.3f | %14.3f | %14.3f | %22"PRIu64" |\n"
        "+----------------+----------------+----------------+------------------------+\n"
        "| Distribution                                                               |\n",
        node->name, node->minMcs, node->maxMcs, node->avgMcs, node->count);

    if (!node->distributionReady)
    {
        char warmup[128];
        snprintf(warmup, sizeof(warmup), "Warmup: %zu / %d samples",
            node->measurements.size, LITE3D_MEASUREMENTS_WARMUP);
        if (wr < sizeof(output))
        {
            snprintf(output + wr, sizeof(output) - wr,
                "| %-74.74s |\n"
                "+----------------------------------------------------------------------------+\n",
                warmup);
        }

        SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, output);
        return;
    }

    wr += snprintf(output + wr, sizeof(output) - wr,
        "+-----+------------------+------------------+----------------+--------------+\n"
        "|  #  | From, mcs        | To, mcs          | Hits           | Percent      |\n"
        "+-----+------------------+------------------+----------------+--------------+\n");

    for (i = 0; i < count && wr < sizeof(output); i++)
    {
        char lo[32], hi[32];
        double percentage = node->count > 0 ?
            (double)node->distribution[i].hit / node->count * 100.0 : 0.0;

        snprintf(lo, sizeof(lo), "%.3f", node->distribution[i].lo);

        if (i == count - 1)
            snprintf(hi, sizeof(hi), "+inf");
        else
            snprintf(hi, sizeof(hi), "%.3f", node->distribution[i].hi);

        wr += snprintf(output + wr, sizeof(output) - wr,
            "| %3d | %16s | %16s | %14"PRIu64" | %11.2f%% |\n",
            i,
            lo,
            hi,
            node->distribution[i].hit,
            percentage);
    }

    if (wr < sizeof(output))
    {
        snprintf(output + wr, sizeof(output) - wr,
            "+-----+------------------+------------------+----------------+--------------+\n");
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
