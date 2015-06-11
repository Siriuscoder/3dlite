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
        mResourceManager(this),
        mConfigRoot(NULL)
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
        /* read whole file */
        if (SDL_RWread(desc, json, fileSize, 1) == 0)
        {
            Manageable::free(json);
            SDL_RWclose(desc);
            return false;
        }

        SDL_RWclose(desc);
        /* Parse config data */
        mConfigRoot = JSON::Parse(json);
        if (mConfigRoot == NULL)
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

        if (!mConfigRoot->IsObject())
        {
            delete mConfigRoot;
            mConfigRoot = NULL;
            Manageable::free(json);
            return false;
        }

        root = mConfigRoot->AsObject();
        if (root.find(L"LogLevel") != root.end() && root[L"LogLevel"]->IsNumber())
        {
            mSettings.logLevel = root[L"LogLevel"]->AsInt();
        }

        if (root.find(L"TextureSettings") != root.end() && root[L"TextureSettings"]->IsObject())
        {
            JSONObject textureSettings = root[L"TextureSettings"]->AsObject();

            if (textureSettings.find(L"Anisotropy") != textureSettings.end() &&
                textureSettings[L"Anisotropy"]->IsNumber())
            {
                mSettings.textureSettings.anisotropy = textureSettings[L"Anisotropy"]->AsInt();
            }

            if (textureSettings.find(L"Compression") != textureSettings.end() &&
                textureSettings[L"Compression"]->IsBool())
            {
                mSettings.textureSettings.useGLCompression =
                    textureSettings[L"Compression"]->AsBool() ? LITE3D_TRUE : LITE3D_FALSE;
            }
        }

        if (root.find(L"VideoSettings") != root.end() && root[L"VideoSettings"]->IsObject())
        {
            JSONObject videoSettings = root[L"VideoSettings"]->AsObject();
            if (videoSettings.find(L"Width") != videoSettings.end() &&
                videoSettings[L"Width"]->IsNumber())
            {
                mSettings.videoSettings.screenWidth = videoSettings[L"Width"]->AsInt();
            }

            if (videoSettings.find(L"Height") != videoSettings.end() &&
                videoSettings[L"Height"]->IsNumber())
            {
                mSettings.videoSettings.screenHeight = videoSettings[L"Height"]->AsInt();
            }

            if (videoSettings.find(L"ColorBits") != videoSettings.end() &&
                videoSettings[L"ColorBits"]->IsNumber())
            {
                mSettings.videoSettings.colorBits = videoSettings[L"ColorBits"]->AsInt();
            }

            if (videoSettings.find(L"FSAA") != videoSettings.end() &&
                videoSettings[L"FSAA"]->IsNumber())
            {
                mSettings.videoSettings.FSAA = videoSettings[L"FSAA"]->AsInt();
            }

            if (videoSettings.find(L"VSync") != videoSettings.end() &&
                videoSettings[L"VSync"]->IsBool())
            {
                mSettings.videoSettings.vsync =
                    videoSettings[L"VSync"]->AsBool() ? LITE3D_TRUE : LITE3D_FALSE;
            }

            if (videoSettings.find(L"Fullscreen") != videoSettings.end() &&
                videoSettings[L"Fullscreen"]->IsBool())
            {
                mSettings.videoSettings.fullscreen =
                    videoSettings[L"Fullscreen"]->AsBool() ? LITE3D_TRUE : LITE3D_FALSE;
            }

            if (videoSettings.find(L"Caption") != videoSettings.end() &&
                videoSettings[L"Caption"]->IsString())
            {
                strcpy(mSettings.videoSettings.caption,
                       JSON::wStringToString(videoSettings[L"Caption"]->AsString()).c_str());
            }
        }

        mSettings.renderLisneters.userdata = reinterpret_cast<void *> (this);
        mSettings.renderLisneters.preRender = Main::engineInit;
        mSettings.renderLisneters.postRender = Main::engineLeave;
        mSettings.renderLisneters.preFrame = Main::engineFrameBegin;
        mSettings.renderLisneters.postFrame = Main::engineFrameEnd;
        return true;
    }

    const lite3d_global_settings &Main::getSettings() const
    {
        return *lite3d_get_global_settings();
    }

    void Main::setResourceLocation(const lite3dpp_string &name, 
        const lite3dpp_string &location,
        size_t fileCacheMaxSize)
    {
        mResourceManager.addResourceLocation(name,
            location,
            fileCacheMaxSize);
    }

    Main::~Main()
    {
        if(mConfigRoot)
            delete mConfigRoot;
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
        JSONObject root = mConfigRoot->AsObject();
        if (root.find(L"ResourceLocations") != root.end() && root[L"ResourceLocations"]->IsArray())
        {
            JSONArray locations = root[L"ResourceLocations"]->AsArray();

            for (uint32_t i = 0; i < locations.size(); ++i)
            {
                if (locations[i]->IsObject())
                {
                    lite3dpp_string locationName;
                    lite3dpp_string locationPath;
                    size_t fileCacheLimit;

                    JSONObject locationObj = locations[i]->AsObject();
                    if (locationObj.find(L"Name") != locationObj.end() && locationObj[L"Name"]->IsString())
                    {
                        locationName = JSON::wStringToString(locationObj[L"Name"]->AsString());
                    }

                    if (locationObj.find(L"Path") != locationObj.end() && locationObj[L"Path"]->IsString())
                    {
                        locationPath = JSON::wStringToString(locationObj[L"Path"]->AsString());
                    }

                    if (locationObj.find(L"FileCacheMaxSize") != locationObj.end() && locationObj[L"FileCacheMaxSize"]->IsNumber())
                    {
                        fileCacheLimit = locationObj[L"FileCacheMaxSize"]->AsInt();
                    }
                    
                    setResourceLocation(locationName, locationPath, fileCacheLimit);
                }
            }
        }
    }

    void Main::init()
    {
        if(!mConfigRoot)
            throw std::runtime_error("Bad configuration");

        initResourceLocations();
        int32_t fixedUpdatesInterval = 200;
        lite3dpp_string initialScriptName;


        JSONObject root = mConfigRoot->AsObject();
        if (root.find(L"FixedUpdatesInterval") != root.end() && root[L"FixedUpdatesInterval"]->IsNumber())
        {
            fixedUpdatesInterval = root[L"FixedUpdatesInterval"]->AsInt();
        }

        if (root.find(L"InitScript") != root.end() && root[L"InitScript"]->IsString())
        {
            initialScriptName = JSON::wStringToString(root[L"InitScript"]->AsString());               
        }

        /* perform fixed update timer */    
        mFixedUpdatesTimer = 
            lite3d_timer_add(fixedUpdatesInterval, timerFixed, this);
    }

    void Main::shut()
    {
        lite3d_timer_purge(mFixedUpdatesTimer);
        mResourceManager.releaseAllResources();
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
        catch (std::exception &ex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "engineInit: %s", ex.what());
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
        catch (std::exception &ex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "engineLeave: %s", ex.what());
        }

        return LITE3D_FALSE;
    }

    int Main::engineFrameBegin(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            //mainObj->mScriptManager.performFrameBegin();
            return LITE3D_TRUE;
        }
        catch (std::exception &ex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "engineFrameBegin: %s", ex.what());
        }

        return LITE3D_FALSE;
    }

    int Main::engineFrameEnd(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            //mainObj->mScriptManager.performFrameEnd();
            return LITE3D_TRUE;
        }
        catch (std::exception &ex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "engineFrameEnd: ", ex.what());
        }

        return LITE3D_FALSE;
    }

    void Main::timerFixed(lite3d_timer *timer)
    {
        Main *mainObj = reinterpret_cast<Main *> (timer->userdata);

        try
        {
            //mainObj->mScriptManager.performFixedUpdate();
        }
        catch (std::exception &ex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "timerFixed: %s", ex.what());
            mainObj->stop();
        }
    }
}
