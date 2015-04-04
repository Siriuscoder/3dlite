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
#include <SDL_rwops.h>
#include <SDL_log.h>

#include <3dlite/3dlite_alloc.h>

#include <3dlitepp/json/JSON.h>
#include <3dlitepp/3dlitepp_main.h>
#include <algorithm>

namespace lite3dpp
{

    Main::Main() :
        mResourcePackManager(this),
        mScriptManager(this)
    {
        /* init memory model first
        * json parser used lite3d allocator model,
        * and if we do not do it - application will 
        * be crash on config parse step 
        */
        lite3d_memory_init(NULL);
    }

    bool Main::initFromConfig(const char *config)
    {
        SDL_RWops *desc = NULL;
        size_t fileSize;
        char *json;
        /* check open file */
        desc = SDL_RWFromFile(config, "r");
        if (!desc)
            return false;
        fileSize = SDL_RWsize(desc);
        json = (char *) Manageable::alloc(fileSize + 1);
        /* guard */
        json[fileSize] = 0;
        /* read all file */
        if (SDL_RWread(desc, json, fileSize, 1) == 0)
        {
            Manageable::free(json);
            SDL_RWclose(desc);
            return false;
        }

        SDL_RWclose(desc);
        // Parse config data
        JSONValue *value = JSON::Parse(json);
        if (value == NULL)
        {
            Manageable::free(json);
            return false;
        }

        JSONObject root;
        memset(&mSettings, 0, sizeof (mSettings));

        mSettings.logLevel = LITE3D_LOGLEVEL_ERROR;
        mSettings.maxFileCacheSize = 0x100000;
        mSettings.textureSettings.anisotropy = 2;
        mSettings.textureSettings.useGLCompression = LITE3D_FALSE;
        mSettings.videoSettings.FSAA = 1;
        strcpy(mSettings.videoSettings.caption, "TEST window");
        mSettings.videoSettings.colorBits = 24;
        mSettings.videoSettings.fullscreen = LITE3D_FALSE;
        mSettings.videoSettings.screenWidth = 800;
        mSettings.videoSettings.screenHeight = 600;
        mSettings.videoSettings.vsync = LITE3D_TRUE;

        if (!value->IsObject())
        {
            delete value;
            Manageable::free(json);
            return false;
        }

        root = value->AsObject();
        if (root.find(L"LogLevel") != root.end() && root[L"LogLevel"]->IsNumber())
        {
            mSettings.logLevel = root[L"LogLevel"]->AsInt();
        }

        if (root.find(L"FileCacheSize") != root.end() && root[L"FileCacheSize"]->IsNumber())
        {
            mSettings.maxFileCacheSize = root[L"FileCacheSize"]->AsInt();
        }

        if (root.find(L"TextureSettings") != root.end() && root[L"TextureSettings"]->IsObject())
        {
            JSONObject textureSettings = root[L"TextureSettings"]->AsObject();
            if (textureSettings.find(L"Anisotropy") != root.end() &&
                textureSettings[L"Anisotropy"]->IsNumber())
            {
                mSettings.textureSettings.anisotropy = textureSettings[L"Anisotropy"]->AsInt();
            }

            if (textureSettings.find(L"Compression") != root.end() &&
                textureSettings[L"Compression"]->IsString())
            {
                mSettings.textureSettings.useGLCompression =
                    textureSettings[L"Compression"]->AsString() == L"yes" ? LITE3D_TRUE : LITE3D_FALSE;
            }
        }

        if (root.find(L"VideoSettings") != root.end() && root[L"VideoSettings"]->IsObject())
        {
            JSONObject videoSettings = root[L"VideoSettings"]->AsObject();
            if (videoSettings.find(L"Wight") != root.end() &&
                videoSettings[L"Wight"]->IsNumber())
            {
                mSettings.videoSettings.screenWidth = videoSettings[L"Wight"]->AsInt();
            }

            if (videoSettings.find(L"Heigth") != root.end() &&
                videoSettings[L"Heigth"]->IsNumber())
            {
                mSettings.videoSettings.screenHeight = videoSettings[L"Heigth"]->AsInt();
            }

            if (videoSettings.find(L"ColorBits") != root.end() &&
                videoSettings[L"ColorBits"]->IsNumber())
            {
                mSettings.videoSettings.colorBits = videoSettings[L"ColorBits"]->AsInt();
            }

            if (videoSettings.find(L"FSAA") != root.end() &&
                videoSettings[L"FSAA"]->IsNumber())
            {
                mSettings.videoSettings.FSAA = videoSettings[L"FSAA"]->AsInt();
            }

            if (videoSettings.find(L"VSync") != root.end() &&
                videoSettings[L"VSync"]->IsString())
            {
                mSettings.videoSettings.vsync =
                    videoSettings[L"VSync"]->AsString() == L"yes" ? LITE3D_TRUE : LITE3D_FALSE;
            }

            if (videoSettings.find(L"Fullscreen") != root.end() &&
                videoSettings[L"Fullscreen"]->IsString())
            {
                mSettings.videoSettings.fullscreen =
                    videoSettings[L"Fullscreen"]->AsString() == L"yes" ? LITE3D_TRUE : LITE3D_FALSE;
            }

            if (videoSettings.find(L"Caption") != root.end() &&
                videoSettings[L"Caption"]->IsString())
            {
                strcpy(mSettings.videoSettings.caption,
                    JSON::wStringToString(videoSettings[L"Caption"]->AsString()).c_str());
            }
        }

        if (root.find(L"ResourceLocations") != root.end() && root[L"ResourceLocations"]->IsArray())
        {
            JSONArray locations = root[L"ResourceLocations"]->AsArray();

            for (uint32_t i = 0; i < locations.size(); ++i)
            {
                if (locations[i]->IsString())
                {
                    mResourceLocations.insert(JSON::wStringToString(locations[i]->AsString()));
                }
            }
        }

        mSettings.renderLisneters.userdata = reinterpret_cast<void *> (this);
        mSettings.renderLisneters.preRender = Main::engineInit;
        mSettings.renderLisneters.postRender = Main::engineLeave;
        mSettings.renderLisneters.preFrame = Main::engineFrameBegin;
        mSettings.renderLisneters.postFrame = Main::engineFrameEnd;

        delete value;
        return true;
    }

