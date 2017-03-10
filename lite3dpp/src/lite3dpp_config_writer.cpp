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
#include <SDL_log.h>
#include <SDL_rwops.h>

#include <lite3dpp/lite3dpp_config_writer.h>

namespace lite3dpp
{
    ConfigurationWriter::~ConfigurationWriter() 
    {}

    // private
    ConfigurationWriter::ConfigurationWriter(const JSONObject &fromJsonObject) : 
        mObject(fromJsonObject)
    {}

    ConfigurationWriter::ConfigurationWriter()
    {}

    // copy 
    ConfigurationWriter::ConfigurationWriter(const ConfigurationWriter &other) : 
        mObject(other.mObject)
    {}

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, int32_t value)
    {
        remove(name);
        mObject[name] = new JSONValue((float)value);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, float value)
    {
        remove(name);
        mObject[name] = new JSONValue(value);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, bool value)
    {
        remove(name);
        mObject[name] = new JSONValue(value);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const WString &value)
    {
        remove(name);
        mObject[name] = new JSONValue(value);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const String &value)
    {
        remove(name);

        WString localWString(value.begin(), value.end());
        mObject[name] = new JSONValue(localWString);
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
        remove(name);
        mObject[name] = new JSONValue(object.mObject);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const stl<ConfigurationWriter>::vector &objects)
    {
        remove(name);
        JSONArray jarray;
        for(const ConfigurationWriter& object : objects)
        {
            jarray.push_back(new JSONValue(object.mObject));
        }

        mObject[name] = new JSONValue(jarray);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const stl<WString>::vector &strings)
    {
        remove(name);
        JSONArray jarray;
        for(const WString& object : strings)
        {
            jarray.push_back(new JSONValue(object));
        }

        mObject[name] = new JSONValue(jarray);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const stl<int32_t>::vector &ints)
    {
        remove(name);
        JSONArray jarray;
        for(const int32_t object : ints)
        {
            jarray.push_back(new JSONValue((float)object));
        }

        mObject[name] = new JSONValue(jarray);
        return *this;
    }
    
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const stl<float>::vector &floats)
    {
        remove(name);
        JSONArray jarray;
        for(const float object : floats)
        {
            jarray.push_back(new JSONValue(object));
        }

        mObject[name] = new JSONValue(jarray);
        return *this;
    }
     
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const stl<bool>::vector &bools)
    {
        remove(name);
        JSONArray jarray;
        for(const bool object : bools)
        {
            jarray.push_back(new JSONValue(object));
        }

        mObject[name] = new JSONValue(jarray);
        return *this;
    }

    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const kmVec2 &value)
    {
        remove(name);
        stl<float>::vector vec(&value.x, &value.x+2);
        return set(name, vec);
    }
    
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const kmVec3 &value)
    {
        remove(name);
        stl<float>::vector vec(&value.x, &value.x+3);
        return set(name, vec);
    }
     
    
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const kmVec4 &value)
    {
        remove(name);
        stl<float>::vector vec(&value.x, &value.x+4);
        return set(name, vec);
    }
     
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const kmQuaternion &value)
    {
        remove(name);
        stl<float>::vector vec(&value.x, &value.x+4);
        return set(name, vec);
    }
     
    ConfigurationWriter &ConfigurationWriter::set(const WString &name, const kmMat4 &value)
    {
        remove(name);
        stl<float>::vector mat(value.mat, value.mat+16);
        return set(name, mat);
    }

    void ConfigurationWriter::remove(const WString &name)
    {
        auto it = mObject.find(name);
        if(it != mObject.end())
        {
            delete it->second;
            mObject.erase(it);
        }
    }

    String ConfigurationWriter::write(bool fin)
    {
        JSONValue value(mObject);
        String result = JSON::wStringToString(value.Stringify(true));
        if (!fin)
            value.setValue();
        return result;
    }

    void ConfigurationWriter::clear()
    {
        JSONValue value;
        value.setValue(mObject);
    }
}