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
#include <SDL_assert.h>

#include <lite3d/lite3d_alloc.h>
#include <lite3dpp/lite3dpp_manageable.h>

namespace lite3dpp
{
    void *Manageable::allocPulled(size_t size)
    {
        void *mem = nullptr;
        if (!(mem = lite3d_malloc_pooled(LITE3D_POOL_NO2, size)))
        {
            throw std::bad_alloc();
        }

        return mem;
    }

    void Manageable::freePulled(void * mem)
    {
        if(mem)
            lite3d_free_pooled(LITE3D_POOL_NO2, mem);
    }


    void *Manageable::alloc(size_t size)
    {
        void *mem = nullptr;
        if (!(mem = lite3d_malloc(size)))
        {
            throw std::bad_alloc();
        }

        return mem;
    }

    void Manageable::free(void * mem)
    {
        if(mem)
            lite3d_free(mem);
    }

    void *Manageable::callocPulled(size_t size)
    {
        void *mem = nullptr;
        if (!(mem = lite3d_calloc_pooled(LITE3D_POOL_NO2, size)))
        {
            throw std::bad_alloc();
        }

        return mem;
    }

    void *Manageable::calloc(size_t size)
    {
        void *mem = nullptr;
        if (!(mem = lite3d_calloc(size)))
        {
            throw std::bad_alloc();
        }
        
        return mem;
    }

    void *Manageable::operator new(size_t size)
    {
        return Manageable::callocPulled(size);
    }

    void Manageable::operator delete(void *p)
    {
        Manageable::freePulled(p);
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
        return Manageable::callocPulled(size);
    }

    void Manageable::operator delete(void *p, const std::nothrow_t &n)
    {
        Manageable::freePulled(p);
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

