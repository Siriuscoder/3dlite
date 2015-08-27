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
#pragma once

#include <3dlitepp/3dlitepp_common.h>
#include <3dlitepp/json/JSON.h>

#include <3dlite/kazmath/vec2.h>
#include <3dlite/kazmath/vec3.h>
#include <3dlite/kazmath/vec4.h>
#include <3dlite/kazmath/quaternion.h>

namespace lite3dpp
{
    class JsonHelper : public Manageable
    {
    public:

        JsonHelper(const lite3dpp_string &file);
        JsonHelper(const char *data, size_t size);
        JsonHelper(const JsonHelper &other);
        ~JsonHelper();

        int32_t getInt(const lite3dpp_wstring &name, int32_t def = 0) const;
        double getDouble(const lite3dpp_wstring &name, double def = 0) const;
        bool getBool(const lite3dpp_wstring &name, bool def = true) const;
        lite3dpp_string getString(const lite3dpp_wstring &name, const lite3dpp_string &def = "") const;
        JsonHelper getObject(const lite3dpp_wstring &name) const;

        stl<JsonHelper>::vector getObjects(const lite3dpp_wstring &name) const;
        stl<lite3dpp_string>::vector getStrings(const lite3dpp_wstring &name) const;
        stl<int32_t>::vector getInts(const lite3dpp_wstring &name) const;
        stl<double>::vector getFloats(const lite3dpp_wstring &name) const;
        stl<bool>::vector getBools(const lite3dpp_wstring &name) const;

        kmVec2 getVec2(const lite3dpp_wstring &name, const kmVec2 &def = KM_VEC2_ZERO) const;
        kmVec3 getVec3(const lite3dpp_wstring &name, const kmVec3 &def = KM_VEC3_ZERO) const;
        kmVec4 getVec4(const lite3dpp_wstring &name, const kmVec4 &def = KM_VEC4_ZERO) const;
        kmQuaternion getQuaternion(const lite3dpp_wstring &name, const kmQuaternion &def = KM_QUATERNION_IDENTITY ) const;


        bool isEmpty() const;

    private:

        JsonHelper();
        JsonHelper(const JSONObject &fromJsonObject);
        void parseFromFile(const lite3dpp_string &file);
        bool parseFromBuffer(const char *data, size_t size);

        JSONValue *mRoot;
        JSONObject mObject;
    };
}

