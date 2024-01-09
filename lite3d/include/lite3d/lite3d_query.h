/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024  Sirius (Korolev Nikita)
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
#ifndef LITE3D_QUERY_H
#define	LITE3D_QUERY_H

#include <lite3d/lite3d_common.h>

typedef struct lite3d_query
{
    uint32_t queryID;
    int32_t inProgress;
    int32_t isVisible;
    void *userdata;
} lite3d_query;

LITE3D_CEXPORT int lite3d_query_technique_init(void);
LITE3D_CEXPORT int lite3d_query_init(struct lite3d_query *query);
LITE3D_CEXPORT void lite3d_query_purge(struct lite3d_query *query);
LITE3D_CEXPORT void lite3d_query_begin(struct lite3d_query *query);
LITE3D_CEXPORT void lite3d_query_end(struct lite3d_query *query);
LITE3D_CEXPORT void lite3d_query_result(struct lite3d_query *query);

#endif