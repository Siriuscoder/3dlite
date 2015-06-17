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
        mScriptDispatcher(this),
        mConfig(NULL)
    {
        /* init memory model first
         * json parser used lite3d allocator model,
         * and if we do not do it - application will 
         * be crash on config parse step 
         */
        lite3d_memory_init(NULL);
    }

    void Main::initFromConfig(const char *config)
    {           
        if (mConfig)
        {
            delete mConfig;
            mConfig = NULL;
        }

        mConfig = new JsonHelper(config);

        memset(&mSettings, 0, sizeof (mSettings));
        mSettings.maxFileCacheSize = 0x100000;

        mSettings.logLevel = mConfig->getInt(L"LogLevel", LITE3D_LOGLEVEL_ERROR);
        JsonHelper textureSettings = mConfig->getObject(L"TextureSettings");
        JsonHelper videoSettings = mConfig->getObject(L"VideoSettings");

        mSettings.textureSettings.anisotropy = textureSettings.getInt(L"Anisotropy", 2);
        mSettings.textureSettings.useGLCompression =
            textureSettings.getBool(L"Compression", false) ? LITE3D_TRUE : LITE3D_FALSE;

        mSettings.videoSettings.screenWidth = videoSettings.getInt(L"Width", 800);
        mSettings.videoSettings.screenHeight = videoSettings.getInt(L"Height", 600);
        mSettings.videoSettings.colorBits = videoSettings.getInt(L"ColorBits", 24);
        mSettings.videoSettings.FSAA = videoSettings.getInt(L"FSAA", 1);
        mSettings.videoSettings.vsync =
            videoSettings.getBool(L"VSync", true) ? LITE3D_TRUE : LITE3D_FALSE;
        mSettings.videoSettings.fullscreen =
            videoSettings.getBool(L"Fullscreen", false) ? LITE3D_TRUE : LITE3D_FALSE;
        videoSettings.getString(L"Caption", "TEST window").copy(mSettings.videoSettings.caption,
            sizeof(mSettings.videoSettings.caption)-1);

        mSettings.renderLisneters.userdata = reinterpret_cast<void *> (this);
        mSettings.renderLisneters.preRender = Main::engineInit;
        mSettings.renderLisneters.postRender = Main::engineShutdown;
        mSettings.renderLisneters.preFrame = Main::engineFrameBegin;
        mSettings.renderLisneters.postFrame = Main::engineFrameEnd;
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
        stl<JsonHelper>::vector locations = mConfig->getObjects(L"ResourceLocations");
        for (uint32_t i = 0; i < locations.size(); ++i)
        {           
            setResourceLocation(locations[i].getString(L"Name"), 
                locations[i].getString(L"Path"),
                locations[i].getInt(L"FileCacheMaxSize"));
        }
    }

    void Main::init()
    {
        if(!mConfig)
            throw std::runtime_error("Bad configuration");

        /* basic initialization */
        initResourceLocations();
        mScriptDispatcher.registerGlobals();

        /* load first script */
        /* after script been loaded, init script function will be executed */
        mResourceManager.queryResource<Script>("", mConfig->getString(L"InitScript"));

        /* perform fixed update timer */    
        mFixedUpdatesTimer = 
            lite3d_timer_add(mConfig->getInt(L"FixedUpdatesInterval", 200), timerFixed, this);
    }

    void Main::shut()
    {
        lite3d_timer_purge(mFixedUpdatesTimer);
        mResourceManager.releaseAllResources();
        
        if (mConfig)
        {
            delete mConfig;
            mConfig = NULL;
        }
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

    int Main::engineShutdown(void *userdata)
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
            mainObj->mScriptDispatcher.performFrameBegin();
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
            mainObj->mScriptDispatcher.performFrameEnd();
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
            mainObj->mScriptDispatcher.performFixedUpdate();
        }
        catch (std::exception &ex)
        {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION,
                         "timerFixed: %s", ex.what());
            mainObj->stop();
        }
    }
}
