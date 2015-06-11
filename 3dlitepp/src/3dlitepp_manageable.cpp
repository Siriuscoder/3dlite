/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
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

#include <3dlite/3dlite_alloc.h>
#include <3dlitepp/3dlitepp_manageable.h>

namespace lite3dpp
{
    void *Manageable::allocPuled(size_t size)
    {
        void *mem = lite3d_malloc_pooled(LITE3D_POOL_NO2, size);
        SDL_assert_release(mem);

        return mem;
    }

    void Manageable::freePolled(void * mem)
    {
        if(mem)
            lite3d_free_pooled(LITE3D_POOL_NO2, mem);
    }


    void *Manageable::alloc(size_t size)
    {
        void *mem = lite3d_malloc(size);
        SDL_assert_release(mem);

        return mem;
    }

    void Manageable::free(void * mem)
    {
        if(mem)
            lite3d_free(mem);
    }

    void *Manageable::callocPuled(size_t size)
    {
        void *mem = lite3d_calloc_pooled(LITE3D_POOL_NO2, size);
        SDL_assert_release(mem);

        return mem;
    }

    void *Manageable::calloc(size_t size)
    {
        void *mem = lite3d_calloc(size);
        SDL_assert_release(mem);

        return mem;
    }

    void *Manageable::operator new(size_t size)
    {
        return Manageable::callocPuled(size);
    }

    void Manageable::operator delete(void *p)
    {
        Manageable::freePolled(p);
    }

    void *Manageable::operator new[](size_t size)
    {
        return Manageable::alloc(size);
    }

    void Manageable::operator delete[](void *p)
    {
        Manageable::free(p);
    }

    void *Manageable::operator new(size_t size, const std::nothrow_t &n)
    {
        return Manageable::callocPuled(size);
    }

    void Manageable::operator delete(void *p, const std::nothrow_t &n)
    {
        Manageable::freePolled(p);
    }

    void *Manageable::operator new[](size_t size, const std::nothrow_t &n)
    {
        return Manageable::alloc(size);
    }

    void Manageable::operator delete[](void *p, const std::nothrow_t &n)
    {
        Manageable::free(p);
    }
}

