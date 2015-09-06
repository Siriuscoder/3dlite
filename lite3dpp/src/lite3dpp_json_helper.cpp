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
#include <SDL_assert.h>
#include <SDL_log.h>
#include <SDL_rwops.h>

#include <lite3dpp/lite3dpp_json_helper.h>

namespace lite3dpp
{
    JsonHelper::JsonHelper(const String &file)
    {
        parseFromFile(file);
    }

    JsonHelper::JsonHelper(const char *data, size_t size)
    {
        if(!parseFromBuffer(data, size))
            throw std::runtime_error("json parse failed..");
    }

    JsonHelper::~JsonHelper()
    {}

    // private
    JsonHelper::JsonHelper(const JSONObject &fromJsonObject) : 
        mObject(fromJsonObject)
    {}

    // private default
    JsonHelper::JsonHelper()
    {}

    // copy 
    JsonHelper::JsonHelper(const JsonHelper &other) : 
        mObject(other.mObject)
    {}

    void JsonHelper::parseFromFile(const String &file)
    {
        SDL_RWops *desc = NULL;
        size_t fileSize;
        char *json;
        /* check open file */
        desc = SDL_RWFromFile(file.c_str(), "r");
        if (!desc)
            throw std::runtime_error(file + " file open failed..");

        fileSize = static_cast<size_t>(SDL_RWsize(desc));
        json = (char *) Manageable::alloc(fileSize);
        /* read whole file */
        if (SDL_RWread(desc, json, fileSize, 1) == 0)
        {
            Manageable::free(json);
            SDL_RWclose(desc);
            throw std::runtime_error(file + " file read failed..");
        }

        SDL_RWclose(desc);
        
        if(!parseFromBuffer(json, fileSize))
        {
            Manageable::free(json);
            throw std::runtime_error(file + " file parse failed..");
        }

        Manageable::free(json);
    }

    bool JsonHelper::parseFromBuffer(const char *data, size_t size)
    {
        String bufCopy(data, size);
        /* Parse data from buffer */
        mRoot.reset(JSON::Parse(bufCopy.c_str()));

        if (!mRoot)
            return false;

        if (!mRoot->IsObject())
            return false;

        mObject = mRoot->AsObject();
        return true;
    }

    int32_t JsonHelper::getInt(const WString &name, int32_t def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsNumber())
        {
            return it->second->AsInt();
        }

        return def;
    }

    double JsonHelper::getDouble(const WString &name, double def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsNumber())
        {
            return it->second->AsNumber();
        }

        return def;
    }

    bool JsonHelper::getBool(const WString &name, bool def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsBool())
        {
            return it->second->AsBool();
        }

        return def;
    }

    String JsonHelper::getString(const WString &name, const String &def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsString())
        {
            return JSON::wStringToString(it->second->AsString());
        }

        return def;
    }

    JsonHelper JsonHelper::getObject(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsObject())
        {
            return JsonHelper(it->second->AsObject());
        }

        return JsonHelper();
    }

    bool JsonHelper::isEmpty() const
    {
        return mObject.size() == 0;
    }

    kmVec2 JsonHelper::getVec2(const WString &name, const kmVec2 &def) const
    {
        JsonHelper helper = getObject(name);
        if(helper.isEmpty())
            return def;

        kmVec2 vec2 = {
            static_cast<float>(helper.getDouble(L"x")),
            static_cast<float>(helper.getDouble(L"y"))
        };

        return vec2;
    }

    kmVec3 JsonHelper::getVec3(const WString &name, const kmVec3 &def) const
    {
        JsonHelper helper = getObject(name);
        if(helper.isEmpty())
            return def;

        kmVec3 vec3 = {
            static_cast<float>(helper.getDouble(L"x")),
            static_cast<float>(helper.getDouble(L"y")),
            static_cast<float>(helper.getDouble(L"z"))
        };

        return vec3;
    }

    kmVec4 JsonHelper::getVec4(const WString &name, const kmVec4 &def) const
    {
        JsonHelper helper = getObject(name);
        if(helper.isEmpty())
            return def;

        kmVec4 vec4 = {
            static_cast<float>(helper.getDouble(L"x")),
            static_cast<float>(helper.getDouble(L"y")),
            static_cast<float>(helper.getDouble(L"z")),
            static_cast<float>(helper.getDouble(L"w"))
        };

        return vec4;
    }

    kmQuaternion JsonHelper::getQuaternion(const WString &name, const kmQuaternion &def) const
    {
        JsonHelper helper = getObject(name);
        if(helper.isEmpty())
            return def;

        kmQuaternion quat = {
            static_cast<float>(helper.getDouble(L"x")),
            static_cast<float>(helper.getDouble(L"y")),
            static_cast<float>(helper.getDouble(L"z")),
            static_cast<float>(helper.getDouble(L"w"))
        };

        return quat;
    }

    stl<JsonHelper>::vector JsonHelper::getObjects(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        stl<JsonHelper>::vector result;

        if (it != mObject.end() && it->second->IsArray())
        {
            const JSONArray &jarray = it->second->AsArray();
            for (uint32_t i = 0; i < jarray.size(); ++i)
            {
                if(jarray[i]->IsObject())
                    result.push_back(JsonHelper(jarray[i]->AsObject()));
            }
        }

        return result;
    }

    stl<String>::vector JsonHelper::getStrings(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        stl<String>::vector result;

        if (it != mObject.end() && it->second->IsArray())
        {
            const JSONArray &jarray = it->second->AsArray();
            for (uint32_t i = 0; i < jarray.size(); ++i)
            {
                if(jarray[i]->IsString())
                    result.push_back(std::move(JSON::wStringToString(jarray[i]->AsString())));
            }
        }

        return result;
    }

    stl<int32_t>::vector JsonHelper::getInts(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        stl<int32_t>::vector result;

        if (it != mObject.end() && it->second->IsArray())
        {
            const JSONArray &jarray = it->second->AsArray();
            for (uint32_t i = 0; i < jarray.size(); ++i)
            {
                if(jarray[i]->IsNumber())
                    result.push_back(jarray[i]->AsInt());
            }
        }

        return result;
    }

    stl<double>::vector JsonHelper::getFloats(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        stl<double>::vector result;

        if (it != mObject.end() && it->second->IsArray())
        {
            const JSONArray &jarray = it->second->AsArray();
            for (uint32_t i = 0; i < jarray.size(); ++i)
            {
                if(jarray[i]->IsNumber())
                    result.push_back(jarray[i]->AsNumber());
            }
        }

        return result;
    }

    stl<bool>::vector JsonHelper::getBools(const WString &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        stl<bool>::vector result;

        if (it != mObject.end() && it->second->IsArray())
        {
            const JSONArray &jarray = it->second->AsArray();
            for (uint32_t i = 0; i < jarray.size(); ++i)
            {
                if(jarray[i]->IsNumber())
                    result.push_back(jarray[i]->AsBool());
            }
        }

        return result;
    }
}