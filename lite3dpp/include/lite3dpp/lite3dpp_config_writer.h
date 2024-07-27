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

#include <lite3dpp/lite3dpp_common.h>
#include <lite3dpp/json/JSON.h>

namespace lite3dpp
{
    class LITE3DPP_EXPORT ConfigurationWriter : public Manageable
    {
    public:

        ConfigurationWriter() = default;
        ConfigurationWriter(const ConfigurationWriter &other);
        // Загрузить начальное состояние из файла
        explicit ConfigurationWriter(const std::string_view &filePath);
        explicit ConfigurationWriter(const char *data, size_t size);
        ~ConfigurationWriter() = default;

        ConfigurationWriter &set(const WString &name, int32_t value);
        ConfigurationWriter &set(const WString &name, uint32_t value);
        ConfigurationWriter &set(const WString &name, float value);
        ConfigurationWriter &set(const WString &name, bool value);
        ConfigurationWriter &set(const WString &name, const WString &value);
        ConfigurationWriter &set(const WString &name, const String &value);
        ConfigurationWriter &set(const WString &name, const char *value);
        ConfigurationWriter &set(const WString &name, const wchar_t *value);
        ConfigurationWriter &set(const WString &name, const ConfigurationWriter &object);

        ConfigurationWriter &set(const WString &name, const stl<ConfigurationWriter>::vector &objects);
        ConfigurationWriter &set(const WString &name, const stl<WString>::vector &strings);
        ConfigurationWriter &set(const WString &name, const stl<int32_t>::vector &ints);
        ConfigurationWriter &set(const WString &name, const stl<float>::vector &floats);
        ConfigurationWriter &set(const WString &name, const stl<bool>::vector &bools);

        ConfigurationWriter &set(const WString &name, const kmVec2 &value);
        ConfigurationWriter &set(const WString &name, const kmVec3 &value);
        ConfigurationWriter &set(const WString &name, const kmVec4 &value);
        ConfigurationWriter &set(const WString &name, const kmQuaternion &value);
        ConfigurationWriter &set(const WString &name, const kmMat4 &value);

        void remove(const WString &name);
        /* WARNING: if true passed then internal state will be released (effect like clear call) */
        String write();
        void clear();

        ConfigurationWriter& operator=(const ConfigurationWriter& other);

    private:

        ConfigurationWriter(const JSONObject &fromObject);

        JSONObject mObject;
    };
}

