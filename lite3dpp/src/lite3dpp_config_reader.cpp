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
#include <algorithm>
#include <cctype>

#include <SDL_assert.h>
#include <SDL_log.h>
#include <SDL_rwops.h>

#include <lite3dpp/lite3dpp_config_reader.h>

namespace lite3dpp
{
    ConfigurationReader::ConfigurationReader(const String &file)
    {
        parseFromFile(file);
    }

    ConfigurationReader::ConfigurationReader(const char *data, size_t size)
    {
        if(!parseFromBuffer(data, size))
            LITE3D_THROW("json parse failed..");
    }

    ConfigurationReader::~ConfigurationReader()
    {}

    // private
    ConfigurationReader::ConfigurationReader(const JSONObject &fromJsonObject) : 
        mObject(fromJsonObject)
    {}

    // private default
    ConfigurationReader::ConfigurationReader()
    {}

    // copy 
    ConfigurationReader::ConfigurationReader(const ConfigurationReader &other) : 
        mObject(other.mObject)
    {}

    void ConfigurationReader::parseFromFile(const String &file)
    {
        SDL_RWops *desc = NULL;
        size_t fileSize;
        char *json;
        /* check open file */
        desc = SDL_RWFromFile(file.c_str(), "r");
        if (!desc)
            LITE3D_THROW(file << " file open failed..");

        fileSize = static_cast<size_t>(SDL_RWsize(desc));
        json = (char *) Manageable::alloc(fileSize);
        /* read whole file */
        if (SDL_RWread(desc, json, fileSize, 1) == 0)
        {
            Manageable::free(json);
            SDL_RWclose(desc);
            LITE3D_THROW(file << " file read failed..");
        }

        SDL_RWclose(desc);
        
        if(!parseFromBuffer(json, fileSize))
        {
            Manageable::free(json);
            LITE3D_THROW(file << " file parse failed..");
        }

        Manageable::free(json);
    }

    bool ConfigurationReader::parseFromBuffer(const char *data, size_t size)
    {
        String bufCopy(data, size);
        /* Parse data from buffer */
        mRoot = JSON::Parse(bufCopy.c_str());

        if (!mRoot)
            return false;

        if (!mRoot->IsObject())
            return false;

        mObject = mRoot->AsObject();
        return true;
    }

    int32_t ConfigurationReader::getInt(const WString &name, int32_t def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsNumber())
        {
            return it->second->AsInt();
        }

