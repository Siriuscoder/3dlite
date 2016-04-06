/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2014  Sirius (Korolev Nikita)
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
#include <SDL_assert.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3dpp/lite3dpp_manageable.h>

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

    Noncopiable::Noncopiable()
    {}

    Noncopiable::Noncopiable(const Noncopiable &other)
    {}
}

