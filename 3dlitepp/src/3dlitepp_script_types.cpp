/******************************************************************************
 *	This file is part of 3dlite (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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

#include <3dlite/kazmath/quaternion.h>
#include <3dlite/kazmath/ray3.h>
#include <3dlite/kazmath/mat4.h>
#include <3dlite/kazmath/mat3.h>
#include <3dlite/kazmath/vec4.h>
#include <3dlite/kazmath/vec3.h>

#include <3dlitepp/as/angelscript.h>

#include <3dlitepp/3dlitepp_script_binding.h>

namespace lite3dpp
{
    void RegisterScriptTypes(asIScriptEngine *engine)
    {
        /* register types */
        SDL_assert(engine->RegisterObjectType("Vec3", sizeof(kmVec3), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK) >= 0);
        SDL_assert(engine->RegisterObjectType("Vec4", sizeof(kmVec4), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK) >= 0);
        SDL_assert(engine->RegisterObjectType("Mat3", sizeof(kmMat3), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK) >= 0);
        SDL_assert(engine->RegisterObjectType("Mat4", sizeof(kmMat4), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK) >= 0);
        SDL_assert(engine->RegisterObjectType("Quaternion", sizeof(kmQuaternion), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK) >= 0);
        //SDL_assert(engine->RegisterObjectType("Ray3", sizeof(kmRay3), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK) >= 0);
    }
}