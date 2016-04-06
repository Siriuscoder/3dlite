/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2015  Sirius (Korolev Nikita)
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

#include <lite3dpp/as/angelscript.h>
#include <lite3dpp/lite3dpp_script_binding.h>

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