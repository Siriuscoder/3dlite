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
#include <string.h>
#include <stdlib.h> 

#include <SDL_assert.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3d/lite3d_array.h>

void lite3d_array_init(lite3d_array *a, size_t elemSize, size_t capacity)
{
    SDL_assert(a);
    a->data = lite3d_malloc(elemSize * capacity);
    SDL_assert_release(a->data);

    a->elemSize = elemSize;
    a->size = 0;
    a->capacity = capacity;
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

void *lite3d_array_add(lite3d_array *a)
{
    SDL_assert(a);
    /* Is it necessary to expand the array? */
    if (a->size == a->capacity)
    {
        void *pnew;
        a->capacity <<= 1;
        pnew = lite3d_malloc(a->capacity * a->elemSize);
        SDL_assert_release(pnew);

        memcpy(pnew, a->data, a->size * a->elemSize);
        lite3d_free(a->data);
        a->data = pnew;
    }

    a->size++;
    return lite3d_array_get(a, a->size - 1);
}

void *lite3d_array_get(lite3d_array *a, size_t index)
{
    SDL_assert(a);
    SDL_assert(index < a->size);
    return ((char *) a->data) +(index * a->elemSize);
}

void lite3d_array_remove(lite3d_array *a, size_t index)
{
    size_t moveCount;
    SDL_assert(a);
    SDL_assert(index < a->size);

    if (index < a->size - 1)
    {
        moveCount = a->size - index - 1;
        memmove(lite3d_array_get(a, index), lite3d_array_get(a, index + 1), a->elemSize * moveCount);
    }

    a->size--;
}

void lite3d_array_qsort(lite3d_array *a, lite3d_array_compare_t comparator)
{
    SDL_assert(a);
    if (!a->size)
        return;

    qsort(a->data, a->size, a->elemSize, comparator);
}
