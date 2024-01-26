/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2024 Sirius (Korolev Nikita)
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
#pragma once 

#include <new>
#include <memory>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <list>
#include <stack>
#include <unordered_map>

#include <lite3dpp/lite3dpp_common.h>

namespace lite3dpp
{

    class LITE3DPP_EXPORT Manageable
    {
    public:

        static void *allocPulled(size_t size);
        static void freePulled(void * mem);
        static void *alloc(size_t size);
        static void free(void * mem);
        static void *callocPulled(size_t size);
        static void *calloc(size_t size);

        /* memory management */
        void *operator new(size_t size);

        void operator delete(void *p);

        void *operator new[](size_t size);

        void operator delete[](void *p);

        /* memory management nothrow */
        void *operator new(size_t size, const std::nothrow_t &n);

        void operator delete(void *p, const std::nothrow_t &n);

        void *operator new[](size_t size, const std::nothrow_t &n);

        void operator delete[](void *p, const std::nothrow_t &n);
    };


    class LITE3DPP_EXPORT Noncopiable
    {
    public:

        Noncopiable() = default;
        Noncopiable(const Noncopiable &other) = delete;
        Noncopiable(Noncopiable &&other) = delete;
    };

#ifdef LITE3DPP_USE_STL_ALLOCATOR
    
    template<class T>
    class LITE3DPP_EXPORT ManageableStlAllocator :
        public std::allocator<T>
    {
    public:
        /// \brief Указатель на тип
        typedef typename std::allocator<T>::pointer pointer;
        typedef typename std::allocator<T>::size_type size_type;
        typedef typename std::allocator<T>::value_type value_type;

        pointer allocate(size_type n)
        {
            if(n == 0)
                return NULL;

            return static_cast<pointer>(Manageable::alloc(sizeof (value_type) * n));
        }

        void deallocate(pointer p, size_type n)
        {
            Manageable::free(p);
        }
    };

    template<class T, class Y = void>
    class LITE3DPP_EXPORT stl
    {
    public:

        typedef std::vector<T, ManageableStlAllocator<T> > vector;

        typedef std::map<T, Y, std::less<T>, 
            ManageableStlAllocator<std::pair<const T, Y> > > map;

        typedef std::list<T, ManageableStlAllocator<T> > list;
        
        typedef std::set<T, std::less<T>, ManageableStlAllocator<T> > set;

        typedef std::stack<T, ManageableStlAllocator<T> > stack;
        
        typedef std::unordered_map<T, Y, std::hash<T>, std::equal_to<T>,
            ManageableStlAllocator<std::pair<const T, Y>>> unordered_map;
    };
 
#else
    template<class T, class Y = void>
    class LITE3DPP_EXPORT stl
    {
    public:

        typedef std::vector<T> vector;

        typedef std::map<T, Y> map;

        typedef std::list<T> list;
        
        typedef std::set<T> set;

        typedef std::stack<T> stack;
        
        typedef std::unordered_map<T, Y> unordered_map;
    };
    
    typedef std::string String;
    typedef std::wstring WString;
    typedef std::stringstream Stringstream;
    typedef std::wstringstream WStringstream;
    
#endif
}

