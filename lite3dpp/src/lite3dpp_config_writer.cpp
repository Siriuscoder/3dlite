/******************************************************************************
 *	This file is part of lite3d (Light-weight 3d engine).
 *	Copyright (C) 2025 Sirius (Korolev Nikita)
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
#include <SDL_log.h>
#include <SDL_rwops.h>

#include <lite3dpp/lite3dpp_config_writer.h>
#include <lite3dpp/lite3dpp_config_reader.h>

namespace lite3dpp
{
    // private
    ConfigurationWriter::ConfigurationWriter(const JSONObject &fromJsonObject) : 
        mObject(fromJsonObject)
    {}

    // copy 
    ConfigurationWriter::ConfigurationWriter(const ConfigurationWriter &other) : 
        mObject(other.mObject)
    {}

    ConfigurationWriter::ConfigurationWriter(const std::string_view &filePath)
    {
        ConfigurationReader reader(filePath);
        mObject = reader.mObject;
    }

    ConfigurationWriter::ConfigurationWriter(const char *data, size_t size)
    {
        ConfigurationReader reader(data, size);
        mObject = reader.mObject;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, int32_t value)
    {
        mObject[name] = std::make_shared<JSONValue>(static_cast<float>(value));
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, uint32_t value)
    {
        mObject[name] = std::make_shared<JSONValue>(static_cast<float>(value));
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, float value)
    {
        mObject[name] = std::make_shared<JSONValue>(value);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, bool value)
    {
        mObject[name] = std::make_shared<JSONValue>(value);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const WString &value)
    {
        mObject[name] = std::make_shared<JSONValue>(value);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const String &value)
    {
        WString localWString(value.begin(), value.end());
        mObject[name] = std::make_shared<JSONValue>(localWString);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const char *value)
    {
        return set(name, String(value));
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const wchar_t *value)
    {
        return set(name, WString(value));
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const ConfigurationWriter &object)
    {
        mObject[name] = std::make_shared<JSONValue>(object.mObject);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const stl<ConfigurationWriter>::vector &objects)
    {
        JSONArray jarray;
        for(const ConfigurationWriter& object : objects)
        {
            jarray.emplace_back(std::make_shared<JSONValue>(object.mObject));
        }

        mObject[name] = std::make_shared<JSONValue>(jarray);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const stl<WString>::vector &strings)
    {
        JSONArray jarray;
        for(const WString& object : strings)
        {
            jarray.emplace_back(std::make_shared<JSONValue>(object));
        }

        mObject[name] = std::make_shared<JSONValue>(jarray);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const stl<int32_t>::vector &ints)
    {
        JSONArray jarray;
        for(const int32_t object : ints)
        {
            jarray.emplace_back(std::make_shared<JSONValue>(static_cast<float>(object)));
        }

        mObject[name] = std::make_shared<JSONValue>(jarray);
        return *this;
    }
    
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const stl<float>::vector &floats)
    {
        JSONArray jarray;
        for(const float object : floats)
        {
            jarray.emplace_back(std::make_shared<JSONValue>(object));
        }

        mObject[name] = std::make_shared<JSONValue>(jarray);
        return *this;
    }
     
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const stl<bool>::vector &bools)
    {
        JSONArray jarray;
        for(const bool object : bools)
        {
            jarray.emplace_back(std::make_shared<JSONValue>(object));
        }

        mObject[name] = std::make_shared<JSONValue>(jarray);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const kmVec2 &value)
    {
        stl<float>::vector vec(&value.x, &value.x+2);
        return set(name, vec);
    }
    
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const kmVec3 &value)
    {
        stl<float>::vector vec(&value.x, &value.x+3);
        return set(name, vec);
    }
     
    
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const kmVec4 &value)
    {
        stl<float>::vector vec(&value.x, &value.x+4);
        return set(name, vec);
    }
     
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const kmQuaternion &value)
    {
        stl<float>::vector vec(&value.x, &value.x+4);
        return set(name, vec);
    }
     
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const kmMat4 &value)
    {
        stl<float>::vector mat(value.mat, value.mat+16);
        return set(name, mat);
    }

    void ConfigurationWriter::remove(const WString &name)
    {
        auto it = mObject.find(name);
        if (it != mObject.end())
        {
            mObject.erase(it);
        }
    }

    String ConfigurationWriter::write()
    {
        JSONValue value(mObject);
        return JSON::wStringToString(value.Stringify(true));
    }

    void ConfigurationWriter::clear()
    {
        mObject.clear();
    }

    ConfigurationWriter& ConfigurationWriter::operator=(const ConfigurationWriter& other)
    {
        mObject = other.mObject;
        return *this;
    }
}