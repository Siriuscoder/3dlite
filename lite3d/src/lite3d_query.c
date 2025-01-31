/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025  Sirius (Korolev Nikita)
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
#include <SDL_log.h>
#include <SDL_assert.h>

#include <lite3d/lite3d_gl.h>
#include <lite3d/lite3d_glext.h>
#include <lite3d/lite3d_misc.h>
#include <lite3d/lite3d_render.h>
#include <lite3d/lite3d_query.h>

int lite3d_query_technique_init(void)
{
    if (!lite3d_check_occlusion_query())
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "%s: Hardware occlusion query is not supported",
            LITE3D_CURRENT_FUNCTION);
    
        return LITE3D_FALSE;
    }

    return LITE3D_TRUE;
}

int lite3d_query_init(struct lite3d_query *query)
{
    SDL_assert(query);

    lite3d_misc_gl_error_stack_clean();

    memset(query, 0, sizeof(lite3d_query));
    query->anyPassed = -1;

    glGenQueries(1, &query->queryID);
    if (!LITE3D_CHECK_GL_ERROR)
    {
        lite3d_render_stats_get()->queryCount++;
        return LITE3D_TRUE;
    }

    return LITE3D_FALSE;
}

void lite3d_query_purge(struct lite3d_query *query)
{
    SDL_assert(query);
    
    glDeleteQueries(1, &query->queryID);
    memset(query, 0, sizeof(lite3d_query));
    query->anyPassed = -1;
    lite3d_render_stats_get()->queryCount--;
}

void lite3d_query_begin(struct lite3d_query *query)
{
    SDL_assert(query);

    glBeginQuery(GL_ANY_SAMPLES_PASSED, query->queryID);
    query->inProgress = LITE3D_TRUE;
}

void lite3d_query_end(struct lite3d_query *query)
{
    SDL_assert(query);
    (void)query;

    glEndQuery(GL_ANY_SAMPLES_PASSED);
}

void lite3d_query_result(struct lite3d_query *query)
{
    SDL_assert(query);
    GLuint result;

    if (query->inProgress)
    {
        glGetQueryObjectuiv(query->queryID, GL_QUERY_RESULT, &result);
        query->inProgress = LITE3D_FALSE;
        query->anyPassed = (result == GL_TRUE ? LITE3D_TRUE : LITE3D_FALSE);
    }
}
