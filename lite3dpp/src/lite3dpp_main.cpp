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
#include <SDL_rwops.h>
#include <SDL_log.h>

#include <lite3d/lite3d_alloc.h>

#include <lite3dpp/json/JSON.h>
#include <lite3dpp/lite3dpp_main.h>

namespace lite3dpp
{

    Main::Main() :
        mResourceManager(this),
        mScriptDispatcher(this),
        mConfig(NULL),
        mLifeCycleListener(NULL)
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
        mSettings.logFlushAlways = mConfig->getBool(L"LogFlushAlways", false) ? LITE3D_TRUE : LITE3D_FALSE;

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
        mSettings.renderLisneters.preRender = Main::onInit;
        mSettings.renderLisneters.postRender = Main::onShutdown;
        mSettings.renderLisneters.preFrame = Main::onFrameBegin;
        mSettings.renderLisneters.postFrame = Main::onFrameEnd;
        mSettings.renderLisneters.processEvent = Main::onProcessEvent;

    }

    const lite3d_global_settings &Main::getSettings() const
    {
        return *lite3d_get_global_settings();
    }

    void Main::setResourceLocation(const String &name, 
        const String &location,
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
            throw std::runtime_error("Engine startup failed..");
    }

    void Main::stop()
    {
        lite3d_render_stop();
    }

    void Main::initResourceLocations()
    {
        for (auto &location : mConfig->getObjects(L"ResourceLocations"))
        {           
            setResourceLocation(location.getString(L"Name"), 
                location.getString(L"Path"),
                location.getInt(L"FileCacheMaxSize"));
        }
    }

    void Main::init()
    {
        if(!mConfig)
            throw std::runtime_error("Bad configuration");

        /* basic initialization */
        initResourceLocations();
        mScriptDispatcher.registerGlobals();

        /* create image of main window render target (not json needed, in this case use special dummy.json =) ) */
        mResourceManager.queryResource<WindowRenderTarget>("MainWindow", "dummy.json");

        /* perform fixed update timer */    
        mFixedUpdatesTimer = 
            lite3d_timer_add(mConfig->getInt(L"FixedUpdatesInterval", 200), onTimerTick, this);
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

    WindowRenderTarget *Main::window()
    {
        /* query main windows from resources */
        return mResourceManager.queryResource<WindowRenderTarget>("MainWindow");
    }

    /* callbackes */
    int Main::onInit(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            mainObj->init();

            if(mainObj->mLifeCycleListener)
                mainObj->mLifeCycleListener->init(mainObj);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
                         "init: %s", ex.what());
            return LITE3D_FALSE;
        }

        return LITE3D_TRUE;
    }

    int Main::onShutdown(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            mainObj->shut();

            if(mainObj->mLifeCycleListener)
                mainObj->mLifeCycleListener->shut(mainObj);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
                         "engineLeave: %s", ex.what());
            return LITE3D_FALSE;
        }

        return LITE3D_TRUE;
    }

    int Main::onFrameBegin(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            mainObj->mScriptDispatcher.performFrameBegin();

            if(mainObj->mLifeCycleListener)
                mainObj->mLifeCycleListener->frameBegin(mainObj);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
                         "engineFrameBegin: %s", ex.what());
            return LITE3D_FALSE;
        }

        return LITE3D_TRUE;
    }

    int Main::onFrameEnd(void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            mainObj->mScriptDispatcher.performFrameEnd();

            if(mainObj->mLifeCycleListener)
                mainObj->mLifeCycleListener->frameEnd(mainObj);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
                         "engineFrameEnd: ", ex.what());
            return LITE3D_FALSE;
        }

        return LITE3D_TRUE;
    }

    void Main::onTimerTick(lite3d_timer *timer)
    {
        Main *mainObj = reinterpret_cast<Main *> (timer->userdata);

        try
        {
            if(timer == mainObj->mFixedUpdatesTimer)
                mainObj->mScriptDispatcher.performFixedUpdate();

            if(mainObj->mLifeCycleListener)
                mainObj->mLifeCycleListener->timerTick(mainObj, timer);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
                         "timerFixed: %s", ex.what());
            mainObj->stop();
        }
    }

    int Main::onProcessEvent(SDL_Event *e, void *userdata)
    {
        try
        {
            Main *mainObj = reinterpret_cast<Main *> (userdata);
            mainObj->mScriptDispatcher.performEvent(e);

            if(mainObj->mLifeCycleListener)
                mainObj->mLifeCycleListener->processEvent(mainObj, e);
        }
        catch (std::exception &ex)
        {
            SDL_LogCritical(SDL_LOG_CATEGORY_APPLICATION,
                         "onProcessEvent: %s", ex.what());
            return LITE3D_FALSE;
        }

        return LITE3D_TRUE;
    }

    Main::LifecycleListener::~LifecycleListener()
    {}
}