    const lite3d_global_settings &Main::getSettings() const
    {
        return *lite3d_get_global_settings();
    }

    void Main::setResourceLocation(const lite3dpp_string &location)
    {
        mResourcePackManager.addResourceLocation(location);
    }

    Main::~Main()
    {
        shut();
        lite3d_memory_cleanup();
    }

    void Main::run()
    {
        if (!lite3d_main(&mSettings))
            throw std::runtime_error("Main exited with error");
    }

    void Main::stop()
    {
        lite3d_render_stop();
    }

    void Main::initResourceLocations()
    {
        stl<lite3dpp_string>::set::iterator it =
            mResourceLocations.begin();
        for (; it != mResourceLocations.end(); ++it)
            setResourceLocation((*it));
    }

    void Main::init()
    {
        initResourceLocations();
        mScriptManager.init();
        mScriptManager.loadResourceFromFile(mInitialScriptName);
    }

    void Main::shut()
    {
        mScriptManager.shut();
        mResourcePackManager.shut();
    }
    /* callbackes */
    int Main::engineInit(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            mainObj->init();

            return LITE3D_TRUE;
        }
        catch(std::exception &ex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "engineInit error detected: %s", ex.what());
        }

        return LITE3D_FALSE;
    }

    int Main::engineLeave(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            mainObj->shut();
            return LITE3D_TRUE;
        }
        catch(std::exception &ex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "engineLeave error detected: %s", ex.what());
        }

        return LITE3D_FALSE;
    }

    int Main::engineFrameBegin(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            mainObj->mScriptManager.performFrameBegin();
            return LITE3D_TRUE;
        }
        catch(std::exception &ex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "engineFrameBegin error detected: %s", ex.what());
        }

        return LITE3D_FALSE;
    }

    int Main::engineFrameEnd(void *userdata)
    {        
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            mainObj->mScriptManager.performFrameEnd();
            return LITE3D_TRUE;
        }
        catch(std::exception &ex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                "engineFrameEnd error detected: %s", ex.what());
        }

        return LITE3D_FALSE;
    }
}
