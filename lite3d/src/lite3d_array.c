/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
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
#include <SDL_assert.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_array.h>

void lite3d_array_init(lite3d_array *a, size_t elemSize, size_t size)
{
    SDL_assert(a);
    a->data = lite3d_malloc(elemSize * size);
    SDL_assert_release(a->data);

    a->elemSize = elemSize;
    a->size = size;
    a->capacity = size;
}

void lite3d_array_clean(lite3d_array *a)
{
    SDL_assert(a);
    a->size = 0;
}

void lite3d_array_purge(lite3d_array *a)
{
    SDL_assert(a);
    
    lite3d_free(a->data);

    a->data = NULL;
    a->capacity = a->size = 0;
}