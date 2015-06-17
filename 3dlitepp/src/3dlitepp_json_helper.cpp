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
#include <SDL_log.h>
#include <SDL_rwops.h>

#include <3dlitepp/3dlitepp_json_helper.h>

namespace lite3dpp
{
    JsonHelper::JsonHelper(const lite3dpp_string &file)
    {
        parseFromFile(file);
    }

    JsonHelper::JsonHelper(const char *data, size_t size)
    {
        if(!parseFromBuffer(data, size))
            throw std::runtime_error("json parse failed..");
    }

    JsonHelper::~JsonHelper()
    {
        if(mRoot)
            delete mRoot;
        mRoot = NULL;
    }

    // private
    JsonHelper::JsonHelper(const JSONObject &fromJsonObject) : 
        mRoot(NULL),
        mObject(fromJsonObject)
    {}

    // private default
    JsonHelper::JsonHelper() : 
        mRoot(NULL)
    {}

    // copy 
    JsonHelper::JsonHelper(const JsonHelper &other) : 
        mRoot(NULL),
        mObject(other.mObject)
    {

    }

    void JsonHelper::parseFromFile(const lite3dpp_string &file)
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
        lite3dpp_string bufCopy(data, size);
        /* Parse data from buffer */
        mRoot = JSON::Parse(bufCopy.c_str());

        if (mRoot == NULL)
            return false;

        if (!mRoot->IsObject())
        {
            delete mRoot;
            mRoot = NULL;
            return false;
        }

        mObject = mRoot->AsObject();
        return true;
    }

    int32_t JsonHelper::getInt(const lite3dpp_wstring &name, int32_t def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsNumber())
        {
            return it->second->AsInt();
        }

        return def;
    }

    double JsonHelper::getDouble(const lite3dpp_wstring &name, double def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsNumber())
        {
            return it->second->AsNumber();
        }

        return def;
    }

    bool JsonHelper::getBool(const lite3dpp_wstring &name, bool def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsBool())
        {
            return it->second->AsBool();
        }

        return def;
    }

    lite3dpp_string JsonHelper::getString(const lite3dpp_wstring &name, const lite3dpp_string &def) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsString())
        {
            return JSON::wStringToString(it->second->AsString());
        }

        return def;
    }

    JsonHelper JsonHelper::getObject(const lite3dpp_wstring &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        if (it != mObject.end() && it->second->IsObject())
        {
            return JsonHelper(it->second->AsObject());
        }

        return JsonHelper();
    }

    stl<JsonHelper>::vector JsonHelper::getObjects(const lite3dpp_wstring &name) const
    {
        JSONObject::const_iterator it = mObject.find(name);
        stl<JsonHelper>::vector result;

        if (it != mObject.end() && it->second->IsArray())
        {
            const JSONArray jarray = it->second->AsArray();
            for (uint32_t i = 0; i < jarray.size(); ++i)
            {
                if(jarray[i]->IsObject())
                    result.push_back(JsonHelper(jarray[i]->AsObject()));
            }
        }

        return result;
    }
}