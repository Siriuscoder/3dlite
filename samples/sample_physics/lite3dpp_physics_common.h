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

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wunused-but-set-variable"
#endif

#include <btBulletDynamicsCommon.h>

#ifdef __clang__
#   pragma clang diagnostic pop
#endif

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/lite3dpp_scene.h>

namespace lite3dpp {
namespace lite3dpp_phisics {

    class BulletUtils 
    {
    public:

        static inline btVector3 convert(const kmVec3& v)
        {
            return btVector3 {
                v.x,
                v.y, 
                v.z
            };
        }

        static inline kmVec3 convert(const btVector3& v)
        {
            return kmVec3 {
                v.getX(),
                v.getY(), 
                v.getZ()
            };
        }

        static inline btQuaternion convert(const kmQuaternion& q)
        {
            return btQuaternion {
                q.x,
                q.y, 
                q.z,
                q.w,
            };
        }

        static inline kmQuaternion convert(const btQuaternion& q)
        {
            return kmQuaternion {
                q.getX(),
                q.getY(), 
                q.getZ(),
                q.getW()
            };
        }
    };

}}
