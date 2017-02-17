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
#pragma once

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/json/JSON.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT ConfigurationReader : public Manageable
    {
    public:

        ConfigurationReader(const String &file);
        ConfigurationReader(const char *data, size_t size);
        ConfigurationReader(const ConfigurationReader &other);
        ~ConfigurationReader();

        int32_t getInt(const WString &name, int32_t def = 0) const;
        float getDouble(const WString &name, float def = 0) const;
        bool getBool(const WString &name, bool def = true) const;
        String getString(const WString &name, const String &def = "") const;
        ConfigurationReader getObject(const WString &name) const;

        stl<ConfigurationReader>::vector getObjects(const WString &name) const;
        stl<String>::vector getStrings(const WString &name) const;
        stl<int32_t>::vector getInts(const WString &name) const;
        stl<float>::vector getFloats(const WString &name) const;
        stl<bool>::vector getBools(const WString &name) const;

        kmVec2 getVec2(const WString &name, const kmVec2 &def = KM_VEC2_ZERO) const;
        kmVec3 getVec3(const WString &name, const kmVec3 &def = KM_VEC3_ZERO) const;
        kmVec4 getVec4(const WString &name, const kmVec4 &def = KM_VEC4_ZERO) const;
        kmQuaternion getQuaternion(const WString &name, const kmQuaternion &def = KM_QUATERNION_IDENTITY ) const;
        kmMat3 getMat3(const WString &name, const kmMat3 &def) const;
        kmMat4 getMat4(const WString &name, const kmMat4 &def) const;

        bool has(const WString &name) const;
        bool isEmpty() const;

    private:

        ConfigurationReader();
        ConfigurationReader(const JSONObject &fromJsonObject);
        void parseFromFile(const String &file);
        bool parseFromBuffer(const char *data, size_t size);

        std::unique_ptr<JSONValue> mRoot;
        JSONObject mObject;
    };
}

