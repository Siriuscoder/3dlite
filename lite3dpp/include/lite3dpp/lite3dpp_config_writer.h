/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
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

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/json/JSON.h>

#include <lite3d/kazmath/vec2.h>
#include <lite3d/kazmath/vec3.h>
#include <lite3d/kazmath/vec4.h>
#include <lite3d/kazmath/mat4.h>
#include <lite3d/kazmath/quaternion.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT ConfigurationWriter : public Manageable
    {
    public:

        ConfigurationWriter();
        ConfigurationWriter(const ConfigurationWriter &other);
        ~ConfigurationWriter();

        void set(const WString &name, int32_t value);
        void set(const WString &name, double value);
        void set(const WString &name, bool value);
        void set(const WString &name, const WString &value);
        void set(const WString &name, const ConfigurationWriter &object);

        void set(const WString &name, const stl<ConfigurationWriter>::vector &objects);
        void set(const WString &name, const stl<WString>::vector &strings);
        void set(const WString &name, const stl<int32_t>::vector &ints);
        void set(const WString &name, const stl<double>::vector &floats);
        void set(const WString &name, const stl<bool>::vector &bools);

        void set(const WString &name, const kmVec2 &value);
        void set(const WString &name, const kmVec3 &value);
        void set(const WString &name, const kmVec4 &value);
        void set(const WString &name, const kmQuaternion &value);
        void set(const WString &name, const kmMat4 &value);

        void remove(const WString &name);

        String write();
        void write(const String &filename);

        void clear();

    private:

        ConfigurationWriter(const JSONObject &fromObject);

        JSONObject mObject;
    };
}