        return def;
    }

    float ConfigurationReader::getDouble(const WString &name, float def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsNumber())
        {
            return it->second->AsNumber();
        }

        return def;
    }

    bool ConfigurationReader::getBool(const WString &name, bool def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsBool())
        {
            return it->second->AsBool();
        }

        return def;
    }

    String ConfigurationReader::getString(const WString &name, const String &def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsString())
        {
            return JSON::wStringToString(it->second->AsString());
        }

        return def;
    }

    String ConfigurationReader::getUpperString(const WString &name, const String &def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsString())
        {
            String res = JSON::wStringToString(it->second->AsString());
            std::transform(res.begin(), res.end(), res.begin(), [](char a) -> char
            { return std::toupper(a); });
            return res;
        }

        return def;
    }

    ConfigurationReader ConfigurationReader::getObject(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsObject())
        {
            return ConfigurationReader(it->second->AsObject());
        }

        return ConfigurationReader();
    }

    bool ConfigurationReader::isEmpty() const
    {
        return mObject.size() == 0;
    }

    kmVec2 ConfigurationReader::getVec2(const WString &name, const kmVec2 &def) const
    {
        auto floats = getFloats(name);
        if(floats.size() != 2)
            return def;

        kmVec2 vec2 = {
            (float)floats[0],
            (float)floats[1]
        };

        return vec2;
    }

    kmVec3 ConfigurationReader::getVec3(const WString &name, const kmVec3 &def) const
    {
        auto floats = getFloats(name);
        if(floats.size() != 3)
            return def;

        kmVec3 vec3 = {
            (float)floats[0],
            (float)floats[1],
            (float)floats[2]
        };

        return vec3;
    }

    kmVec4 ConfigurationReader::getVec4(const WString &name, const kmVec4 &def) const
    {
        auto floats = getFloats(name);
        if(floats.size() != 4)
            return def;

        kmVec4 vec4 = {
            (float)floats[0],
            (float)floats[1],
            (float)floats[2],
            (float)floats[3]
        };

        return vec4;
    }

    kmQuaternion ConfigurationReader::getQuaternion(const WString &name, const kmQuaternion &def) const
    {
        auto floats = getFloats(name);
        if(floats.size() != 4)
            return def;

        kmQuaternion quat = {
            (float)floats[0],
            (float)floats[1],
            (float)floats[2],
            (float)floats[3]
        };

        return quat;
    }

    kmMat3 ConfigurationReader::getMat3(const WString &name, const kmMat3 &def) const
    {
        auto floats = getFloats(name);
        if(floats.size() != 9)
            return def;

        kmMat3 mat3 = {
            (float)floats[0],
            (float)floats[1],
            (float)floats[2],
            (float)floats[3],
            (float)floats[4],
            (float)floats[5],
            (float)floats[6],
            (float)floats[7],
            (float)floats[8]
        };

        return mat3;
    }

    kmMat4 ConfigurationReader::getMat4(const WString &name, const kmMat4 &def) const
    {
        auto floats = getFloats(name);
        if(floats.size() != 16)
            return def;

        kmMat4 mat4 = {
            (float)floats[0],
            (float)floats[1],
            (float)floats[2],
            (float)floats[3],
            (float)floats[4],
            (float)floats[5],
            (float)floats[6],
            (float)floats[7],
            (float)floats[8],
            (float)floats[9],
            (float)floats[10],
            (float)floats[11],
            (float)floats[12],
            (float)floats[13],
            (float)floats[14],
            (float)floats[15],
        };

        return mat4;
    }

    stl<ConfigurationReader>::vector ConfigurationReader::getObjects(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        stl<ConfigurationReader>::vector result;

        if (it != mObject.end() && it->second->IsArray())
        {
            const JSONArray &jarray = it->second->AsArray();
            for (uint32_t i = 0; i < jarray.size(); ++i)
            {
                if(jarray[i]->IsObject())
                    result.emplace_back(ConfigurationReader(jarray[i]->AsObject()));
            }
        }

        return result;
    }

    stl<String>::vector ConfigurationReader::getStrings(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        stl<String>::vector result;

        if (it != mObject.end() && it->second->IsArray())
        {
            const JSONArray &jarray = it->second->AsArray();
            for (uint32_t i = 0; i < jarray.size(); ++i)
            {
                if(jarray[i]->IsString())
                    result.emplace_back(JSON::wStringToString(jarray[i]->AsString()));
            }
        }

        return result;
    }

    stl<int32_t>::vector ConfigurationReader::getInts(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        stl<int32_t>::vector result;

        if (it != mObject.end() && it->second->IsArray())
        {
            const JSONArray &jarray = it->second->AsArray();
            for (uint32_t i = 0; i < jarray.size(); ++i)
            {
                if(jarray[i]->IsNumber())
                    result.emplace_back(jarray[i]->AsInt());
            }
        }

        return result;
    }

    stl<float>::vector ConfigurationReader::getFloats(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        stl<float>::vector result;

        if (it != mObject.end() && it->second->IsArray())
        {
            const JSONArray &jarray = it->second->AsArray();
            for (uint32_t i = 0; i < jarray.size(); ++i)
            {
                if(jarray[i]->IsNumber())
                    result.emplace_back(jarray[i]->AsNumber());
            }
        }

        return result;
    }

    stl<bool>::vector ConfigurationReader::getBools(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        stl<bool>::vector result;

        if (it != mObject.end() && it->second->IsArray())
        {
            const JSONArray &jarray = it->second->AsArray();
            for (uint32_t i = 0; i < jarray.size(); ++i)
            {
                if(jarray[i]->IsNumber())
                    result.emplace_back(jarray[i]->AsBool());
            }
        }

        return result;
    }

    bool ConfigurationReader::has(const WString &name) const
    {
        return mObject.find(name) != mObject.end();
    }

    ConfigurationReader& ConfigurationReader::operator=(const ConfigurationReader& other)
    {
        mObject = other.mObject;
        return *this;
    }
}